
 #include <stdio.h>
 #include <unistd.h>
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
  printf("  %s <hostname> copyfrom <source> <destination>\n", progname);
  printf("  %s <hostname> copyto <source> <destination>\n", progname);
  printf("  %s <hostname> delete <filename>\n", progname);
  printf("  %s <hostname> devlist\n", progname);
  printf("  %s <hostname> dir <directory>\n", progname);
  printf("  %s <hostname> format <device>\n", progname);
  printf("  %s <hostname> mkdir <directory>\n", progname);
  printf("  %s <hostname> mount <device> <fsname>\n", progname);
  printf("  %s <hostname> rename <source> <destination>\n", progname);
  printf("  %s <hostname> rmdir <directory>\n", progname);
  printf("  %s <hostname> sync <device>\n", progname);
  printf("  %s <hostname> umount <device>\n\n", progname);

#endif

  // End function.
  return 0;

 }

 int main(int argc, char *argv[]) {

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
  if (!strcmp(command, "reset"))    { return ps2link_command_reset(hostname); }
  if (!strcmp(command, "execiop"))  { ps2link_connect(hostname); ps2link_command_execiop(hostname, arg0); ps2link_mainloop(); return ps2link_disconnect(); }
  if (!strcmp(command, "execee"))   { ps2link_connect(hostname); ps2link_command_execee(hostname, arg0); ps2link_mainloop(); return ps2link_disconnect(); }
  if (!strcmp(command, "poweroff")) { return ps2link_command_poweroff(hostname); }
  if (!strcmp(command, "dumpmem"))  { ps2link_connect(hostname); ps2link_command_dumpmem(hostname, arg0, atoi(arg1), atoi(arg2)); ps2link_mainloop(); return ps2link_disconnect(); }
  if (!strcmp(command, "startvu"))  { return ps2link_command_startvu(hostname, atoi(arg0)); }
  if (!strcmp(command, "stopvu"))   { return ps2link_command_stopvu(hostname, atoi(arg0)); }
  if (!strcmp(command, "dumpreg"))  { ps2link_connect(hostname); ps2link_command_dumpreg(hostname, arg0, atoi(arg1)); ps2link_mainloop(); return ps2link_disconnect(); }
  if (!strcmp(command, "gsexec"))   { ps2link_connect(hostname); ps2link_command_gsexec(hostname, arg0,  atoi(arg1)); ps2link_mainloop(); return ps2link_disconnect(); }
  if (!strcmp(command, "listen"))   { ps2link_connect(hostname); ps2link_mainloop(); return ps2link_disconnect(); }

  // Perform any ps2netfs commands.
  if (!strcmp(command, "copyfrom")) { return ps2netfs_command_copyfrom(hostname, arg0, arg1); }
  if (!strcmp(command, "copyto"))   { return ps2netfs_command_copyto(hostname, arg0, arg1); }
  if (!strcmp(command, "delete"))   { return ps2netfs_command_delete(hostname, arg0); }
  if (!strcmp(command, "devlist"))  { return ps2netfs_command_devlist(hostname); }
  if (!strcmp(command, "dir"))      { return ps2netfs_command_dir(hostname, arg0); }
  if (!strcmp(command, "format"))   { return ps2netfs_command_format(hostname, arg0); }
  if (!strcmp(command, "mkdir"))    { return ps2netfs_command_mkdir(hostname, arg0); }
  if (!strcmp(command, "mount"))    { return ps2netfs_command_mount(hostname, arg0, arg1); }
  if (!strcmp(command, "rename"))   { return ps2netfs_command_rename(hostname, arg0, arg1); }
  if (!strcmp(command, "rmdir"))    { return ps2netfs_command_rmdir(hostname, arg0); }
  if (!strcmp(command, "sync"))     { return ps2netfs_command_sync(hostname, arg0); }
  if (!strcmp(command, "umount"))   { return ps2netfs_command_umount(hostname, arg0); }

  // Unknown command.
  ps2client_usage(argv[0]);

  // End program.
  return 0;

 }
