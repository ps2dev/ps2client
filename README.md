# PS2Client

![CI](https://github.com/ps2dev/ps2client/workflows/CI/badge.svg)

## **ATTENTION**

If you are confused on how to start developing for PS2, see the
[getting started](https://ps2dev.github.io/#getting-started) section on
the ps2dev main page.  

## PS2CLIENT USAGE AND INFORMATION

### THE INTRODUCTION

These programs, ps2client & fsclient, are command line tools used for interacting with a ps2 system running ps2link and/or ps2netfs. It will allow you to send various commands as well as respond to requests for data. This program was written and tested under Linux, Cygwin and Mac OS X but should compile on any reasonably unixlike system.

### BUILDING THE CLIENTS

Building and installing is very simple:

`make clean; make install`

There are some build options, however:

`PREFIX=/dir`

Install directory, defaults to: `$PS2DEV`. `/bin` will be automatically appended.
The software will be installed to the `/bin` sub-directory, under `PREFIX` (`PREFIX/bin`).

These options can be used as follows:

`make clean; make PREFIX=/new/path/prefix install`

### BUILDING WINDOWS BINARIES

For building windows binaries on Unix system, you need to setup mingw cross-compiler and win32 threads.
Build command:

`make -f Makefile.mingw32`

### BASIC CLIENT USAGE

Basic usage:

```txt
ps2client [-h hostname] [-t timeout] [-e marker] <command> [arguments]
fsclient [-h hostname] <command> <arguments>
```

#### \[-h hostname]

The address of the remote ps2. This can be an IP or a hostname. If none is given, the environment variable `$PS2HOSTNAME` will be checked for a valid address. If this variable isn't set, a default of 192.168.0.10 will be used.

#### \[-t timeout]

An idle timeout period in seconds that the client will wait before exiting. This is useful to allow a script to continue after calling ps2client to send a command to ps2link or ps2netfs.
For `ping`, it is how long to try (default 2 seconds).

#### \[-e marker]

Exit as soon as a line of console output contains `marker`. The exit status
is the number after `status=` on that line, or 0 if there is none. A test
program can print for example `@@END@@ status=1` as its last line, so a script
gets the result without waiting for a timeout.

### NETWORK PORTS

These are the default ports used by `ps2client` and `fsclient`:

| Purpose | Protocol | Port (decimal) | Direction |
| --- | --- | --- | --- |
| ps2link commands | UDP | 18194 (`0x4712`) | Host to PS2 |
| ps2link host: requests | TCP | 18193 (`0x4711`) | Host to PS2 |
| ps2link console logs | UDP | 18194 (`0x4712`) | PS2 to host |
| ps2netfs file operations | TCP | 18195 (`0x4713`) | Host to PS2 |

The command and console channels use the same UDP port number on different
machines; they are not separate ports on the same device. `ps2netfs` is a
separate service, not FTP. Commands such as `reset` use only the UDP command
channel, while `ping`, `listen` and commands that access `host:` also use the
TCP request connection. TCP connections are bidirectional, while the console
log channel is independent of the TCP host: file-serving service.

### EXIT STATUS

* `0`: done (or the idle timeout passed, without `-e`)
* `64`: usage error
* `69`: ps2link is not answering
* `74`: ps2link closed the connection (reset, reboot or crash of the ps2)
* `75`: the console port is in use, probably by another ps2client
* `124`: with `-e`: the idle timeout passed before the marker
* with `-e`: the status from the marker line

Error messages go to stderr; stdout carries only the console output of the ps2.

### PS2LINK COMMANDS

*   `reset`

Send a reset request to ps2link.

*   `execiop <filename> [arguments]`

Tell ps2link to load and execute a file on the IOP.

*   `execee <filename> [arguments]`

Tell ps2link to load and execute a file on the EE.

*   `poweroff`

Send a poweroff request to ps2link.

*   `scrdump`

Tell ps2link to dump exceptions to the screen.

*   `netdump`

Tell ps2link to dump execetions to the network console.

*   `dumpmem <offset> <size> <filename>`

Dump the contents of memory into a file.

*   `startvu <vu>`

Tell the specified vector unit to start operation.

*   `stopvu <vu>`

Tell the specified vector unit to stop operation.

*   `dumpreg <type> <filename>`

Dump the registers of a given type into a file. The type is a number or one
of `dma`, `intc`, `timer`, `gs`, `sif`, `fifo`, `gif`, `vif0`, `vif1`, `ipu`,
`all`, `vu0`, `vu1`.

*   `gsexec <size> <filename>`

Tell ps2link to load and send a file to the GS.

*   `writemem <offset> <size> <filename>`

Write the contents of a file into memory.

*   `iopexcep`

I really don't know! OH NOES!!

*   `listen`

Listen to the ps2link network console and serve `host:` file requests, as in
previous releases. This requires a working ps2link connection.

* `console`

Listen to UDP console logs without connecting to the PS2, opening the TCP
request channel, or serving `host:` file requests. This also works while
ps2link is temporarily unavailable.

* `ping`

Check that ps2link is up: ps2link is asked to dump two registers to a host
file, which the client discards, so this checks the EE side, the IOP side and
the host connection. Exits 0 and prints the round trip time, or exits 69.
Note that ps2link serves one host connection, and a new client replaces the
old one: do not ping while another client is serving a running program.

Commands that load host files (`execee`, `execiop`, ...) do the same check
before they send the command, instead of waiting a fixed time.

### SYMBOLIZING EXCEPTION LOGS

The optional `bin/ps2symbolize` annotates ps2link's EE/IOP exception logs
with function names and source locations from matching **unstripped** ELF files.
It does not change ps2link or require a new wire protocol. For example:

```sh
ps2client console | bin/ps2symbolize --ee game.elf
bin/ps2symbolize --ee game.elf crash.log
bin/ps2symbolize --iop mydriver=driver.elf crash.log
```

The C tool uses the PS2DEV `mips64r5900el-ps2-elf-addr2line` and
`mipsel-none-elf-addr2line` tools found on `PATH`; override them with
`--ee-tool` or `--iop-tool`. For IOP modules, the file must have debug symbols
and the logged **relative EPC** must match its ELF section addresses. Keep the
ELFs from the exact build being debugged. Unresolved locations are left as
original log text.

### TESTS

`make check` builds and runs plain-C portable smoke tests (including invalid
hostnames and the `console` command) on macOS and Linux without a PS2.

`tests/run-tests.sh` runs the client against a fake ps2link
(`tests/fake_ps2link.c`) in unprivileged Linux network namespaces, so no ps2
is needed.

CI also runs the full Linux protocol suite in isolated network namespaces
with `sudo unshare --net` on the GitHub-hosted runner. The job covers
handshakes, host-file transfers, timeouts, disconnects and console packets
against the native C fake server. On Windows, the native C smoke test runs
through `make -f Makefile.mingw32 check`. Neither test needs Python.

The Linux integration suite can be reproduced on a machine that supports
network namespaces using:

```sh
make all bin/fake_ps2link bin/crc32
sudo unshare --net -- env PS2CLIENT_TEST_NS=1 \
  bash tests/run-tests.sh "$(pwd)/bin/ps2client"
```

## Community

Links for discussion and chat are available
[here](https://ps2dev.github.io/#community).  
