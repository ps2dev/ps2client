
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "ps2link.h"
 #include "ps2netfs.h"

 int main(int argc, char *argv[]) { setbuf(stdout, NULL);

  // Output the startup message.
  printf("[***] ps2client - Your friendly, neighbourhood PS2 client.\n");

  // Check the number of arguments.
  if (argc < 3) { printf("[***] Usage: %s <hostname> <command> [arguments]\n", argv[0]); return -1; }

  // Peform any ps2link commands.
  if (!strcmp(argv[2], "reset"))       { return ps2link_command_reset(argv[1]); }
  if (!strcmp(argv[2], "execiop"))     { ps2link_command_execiop(argv[1], argv[3]); return ps2link_mainloop(argv[1]); }
  if (!strcmp(argv[2], "execee"))      { ps2link_command_execee(argv[1], argv[3]); return ps2link_mainloop(argv[1]); }
  if (!strcmp(argv[2], "poweroff"))    { return ps2link_command_poweroff(argv[1]); }
  if (!strcmp(argv[2], "dumpmem"))     { ps2link_command_dumpmem(argv[1], argv[3], atoi(argv[4]), atoi(argv[5])); return ps2link_mainloop(argv[1]); }
  if (!strcmp(argv[2], "startvu"))     { return ps2link_command_startvu(argv[1], atoi(argv[3])); }
  if (!strcmp(argv[2], "stopvu"))      { return ps2link_command_stopvu(argv[1], atoi(argv[3])); }
  if (!strcmp(argv[2], "dumpreg"))     { ps2link_command_dumpreg(argv[1], argv[3], atoi(argv[4])); return ps2link_mainloop(argv[1]); }
  if (!strcmp(argv[2], "gsexec"))      { ps2link_command_gsexec(argv[1], argv[3], atoi(argv[4])); return ps2link_mainloop(argv[1]); }
  if (!strcmp(argv[2], "listen"))      { printf("[***] Entering listen mode...\n"); return ps2link_listenloop(); }

  // Perform any ps2netfs commands.
  if (!strcmp(argv[2], "ps2dir"))      { return ps2netfs_command_ps2dir(argv[1], argv[3]); }
  if (!strcmp(argv[2], "ps2devlist"))  { return ps2netfs_command_ps2devlist(argv[1]); }
  if (!strcmp(argv[2], "ps2copyto"))   { return ps2netfs_command_ps2copyto(argv[1], argv[3], argv[4]); }
  if (!strcmp(argv[2], "ps2copyfrom")) { return ps2netfs_command_ps2copyfrom(argv[1], argv[3], argv[4]); }
  if (!strcmp(argv[2], "ps2mount"))    { return ps2netfs_command_ps2mount(argv[1], argv[3], argv[4]); }
  if (!strcmp(argv[2], "ps2umount"))   { return ps2netfs_command_ps2umount(argv[1], argv[3]); }
  if (!strcmp(argv[2], "ps2mkdir"))    { return ps2netfs_command_ps2mkdir(argv[1], argv[3]); }
  if (!strcmp(argv[2], "ps2sync"))     { return ps2netfs_command_ps2sync(argv[1], argv[3]); }

  // End program.
  return 0;

 }
