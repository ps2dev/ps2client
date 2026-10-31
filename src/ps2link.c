
 #include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <string.h>
 #include <dirent.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/stat.h>
 #include <pthread.h>
#ifndef _WIN32
 #include <netinet/in.h>
 #include <netinet/tcp.h>
#else
 #include <windows.h>
#endif

 #include "network.h"
 #include "ps2link.h"
 #include "utility.h"

 int console_socket = -1;
 int request_socket = -1;
 int command_socket = -1;

 pthread_t console_thread_id;
 pthread_t request_thread_id;

 // State shared between the threads and the main loop.
 pthread_mutex_t ps2link_lock = PTHREAD_MUTEX_INITIALIZER;
 long long ps2link_activity = 0;    // time_ms() of the last console output or request
 int ps2link_lost = 0;              // ps2link closed the request connection
 int ps2link_ping_answered = 0;     // the handshake round trip has completed
 int ps2link_marker_seen = 0;
 int ps2link_marker_status = 0;

 // The console line being scanned for the marker.
 char *ps2link_marker = NULL;
 char ps2link_line[4096];
 int ps2link_line_length = 0;

 // The handshake has ps2link dump a few registers to this host: file, which
 // the request thread sends to the null device.
 #define PS2LINK_PING_PATHNAME "ps2client.ping"
#ifdef _WIN32
 #define PS2LINK_NULL_DEVICE "NUL"
#else
 #define PS2LINK_NULL_DEVICE "/dev/null"
#endif
 #define PS2LINK_PING_REGS 1 // INTC: two registers, read without side effects
 int ps2link_ping_fd = -1;

 // ps2link_dd is now an array of structs
 struct {
    char *pathname; // remember to free when closing dir
    DIR *dir;
 } ps2link_dd[10] = {
  { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL },
  { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }, { NULL, NULL }
 };

 static void ps2link_set_activity(void) {
  pthread_mutex_lock(&ps2link_lock);
  ps2link_activity = time_ms();
  pthread_mutex_unlock(&ps2link_lock);
 }

 static int ps2link_get_flag(int *flag) { int value;
  pthread_mutex_lock(&ps2link_lock);
  value = *flag;
  pthread_mutex_unlock(&ps2link_lock);
  return value;
 }

 static void ps2link_set_flag(int *flag) {
  pthread_mutex_lock(&ps2link_lock);
  *flag = 1;
  pthread_mutex_unlock(&ps2link_lock);
 }

 // Check a complete console line for the marker. Call with ps2link_lock held.
 static void ps2link_check_line(void) { char *found, *status;

  ps2link_line[ps2link_line_length] = 0;
  if (ps2link_marker_seen || (found = strstr(ps2link_line, ps2link_marker)) == NULL) { return; }

  // The status is the number after "status=", if there is one.
  status = strstr(found, "status=");
  ps2link_marker_status = status ? (int)strtol(status + 7, NULL, 10) : 0;
  ps2link_marker_seen = 1;

 }

 // Scan console output for the marker, line by line.
 static void ps2link_scan_console(char *buffer, int size) { int loop0;

  if (ps2link_marker == NULL) { return; }

  pthread_mutex_lock(&ps2link_lock);
  for (loop0=0; loop0<size; loop0++) {
   char c = buffer[loop0];

   if (c == '\n') { ps2link_check_line(); ps2link_line_length = 0; continue; }
   if (c == '\r' || c == 0) { continue; }

   // A very long line: keep its end, where a marker can still start.
   if (ps2link_line_length == sizeof(ps2link_line) - 1) {
    int keep = 256;
    memmove(ps2link_line, &ps2link_line[ps2link_line_length - keep], keep);
    ps2link_line_length = keep;
   }

   ps2link_line[ps2link_line_length++] = c;
  }
  pthread_mutex_unlock(&ps2link_lock);

 }

 ///////////////////////
 // PS2LINK FUNCTIONS //
 ///////////////////////

 void ps2link_set_marker(char *marker) {

  ps2link_marker = marker;

 }

 int ps2link_connect(char *hostname, int what, int timeout_ms) {

  // Listen on the console port. Only one client can, and a second one
  // would silently get no output, so this is an error.
  if (what & PS2LINK_CONNECT_CONSOLE) {
   console_socket = network_listen(0x4712, SOCK_DGRAM);
   if (console_socket < 0) {
    fprintf(stderr, "Error: Could not listen on the console port (UDP 0x4712). Is another ps2client running?\n");
    return PS2CLIENT_EXIT_PORTBUSY;
   }
  }

  // Connect to the request port.
  if (what & PS2LINK_CONNECT_REQUEST) {
   request_socket = network_connect_timeout(hostname, 0x4711, SOCK_STREAM, timeout_ms);
   if (request_socket < 0) {
    fprintf(stderr, "Error: Could not connect to the ps2link server. (%s)\n", hostname);
    return PS2CLIENT_EXIT_NOCONNECT;
   }

   // Disable Nagle algorithm: without TCP_NODELAY, the two-part response
   // (small header then data) interacts with PS2's delayed-ACK and causes
   // ~200 ms stall per read() syscall, making fread() ~400x slower than read().
   int one = 1;
   setsockopt(request_socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof(one));
  }

  // The console-only command does not need an outgoing command socket.
  if (what & PS2LINK_CONNECT_COMMAND) {
   command_socket = network_connect(hostname, 0x4712, SOCK_DGRAM);
   if (command_socket < 0) {
    fprintf(stderr, "Error: Could not open the command port. (%s)\n", hostname);
    return PS2CLIENT_EXIT_NOCONNECT;
   }
  }

  // Create the threads.
  ps2link_set_activity();
  if (console_socket >= 0) { pthread_create(&console_thread_id, NULL, ps2link_thread_console, NULL); }
  if (request_socket >= 0) { pthread_create(&request_thread_id, NULL, ps2link_thread_request, NULL); }

  // End function.
  return 0;

 }

 int ps2link_handshake(int timeout_ms) {
  long long start = time_ms(), next = start;

  // Repeat the command if it gets no answer, but slowly: when the EE side of
  // ps2link is not running (e.g. right after a program exits), the IOP side
  // cannot pass commands on, and a flood of them can keep the ps2 from
  // coming back.
  while (!ps2link_get_flag(&ps2link_ping_answered)) {
   long long now = time_ms();
   if (now - start >= timeout_ms || ps2link_get_flag(&ps2link_lost)) { return -1; }
   if (now >= next) { ps2link_command_dumpreg(PS2LINK_PING_REGS, PS2LINK_PING_PATHNAME); next = now + 1000; }
   sleep_ms(5);
  }

  // Return the round trip time.
  return (int)(time_ms() - start);

 }

 int ps2link_mainloop(int timeout) { int status = PS2CLIENT_EXIT_OK;

  // Disconnect from the command port.
  if (command_socket >= 0) { network_disconnect(command_socket); command_socket = -1; }

  // If no timeout was given, timeout immediately.
  if (timeout == 0) { return PS2CLIENT_EXIT_OK; }

  // Wait for the marker, a lost connection, or the idle timeout.
  ps2link_set_activity();
  for (;;) { long long activity; int lost;

   pthread_mutex_lock(&ps2link_lock);
   if (ps2link_marker_seen) { status = ps2link_marker_status & 0xff; pthread_mutex_unlock(&ps2link_lock); break; }
   activity = ps2link_activity; lost = ps2link_lost;
   pthread_mutex_unlock(&ps2link_lock);

   // ps2link closed the connection (the ps2 was reset, or the program
   // rebooted it). Give the last console output a moment to come in.
   if (lost) {
    sleep_ms(500);
    pthread_mutex_lock(&ps2link_lock);
    if (ps2link_marker != NULL && !ps2link_marker_seen && ps2link_line_length > 0) { ps2link_check_line(); }
    status = ps2link_marker_seen ? (ps2link_marker_status & 0xff) : PS2CLIENT_EXIT_CONNLOST;
    pthread_mutex_unlock(&ps2link_lock);
    if (status == PS2CLIENT_EXIT_CONNLOST) { fprintf(stderr, "ps2client: ps2link closed the connection.\n"); }
    break;
   }

   // Idle timeout.
   if (timeout > 0 && time_ms() - activity >= (long long)timeout * 1000) {
    pthread_mutex_lock(&ps2link_lock);
    if (ps2link_marker != NULL && !ps2link_marker_seen && ps2link_line_length > 0) { ps2link_check_line(); }
    if (ps2link_marker_seen) { status = ps2link_marker_status & 0xff; }
    else if (ps2link_marker != NULL) { status = PS2CLIENT_EXIT_TIMEOUT; }
    pthread_mutex_unlock(&ps2link_lock);
    if (status == PS2CLIENT_EXIT_TIMEOUT) { fprintf(stderr, "ps2client: no output for %d seconds and no marker; giving up.\n", timeout); }
    break;
   }

   sleep_ms(20);
  }

  // End function.
  return status;

 }

 int ps2link_disconnect(void) {
  // Kill created threads.
  if (request_socket >= 0) { pthread_cancel(request_thread_id); }
  if (console_socket >= 0) { pthread_cancel(console_thread_id); }

  // Disconnect from the command, request and console ports.
  if (command_socket >= 0) { network_disconnect(command_socket); command_socket = -1; }
  if (request_socket >= 0) { network_disconnect(request_socket); request_socket = -1; }
  if (console_socket >= 0) { network_disconnect(console_socket); console_socket = -1; }

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK COMMAND FUNCTIONS //
 ///////////////////////////////

 int ps2link_command_reset(void) {
  struct { unsigned int number; unsigned short length; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_RESET);
  command.length = htons(sizeof(command));

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_execiop(int argc, char **argv) {
  struct { unsigned int number; unsigned short length; int argc; char argv[256]; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_EXECIOP);
  command.length = htons(sizeof(command));
  command.argc   = htonl(argc);
  if (fix_argv(command.argv, sizeof(command.argv), argv) < 0) { return -1; }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_execee(int argc, char **argv) {
  struct { unsigned int number; unsigned short length; int argc; char argv[256]; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_EXECEE);
  command.length = htons(sizeof(command));
  command.argc   = htonl(argc);
  if (fix_argv(command.argv, sizeof(command.argv), argv) < 0) { return -1; }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_poweroff(void) {
  struct { unsigned int number; unsigned short length; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_POWEROFF);
  command.length = htons(sizeof(command));

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_scrdump(void) {
  struct { unsigned int number; unsigned short length; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_SCRDUMP);
  command.length = htons(sizeof(command));

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_netdump(void) {
  struct { unsigned int number; unsigned short length; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_NETDUMP);
  command.length = htons(sizeof(command));

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_dumpmem(unsigned int offset, unsigned int size, char *pathname) {
  struct { unsigned int number; unsigned short length; unsigned int offset; unsigned int size; char pathname[256]; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_DUMPMEM);
  command.length = htons(sizeof(command));
  command.offset = htonl(offset);
  command.size   = htonl(size);
  if (pathname) { snprintf(command.pathname, sizeof(command.pathname), "%s", pathname); }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_startvu(int vu) {
  struct { unsigned int number; unsigned short length; int vu; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_STARTVU);
  command.length = htons(sizeof(command));
  command.vu     = htonl(vu);

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_stopvu(int vu) {
  struct { unsigned int number; unsigned short length; int vu; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_STOPVU);
  command.length = htons(sizeof(command));
  command.vu     = htonl(vu);

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_dumpreg(int type, char *pathname) {
  struct { unsigned int number; unsigned short length; int type; char pathname[256]; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_DUMPREG);
  command.length = htons(sizeof(command));
  command.type   = htonl(type);
  if (pathname) { snprintf(command.pathname, sizeof(command.pathname), "%s", pathname); }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_gsexec(unsigned short size, char *pathname) {
  struct { unsigned int number; unsigned short length; unsigned short size; char pathname[256]; } PACKED command;

  // Build the command packet..
  command.number = htonl(PS2LINK_COMMAND_GSEXEC);
  command.length = htons(sizeof(command));
  command.size   = htons(size);
  if (pathname) { snprintf(command.pathname, sizeof(command.pathname), "%s", pathname); }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_writemem(unsigned int offset, unsigned int size, char *pathname) {
  struct { unsigned int number; unsigned short length; unsigned int offset; unsigned int size; char pathname[256]; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_WRITEMEM);
  command.length = htons(sizeof(command));
  command.offset = htonl(offset);
  command.size   = htonl(size);
  if (pathname) { snprintf(command.pathname, sizeof(command.pathname), "%s", pathname); }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_iopexcep(void) {
  struct { unsigned int number; unsigned short length; } PACKED command;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_IOPEXCEP);
  command.length = htons(sizeof(command));

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_request_open(void *packet) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED *request = packet;
  int result = -1;
  struct stat stats;

  // Fix the arguments.
  fix_pathname(request->pathname);
  request->flags = fix_flags(ntohl(request->flags));

  // The handshake's register dump goes nowhere.
  if (strcmp(request->pathname, PS2LINK_PING_PATHNAME) == 0) {
   result = ps2link_ping_fd = open(PS2LINK_NULL_DEVICE, O_WRONLY);
   return ps2link_response_open(result);
  }

  if(((stat(request->pathname, &stats) == 0) && (!S_ISDIR(stats.st_mode))) || (request->flags & O_CREAT))
  {
  // Perform the request.
#if defined (__CYGWIN__) || defined (__MINGW32__)
    result = open(request->pathname, request->flags | O_BINARY, 0644);
#else
    result = open(request->pathname, request->flags, 0644);
#endif
  }

  // Send the response.
  return ps2link_response_open(result);

 }

 int ps2link_request_close(void *packet) {
  struct { unsigned int number; unsigned short length; int fd; } PACKED *request = packet;
  int result = -1;

  // Perform the request.
  result = close(ntohl(request->fd));

  // The handshake's register dump is complete.
  if (ps2link_ping_fd >= 0 && (int)ntohl(request->fd) == ps2link_ping_fd) {
   ps2link_ping_fd = -1;
   ps2link_set_flag(&ps2link_ping_answered);
  }

  // Send the response.
  return ps2link_response_close(result);

 }

 int ps2link_request_read(void *packet) {
  struct { unsigned int number; unsigned short length; int fd; int size; } PACKED *request = packet;
  int result = -1, size = -1; char buffer[65536];

  // If a big read is requested...
  if (ntohl(request->size) > sizeof(buffer)) {

   // Allocate the bigbuffer.
   char *bigbuffer = malloc(ntohl(request->size));
   if (bigbuffer == NULL) { ps2link_response_read(-1, -1); return -1; }

   // Perform the request.
   result = size = read(ntohl(request->fd), bigbuffer, ntohl(request->size));

   // Send the response.
   ps2link_response_read(result, size);

   // Send the response data.
   network_send(request_socket, bigbuffer, size);

   // Free the bigbuffer.
   free(bigbuffer);

  // Else, a normal read is requested...
  } else {

   // Perform the request.
   result = size = read(ntohl(request->fd), buffer, ntohl(request->size));

   // Send the response.
   ps2link_response_read(result, size);

   // Send the response data.
   network_send(request_socket, buffer, size);

  }

  // End function.
  return 0;

 }

 int ps2link_request_write(void *packet) {
  struct { unsigned int number; unsigned short length; int fd; int size; } PACKED *request = packet;
  int result = -1; char buffer[65536];

  // If a big write is requested...
  if (ntohl(request->size) > sizeof(buffer)) {

   // Allocate the bigbuffer.
   char *bigbuffer = malloc(ntohl(request->size));
   if (bigbuffer == NULL) { return -1; }

   // Read the request data.
   if (network_receive_all(request_socket, bigbuffer, ntohl(request->size)) < 0) { free(bigbuffer); return -1; }

   // Perform the request.
   result = write(ntohl(request->fd), bigbuffer, ntohl(request->size));

   // Send the response.
   ps2link_response_write(result);

   // Free the bigbuffer.
   free(bigbuffer);

  // Else, a normal write is requested...
  } else {

   // Read the request data.
   if (network_receive_all(request_socket, buffer, ntohl(request->size)) < 0) { return -1; }

   // Perform the request.
   result = write(ntohl(request->fd), buffer, ntohl(request->size));

   // Send the response.
   ps2link_response_write(result);

  }

  // End function.
  return 0;

 }

 int ps2link_request_lseek(void *packet) {
  struct { unsigned int number; unsigned short length; int fd, offset, whence; } PACKED *request = packet;
  int result = -1;

  // Perform the request.
  result = lseek(ntohl(request->fd), ntohl(request->offset), ntohl(request->whence));

  // Send the response.
  return ps2link_response_lseek(result);

 }

 int ps2link_request_opendir(void *packet) {
  struct { unsigned int command; unsigned short length; int flags; char pathname[256]; } PACKED *request = packet;
  int result = -1;
  struct stat stats;

  // Fix the arguments.
  fix_pathname(request->pathname);

  if((stat(request->pathname, &stats) == 0) && (S_ISDIR(stats.st_mode)))
  {
      // Allocate an available directory descriptor.
      for (int loop0=0; loop0<10; loop0++) { if (ps2link_dd[loop0].dir == NULL) { result = loop0; break; } }

      // Perform the request.
      if (result != -1)
      {
        ps2link_dd[result].pathname = (char *) malloc(strlen(request->pathname) + 1);
        strcpy(ps2link_dd[result].pathname, request->pathname);
        ps2link_dd[result].dir = opendir(request->pathname);
      }
  }

  // Send the response.
  return ps2link_response_opendir(result);
}

 int ps2link_request_closedir(void *packet) {
  struct { unsigned int number; unsigned short length; int dd; } PACKED *request = packet;
  int result = -1;

  // Check the directory descriptor.
  if (ntohl(request->dd) >= 10 || ps2link_dd[ntohl(request->dd)].dir == NULL) { return ps2link_response_closedir(-1); }

  // Perform the request.
  result = closedir(ps2link_dd[ntohl(request->dd)].dir);

  if(ps2link_dd[ntohl(request->dd)].pathname)
  {
    free(ps2link_dd[ntohl(request->dd)].pathname);
    ps2link_dd[ntohl(request->dd)].pathname = NULL;
  }

  // Free the directory descriptor.
  ps2link_dd[ntohl(request->dd)].dir = NULL;

  // Send the response.
  return ps2link_response_closedir(result);

 }

 int ps2link_request_readdir(void *packet) {
    DIR *dir;
  struct { unsigned int number; unsigned short length; int dd; } PACKED *request = packet;
  struct dirent *dirent; struct stat stats; struct tm *loctime;
  unsigned int mode; unsigned char ctime[8]; unsigned char atime[8]; unsigned char mtime[8];
  char tname[512];

  // Check the directory descriptor.
  if (ntohl(request->dd) >= 10 || ps2link_dd[ntohl(request->dd)].dir == NULL) { return ps2link_response_readdir(-1, 0, 0, 0, NULL, NULL, NULL, 0, NULL); }

    dir = ps2link_dd[ntohl(request->dd)].dir;

  // Perform the request.
  dirent = readdir(dir);

  // If no more entries were found...
  if (dirent == NULL) {

   // Tell the user an entry wasn't found.
   return ps2link_response_readdir(0, 0, 0, 0, NULL, NULL, NULL, 0, NULL);

  }

  // need to specify the directory as well as file name otherwise uses CWD!
  sprintf(tname, "%s/%s", ps2link_dd[ntohl(request->dd)].pathname, dirent->d_name);

  // Fetch the entry's statistics.
  stat(tname, &stats);

  // Convert the mode.
  mode = (stats.st_mode & 0x07);
  if (S_ISDIR(stats.st_mode)) { mode |= 0x20; }
#ifndef _WIN32
  if (S_ISLNK(stats.st_mode)) { mode |= 0x08; }
#endif
  if (S_ISREG(stats.st_mode)) { mode |= 0x10; }

  // Convert the creation time.
  loctime = localtime(&(stats.st_ctime));
  ctime[6] = (unsigned char)loctime->tm_year;
  ctime[5] = (unsigned char)loctime->tm_mon + 1;
  ctime[4] = (unsigned char)loctime->tm_mday;
  ctime[3] = (unsigned char)loctime->tm_hour;
  ctime[2] = (unsigned char)loctime->tm_min;
  ctime[1] = (unsigned char)loctime->tm_sec;

  // Convert the access time.
  loctime = localtime(&(stats.st_atime));
  atime[6] = (unsigned char)loctime->tm_year;
  atime[5] = (unsigned char)loctime->tm_mon + 1;
  atime[4] = (unsigned char)loctime->tm_mday;
  atime[3] = (unsigned char)loctime->tm_hour;
  atime[2] = (unsigned char)loctime->tm_min;
  atime[1] = (unsigned char)loctime->tm_sec;

  // Convert the last modified time.
  loctime = localtime(&(stats.st_mtime));
  mtime[6] = (unsigned char)loctime->tm_year;
  mtime[5] = (unsigned char)loctime->tm_mon + 1;
  mtime[4] = (unsigned char)loctime->tm_mday;
  mtime[3] = (unsigned char)loctime->tm_hour;
  mtime[2] = (unsigned char)loctime->tm_min;
  mtime[1] = (unsigned char)loctime->tm_sec;

  // Send the response.
  return ps2link_response_readdir(1, mode, 0, stats.st_size, ctime, atime, mtime, 0, dirent->d_name);

 }

 int ps2link_request_remove(void *packet) {
  struct { unsigned int number; unsigned short length; char name[256]; } PACKED *request = packet;
  int result = -1;

  // Fix the arguments.
  fix_pathname(request->name);

  // Perform the request.
  result = remove(request->name);

  // Send the response.
  return ps2link_response_remove(result);
 }

 int ps2link_request_mkdir(void *packet) {
  struct { unsigned int number; unsigned short length; int mode; char name[256]; } PACKED *request = packet;
  int result = -1;

  // Fix the arguments.
  fix_pathname(request->name);
  // request->flags = fix_flags(ntohl(request->flags));

  // Perform the request.
  // do we need to use mode in here: request->mode ?

#ifdef _WIN32
  result = mkdir(request->name);
#else
  result = mkdir(request->name, request->mode);
#endif

  // Send the response.
  return ps2link_response_mkdir(result);
 }

 int ps2link_request_rmdir(void *packet) {
  struct { unsigned int number; unsigned short length; char name[256]; } PACKED *request = packet;
  int result = -1;

  // Fix the arguments.
  fix_pathname(request->name);

  // Perform the request.
  result = rmdir(request->name);

  // Send the response.
  return ps2link_response_rmdir(result);
 }

int ps2link_request_getstat(void *packet) {
  struct { unsigned int number; unsigned short length; char name[256]; } PACKED *request = packet;
    struct stat stats; struct tm *loctime;
    int ret;
    unsigned int mode = 0;
    unsigned char ctime[8] = { 0 }; unsigned char atime[8] = { 0 }; unsigned char mtime[8] = { 0 };

    // Fix the arguments.
    fix_pathname(request->name);

    // Fetch the entry's statistics.
    memset(&stats, 0, sizeof(stats));
    ret = stat(request->name, &stats);

    if (ret == 0) {
        // Convert the mode.
          mode = (stats.st_mode & 0x07);
          if (S_ISDIR(stats.st_mode)) { mode |= 0x20; }
        #ifndef _WIN32
          if (S_ISLNK(stats.st_mode)) { mode |= 0x08; }
        #endif
          if (S_ISREG(stats.st_mode)) { mode |= 0x10; }

          // Convert the creation time.
          loctime = localtime(&(stats.st_ctime));
          ctime[6] = (unsigned char)loctime->tm_year;
          ctime[5] = (unsigned char)loctime->tm_mon + 1;
          ctime[4] = (unsigned char)loctime->tm_mday;
          ctime[3] = (unsigned char)loctime->tm_hour;
          ctime[2] = (unsigned char)loctime->tm_min;
          ctime[1] = (unsigned char)loctime->tm_sec;

          // Convert the access time.
          loctime = localtime(&(stats.st_atime));
          atime[6] = (unsigned char)loctime->tm_year;
          atime[5] = (unsigned char)loctime->tm_mon + 1;
          atime[4] = (unsigned char)loctime->tm_mday;
          atime[3] = (unsigned char)loctime->tm_hour;
          atime[2] = (unsigned char)loctime->tm_min;
          atime[1] = (unsigned char)loctime->tm_sec;

          // Convert the last modified time.
          loctime = localtime(&(stats.st_mtime));
          mtime[6] = (unsigned char)loctime->tm_year;
          mtime[5] = (unsigned char)loctime->tm_mon + 1;
          mtime[4] = (unsigned char)loctime->tm_mday;
          mtime[3] = (unsigned char)loctime->tm_hour;
          mtime[2] = (unsigned char)loctime->tm_min;
          mtime[1] = (unsigned char)loctime->tm_sec;
    }

    return ps2link_response_getstat(ret, mode, 0, stats.st_size, ctime, atime, mtime, 0);
 }

 ////////////////////////////////
 // PS2LINK RESPONSE FUNCTIONS //
 ////////////////////////////////

 int ps2link_response_open(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_OPEN);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_close(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_CLOSE);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_read(int result, int size) {
  struct { unsigned int number; unsigned short length; int result; int size; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_READ);
  response.length = htons(sizeof(response));
  response.result = htonl(result);
  response.size   = htonl(size);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_write(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_WRITE);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_lseek(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_LSEEK);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_opendir(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_OPENDIR);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_closedir(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_CLOSEDIR);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_readdir(int result, unsigned int mode, unsigned int attr, unsigned int size, unsigned char *ctime, unsigned char *atime, unsigned char *mtime, unsigned int hisize, char *name) {
  struct { unsigned int number; unsigned short length; int result; unsigned int mode; unsigned int attr; unsigned int size; unsigned char ctime[8]; unsigned char atime[8]; unsigned char mtime[8]; unsigned int hisize; char name[256]; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_READDIR);
  response.length = htons(sizeof(response));
  response.result = htonl(result);
  response.mode   = htonl(mode);
  response.attr   = htonl(attr);
  response.size   = htonl(size);
  if (ctime) { memcpy(response.ctime, ctime, 8); }
  if (atime) { memcpy(response.atime, atime, 8); }
  if (mtime) { memcpy(response.mtime, mtime, 8); }
  response.hisize = htonl(hisize);
#ifdef _WIN32
  if (name) { sprintf(response.name, "%s", name); }
#else
  if (name) { snprintf(response.name, 256, "%s", name); }
#endif

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));

 }

 int ps2link_response_remove(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_REMOVE);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));
 }

 int ps2link_response_mkdir(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_MKDIR);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));
 }

 int ps2link_response_rmdir(int result) {
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_RMDIR);
  response.length = htons(sizeof(response));
  response.result = htonl(result);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));
 }

int ps2link_response_getstat(int result, unsigned int mode, unsigned int attr, unsigned int size, unsigned char *ctime, unsigned char *atime, unsigned char *mtime, unsigned int hisize) {
  struct { unsigned int number; unsigned short length; int result; unsigned int mode; unsigned int attr; unsigned int size; unsigned char ctime[8]; unsigned char atime[8]; unsigned char mtime[8]; unsigned int hisize; } PACKED response;

  // Build the response packet.
  response.number = htonl(PS2LINK_RESPONSE_GETSTAT);
  response.length = htons(sizeof(response));
  response.result = htonl(result);
  response.mode   = htonl(mode);
  response.attr   = htonl(attr);
  response.size   = htonl(size);
  if (ctime) { memcpy(response.ctime, ctime, 8); }
  if (atime) { memcpy(response.atime, atime, 8); }
  if (mtime) { memcpy(response.mtime, mtime, 8); }
  response.hisize = htonl(hisize);

  // Send the response packet.
  return network_send(request_socket, &response, sizeof(response));
 }

 //////////////////////////////
 // PS2LINK THREAD FUNCTIONS //
 //////////////////////////////

 void *ps2link_thread_console(void *userdata) {
  // As big as a UDP packet can be: recv cuts off what does not fit.
  static char buffer[65536];

  (void)userdata;
  // If the socket isn't open, this thread isn't needed.
  if (console_socket < 0) { return NULL; }

  // Loop forever...
  for (;;) { int size;

   // Wait for network activity.
   network_wait(console_socket, -1);

   // Receive the console buffer.
   size = network_receive(console_socket, buffer, sizeof(buffer));
   if (size < 0) { sleep_ms(10); continue; }

   // Print out exactly what came in: a packet is not null-terminated.
   fwrite(buffer, 1, size, stdout);

   // Look for the marker.
   ps2link_scan_console(buffer, size);

   // Reset the timeout counter.
   ps2link_set_activity();

  }

  // End function.
  return NULL;

 }

 void *ps2link_thread_request(void *userdata) {
  struct { unsigned int number; unsigned short length; char buffer[512]; } PACKED packet;

  (void)userdata;
  // If the socket isn't open, this thread isn't needed.
  if (request_socket < 0) { return NULL; }

  // Loop forever...
  for (;;) { int length;

   // Wait for network activity.
   network_wait(request_socket, -1);

   // Read in the request packet header. Failing means ps2link closed the
   // connection (reset or reboot) or the connection broke.
   if (network_receive_all(request_socket, &packet, 6) < 0) { break; }

   // Read in the rest of the packet.
   length = ntohs(packet.length);
   if (length < 6 || length - 6 > (int)sizeof(packet.buffer)) {
    fprintf(stderr, "Error: Bad request packet from ps2link (0x%08X, length %d).\n", ntohl(packet.number), length);
    break;
   }
   if (network_receive_all(request_socket, packet.buffer, length - 6) < 0) { break; }

   // Perform the requested action.
   if (ntohl(packet.number) == PS2LINK_REQUEST_OPEN)     { ps2link_request_open(&packet);     } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_CLOSE)    { ps2link_request_close(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_READ)     { ps2link_request_read(&packet);     } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_WRITE)    { ps2link_request_write(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_LSEEK)    { ps2link_request_lseek(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_OPENDIR)  { ps2link_request_opendir(&packet);  } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_CLOSEDIR) { ps2link_request_closedir(&packet); } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_READDIR)  { ps2link_request_readdir(&packet);  } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_REMOVE)   { ps2link_request_remove(&packet);   } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_MKDIR)    { ps2link_request_mkdir(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_RMDIR)    { ps2link_request_rmdir(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_GETSTAT)  { ps2link_request_getstat(&packet);  }

   // Reset the timeout counter.
   ps2link_set_activity();

  }

  // Tell the main loop the connection is gone.
  ps2link_set_flag(&ps2link_lost);

  // End function.
  return NULL;

 }
