
                    ===================================
                      PS2CLIENT USAGE AND INFORMATION
                    ===================================

 ----------------
 THE INTRODUCTION
 ----------------

  This program is a simple, command-line tool used to access a PS2 
  running either pukklink or ps2link for development purposes. It allows
  you to reset or poweroff the PS2, or upload a program to either the EE
  or IOP. Other features will most likely be added in the future as the
  PS2-side software expands and improves.

 ------------------
 BUILDING PS2CLIENT
 ------------------

  Building ps2client is very simple:

   make

  But say you want to install it? Try:

   make install

  There are some build options:

   PREFIX=/dir	- Install directory, defaults to: $PS2DEV/bin

   QUIET=YES	- Disable non-essential text output.

   DEBUG=YES	- Enable all debug text output.

  A building example:

   make clean; make DEBUG=NO QUIET=YES install

 ---------------
 PS2CLIENT USAGE
 ---------------

  Basic usage:

   ps2client [-h hostname] [-t timeout] <command> [arguments]

   The hostname is the actual address, or ip of the PS2. If none is
   specified, ps2client will check for a hostname in $PS2HOSTNAME.

   Timeout is the length of inactivity (in seconds) that ps2client will
   wait before exiting. If none is specified, -1 (never exit) is assumed.

   <command> is what you would like the ps2 to do. Some valid commands
   are reset, poweroff, execiop and execee. See below for a full listing.

   [argument] is an optional argument for the given command.

  Commands for ps2link:

   reset
   execiop <filename> [arguments]
   execee <filename> [arguments]
   poweroff
   dumpmem <filename> <offset> <size>
   startvu <vu>
   stopvu <vu>
   dumpreg <filename> <type>
   gsexec <filename>
   listen

  Commands for ps2netfs:

   copyfrom <source> <destination>
   copyto <source> <destination>
   delete <filename>
   devlist
   dir <directory>
   format <device>
   mkdir <directory>
   mount <device> <fsname>
   rename <source> <destination>
   rmdir <directory>
   sync <device>
   umount <device>

 ----------------
 MORE INFORMATION
 ----------------

  If you want to get into the wonderful world of homebrew PS2
  development, there's a few places you can check out:

  http://www.ps2dev.org  - News, tutorials and documentation.

  #ps2dev on efnet (IRC) - Come banter in realtime.
