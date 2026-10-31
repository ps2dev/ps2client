#!/usr/bin/env python3
"""Portable ps2client smoke tests; no physical PS2 or Linux namespaces needed."""

import os
import socket
import struct
import subprocess  # nosec B404
import sys
import time
import unittest


BAD_HOST = "1.1.l.1"
CONSOLE_PORT = 0x4712
RESET_PACKET = 0xBABE0201


class SmokeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.client = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "bin/ps2client")

    def run_client(self, *args, env=None):
        # The test runner supplies a local executable and fixed argument lists;
        # no shell is involved.
        return subprocess.run(  # nosec B603
            [self.client, *args], capture_output=True, text=True, timeout=8, env=env
        )

    def test_invalid_hostname_option(self):
        result = self.run_client("-h", BAD_HOST, "ping")
        self.assertEqual(result.returncode, 69, result.stderr)
        self.assertIn(BAD_HOST, result.stderr)

    def test_invalid_hostname_environment(self):
        env = os.environ.copy()
        env["PS2HOSTNAME"] = BAD_HOST
        result = self.run_client("ping", env=env)
        self.assertEqual(result.returncode, 69, result.stderr)
        self.assertIn(BAD_HOST, result.stderr)

    def test_console_without_ps2_or_resolvable_host(self):
        # console must bind the UDP console port without trying to resolve
        # the PS2 address or connect to the request/command channels.
        # The executable and command arguments are controlled by this test.
        proc = subprocess.Popen(  # nosec B603
            [self.client, "-h", BAD_HOST, "-t", "1", "console"],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sender:
                time.sleep(0.2)
                for _ in range(5):
                    sender.sendto(b"smoke-console-line\n", ("127.0.0.1", CONSOLE_PORT))
                    time.sleep(0.05)
            stdout, stderr = proc.communicate(timeout=5)
            self.assertEqual(proc.returncode, 0, stderr)
            self.assertIn("smoke-console-line", stdout)
        finally:
            if proc.poll() is None:
                proc.kill()
                proc.communicate(timeout=5)

    def test_legacy_listen_still_requires_host_service(self):
        result = self.run_client("-h", BAD_HOST, "-t", "1", "listen")
        self.assertEqual(result.returncode, 69, result.stderr)

    def test_reset_uses_udp_without_tcp_server(self):
        # Capture the reset packet on loopback: the test must not reset a PS2.
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as receiver:
            receiver.bind(("127.0.0.1", CONSOLE_PORT))
            receiver.settimeout(3)
            result = self.run_client("-h", "127.0.0.1", "reset")
            self.assertEqual(result.returncode, 0, result.stderr)
            packet, _ = receiver.recvfrom(256)
        self.assertEqual(struct.unpack(">IH", packet), (RESET_PACKET, 6))


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]], verbosity=2)
