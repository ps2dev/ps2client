
 #include <time.h>
 #include <stdio.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <dirent.h>
 #include <sys/stat.h>
 #include <netinet/in.h>
 #include "network.h"
 #include "ps2link.h"

 char buffer[65536];

 int command_sock, request_sock, textlog_sock;

 ///////////////////////
 // PS2LINK FUNCTIONS //
 ///////////////////////

 int ps2link_mainloop(char *hostname) {
  struct { int number; short length; char data[65544]; } __attribute__((packed)) request;

  // Open the connections.
  request_sock = network_connect(hostname, 0x4711, SOCKET_TCP);
  textlog_sock = network_listen(0x4712, SOCKET_UDP);

  // Main loop.
  while (1) {

   // Wait until something happens.
   network_wait(100000);

   // Read and perform any requests as needed.
   if (network_recv(request_sock, &request, sizeof(request)) > 0) {
    if (ntohl(request.number) == 0xBABE0111) { ps2link_request_open   ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0121) { ps2link_request_close  ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0131) { ps2link_request_read   ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0141) { ps2link_request_write  ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0151) { ps2link_request_lseek  ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0161) { ps2link_request_dopen  ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0171) { ps2link_request_dclose ((void *)&request); } else
    if (ntohl(request.number) == 0xBABE0181) { ps2link_request_dread  ((void *)&request); }
   }

   // Read and display any textlog information.
   while (network_recvfrom(textlog_sock, buffer, sizeof(buffer)) > 0) { printf(buffer); }

  }

  // Close the connections.
  network_close(request_sock);
  network_close(textlog_sock);

  // End function.
  return 0;

 }

 int ps2link_listenloop(void) {

  // Open the connection.
  textlog_sock = network_listen(0x4712, SOCKET_UDP);

  // Main loop.
  while(1) {

   // Wait until something happens.
   network_wait(100000);

   // Read and display any textlog information.
   while (network_recvfrom(textlog_sock, buffer, sizeof(buffer)) > 0) { printf(buffer); }

  }

  // Close the connection.
  network_close(textlog_sock);

  // End function.
  return 0;

 }

 int ps2link_fixflags(int flags) { int retval = 0;

  // Convert the read and write flags.
  if ((flags & 0x03) == 0x0001) { retval |= O_RDONLY; }
  if ((flags & 0x03) == 0x0002) { retval |= O_WRONLY; }
  if ((flags & 0x03) == 0x0003) { retval |= O_RDWR;   }

  // Convert the other flags.
  if (flags & 0x0010) { retval |= O_NONBLOCK; }
  if (flags & 0x0100) { retval |= O_APPEND;   }
  if (flags & 0x0200) { retval |= O_CREAT;    }
  if (flags & 0x0400) { retval |= O_TRUNC;    }

  // End function.
  return retval;

 }

 int ps2link_fixpathname(char *pathname) { int loop0;

  // If empty, set a pathname default.
  if (pathname[0] == 0) { strcpy(pathname, "."); }

  // Convert \ to / for unix compatibility.
  for(loop0=0;loop0<strlen(pathname);loop0++) { if (pathname[loop0] == '\\') { pathname[loop0] = '/'; } }

  // Check for a leading slash.
  if (pathname[0] == '/') {

   // Kill off the leading slash.
   for(loop0=0;loop0<strlen(pathname)-1;loop0++) { pathname[loop0] = pathname[loop0+1]; }

   // Terminate the pathname with a null.
   pathname[loop0] = 0;

  }

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK COMMAND FUNCTIONS //
 ///////////////////////////////

 int ps2link_send_command(char *hostname, void *packet, int size) {

  // Open the connection.
  command_sock = network_connect(hostname, 0x4712, SOCKET_UDP);

  // Send the command packet.
  network_send(command_sock, packet, size);

  // Close the connection.
  network_close(command_sock);

  // End function.
  return 0;

 }

 int ps2link_command_reset(char *hostname) {
  struct { int number; short length; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending reset() command...\n");

  // Build the command packet.
  command.number = htonl(0xBABE0201);
  command.length = htons(sizeof(command));

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_execiop(char *hostname, char *pathname) {
  struct { int number; short length; int argc; char argv[256]; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending execiop(\"%s\") command...\n", pathname);

  // Build the command packet.
  command.number = htonl(0xBABE0202);
  command.length = htons(sizeof(command));
  command.argc   = htonl(1);
  if (pathname) { strncpy(command.argv, pathname, 256); }

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_execee(char *hostname, char *pathname) {
  struct { int number; short length; int argc; char argv[256]; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending execee(\"%s\") command...\n", pathname);

  // Build the command packet.
  command.number = htonl(0xBABE0203);
  command.length = htons(sizeof(command));
  command.argc   = htonl(1);
  if (pathname) { strncpy(command.argv, pathname, 256); }

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_poweroff(char *hostname) {
  struct { int number; short length; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending poweroff() command...\n");

  // Build the command packet.
  command.number = htonl(0xBABE0204);
  command.length = htons(sizeof(command));

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_dumpmem(char *hostname, char *pathname, int offset, int size) {
  struct { int number; short length; int offset, size; char pathname[256]; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending dumpmem(\"%s\", %d, %d) command...\n", pathname, offset, size);

  // Build the command packet.
  command.number = htonl(0xBABE0207);
  command.length = htons(sizeof(command));
  command.offset = offset;
  command.size   = htonl(size);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_startvu(char *hostname, int vu) {
  struct { int number; short length; int vu; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending startvu(%d) command...\n", vu);

  // Build the command packet.
  command.number = htonl(0xBABE0208);
  command.length = htons(sizeof(command));
  command.vu     = htonl(vu);

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_stopvu(char *hostname, int vu) {
  struct { int number; short length; int vu; } __attribute__((packed)) command;

  // Tell the user what we're doing.
  printf("[***] Sending stopvu(%d) command...\n", vu);

  // Build the command packet.
  command.number = htonl(0xBABE0209);
  command.length = htons(sizeof(command));
  command.vu     = htonl(vu);

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_dumpreg(char *hostname, char *pathname, int type) {
  struct { int number; short length; int type; char pathname[256]; } command;

  // Tell the user what we're doing.
  printf("[***] Sending dumpreg(\"%s\", %d) command...\n", pathname, type);

  // Build the command packet.
  command.number = htonl(0xBABE020A);
  command.length = htons(sizeof(command));
  command.type   = htonl(type);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 int ps2link_command_gsexec(char *hostname, char *pathname, int size) {
  struct { int number; short length; int size; char pathname[256]; } command;

  // Tell the user what we're doing.
  printf("[***] Sending gsexec(\"%s\", %d) command...\n", pathname, size);

  // Build the command packet.
  command.number = htonl(0xBABE020B);
  command.length = htons(sizeof(command));
  command.size   = htonl(size);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  ps2link_send_command(hostname, &command, sizeof(command));

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_request_open(void *packet) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Fix the requested pathname.
  ps2link_fixpathname(request->pathname);

  // Build the response packet.
  response.number = htonl(0xBABE0112);
  response.length = htons(sizeof(response));
  response.result = htonl(open(request->pathname, ps2link_fixflags(ntohl(request->flags)), 0644));

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] open(\"host0:%s\", %d) = %d\n", request->pathname, ps2link_fixflags(ntohl(request->flags)), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_close(void *packet) {
  struct { int number; short length; int fd; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the response packet.
  response.number = htonl(0xBABE0122);
  response.length = htons(sizeof(response));
  response.result = htonl(close(ntohl(request->fd)));

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] close(%d) = %d\n", ntohl(request->fd), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_read(void *packet) {
  struct { int number; short length; int fd, size; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result, size; } __attribute__((packed)) response;

  // Build the response packet.
  response.number = htonl(0xBABE0132);
  response.length = htons(sizeof(response));
  response.result = htonl(read(ntohl(request->fd), buffer, ntohl(request->size)));
  response.size   = response.result;

  // Send the response packet and read data.
  network_send(request_sock, &response, sizeof(response));
  network_send(request_sock, buffer, ntohl(response.result));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] read(%d, buffer, %d) = %d\n", ntohl(request->fd), ntohl(request->size), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_write(void *packet) {
  struct { int number; short length; int fd, size; char data[65536]; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the response packet.
  response.number = htonl(0xBABE0142);
  response.length = htons(sizeof(response));
  response.result = htonl(write(ntohl(request->fd), request->data, ntohl(request->size)));

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] write(%d, buffer, %d) = %d\n", ntohl(request->fd), ntohl(request->size), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_lseek(void *packet) {
  struct { int number; short length; int fd, offset, whence; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the response packet.
  response.number = htonl(0xBABE0152);
  response.length = htons(sizeof(response));
  response.result = htonl(lseek(ntohl(request->fd), ntohl(request->offset), ntohl(request->whence)));

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] lseek(%d, %d, %d) = %d\n", ntohl(request->fd), ntohl(request->offset), ntohl(request->whence), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_dopen(void *packet) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int dd; } __attribute__((packed)) response;

  // Fix the requested pathname.
  ps2link_fixpathname(request->pathname);

  // Build the response packet.
  response.number = htonl(0xBABE0162);
  response.length = htons(sizeof(response));
  response.dd     = htonl((int)opendir(request->pathname));

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] dopen(\"host0:%s\") = %d\n", request->pathname, ntohl(response.dd));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_dread(void *packet) {
  struct { int number; short length; int dd; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result, mode, attr, size; char ctime[8], atime[8], mtime[8]; int hisize; char name[256]; } __attribute__((packed)) response;
  struct dirent *direptr; struct stat stats; struct tm loctime;

  // Build the response packet.
  response.number = htonl(0xBABE0182);
  response.length = htons(sizeof(response));
  response.result = htonl((int)direptr = readdir((DIR *)ntohl(request->dd)));

  // If an entry was returned, add the entry information.
  if (direptr != 0) {

   // Fetch the stats for the entry.
   stat(direptr->d_name, &stats);

   // Convert and add the mode.
   response.mode = (stats.st_mode & 0x07);
   if (S_ISDIR(stats.st_mode)) { response.mode |= 0x20; }
   if (S_ISLNK(stats.st_mode)) { response.mode |= 0x08; }
   if (S_ISREG(stats.st_mode)) { response.mode |= 0x10; }
   response.mode = htonl(response.mode);

   // Add the attributes.
   response.attr = htonl(0);

   // Add the size.
   response.size = htonl(stats.st_size);

   // Convert and add the creation time.
   if (localtime_r(&(stats.st_ctime), &loctime)) {
     response.ctime[6] = (char)loctime.tm_year;
     response.ctime[5] = (char)loctime.tm_mon + 1;
     response.ctime[4] = (char)loctime.tm_mday;
     response.ctime[3] = (char)loctime.tm_hour;
     response.ctime[2] = (char)loctime.tm_min;
     response.ctime[1] = (char)loctime.tm_sec;
   }

   // Convert and add the access time.
   if (localtime_r(&(stats.st_atime), &loctime)) {
     response.atime[6] = (char)loctime.tm_year;
     response.atime[5] = (char)loctime.tm_mon + 1;
     response.atime[4] = (char)loctime.tm_mday;
     response.atime[3] = (char)loctime.tm_hour;
     response.atime[2] = (char)loctime.tm_min;
     response.atime[1] = (char)loctime.tm_sec;
   }

   // Convert and add the modification time.
   if (localtime_r(&(stats.st_mtime), &loctime)) {
     response.mtime[6] = (char)loctime.tm_year;
     response.mtime[5] = (char)loctime.tm_mon + 1;
     response.mtime[4] = (char)loctime.tm_mday;
     response.mtime[3] = (char)loctime.tm_hour;
     response.mtime[2] = (char)loctime.tm_min;
     response.mtime[1] = (char)loctime.tm_sec;
   }

   // Add the hsize. (what is this?)
   response.hisize = htonl(0);

   // Add the name.
   strncpy(response.name, direptr->d_name, 256);

  }

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] dread(%d) = %d\n", ntohl(request->dd), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_dclose(void *packet) {
  struct { int number; short length; int dd; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the response packet.
  response.number = htonl(0xBABE0172);
  response.length = htons(sizeof(response));
  response.result = htonl(closedir((DIR *)ntohl(request->dd)));

  // Send the response packet.
  network_send(request_sock, &response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] dclose(%d) = %d\n", ntohl(request->dd), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK TEXTLOG FUNCTIONS //
 ///////////////////////////////

 int ps2link_read_textlog(void *packet, int size) {

  // Read the textlog packet.
  return network_recvfrom(textlog_sock, packet, size);

 }

