
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include "utility.h"
 #include "ps2link.h"
 #include "network.h"

 char hostname[256] = { "192.168.0.10" };

 int timeout = -1;

 char *marker = NULL;

 // How long to wait for ps2link to accept the connection and to answer.
 #define CONNECT_TIMEOUT_MS 3000
 #define HANDSHAKE_TIMEOUT_MS 3000

 // The commands: their minimum number of arguments, what they need from the
 // connection, and whether they wait for output afterwards.
 enum { CMD_RESET, CMD_EXECIOP, CMD_EXECEE, CMD_POWEROFF, CMD_SCRDUMP, CMD_NETDUMP, CMD_DUMPMEM, CMD_STARTVU,
        CMD_STOPVU, CMD_DUMPREG, CMD_GSEXEC, CMD_WRITEMEM, CMD_IOPEXCEP, CMD_LISTEN, CMD_PING };

 #define NEEDS_HOST 1 // ps2link reads or writes host: files, so wait until it can
 #define NO_WAIT    2 // exit right after sending the command

 struct { char *name; int id; int args; int flags; } commands[] = {
  { "reset",    CMD_RESET,    0, NO_WAIT    },
  { "execiop",  CMD_EXECIOP,  1, NEEDS_HOST },
  { "execee",   CMD_EXECEE,   1, NEEDS_HOST },
  { "poweroff", CMD_POWEROFF, 0, NO_WAIT    },
  { "scrdump",  CMD_SCRDUMP,  0, NO_WAIT    },
  { "netdump",  CMD_NETDUMP,  0, NO_WAIT    },
  { "dumpmem",  CMD_DUMPMEM,  3, NEEDS_HOST },
  { "startvu",  CMD_STARTVU,  1, NO_WAIT    },
  { "stopvu",   CMD_STOPVU,   1, NO_WAIT    },
  { "dumpreg",  CMD_DUMPREG,  2, NEEDS_HOST },
  { "gsexec",   CMD_GSEXEC,   2, NEEDS_HOST },
  { "writemem", CMD_WRITEMEM, 3, NEEDS_HOST },
  { "iopexcep", CMD_IOPEXCEP, 0, NO_WAIT    },
  { "listen",   CMD_LISTEN,   0, 0          },
  { "ping",     CMD_PING,     0, NEEDS_HOST },
 };

 // Register dump types, by name.
 char *regtypes[] = { "dma", "intc", "timer", "gs", "sif", "fifo", "gif", "vif0", "vif1", "ipu", "all", "vu0", "vu1" };

 // Parse a number, decimal or 0x hexadecimal. Returns -1 if it is not one.
 int parse_number(char *string, unsigned int *value) { char *end;

  *value = strtoul(string, &end, 0);
  return (*string == 0 || *end != 0) ? -1 : 0;

 }

 int parse_regtype(char *string) { unsigned int loop0, value;

  for (loop0=0; loop0<sizeof(regtypes)/sizeof(regtypes[0]); loop0++) { if (strcmp(string, regtypes[loop0]) == 0) { return loop0; } }
  if (parse_number(string, &value) == 0) { return value; }
  return -1;

 }

 int main(int argc, char **argv, char **env) { int loop0 = 0, command = -1, result = 0, what = 0;
  unsigned int number0 = 0, number1 = 0;

  // Turn off stdout buffering.
  setbuf(stdout, NULL);

  // Parse the environment list for optional arguments.
  for (loop0=0; env[loop0]; loop0++) {

   // A hostname has been specified...
   if (strncmp(env[loop0], "PS2HOSTNAME=", 12) == 0) { snprintf(hostname, sizeof(hostname), "%s", &env[loop0][12]); }

  }

  // Check the number of arguments.
  if (argc < 2) { print_usage(); return PS2CLIENT_EXIT_USAGE; }

  // Parse the argument list for optional arguments.
  for (loop0=1; argv[loop0]; loop0++) {

   // If an optional hostname has been specified...
   if (strcmp(argv[loop0], "-h") == 0) { loop0++;

    // Check to make sure the hostname value was actually supplied.
    if (argc == loop0) { fprintf(stderr, "Error: No hostname was supplied the '-h' option.\n"); print_usage(); return PS2CLIENT_EXIT_USAGE; }

    // Set the hostname to the supplied value.
    snprintf(hostname, sizeof(hostname), "%s", argv[loop0]);

   }

   // Else, if an optional timeout has been specified...
   else if (strcmp(argv[loop0], "-t") == 0) { loop0++;

    // Check to make sure a value was actually supplied.
    if (argc == loop0) { fprintf(stderr, "Error: No timeout was supplied the '-t' option.\n"); print_usage(); return PS2CLIENT_EXIT_USAGE; }

    // Set the timeout to the supplied value.
    timeout = atoi(argv[loop0]);

   }

   // Else, if an end marker has been specified...
   else if (strcmp(argv[loop0], "-e") == 0) { loop0++;

    // Check to make sure a marker was actually supplied.
    if (argc == loop0 || argv[loop0][0] == 0) { fprintf(stderr, "Error: No marker was supplied the '-e' option.\n"); print_usage(); return PS2CLIENT_EXIT_USAGE; }

    // Set the marker to the supplied value.
    marker = argv[loop0];

   }

   // Else, the end of the options has been reached...
   else { break; }

  }

  // Increment the argument counters past any optional arguments.
  loop0++; argc -= loop0; argv += loop0;

  // Check to make sure a command was actually supplied.
  if (argc < 0) { fprintf(stderr, "Error: No command was supplied.\n"); print_usage(); return PS2CLIENT_EXIT_USAGE; }

  // Find the command and check its arguments.
  for (loop0=0; loop0<(int)(sizeof(commands)/sizeof(commands[0])); loop0++) { if (strcmp(argv[-1], commands[loop0].name) == 0) { command = loop0; break; } }
  if (command < 0) { fprintf(stderr, "Error: Unknown command requested. (%s)\n", argv[-1]); print_usage(); return PS2CLIENT_EXIT_USAGE; }
  if (argc < commands[command].args) { fprintf(stderr, "Error: Not enough arguments for '%s'.\n", argv[-1]); print_usage(); return PS2CLIENT_EXIT_USAGE; }
  switch (commands[command].id) {
   case CMD_DUMPMEM: case CMD_WRITEMEM:
    if (parse_number(argv[0], &number0) < 0 || parse_number(argv[1], &number1) < 0) { fprintf(stderr, "Error: Bad offset or size. (%s %s)\n", argv[0], argv[1]); return PS2CLIENT_EXIT_USAGE; }
    break;
   case CMD_DUMPREG:
    if ((int)(number0 = parse_regtype(argv[0])) < 0) { fprintf(stderr, "Error: Unknown register type. (%s)\n", argv[0]); return PS2CLIENT_EXIT_USAGE; }
    break;
   case CMD_EXECIOP: case CMD_EXECEE: { int length = 0;
    for (loop0=0; loop0<argc; loop0++) { length += strnlen(argv[loop0], 257) + 1; }
    if (length > 256) { fprintf(stderr, "Error: The filename and arguments are too long (%d bytes, at most 256 with a null after each).\n", length); return PS2CLIENT_EXIT_USAGE; }
    break;
   }
   case CMD_STARTVU: case CMD_STOPVU: case CMD_GSEXEC:
    if (parse_number(argv[0], &number0) < 0) { fprintf(stderr, "Error: Bad number. (%s)\n", argv[0]); return PS2CLIENT_EXIT_USAGE; }
    break;
  }

  // Commands that exit right away need neither the console nor the request
  // connection. ping needs only the request connection.
  if (commands[command].flags & NO_WAIT) { timeout = 0; }
  if (commands[command].flags & NEEDS_HOST) { what |= PS2LINK_CONNECT_REQUEST; }
  if (!(commands[command].flags & NO_WAIT) && commands[command].id != CMD_PING) { what |= PS2LINK_CONNECT_CONSOLE | PS2LINK_CONNECT_REQUEST; }

  // Startup network.
  if (network_startup() < 0) { fprintf(stderr, "Error: Could not startup network.\n"); return PS2CLIENT_EXIT_NOCONNECT; }

  // ping: is ps2link up and serving host: files? -t is how long to try.
  if (commands[command].id == CMD_PING) { int limit = timeout > 0 ? timeout * 1000 : 2000; long long start = time_ms();

   if ((result = ps2link_connect(hostname, what, limit)) != 0) { return result; }
   result = ps2link_handshake(limit - (int)(time_ms() - start));
   ps2link_disconnect();
   if (result < 0) { fprintf(stderr, "ps2link at %s is not answering.\n", hostname); return PS2CLIENT_EXIT_NOCONNECT; }
   printf("ps2link at %s answered in %d ms.\n", hostname, result);
   return PS2CLIENT_EXIT_OK;

  }

  // Connect to the ps2link server.
  ps2link_set_marker(marker);
  if ((result = ps2link_connect(hostname, what, CONNECT_TIMEOUT_MS)) != 0) { return result; }

  // Wait until ps2link can serve the host: files the command needs.
  if ((what & PS2LINK_CONNECT_REQUEST) && ps2link_handshake(HANDSHAKE_TIMEOUT_MS) < 0) {
   fprintf(stderr, "Error: ps2link at %s accepted the connection but does not answer.\n", hostname);
   ps2link_disconnect();
   return PS2CLIENT_EXIT_NOCONNECT;
  }

  // Perform the requested command.
  switch (commands[command].id) {
   case CMD_RESET:    result = ps2link_command_reset();                         break;
   case CMD_EXECIOP:  result = ps2link_command_execiop(argc, argv);             break;
   case CMD_EXECEE:   result = ps2link_command_execee(argc, argv);              break;
   case CMD_POWEROFF: result = ps2link_command_poweroff();                      break;
   case CMD_SCRDUMP:  result = ps2link_command_scrdump();                       break;
   case CMD_NETDUMP:  result = ps2link_command_netdump();                       break;
   case CMD_DUMPMEM:  result = ps2link_command_dumpmem(number0, number1, argv[2]);  break;
   case CMD_STARTVU:  result = ps2link_command_startvu(number0);                break;
   case CMD_STOPVU:   result = ps2link_command_stopvu(number0);                 break;
   case CMD_DUMPREG:  result = ps2link_command_dumpreg(number0, argv[1]);       break;
   case CMD_GSEXEC:   result = ps2link_command_gsexec(number0, argv[1]);        break;
   case CMD_WRITEMEM: result = ps2link_command_writemem(number0, number1, argv[2]); break;
   case CMD_IOPEXCEP: result = ps2link_command_iopexcep();                      break;
   case CMD_LISTEN:   result = 0;                                               break;
  }
  if (result < 0) { fprintf(stderr, "Error: Could not send the command.\n"); ps2link_disconnect(); return PS2CLIENT_EXIT_NOCONNECT; }

  // Enter the main loop.
  result = ps2link_mainloop(timeout);

  // Disconnect from the ps2link server.
  ps2link_disconnect();

  // End program.
  return result;

 }
