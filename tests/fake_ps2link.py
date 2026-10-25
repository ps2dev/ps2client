#!/usr/bin/env python3
"""A fake ps2link for testing ps2client without a PS2."""

# It speaks the ps2link protocol: it accepts the request connection on TCP
# 0x4711, takes commands on UDP 0x4712 and sends console output to UDP 0x4712
# of the client. Run it on another address than the client (see run-tests.sh),
# since both use UDP port 0x4712.
#
# execee loads its file through host: requests like ps2link does, then runs it
# as a script, one command per line:
#
#   print <text>       console output, a newline is added
#   raw <n> <char>     one console packet of n bytes of <char>, no newline
#   sleep <seconds>    wait
#   close              close the request connection (ps2link went away)
#   readback <file>    read a host: file and print its size and checksum
#
# dumpreg writes 8 bytes to the given host: file, like ps2link's INTC dump.

import argparse
import select
import socket
import struct
import threading
import time
import zlib

PORT_REQUEST = 0x4711
PORT_COMMAND = 0x4712
PORT_CONSOLE = 0x4712

CMD_EXECEE = 0xBABE0203
CMD_DUMPREG = 0xBABE020A
REQ_OPEN, REQ_CLOSE, REQ_READ, REQ_WRITE = 0xBABE0111, 0xBABE0121, 0xBABE0131, 0xBABE0141

PS2_O_RDONLY, PS2_O_WRONLY, PS2_O_CREAT = 0x0001, 0x0002, 0x0200


class Fake:
    """The state of the fake ps2link and the host: requests it makes."""

    def __init__(self, ignore_dumpreg):
        """Start without a request connection."""
        self.conn = None
        self.client_ip = None
        self.lock = threading.Lock()
        self.ignore_dumpreg = ignore_dumpreg
        self.console = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def recv_all(self, n):
        data = b""
        while len(data) < n:
            chunk = self.conn.recv(n - len(data))
            if not chunk:
                raise ConnectionError("client closed")
            data += chunk
        return data

    def request(self, number, payload, data=b""):
        # The length covers the header, not the data of a write.
        self.conn.sendall(struct.pack(">IH", number, 6 + len(payload)) + payload + data)

    def response(self, size):
        return self.recv_all(size)

    def open(self, path, flags):
        with self.lock:
            self.request(REQ_OPEN, struct.pack(">i256s", flags, path.encode()))
            _, _, fd = struct.unpack(">IHi", self.response(10))
        return fd

    def close(self, fd):
        with self.lock:
            self.request(REQ_CLOSE, struct.pack(">i", fd))
            self.response(10)

    def read(self, fd, size):
        with self.lock:
            self.request(REQ_READ, struct.pack(">ii", fd, size))
            _, _, _, n = struct.unpack(">IHii", self.response(14))
            return self.recv_all(n) if n > 0 else b""

    def write(self, fd, data):
        with self.lock:
            self.request(REQ_WRITE, struct.pack(">ii", fd, len(data)), data)
            self.response(10)

    def load(self, path):
        fd = self.open(path, PS2_O_RDONLY)
        if fd < 0:
            return None
        data = b""
        while True:
            # Big reads, like the IOP does for ELF loading.
            chunk = self.read(fd, 128 * 1024)
            if not chunk:
                break
            data += chunk
        self.close(fd)
        return data

    def say(self, data):
        self.console.sendto(data, (self.client_ip, PORT_CONSOLE))

    def run_script(self, text):
        for line in text.splitlines():
            cmd, _, arg = line.partition(" ")
            if cmd == "print":
                self.say(arg.encode() + b"\n")
            elif cmd == "raw":
                n, ch = arg.split()
                self.say(ch.encode() * int(n))
            elif cmd == "sleep":
                time.sleep(float(arg))
            elif cmd == "close":
                self.conn.shutdown(socket.SHUT_RDWR)
                self.conn.close()
                return
            elif cmd == "readback":
                data = self.load(arg)
                self.say(b"readback %d %08x\n" % (len(data), zlib.crc32(data)))

    def command(self, packet):
        number, _ = struct.unpack(">IH", packet[:6])
        if number == CMD_DUMPREG:
            if self.ignore_dumpreg > 0:
                self.ignore_dumpreg -= 1
                return
            _, path = struct.unpack(">i256s", packet[6:6 + 260])
            fd = self.open(path.split(b"\0")[0].decode(), PS2_O_WRONLY | PS2_O_CREAT)
            if fd >= 0:
                self.write(fd, b"\0" * 8)
                self.close(fd)
        elif number == CMD_EXECEE:
            argc, argv = struct.unpack(">i256s", packet[6:6 + 260])
            args = argv.split(b"\0")[:argc]
            path = args[0].decode().removeprefix("host:")
            script = self.load(path)
            if script is None:
                self.say(b"fake: could not load %s\n" % path.encode())
                return
            self.say(b"fake: args " + b" ".join(args[1:]) + b"\n")
            threading.Thread(target=self.run_script, args=(script.decode(),), daemon=True).start()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bind", required=True, help="the address to serve on")
    ap.add_argument("--ignore-dumpreg", type=int, default=0, help="ignore the first N dumpreg commands (not ready yet)")
    opts = ap.parse_args()
    fake = Fake(opts.ignore_dumpreg)

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((opts.bind, PORT_REQUEST))
    server.listen(5)
    commands = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    commands.bind((opts.bind, PORT_COMMAND))

    def accept():
        conn, addr = server.accept()
        # Like ps2link: a new client replaces the old one.
        if fake.conn:
            fake.conn.close()
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        fake.conn, fake.client_ip = conn, addr[0]

    while True:
        # A client connects before it sends a command, so take pending
        # connections first: the command is for the newest one.
        ready, _, _ = select.select([server, commands], [], [])
        if server in ready:
            accept()
            continue
        packet, _ = commands.recvfrom(2048)
        # Without a request connection the host: open fails and the command
        # does nothing, as on ps2link.
        if fake.conn is None:
            continue
        try:
            fake.command(packet)
        except (OSError, ConnectionError):
            pass


if __name__ == "__main__":
    main()
