
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 #include "types.h"
 #include "ps2link.h"
 #include "network.h"

 #define MESSAGE_START	"[***] ps2client v1.0.5 by Dan Peori (peori@oopo.net)\n"
 #define MESSAGE_USAGE	"[***] Usage: %s <hostname> <command> [argument]\n", argv[0]

 char hostname[1024], command[1024], filename[1024];

 int main(int argc, char *argv[]) {

  // Output the start message.
  printf(MESSAGE_START);

  // Check for a hostname.
  if (argc > 1) { sprintf(hostname, argv[1]); } else { printf(MESSAGE_USAGE); return -1; }
  if (argc > 2) { sprintf(command,  argv[2]); } else { printf(MESSAGE_USAGE); return -1; }
  if (argc > 3) { sprintf(filename, argv[3]); }

  // Open the connection.
  ps2link_open(hostname);

  // Perform the given command.
  if (!strcmp(command, "reset"))    { ps2link_cmd_reset();           } else
  if (!strcmp(command, "poweroff")) { ps2link_cmd_poweroff();        } else
  if (!strcmp(command, "execiop"))  { ps2link_cmd_execiop(filename); } else
  if (!strcmp(command, "execee"))   { ps2link_cmd_execee(filename);  }

  // Close the connection.
  ps2link_shut();

  // End program.
  return 0;

 }
