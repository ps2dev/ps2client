
                    ===================================
                      PS2CLIENT USAGE AND INFORMATION
                    ===================================

 ------------------
  THE INTRODUCTION
 ------------------

  This program, ps2client, is a command line tool used for interacting with
  a ps2 system running either ps2link or ps2netfs. It will allow you to send
  various commands as well as respond to requests for data. This program was
  written and tested under Linux, Cygwin and Mac OS X but should compile in
  any reasonably unixlike environment.

  If you have any requests, questions or sugestions please feel free to contact
  me at peori@oopo.net. Also, don't be afraid to check out the latest development
  version of ps2client located at cvs.ps2dev.org.

 --------------------
  BUILDING PS2CLIENT
 --------------------

  Building and installing is very simple:

   make clean; make install

  There are some build options, however:

   PREFIX=/dir	- Install directory, defaults to: $PS2DEV/bin

   DEBUG=YES	- Enable all debug text output.

  These options can be used as follows:

   make clean; make DEBUG=NO PREFIX=$PS2DEV/bin install

 -----------------
  PS2CLIENT USAGE
 -----------------

  Basic usage:

   ps2client [-h hostname] [-t timeout] <command> [arguments]

   [-h hostname]

    The address of the remote ps2. This can be an IP or a hostname. If none
    is given, the environment variable $PS2HOSTNAME will be checked for a
    valid address. If this variable isn't set, a default of 192.168.0.10
    will be used.

   [-t timeout]

    An idle timeout period that ps2client will wait before exiting. This is
    useful to allow a script to continue after calling ps2client to send a
    command to the ps2.

 ------------------
  PS2LINK COMMANDS
 ------------------

  reset

   - Send a reset request to the ps2.

  execiop <filename> [arguments]

   - Tell ps2link to load and execute a file on the IOP.

  execee <filename> [arguments]

   - Tell ps2link to load and execute a file on the EE.

  poweroff

   - Send a poweroff request to the ps2.

  dumpmem <filename> <offset> <size>

   - Dump the contents of memory at a given offset into a file.

  startvu <vu>

   - Tell the specified vector unit to start operation.

  stopvu <vu>

   - Tell the specified vector unit to stop operation.

  dumpreg <filename> <type>

   - Dump the registers of a given type into a file.

  gsexec <filename>

   - Tell ps2link to load and send a file to the GS.

  listen

   - Listen for text messages from the ps2.

 -------------------
  PS2NETFS COMMANDS
 -------------------

  copyfrom <source> <destination>

   - Copy a file from the ps2. (download)

  copyto <source> <destination>

   - Copy a file to the ps2. (upload)

  delete <filename>

   - Delete a file on the ps2.

  devlist

   - Get a device listing from the ps2.

  dir <directory>

   - Get a directory listing from the ps2.

  format <device>

   - Format a device on the ps2.

  mkdir <directory>

   - Make a directory on the ps2.

  mount <device> <fsname>

   - Mount a filesystem as a device on the ps2.

  rename <source> <destination>

   - Rename a file on the ps2.

  rmdir <directory>

   - Remove a directory on the ps2.

  sync <device>

   - Sync a device on the ps2.

  umount <device>

   - Unmount a device on the ps2.

 ------------------
  MORE INFORMATION
 ------------------

  If you want to get into the wonderful world of homebrew ps2
  development, there's a few places you can check out:

  http://www.ps2dev.org     - News, tutorials and documentation.

  http://forums.ps2dev.org  - The official ps2dev forums.

  #ps2dev on efnet (IRC)    - Come banter in realtime.
