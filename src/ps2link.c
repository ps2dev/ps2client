
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

 int ps2link_connect(char *hostname) {

  // Connect to request.
  request_sock = network_connect(hostname, 0x4711, SOCKET_TCP);
  if (request_sock < 0) { printf("[ERROR] Connect to request failed. (%d)\n", request_sock); return -1; }

  // Connect to textlog.
  textlog_sock = network_listen(0x4712, SOCKET_UDP); usleep(5000);
  if (textlog_sock < 0) { printf("[ERROR] Connect to textlog failed. (%d)\n", textlog_sock); } // return -2;

  // End function.
  return 0;

 }

 int ps2link_mainloop(int timeout) {
  struct { int number; short length; char data[65544]; } __attribute__((packed)) request;

  // Main loop.
  while (network_wait(timeout) > 0) {

   // Read and perform any requests as needed.
   if (network_recv(request_sock, &request, sizeof(request)) > 0) {
    if (ntohl(request.number) == 0xBABE0111) { ps2link_request_open(&request);   } else
    if (ntohl(request.number) == 0xBABE0121) { ps2link_request_close(&request);  } else
    if (ntohl(request.number) == 0xBABE0131) { ps2link_request_read(&request);   } else
    if (ntohl(request.number) == 0xBABE0141) { ps2link_request_write(&request);  } else
    if (ntohl(request.number) == 0xBABE0151) { ps2link_request_lseek(&request);  } else
    if (ntohl(request.number) == 0xBABE0161) { ps2link_request_dopen(&request);  } else
    if (ntohl(request.number) == 0xBABE0171) { ps2link_request_dclose(&request); } else
    if (ntohl(request.number) == 0xBABE0181) { ps2link_request_dread(&request);  }
   }

   // Read and display any textlog information.
   if (network_recvfrom(textlog_sock, buffer, sizeof(buffer)) > 0) { fprintf(stdout, buffer); }

  }

  // End function.
  return 0;

 }

 int ps2link_disconnect(void) { int result = 0;

  // Disconnect from request.
  result = network_disconnect(request_sock);
  if (result < 0) { printf("[ERROR] Disconnect from request failed. (%d)\n", result); return -1; }

  // Disconnect from textlog.
  result = network_disconnect(textlog_sock);
  if (result < 0) { printf("[ERROR] Disconnect from textlog failed. (%d)\n", result); return -2; }

  // End function.
  return 0;

 }

 int ps2link_fixflags(int flags) { int result = 0;

  // Fix the flags.
  if (flags & 0x0001) { result |= O_RDONLY;   }
  if (flags & 0x0002) { result |= O_WRONLY;   }
  if (flags & 0x0010) { result |= O_NONBLOCK; }
  if (flags & 0x0100) { result |= O_APPEND;   }
  if (flags & 0x0200) { result |= O_CREAT;    }
  if (flags & 0x0400) { result |= O_TRUNC;    }

  // End function.
  return result;

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

 int ps2link_send_command(char *hostname, void *command, int size) { int result = 0;

  // Connect to the command port.
  command_sock = network_connect(hostname, 0x4712, SOCKET_UDP);
  if (command_sock < 0) { printf("[ERROR] Connect to command port. (%d)\n", result); return -1; }

  // Send the command.
  result = network_send(command_sock, command, size);
  if (result < 0) { printf("[ERROR] Send command failed. (%d)\n", result); return -2; }

  // Disconnect from the command port.
  result = network_disconnect(command_sock);
  if (result < 0) { printf("[ERROR] Disconnect from command port failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2link_command_reset(char *hostname) { int result = 0;
  struct { int number; short length; } __attribute__((packed)) command;

  // Build the reset command.
  command.number = htonl(0xBABE0201);
  command.length = htons(sizeof(command));

  // Send the reset command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send reset command failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int ps2link_command_execiop(char *hostname, int timeout, int argc, char *argv) { int result = 0;
  struct { int number; short length; int argc; char argv[256]; } __attribute__((packed)) command;

  // Build the execiop command.
  command.number = htonl(0xBABE0202);
  command.length = htons(sizeof(command));
  command.argc   = htonl(argc);
  if (argv) { memcpy(command.argv, argv, 256); }

  // Connect to ps2link.
  result = ps2link_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2link failed. (%d)\n", result); return -1; }

  // Send the execiop command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send execiop command failed. (%d)\n", result); return -2; }

  // Enter main loop.
  result = ps2link_mainloop(timeout);
  if (result < 0) { printf("[ERROR] Main loop failed. (%d)\n", result); return -3; }

  // Disconnect from ps2link.
  result = ps2link_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2link failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 int ps2link_command_execee(char *hostname, int timeout, int argc, char *argv) { int result = 0;
  struct { int number; short length; int argc; char argv[256]; } __attribute__((packed)) command;

  // Build the execee command.
  command.number = htonl(0xBABE0203);
  command.length = htons(sizeof(command));
  command.argc   = htonl(argc);
  if (argv) { memcpy(command.argv, argv, 256); }

  // Connect to ps2link.
  result = ps2link_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2link failed. (%d)\n", result); return -1; }

  // Send the execee command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send execee command failed. (%d)\n", result); return -2; }

  // Enter main loop.
  result = ps2link_mainloop(timeout);
  if (result < 0) { printf("[ERROR] Main loop failed. (%d)\n", result); return -3; }

  // Disconnect from ps2link.
  result = ps2link_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2link failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 int ps2link_command_poweroff(char *hostname) { int result = 0;
  struct { int number; short length; } __attribute__((packed)) command;

  // Build the poweroff command.
  command.number = htonl(0xBABE0204);
  command.length = htons(sizeof(command));

  // Send the poweroff command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send poweroff command failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int ps2link_command_dumpmem(char *hostname, int timeout, char *pathname, int offset, int size) { int result = 0;
  struct { int number; short length; int offset, size; char pathname[256]; } __attribute__((packed)) command;

  // Build the dumpmem command.
  command.number = htonl(0xBABE0207);
  command.length = htons(sizeof(command));
  command.offset = offset;
  command.size   = htonl(size);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Connect to ps2link.
  result = ps2link_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2link failed. (%d)\n", result); return -1; }

  // Send the dumpmem command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send dumpmem command failed. (%d)\n", result); return -2; }

  // Enter main loop.
  result = ps2link_mainloop(timeout);
  if (result < 0) { printf("[ERROR] Main loop failed. (%d)\n", result); return -3; }

  // Disconnect from ps2link.
  result = ps2link_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2link failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 int ps2link_command_startvu(char *hostname, int vu) { int result = 0;
  struct { int number; short length; int vu; } __attribute__((packed)) command;

  // Build the startvu command.
  command.number = htonl(0xBABE0208);
  command.length = htons(sizeof(command));
  command.vu     = htonl(vu);

  // Send the startvu command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send startvu command failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int ps2link_command_stopvu(char *hostname, int vu) { int result = 0;
  struct { int number; short length; int vu; } __attribute__((packed)) command;

  // Build the stopvu command.
  command.number = htonl(0xBABE0209);
  command.length = htons(sizeof(command));
  command.vu     = htonl(vu);

  // Send the stopvu command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send stopvu command failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int ps2link_command_dumpreg(char *hostname, int timeout, char *pathname, int type) { int result = 0;
  struct { int number; short length; int type; char pathname[256]; } command;

  // Build the dumpreg command.
  command.number = htonl(0xBABE020A);
  command.length = htons(sizeof(command));
  command.type   = htonl(type);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Connect to ps2link.
  result = ps2link_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2link failed. (%d)\n", result); return -1; }

  // Send the dumpreg command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send dumpreg command failed. (%d)\n", result); return -2; }

  // Enter main loop.
  result = ps2link_mainloop(timeout);
  if (result < 0) { printf("[ERROR] Main loop failed. (%d)\n", result); return -3; }

  // Disconnect from ps2link.
  result = ps2link_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2link failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 int ps2link_command_gsexec(char *hostname, int timeout, char *pathname) { int result = 0;
  struct { int number; short length; int size; char pathname[256]; } command;
  int fd, size;

  // Get the file size.
  fd = open(pathname, O_RDONLY); size = lseek(fd, 0, SEEK_END); close(fd);

  // Build the gsexec command.
  command.number = htonl(0xBABE020B);
  command.length = htons(sizeof(command));
  command.size   = htonl(size);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Connect to ps2link.
  result = ps2link_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2link failed. (%d)\n", result); return -1; }

  // Send the gsexec command.
  result = ps2link_send_command(hostname, &command, sizeof(command));
  if (result < 0) { printf("[ERROR] Send gsexec command failed. (%d)\n", result); return -2; }

  // Enter main loop.
  result = ps2link_mainloop(timeout);
  if (result < 0) { printf("[ERROR] Main loop failed. (%d)\n", result); return -3; }

  // Disconnect from ps2link.
  result = ps2link_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2link failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 int ps2link_command_listen(char *hostname, int timeout) { int result = 0;

  // Connect to ps2link.
  result = ps2link_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2link failed. (%d)\n", result); return -1; }

  // Enter main loop.
  result = ps2link_mainloop(timeout);
  if (result < 0) { printf("[ERROR] Main loop failed. (%d)\n", result); return -3; }

  // Disconnect from ps2link.
  result = ps2link_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2link failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_request_open(void *packet) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Fix the requested pathname.
  ps2link_fixpathname(request->pathname);

  // Build the open response.
  response.number = htonl(0xBABE0112);
  response.length = htons(sizeof(response));
  response.result = htonl(open(request->pathname, ps2link_fixflags(ntohl(request->flags)), 0644));

  // Send the open response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send open response failed. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] open(\"host0:%s\", %d) = %d\n", request->pathname, ps2link_fixflags(ntohl(request->flags)), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_close(void *packet) { int result = 0;
  struct { int number; short length; int fd; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the close response.
  response.number = htonl(0xBABE0122);
  response.length = htons(sizeof(response));
  response.result = htonl(close(ntohl(request->fd)));

  // Send the close response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send close response failed. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] close(%d) = %d\n", ntohl(request->fd), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_read(void *packet) { int result = 0;
  struct { int number; short length; int fd, size; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result, size; } __attribute__((packed)) response;

  // Build the read response.
  response.number = htonl(0xBABE0132);
  response.length = htons(sizeof(response));
  response.result = htonl(read(ntohl(request->fd), buffer, ntohl(request->size)));
  response.size   = response.result;

  // Send the read response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send read response failed. (%d)\n", result); return -1; }

  // Send the read response data.
  result = network_send(request_sock, buffer, ntohl(response.result));
  if (result < 0) { printf("[ERROR] Send read response data failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] read(%d, buffer, %d) = %d\n", ntohl(request->fd), ntohl(request->size), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_write(void *packet) { int result = 0;
  struct { int number; short length; int fd, size; char data[65536]; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the write response.
  response.number = htonl(0xBABE0142);
  response.length = htons(sizeof(response));
  response.result = htonl(write(ntohl(request->fd), request->data, ntohl(request->size)));

  // Send the write response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send write response failed. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] write(%d, buffer, %d) = %d\n", ntohl(request->fd), ntohl(request->size), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_lseek(void *packet) { int result = 0;
  struct { int number; short length; int fd, offset, whence; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the lseek response.
  response.number = htonl(0xBABE0152);
  response.length = htons(sizeof(response));
  response.result = htonl(lseek(ntohl(request->fd), ntohl(request->offset), ntohl(request->whence)));

  // Send the lseek response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send lseek response failed. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] lseek(%d, %d, %d) = %d\n", ntohl(request->fd), ntohl(request->offset), ntohl(request->whence), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_dopen(void *packet) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int dd; } __attribute__((packed)) response;

  // Fix the requested pathname.
  ps2link_fixpathname(request->pathname);

  // Build the dopen response.
  response.number = htonl(0xBABE0162);
  response.length = htons(sizeof(response));
  response.dd     = htonl((int)opendir(request->pathname));

  // Send the dopen response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send dopen response failed. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] dopen(\"host0:%s\") = %d\n", request->pathname, ntohl(response.dd));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_dread(void *packet) { int result = 0;
  struct { int number; short length; int dd; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result, mode, attr, size; char ctime[8], atime[8], mtime[8]; int hisize; char name[256]; } __attribute__((packed)) response;
  struct dirent *direptr; struct stat stats; struct tm loctime;

  // Build the dread response.
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

  // Send the dread response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send dread response packet. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] dread(%d) = %d\n", ntohl(request->dd), ntohl(response.result));

#endif

  // End function.
  return 0;

 }

 int ps2link_request_dclose(void *packet) { int result = 0;
  struct { int number; short length; int dd; } __attribute__((packed)) *request = packet;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the dclose response.
  response.number = htonl(0xBABE0172);
  response.length = htons(sizeof(response));
  response.result = htonl(closedir((DIR *)ntohl(request->dd)));

  // Send the dclose response.
  result = network_send(request_sock, &response, sizeof(response));
  if (result < 0) { printf("[ERROR] Send dclose response failed. (%d)\n", result); return -1; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] dclose(%d) = %d\n", ntohl(request->dd), ntohl(response.result));

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

