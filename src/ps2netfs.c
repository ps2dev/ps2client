
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <netinet/in.h>
 #include "network.h"
 #include "ps2netfs.h"

 int request_sock;

 ////////////////////////
 // PS2NETFS FUNCTIONS //
 ////////////////////////

 int ps2netfs_connect(char *hostname) {

  // Open the connection.
  request_sock = network_connect(hostname, 0x4713, SOCKET_TCP);

  // End function.
  return 0;

 }

 int ps2netfs_send(void *buffer, int size) {

  // Send some data.
  return network_send(request_sock, buffer, size);

 }

 int ps2netfs_recv(void *buffer, int size) {

  // Wait for data.
  network_wait(100000);

  // Receive the data.
  return network_recv(request_sock, buffer, size);

 }

 int ps2netfs_close(void) {

  // Close the connection.
  return network_close(request_sock);

 }

 ////////////////////////////////
 // PS2NETFS COMMAND FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_command_ps2dir(char *hostname, char *pathname) { int dd; DIRENT dirent;

  // Open the connection.
  ps2netfs_connect(hostname);

  // Open the directory.
  dd = ps2netfs_request_dopen(pathname, 0);

  // Read in the entries.
  while (ps2netfs_request_dread(dd, &dirent) > 0) {

   // Output the information.
   printf("#");

  }

  // Close the directory.
  ps2netfs_request_dclose(dd);

  // Close the connection.
  ps2netfs_close();

  // End function.
  return 0;

 }

 int ps2netfs_command_ps2devlist(char *hostname) { int result = 0;
  char devlist[256], *device = devlist; int loop0;

  // Tell the user about the operation.
  printf("[***] Requesting device list...\n");

  // Open the connection.
  ps2netfs_connect(hostname);

  // Get the device listing.
  result = ps2netfs_request_devlist(0, 0, devlist);

  // Output the device listing.
  printf("Available devices: ");
  for(loop0=0;loop0<result-1;loop0++) { printf("%s, ", device); device += strlen(device); }
  printf("%s\n", device);

  // Close the connection.
  ps2netfs_close();

  // Tell the user about the result.
  if (result < 0) { printf("[***] Error performing operation. (%d)\n", result); }
  else { printf("[***] Operation complete.\n"); }

  // End function.
  return 0;

 }

 int ps2netfs_command_ps2copyto(char *hostname, char *source, char *destination) { return 0; }

 int ps2netfs_command_ps2copyfrom(char *hostname, char *source, char *destination) { return 0; }

 int ps2netfs_command_ps2mount(char *hostname, char *pathname, char *device) { int result = 0;

  // Tell the user about the operation.
  printf("[***] Mounting device (%s) onto directory (%s).\n", device, pathname);

  // Open the connection.
  ps2netfs_connect(hostname);

  // Mount the device.
  result = ps2netfs_request_mount(pathname, device, MOUNT_READWRITE, "", 0);

  // Close the connection.
  ps2netfs_close();

  // Tell the user about the result.
  if (result < 0) { printf("[***] Error performing operation. (%d)\n", result); }
  else { printf("[***] Operation complete.\n"); }

  // End function.
  return result;

 }

 int ps2netfs_command_ps2umount(char *hostname, char *pathname) { return 0; }

 int ps2netfs_command_ps2mkdir(char *hostname, char *pathname) { int result = 0;

  // Tell the user about the operation.
  printf("[***] Creating new directory (%s).\n", pathname);

  // Open the connection.
  ps2netfs_connect(hostname);

  // Make the directory.
  result = ps2netfs_request_mkdir(pathname, 0);

  // Close the connection.
  ps2netfs_close();

  // Tell the user about the result.
  if (result < 0) { printf("[***] Error performing operation. (%d)\n", result); }
  else { printf("[***] Operation complete.\n"); }

  // End function.
  return 0;

 }

 int ps2netfs_command_ps2sync(char *hostname, char *device) { return 0; }

 ////////////////////////////////
 // PS2NETFS REQUEST FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_request_open(char *pathname, int flags) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8011);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strcpy(request.pathname, pathname); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] open(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_close(int fd) {
  struct { int number; short length; int fd; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8021);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] close(%d) = %d\n", fd, ntohl(response.result));

#endif

  // End function,
  return ntohl(response.result);

 }

 int ps2netfs_request_read(int fd, void *buffer, int size) {
  struct { int number; short length; int fd, size; } __attribute__((packed))  request;
  struct { int number; short length; int result, size; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8031);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);
  request.size   = htonl(size);

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response)); ps2netfs_recv(buffer, size);

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] read(%d, buffer, %d) = %d\n", fd, size, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_write(int fd, void *buffer, int size) {
  struct { int number; short length; int fd, size; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8041);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);
  request.size   = htonl(size);

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request)); ps2netfs_send(buffer, size);
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] write(%d, buffer, %d) = %d\n", fd, size, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_lseek(int fd, int offset, int whence) {
  struct { int number; short length; int fd, offset, whence; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8051);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);
  request.offset = htonl(offset);
  request.whence = htonl(whence);

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] lseek(%d, %d, %d) = %d\n", fd, offset, whence, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_mkdir(char *pathname, int flags) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8071);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] mkdir(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_dopen(char *pathname, int flags) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF80A1);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] dopen(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_dclose(int dd) {
  struct { int number; short length; int dd; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF80B1);
  request.length = htons(sizeof(request));
  request.dd     = htonl(dd);

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] dclose(%d) = %d\n", dd, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_dread(int dd, DIRENT *dirent) {
  struct { int number; short length; int dd; } __attribute__((packed)) request;
  struct { int number; short length; int result; DIRENT dirent; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF80C1);
  request.length = htons(sizeof(request));
  request.dd     = htonl(dd);

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

  // Copy over the directory entry.
  memcpy(dirent, &response.dirent, sizeof(DIRENT));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] dread(%d, dirent) = %d\n", dd, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_sync(char *device, int flags) {
  struct { int number; short length; int flags; char device[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8131);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (device) { strncpy(request.device, device, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] sync(\"%s\", %d) = %d\n", device, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_mount(char *pathname, char *device, int flags, char *argv, int argc) {
  struct { int number; short length; char pathname[256], device[256]; int flags; char argv[256]; int argc; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8141);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  request.argc   = htonl(argc);
  if (pathname) { strncpy(request.pathname, pathname, 256); }
  if (device) { strncpy(request.device, device, 256); }
  if (argv) { strncpy(request.argv, argv, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] mount(\"%s\", \"%s\", %d) = %d\n", pathname, device, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_umount(char *pathname, int flags) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8151);
  request.length = htons(sizeof(request));
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] umount(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_devlist(char *pathname, int flags, char *devlist) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; int count; char devlist[256]; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8F21);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

  // Copy over the device list.
  strncpy(devlist, response.devlist, 256);

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] devlist(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.count));

#endif

  // End function.
  return ntohl(response.count);

 }
