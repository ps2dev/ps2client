
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

 ---------------
 PS2CLIENT USAGE
 ---------------

  Basic usage:

   ps2client <address> <command> [argument]

   <address> is the actual address of the PS2. This can be either an ip
   or even a hostname. You get to choose, but choose carefully...

   <command> is what you would like the PS2 to do. Valid commands are 
   reset, poweroff, execiop and execee. Examples will follow, relax!

   [argument] is an optional argument for specifying a filename to 
   either execiop or execee to be executed. Don't worry, it is quite 
   humane - those executed can't feel the pain. Honest.

  Basic examples:

   ps2client 192.168.100.10 reset

   ps2client 192.168.100.10 poweroff

   ps2client 192.168.100.10 execiop cdrom0:/program.irx

   ps2client 192.168.100.10 execee host:/programs/hello.elf

 ----------------
 MORE INFORMATION
 ----------------

  If you want to get into the wonderful world of homebrew PS2
  development, there's a few places you can check out:

  http://www.ps2dev.org			- News, tutorials and documentation.

  #ps2dev on efnet (IRC)		- Come banter in realtime.
