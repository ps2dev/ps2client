
 #include <stdio.h>
 #include <fcntl.h>
 #include <string.h>
 #include <dirent.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <netinet/in.h>

 #include "network.h"
 #include "ps2link.h"
 #include "utility.h"

 int console_socket = -1;
 int request_socket = -1;
 int command_socket = -1;

 pthread_t console_thread_id;
 pthread_t request_thread_id;

 int ps2link_counter = 0;

 #define PACKED	__attribute__((packed))

 ///////////////////////
 // PS2LINK FUNCTIONS //
 ///////////////////////

 int ps2link_connect(char *hostname) {

  // Connect to the console port.
  console_socket = network_listen(0x4712, SOCK_DGRAM); if (console_socket < 0) { return -1; }

  // Create the console thread.
  pthread_create(&console_thread_id, NULL, ps2link_thread_console, (void *)&console_thread_id);

  // Connect to the request port.
  request_socket = network_connect(hostname, 0x4711, SOCK_STREAM); if (request_socket < 0) { return -1; }

  // Create the request thread.
  pthread_create(&request_thread_id, NULL, ps2link_thread_request, (void *)&request_thread_id);

  // Connect to the command port.
  command_socket = network_connect(hostname, 0x4712, SOCK_DGRAM); if (command_socket < 0) { return -1; }

  // End function.
  return 0;

 }

 int ps2link_mainloop(int timeout) {

  // If no timeout was given, timeout immediately.
  if (timeout == 0) { return 0; }

  // If timeout was never, loop forever.
  if (timeout < 0) { for (;;) { sleep(600); } }

  // Increment the timeout counter until timeout is reached.
  while (ps2link_counter++ < timeout) { sleep(1); fprintf(stderr, "(%d)", ps2link_counter); }

  // End function.
  return 0;

 }

 int ps2link_disconnect(void) {

  // Disconnect from the command port.
  if (network_disconnect(command_socket) < 0) { return -1; }

  // Disconnect from the request port.
  if (network_disconnect(request_socket) < 0) { return -1; }

  // Disconnect from console port.
  if (network_disconnect(console_socket) < 0) { return -1; }

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
  fix_argv(command.argv, argv);

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_execee(int argc, char **argv) {
  struct { unsigned int number; unsigned short length; int argc; char argv[256]; } PACKED command;
  int loop0 = -1; char *temp = command.argv;

  // Build the command packet.
  command.number = htonl(PS2LINK_COMMAND_EXECEE);
  command.length = htons(sizeof(command));
  command.argc   = htonl(argc);
  fix_argv(command.argv, argv);

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
  if (pathname) { strncpy(command.pathname, pathname, 256); }

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
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_gsexec(unsigned short size, char *pathname) {
  struct { unsigned int number; unsigned short length; unsigned short size; char pathname[256]; } PACKED command;

  // Build the command packet..
  command.number = htonl(PS2LINK_COMMAND_GSEXEC);
  command.length = htons(sizeof(command));
  command.size   = htonl(size);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

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
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  return network_send(command_socket, &command, sizeof(command));

 }

 int ps2link_command_iopexcep(void) {

  // FIXME: Unimplemented.

  // End function.
  return 0;

 }

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_request_open(void *packet) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED *request = packet;
  int result = -1;

  // Fix the arguments.
  fix_pathname(request->pathname);
  request->flags = fix_flags(ntohl(request->flags));

  // Perform the request.
  result = open(request->pathname, request->flags, 0644);

  // Send the response.
  return ps2link_response_open(result);

 }

 int ps2link_request_close(void *packet) {
  struct { unsigned int number; unsigned short length; int fd; } PACKED *request = packet;
  int result = -1;

  // Perform the request.
  result = close(ntohl(request->fd));

  // Send the response.
  return ps2link_response_close(result);

 }

 int ps2link_request_read(void *packet) {
  struct { unsigned int number; unsigned short length; int fd; int size; } PACKED *request = packet;
  int result = -1, size = -1; char buffer[65536];

  // Perform the request.
  result = size = read(ntohl(request->fd), buffer, ntohl(request->size));

  // Send the response.
  ps2link_response_read(result, size);

  // Send the response data.
  return network_send(request_socket, buffer, size);

 }

 int ps2link_request_write(void *packet) {
  struct { unsigned int number; unsigned short length; int fd; int size; } PACKED *request = packet;
  int result = -1; int size = -1; char buffer[65536];

  // Read the request data.
  network_receive_all(request_socket, buffer, ntohl(request->size));

  // Perform the request.
  result = write(ntohl(request->fd), buffer, ntohl(request->size));

  // Send the response.
  return ps2link_response_write(result);

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

  // Fix the arguments.
  fix_pathname(request->pathname);

  // Perform the request.
  result = (int)opendir(request->pathname);

  // Send the response.
  return ps2link_response_opendir(result);

 }

 int ps2link_request_closedir(void *packet) {
  struct { unsigned int number; unsigned short length; int dd; } PACKED *request = packet;
  int result = -1;

  // Perform the request.
  result = closedir((DIR *)ntohl(request->dd));

  // Send the response.
  return ps2link_response_closedir(result);

 }

 int ps2link_request_readdir(void *packet) {

  // FIXME: Unimplemented.

  // End function.
  return 0;

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

 int ps2link_response_readdir(int result, unsigned int size, char *name) {

  // FIXME: Unimplemented.

  // End function.
  return 0;

 }

 //////////////////////////////
 // PS2LINK THREAD FUNCTIONS //
 //////////////////////////////

 void *ps2link_thread_console(void *thread_id) {
  char buffer[1024];

  for (;;) {

   // Wait for network activity.
   network_wait(console_socket, -1);

   // Receive the console buffer.
   network_receive(console_socket, buffer, sizeof(buffer));

   // Print out the console buffer.
   printf("%s", buffer);

   // Clear the console buffer.
   memset(buffer, 0, sizeof(buffer));

   // Reset the timeout counter.
   ps2link_counter = 0;

  }

 }

 void *ps2link_thread_request(void *thread_id) {
  struct { unsigned int number; unsigned short length; char buffer[512]; } PACKED packet;

  for (;;) {

   // Wait for network activity.
   network_wait(request_socket, -1);

   // Read in the request packet header.
   network_receive_all(request_socket, &packet, 6);

   // Read in the rest of the packet.
   network_receive_all(request_socket, packet.buffer, ntohs(packet.length) - 6);

   // Perform the requested action.
   if (ntohl(packet.number) == PS2LINK_REQUEST_OPEN)     { ps2link_request_open(&packet);     } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_CLOSE)    { ps2link_request_close(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_READ)     { ps2link_request_read(&packet);     } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_WRITE)    { ps2link_request_write(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_LSEEK)    { ps2link_request_lseek(&packet);    } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_OPENDIR)  { ps2link_request_opendir(&packet);  } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_CLOSEDIR) { ps2link_request_closedir(&packet); } else
   if (ntohl(packet.number) == PS2LINK_REQUEST_READDIR)  { ps2link_request_readdir(&packet);  }

   // Reset the timeout counter.
   ps2link_counter = 0;

  }

 }

