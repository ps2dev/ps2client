
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "ps2link.h"
 #include "ps2netfs.h"

 char hostname[256], command[256], arg0[256], arg1[256], arg2[256];

 int ps2client_usage(char *progname) {

#ifdef __CHATTY__

  // Output the usage instructions.
  printf("\n Basic usage:\n\n");
  printf("  %s <hostname> <command> [arguments]\n\n", progname);

  // Output the ps2link commands.
  printf(" Commands for ps2link:\n\n");
  printf("  %s <hostname> reset\n", progname);
  printf("  %s <hostname> execiop <filename>\n", progname);
  printf("  %s <hostname> execee <filename>\n", progname);
  printf("  %s <hostname> poweroff\n", progname);
  printf("  %s <hostname> dumpmem <filename> <offset> <size>\n", progname);
  printf("  %s <hostname> startvu <0/1>\n", progname);
  printf("  %s <hostname> stopvu <0/1>\n", progname);
  printf("  %s <hostname> dumpreg <filename> <type>\n", progname);
  printf("  %s <hostname> gsexec <filename>\n", progname);
  printf("  %s <hostname> listen\n\n", progname);

  // Output the ps2netfs commands.
  printf(" Commands for ps2netfs:\n\n");
  printf("  %s <hostname> ps2copyfrom <source> <destination>\n", progname);
  printf("  %s <hostname> ps2copyto <source> <destination>\n", progname);
  printf("  %s <hostname> ps2delete <filename>\n", progname);
  printf("  %s <hostname> ps2devlist\n", progname);
  printf("  %s <hostname> ps2dir <directory>\n", progname);
  printf("  %s <hostname> ps2format <device>\n", progname);
  printf("  %s <hostname> ps2mkdir <directory>\n", progname);
  printf("  %s <hostname> ps2mount <directory> <device>\n", progname);
  printf("  %s <hostname> ps2rename <source> <destination>\n", progname);
  printf("  %s <hostname> ps2rmdir <directory>\n", progname);
  printf("  %s <hostname> ps2sync <device>\n", progname);
  printf("  %s <hostname> ps2umount <device>\n\n", progname);

#endif

  // End function.
  return 0;

 }

 int main(int argc, char *argv[]) { setbuf(stdout, NULL);

#ifdef __CHATTY__

  // Output the startup message.
  printf("[***] ps2client - Your friendly, neighbourhood PS2 client.\n");

#endif

  // Check the arguments.
  if (argc > 1) { strncpy(hostname, argv[1], 256); } else { ps2client_usage(argv[0]); return -1; }
  if (argc > 2) { strncpy(command,  argv[2], 256); } else { ps2client_usage(argv[0]); return -1; }
  if (argc > 3) { strncpy(arg0,     argv[3], 256); } else { sprintf(arg0, "BLANK"); }
  if (argc > 4) { strncpy(arg1,     argv[4], 256); } else { sprintf(arg1, "BLANK"); }
  if (argc > 5) { strncpy(arg2,     argv[5], 256); } else { sprintf(arg2, "BLANK"); }

  // Peform any ps2link commands.
  if (!strcmp(command, "listen"))      { return ps2link_listenloop(); }
  if (!strcmp(command, "reset"))       { return ps2link_command_reset(hostname); }
  if (!strcmp(command, "execiop"))     { ps2link_command_execiop(hostname, arg0); return ps2link_mainloop(hostname); }
  if (!strcmp(command, "execee"))      { ps2link_command_execee(hostname, arg0); return ps2link_mainloop(hostname); }
  if (!strcmp(command, "poweroff"))    { return ps2link_command_poweroff(hostname); }
  if (!strcmp(command, "dumpmem"))     { ps2link_command_dumpmem(hostname, arg0, atoi(arg1), atoi(arg2)); return ps2link_mainloop(hostname); }
  if (!strcmp(command, "startvu"))     { return ps2link_command_startvu(hostname, atoi(arg0)); }
  if (!strcmp(command, "stopvu"))      { return ps2link_command_stopvu(hostname, atoi(arg0)); }
  if (!strcmp(command, "dumpreg"))     { ps2link_command_dumpreg(hostname, arg0, atoi(arg1)); return ps2link_mainloop(hostname); }
  if (!strcmp(command, "gsexec"))      { ps2link_command_gsexec(hostname, arg0, atoi(arg1)); return ps2link_mainloop(hostname); }

  // Perform any ps2netfs commands.
  if (!strcmp(command, "ps2copyfrom")) { return ps2netfs_command_ps2copyfrom(hostname, arg0, arg1); }
  if (!strcmp(command, "ps2copyto"))   { return ps2netfs_command_ps2copyto(hostname, arg0, arg1); }
  if (!strcmp(command, "ps2delete"))   { return ps2netfs_command_ps2delete(hostname, arg0); }
  if (!strcmp(command, "ps2devlist"))  { return ps2netfs_command_ps2devlist(hostname); }
  if (!strcmp(command, "ps2dir"))      { return ps2netfs_command_ps2dir(hostname, arg0); }
  if (!strcmp(command, "ps2format"))   { return ps2netfs_command_ps2format(hostname, arg0); }
  if (!strcmp(command, "ps2mkdir"))    { return ps2netfs_command_ps2mkdir(hostname, arg0); }
  if (!strcmp(command, "ps2mount"))    { return ps2netfs_command_ps2mount(hostname, arg0, arg1); }
  if (!strcmp(command, "ps2rename"))   { return ps2netfs_command_ps2rename(hostname, arg0, arg1); }
  if (!strcmp(command, "ps2rmdir"))    { return ps2netfs_command_ps2rmdir(hostname, arg0); }
  if (!strcmp(command, "ps2sync"))     { return ps2netfs_command_ps2sync(hostname, arg0); }
  if (!strcmp(command, "ps2umount"))   { return ps2netfs_command_ps2umount(hostname, arg0); }

  // Unknown command.
  ps2client_usage(argv[0]);

  // End program.
  return 0;

 }
