
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include "ps2link.h"

 char hostname[1024] = { "192.168.1.10" };

 int timeout = -1;

 /////////////////////////
 // PS2CLIENT FUNCTIONS //
 /////////////////////////

 int main(int argc, char **argv, char **env) { int loop0 = 0;

  // Turn off stdout buffering.
  setbuf(stdout, NULL);

  // Parse the environment list for optional arguments.
  for (loop0=0; env[loop0]; loop0++) {

   // A hostname has been specified...
   if (strncmp(env[loop0], "PS2HOSTNAME", 11) == 0) { strncpy(hostname, &env[loop0][12], sizeof(hostname)); }

  }

  // Parse the argument list for optional arguments.
  for (loop0=1; argv[loop0]; loop0++) {

   // A hostname has been specified...
   if (strncmp(argv[loop0], "-h", 2) == 0) { loop0++; strncpy(hostname, argv[loop0], sizeof(hostname)); }

   // A timeout has been specified...
   else if (strncmp(argv[loop0], "-t", 2) == 0) { loop0++; timeout = atoi(argv[loop0]); }

   // The end of the options has been reached.
   else { break; }

  }

  // Fix the argument counters.
  loop0++; argc -= loop0; argv += loop0;

#ifdef _WIN32
  // Startup network, under windows.
  if (network_startup() < 0) { printf("[ERROR] Could not start up winsock.\n"); return 1; }
#endif

  // Connect to the ps2link server.
  if (ps2link_connect(hostname) < 0) { printf("[ERROR] Could not connect to ps2link. (%s)\n", hostname); return -1; }

  // Send the command to the ps2link server.
  if (strcmp(argv[-1], "reset")    == 0) { ps2link_command_reset(); timeout = 0;                            } else
  if (strcmp(argv[-1], "execiop")  == 0) { ps2link_command_execiop(argc, argv);                             } else
  if (strcmp(argv[-1], "execee")   == 0) { ps2link_command_execee(argc, argv);                              } else
  if (strcmp(argv[-1], "poweroff") == 0) { ps2link_command_poweroff(); timeout = 0;                         } else
  if (strcmp(argv[-1], "scrdump")  == 0) { ps2link_command_scrdump(); timeout = 0;                          } else
  if (strcmp(argv[-1], "netdump")  == 0) { ps2link_command_netdump(); timeout = 0;                          } else
  if (strcmp(argv[-1], "dumpmem")  == 0) { ps2link_command_dumpmem(atoi(argv[0]), atoi(argv[1]), argv[2]);  } else
  if (strcmp(argv[-1], "startvu")  == 0) { ps2link_command_startvu(atoi(argv[0])); timeout = 0;             } else
  if (strcmp(argv[-1], "stopvu")   == 0) { ps2link_command_stopvu(atoi(argv[0])); timeout = 0;              } else
  if (strcmp(argv[-1], "dumpreg")  == 0) { ps2link_command_dumpreg(atoi(argv[0]), argv[1]);                 } else
  if (strcmp(argv[-1], "gsexec")   == 0) { ps2link_command_gsexec(atoi(argv[0]), argv[1]);                  } else
  if (strcmp(argv[-1], "writemem") == 0) { ps2link_command_writemem(atoi(argv[0]), atoi(argv[1]), argv[2]); } else
  if (strcmp(argv[-1], "iopexcep") == 0) { ps2link_command_iopexcep(); timeout = 0;                         } else
  if (strcmp(argv[-1], "listen")   == 0) {                                                                  }

  // An unknown command was requested.
  else { printf("[ERROR] Unknown command requested. (%s)\n", argv[-1]); return -1; }

  // Enter the main loop.
  ps2link_mainloop(timeout);

  // Disconnect from the ps2link server.
  ps2link_disconnect();

  // End program.
  return 0;

 }
