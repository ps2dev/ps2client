
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

  If you want to get fancy, you can even do:

   make PREFIX=/some/directory install

  Otherwise, it'll default to $PS2DEV as a prefix.

  There are defines in the makefile for general chattiness and debug
  output. Enable them and recompile if you want more information.

 ---------------
 PS2CLIENT USAGE
 ---------------

  Basic Usage:

   ps2client <hostname> <command> [arguments]

   <hostname> is the actual hostname of the PS2. This can be either
   a hostname or an ip. You get to choose, but choose carefully...

   <command> is what you would like the PS2 to do. Valid commands are 
   reset, poweroff, execiop and execee. Examples will follow, relax!

   [argument] is an optional argument for specifying a filename to 
   either execiop or execee to be executed. Don't worry, it is quite 
   humane - those executed can't feel the pain. Honest.

  Commands for ps2link:

   ps2client <hostname> reset

   ps2client <hostname> execiop <filename>

   ps2client <hostname> execee <filename>

   ps2client <hostname> poweroff

   ps2client <hostname> dumpmem <filename> <offset> <size>

   ps2client <hostname> startvu <0/1>

   ps2client <hostname> stopvu <0/1>

   ps2client <hostname> dumpreg <filename> <type>

   ps2client <hostname> gsexec <filename>

   ps2client <hostname> listen

  Commands for ps2netfs:

   ps2client <hostname> copyfrom <source> <destination>

   ps2client <hostname> copyto <source> <destination>

   ps2client <hostname> delete <filename>

   ps2client <hostname> devlist

   ps2client <hostname> dir <directory>

   ps2client <hostname> format <device>

   ps2client <hostname> mkdir <directory>

   ps2client <hostname> mount <device> <fsname>

   ps2client <hostname> rename <source> <destination>

   ps2client <hostname> rmdir <directory>

   ps2client <hostname> sync <device>

   ps2client <hostname> umount <device>

 ----------------
 MORE INFORMATION
 ----------------

  If you want to get into the wonderful world of homebrew PS2
  development, there's a few places you can check out:

  http://www.ps2dev.org  - News, tutorials and documentation.

  #ps2dev on efnet (IRC) - Come banter in realtime.
