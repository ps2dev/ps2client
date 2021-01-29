# PS2Client

![CI](https://github.com/ps2dev/ps2client/workflows/CI/badge.svg)


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

```
ps2client [-h hostname] [-t timeout] <command> [arguments]
fsclient [-h hostname] <command> <arguments>
```

**[-h hostname]**

The address of the remote ps2. This can be an IP or a hostname. If none is given, the environment variable `$PS2HOSTNAME` will be checked for a valid address. If this variable isn't set, a default of 192.168.0.10 will be used.

**[-t timeout]**

An idle timeout period in seconds that the client will wait before exiting. This is useful to allow a script to continue after calling ps2client to send a command to ps2link or ps2netfs.

### PS2LINK COMMANDS
- `reset`

Send a reset request to ps2link.

- `execiop <filename> [arguments]`

Tell ps2link to load and execute a file on the IOP.

- `execee <filename> [arguments]`

Tell ps2link to load and execute a file on the EE.

- `poweroff`

Send a poweroff request to ps2link.

- `scrdump`

Tell ps2link to dump exceptions to the screen.

- `netdump`

Tell ps2link to dump execetions to the network console.

- `dumpmem <offset> <size> <filename>`

Dump the contents of memory into a file.

- `startvu <vu>`

Tell the specified vector unit to start operation.

- `stopvu <vu>`

Tell the specified vector unit to stop operation.

- `dumpreg <type> <filename>`

Dump the registers of a given type into a file.

- `gsexec <size> <filename>`

Tell ps2link to load and send a file to the GS.

- `writemem <offset> <size> <filename>`

Write the contents of a file into memory.

- `iopexcep`

I really don't know! OH NOES!!

- `listen`

Listen to the ps2link network console.
