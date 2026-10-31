
 #include <stdio.h>
 #include <string.h>
 #include <fcntl.h>
 #include <time.h>
#ifdef _WIN32
 #include <windows.h>
#else
 #include <unistd.h>
#endif

 #include "utility.h"

 ///////////////////////
 // UTILITY FUNCTIONS //
 ///////////////////////

 int fix_flags(int flags) { int result = 0;

  // Fix the flags.
  if (flags & 0x0001) { result |= O_RDONLY;           }
  if (flags & 0x0002) { result |= O_WRONLY | O_TRUNC; } // FIXME: Truncate is needed for some programs.
#ifndef _WIN32
  if (flags & 0x0010) { result |= O_NONBLOCK;         }
#endif
  if (flags & 0x0100) { result |= O_APPEND;           }
  if (flags & 0x0200) { result |= O_CREAT;            }
  if (flags & 0x0400) { result |= O_TRUNC;            }

#ifdef _WIN32

  // Binary mode file access.
  result |= O_BINARY;

#endif

  // End function.
  return result;

 }

 int fix_pathname(char *pathname) { unsigned int loop0 = 0;

  // If empty, set a pathname default.
  if (pathname[0] == 0) { strcpy(pathname, "."); }

  // Convert \ to / for unix compatibility.
  for (loop0=0; loop0<strlen(pathname); loop0++) { if (pathname[loop0] == '\\') { pathname[loop0] = '/'; } }

  // If a leading slash is found...
  if ((pathname[0] == '/') && (pathname[1] != 0)) {

   // Kill off the leading slash.
   for(loop0=0; loop0<strlen(pathname)-1; loop0++) { pathname[loop0] = pathname[loop0+1]; }

   // Terminate the pathname with a null.
   pathname[loop0] = 0;

  }

  // End function.
  return 0;

 }

 int fix_argv(char *destination, int size, char **argv) { int loop0 = 0, used = 0;

  // For each argv...
  for (loop0=0; argv[loop0]; loop0++) { int length = strlen(argv[loop0]) + 1;

   // The arguments, each with its null, must fit in the destination.
   if (used + length > size) { return -1; }

   // Copy the argv and its null to the destination.
   memcpy(&destination[used], argv[loop0], length);

   // Increment the destination offset.
   used += length;

  }

  // Clear the rest of the destination.
  memset(&destination[used], 0, size - used);

  // End function.
  return 0;

 }

 long long time_ms(void) {
#ifdef _WIN32
  return (long long)GetTickCount64();
#else
  struct timespec ts;

  // A monotonic clock, so time changes do not affect timeouts.
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
 }

 void sleep_ms(int ms) {
#ifdef _WIN32
  Sleep(ms);
#else
  struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };

  nanosleep(&ts, NULL);
#endif
 }

 int print_usage(void) {

  // Print out the ps2client usage string.
  fprintf(stderr, "\n");
  fprintf(stderr, " Usage: ps2client [-h hostname] [-t timeout] [-e marker] <command> [arguments]\n\n");
  fprintf(stderr, " Options:\n\n");
  fprintf(stderr, "   -h hostname  address of the ps2 (default: $PS2HOSTNAME, else 192.168.0.10)\n");
  fprintf(stderr, "   -t timeout   exit after <timeout> seconds without console output or requests\n");
  fprintf(stderr, "                (ping: give up after <timeout> seconds, default 2)\n");
  fprintf(stderr, "   -e marker    exit when a console line contains <marker>; the exit status is the\n");
  fprintf(stderr, "                number after 'status=' on that line (0 if there is none)\n\n");
  fprintf(stderr, " Available commands:\n\n");
  fprintf(stderr, "   reset\n");
  fprintf(stderr, "   execiop <filename> [arguments]\n");
  fprintf(stderr, "   execee <filename> [arguments]\n");
  fprintf(stderr, "   poweroff\n");
  fprintf(stderr, "   scrdump\n");
  fprintf(stderr, "   netdump\n");
  fprintf(stderr, "   dumpmem <offset> <size> <filename>\n");
  fprintf(stderr, "   startvu <vu>\n");
  fprintf(stderr, "   stopvu <vu>\n");
  fprintf(stderr, "   dumpreg <type> <filename>  (type: number, or dma intc timer gs sif fifo gif\n");
  fprintf(stderr, "                              vif0 vif1 ipu all vu0 vu1)\n");
  fprintf(stderr, "   gsexec <size> <filename>\n");
  fprintf(stderr, "   writemem <offset> <size> <filename>\n");
  fprintf(stderr, "   iopexcep\n");
  fprintf(stderr, "   listen\n");
  fprintf(stderr, "   console  (UDP-only logging without host: service)\n");
  fprintf(stderr, "   ping\n\n");
  fprintf(stderr, " Exit status: 0 ok, 64 usage error, 69 ps2link not answering, 74 ps2link closed\n");
  fprintf(stderr, " the connection, 75 console port in use (another ps2client running?), 124 timeout\n");
  fprintf(stderr, " before the -e marker. With -e: the status from the marker line.\n\n");

  // Print out the fsclient usage string.
  fprintf(stderr, " Usage: fsclient [-h hostname] <command> [arguments]\n\n");
  fprintf(stderr, " Available commands:\n\n");
  fprintf(stderr, "   copyfrom <source> <destination>\n");
  fprintf(stderr, "   copyto <source> <destination>\n");
  fprintf(stderr, "   delete <pathname>\n");
  fprintf(stderr, "   devlist\n");
  fprintf(stderr, "   dir <pathname>\n");
  fprintf(stderr, "   mkdir <pathname>\n");
  fprintf(stderr, "   mount <device> <fsname>\n");
  fprintf(stderr, "   rmdir <pathname>\n");
  fprintf(stderr, "   sync <device>\n");
  fprintf(stderr, "   umount <device>\n\n");

  // End function.
  return 0;

 }
