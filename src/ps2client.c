
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include "ps2link.h"
 #include "ps2netfs.h"
 #include "ps2client.h"

 char hostname[256], command[256], argvalues[256]; int argcount = 0;

 char arg0[256], arg1[256], arg2[256], arg3[256];

 int timeout = DEFAULT_TIMEOUT;

 //////////////////////////////
 // PS2CLIENT MAIN FUNCTIONS //
 //////////////////////////////

 int main(int argc, char *argv[], char *env[]) { int loop0, count = 1; char *temp = argvalues;

  // Turn off stdout buffering.
  setbuf(stdout, NULL);

  // First, we parse the optional arguments.
  for(loop0=0;loop0<argc;loop0++) {

   // A hostname has been specified.
   if (strcmp(argv[loop0], "-h") == 0) { count = loop0++ + 2; strncpy(hostname, argv[loop0], sizeof(hostname)); }

   // A timeout has been specified.
   if (strcmp(argv[loop0], "-t") == 0) { count = loop0++ + 2; timeout = atoi(argv[loop0]); }

  }

  // Make sure that we have a hostname.
  if (strlen(hostname) == 0) {

   // Assign a default hostname...
   sprintf(hostname, DEFAULT_HOSTNAME);

   // ...but if $PS2HOSTNAME is found we'll use that instead.
   for(loop0=0;env[loop0];loop0++) { if (strncmp(env[loop0], "PS2HOSTNAME", 11) == 0) { sprintf(hostname, &env[loop0][12]); } }

  }

  // Next, determine the command.
  if (argc > count) { strncpy(command, argv[count++], sizeof(command)); } else { sprintf(command, "NULL"); }

  // Get the arguments for execee/execiop.
  if (argc > count) {

   // Clear the argvalues for use.
   memset(argvalues, 0, sizeof(argvalues));

   // Copy over each argument.
   for(loop0=count;loop0<argc;loop0++) { strcpy(temp, argv[loop0]); temp += strlen(argv[loop0]) + 1; }

   // Set the argcount.
   argcount = argc - count;

  }

  // Get the arguments for the other commands.
  if (argc > count) { strncpy(arg0, argv[count++], sizeof(arg0)); } else { sprintf(arg0, "NULL"); }
  if (argc > count) { strncpy(arg1, argv[count++], sizeof(arg1)); } else { sprintf(arg1, "NULL"); }
  if (argc > count) { strncpy(arg2, argv[count++], sizeof(arg2)); } else { sprintf(arg2, "NULL"); }
  if (argc > count) { strncpy(arg3, argv[count++], sizeof(arg3)); } else { sprintf(arg3, "NULL"); }

  // Peform any ps2link commands.
  if (strcmp(command, "reset")    == 0) { return ps2link_command_reset(hostname); }
  if (strcmp(command, "execiop")  == 0) { return ps2link_command_execiop(hostname, timeout, argcount, argvalues); }
  if (strcmp(command, "execee")   == 0) { return ps2link_command_execee(hostname, timeout, argcount, argvalues); }
  if (strcmp(command, "poweroff") == 0) { return ps2link_command_poweroff(hostname); }
  if (strcmp(command, "dumpmem")  == 0) { return ps2link_command_dumpmem(hostname, timeout, arg0, atoi(arg1), atoi(arg2)); }
  if (strcmp(command, "startvu")  == 0) { return ps2link_command_startvu(hostname, atoi(arg0)); }
  if (strcmp(command, "stopvu")   == 0) { return ps2link_command_stopvu(hostname, atoi(arg0)); }
  if (strcmp(command, "dumpreg")  == 0) { return ps2link_command_dumpreg(hostname, timeout, arg0, atoi(arg1)); }
  if (strcmp(command, "gsexec")   == 0) { return ps2link_command_gsexec(hostname, timeout, arg0); }
  if (strcmp(command, "listen")   == 0) { return ps2link_command_listen(hostname, timeout); }

  // Perform any ps2netfs commands.
  if (strcmp(command, "copyfrom") == 0) { return ps2netfs_command_copyfrom(hostname, arg0, arg1); }
  if (strcmp(command, "copyto")   == 0) { return ps2netfs_command_copyto(hostname, arg0, arg1); }
  if (strcmp(command, "delete")   == 0) { return ps2netfs_command_delete(hostname, arg0); }
  if (strcmp(command, "devlist")  == 0) { return ps2netfs_command_devlist(hostname); }
  if (strcmp(command, "dir")      == 0) { return ps2netfs_command_dir(hostname, arg0); }
  if (strcmp(command, "format")   == 0) { return ps2netfs_command_format(hostname, arg0); }
  if (strcmp(command, "mkdir")    == 0) { return ps2netfs_command_mkdir(hostname, arg0); }
  if (strcmp(command, "mount")    == 0) { return ps2netfs_command_mount(hostname, arg0, arg1); }
  if (strcmp(command, "rename")   == 0) { return ps2netfs_command_rename(hostname, arg0, arg1); }
  if (strcmp(command, "rmdir")    == 0) { return ps2netfs_command_rmdir(hostname, arg0); }
  if (strcmp(command, "sync")     == 0) { return ps2netfs_command_sync(hostname, arg0); }
  if (strcmp(command, "umount")   == 0) { return ps2netfs_command_umount(hostname, arg0); }

  // Command not found.
  ps2client_printusage(argv[0]);

  // End program.
  return 0;

 }

 /////////////////////////////////
 // PS2CLIENT TOOLBOX FUNCTIONS //
 /////////////////////////////////

 int ps2client_printusage(char *program) {

  // Output the startup message.
  printf("[***] %s - Your friendly, neighbourhood ps2 client.\n", program);
  printf("[***] Using ps2 hostname: %s\n", hostname);

  // Output the usage instructions.
  printf("\n Basic usage:\n\n");
  printf("  %s [-h hostname] [-t timeout] <command> [arguments]\n\n", program);

  // Output the ps2link commands.
  printf(" Commands for ps2link:\n\n");
  printf("  reset\n");
  printf("  execiop <filename> [arguments]\n");
  printf("  execee <filename> [arguments]\n");
  printf("  poweroff\n");
  printf("  dumpmem <filename> <offset> <size>\n");
  printf("  startvu <vu>\n");
  printf("  stopvu <vu>\n");
  printf("  dumpreg <filename> <type>\n");
  printf("  gsexec <filename>\n");
  printf("  listen\n\n");

  // Output the ps2netfs commands.
  printf(" Commands for ps2netfs:\n\n");
  printf("  copyfrom <source> <destination>\n");
  printf("  copyto <source> <destination>\n");
  printf("  delete <filename>\n");
  printf("  devlist\n");
  printf("  dir <directory>\n");
  printf("  format <device>\n");
  printf("  mkdir <directory>\n");
  printf("  mount <device> <fsname>\n");
  printf("  rename <source> <destination>\n");
  printf("  rmdir <directory>\n");
  printf("  sync <device>\n");
  printf("  umount <device>\n\n");

  // End function.
  return 0;

 }
