
 #include <stdio.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
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

  // Check the result.
  if (request_sock < 0) { return -1; }

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

 int ps2netfs_command_copyfrom(char *hostname, char *source, char *destination) {
  int fd0, fd1, size, total; char buffer[65536];

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Copying a file to remote...\n");

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Open the local file.
  fd0 = open(destination, O_RDWR | O_CREAT, 0644);

  // Open the remote file.
  fd1 = ps2netfs_request_open(source, OPEN_READ);

  // Get the file size.
  size = ps2netfs_request_lseek(fd1, 0, LSEEK_END); ps2netfs_request_lseek(fd1, 0, LSEEK_SET); total = size;

  // Output the display header.
  printf("\n [%s --> %s]\n\n  Progress: ", source, destination);

  // Repeat until the copy is finished.
  while (size > 0) { printf("#");

   // Read in the data.
   ps2netfs_request_read(fd1, buffer, sizeof(buffer));

   // Write out the data.
   if (size < sizeof(buffer)) { write(fd0, buffer, size); } else { write(fd0, buffer, sizeof(buffer)); }

   // Decrement the size left to copy.
   size -= sizeof(buffer);

  }

  // Output the display footer.
  printf("\n\n [%d Bytes Copied]\n\n", total);

  // Close the local file.
  close(fd0);

  // Close the remote file.
  ps2netfs_request_close(fd1);

  // Close the connection.
  ps2netfs_close();

  // End function.
  return 0;

 }

 int ps2netfs_command_copyto(char *hostname, char *source, char *destination) {
  int fd0, fd1, size, total; char buffer[65536];

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Copying a file from remote...\n");

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Open the local file.
  fd0 = open(source, O_RDONLY);

  // Open the remote file.
  fd1 = ps2netfs_request_open(destination, OPEN_WRITE | OPEN_CREATE);

  // Get the file size.
  size = lseek(fd0, 0, SEEK_END); lseek(fd0, 0, SEEK_SET); total = size;

  // Output the display header.
  printf("\n [%s --> %s]\n\n  Progress: ", source, destination);

  // Repeat until the copy is done.
  while (size > 0) { printf("#");

   // Read in the data.
   read(fd0, buffer, sizeof(buffer));

   // Write out the data.
   if (size < sizeof(buffer)) { ps2netfs_request_write(fd1, buffer, size); } else { ps2netfs_request_write(fd1, buffer, sizeof(buffer)); }

   // Decrement the size left to copy.
   size -= sizeof(buffer);

  }

  // Output the display footer.
  printf("\n\n [%d Bytes Copied]\n\n", total);

  // Close the local file.
  close(fd0);

  // Close the remote file.
  ps2netfs_request_close(fd1);

  // Close the connection.
  ps2netfs_close();

  // End function.
  return 0;

 }

 int ps2netfs_command_delete(char *hostname, char *pathname) { int result = 0;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Deleting old file (%s)... ", pathname);

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Mount the device.
  result = ps2netfs_request_delete(pathname, 0);

  // Close the connection.
  ps2netfs_close();

#ifdef __CHATTY__

  // Output the result.
  if (result < 0) { printf("Error!\n"); } else { printf("Done!\n"); }

#endif

  // End function.
  return 0;

 }

 int ps2netfs_command_devlist(char *hostname) {
  int loop0, devcount; char devlist[256], *temp = devlist;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Requesting a device listing...\n");

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Get the device listing.
  devcount = ps2netfs_request_devlist("", 0, devlist);

  // Output the display header.
  printf("\n [Active Devices]\n\n");

  // Output each available device.
  for(loop0=0;loop0<devcount;loop0++) {

   // Output the device description.
   if (!strcmp(temp, "rom"))   { printf("  rom    - Onboard rom device.\n");			} else
   if (!strcmp(temp, "cdrom")) { printf("  cdrom  - Standard cd/dvd device. (cdrom:)\n");	} else
   if (!strcmp(temp, "host"))  { printf("  host   - Host file system. (host:)\n");		} else
   if (!strcmp(temp, "mc"))    { printf("  mc     - Memory card driver. (mc0: mc1:)\n");	} else
   if (!strcmp(temp, "dev9x")) { printf("  dev9x  - Blah blah blah.\n");			} else { printf("  %s\n", temp); }

   // Increment temp.
   temp += strlen(temp) + 1;

  }

  // Output the display footer.
  printf("\n [%d Devices Found]\n\n", devcount);

  // Close the connection.
  ps2netfs_close();

  // End function.
  return 0;

 }

 int ps2netfs_command_dir(char *hostname, char *pathname) {
  int dd, files = 0, size = 0; DIRENT dirent;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Requesting a directory listing...\n");

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Open the directory.
  dd = ps2netfs_request_dopen(pathname, 0);

  // Output the display header.
  printf("\n [Contents of %s]\n\n", pathname);

  // Read in each of the entries.
  while (ps2netfs_request_dread(dd, &dirent) > 0) { printf("  ");

   // Output the mode information.
   if (ntohl(dirent.mode) & 0x4000) { printf("l"); } else
   if (ntohl(dirent.mode) & 0x2000) { printf("-"); } else
   if (ntohl(dirent.mode) & 0x1000) { printf("d"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0100) { printf("r"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0080) { printf("w"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0040) { printf("x"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0020) { printf("r"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0010) { printf("w"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0008) { printf("x"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0004) { printf("r"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0002) { printf("w"); } else { printf("-"); }
   if (ntohl(dirent.mode) & 0x0001) { printf("x"); } else { printf("-"); }

   // Output the file size.
   printf(" %10d", (int)ntohl(dirent.size));

   // Output the date.
   printf(" %02d-%02d-%04d", dirent.mtime[5], dirent.mtime[4], (2048 + dirent.mtime[6]));

   // Output the time.
   printf(" %02d:%02d:%02d", dirent.mtime[3], dirent.mtime[2], dirent.mtime[1]);

   // Output the name.
   printf(" %s\n", dirent.name);

   // Update the counters.
   files++; size += ntohl(dirent.size);

  }

  // Output the display footer.
  printf("\n [%d Files - %d Bytes]\n\n", files, size);

  // Close the directory.
  ps2netfs_request_dclose(dd);

  // Close the connection.
  ps2netfs_close();

  // End function.
  return 0;

 }

 // UNDEFINED - FINISH ME
 int ps2netfs_command_format(char *hostname, char *device) { return -1; }

 int ps2netfs_command_mkdir(char *hostname, char *pathname) { int result = 0;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Creating new directory (%s)... ", pathname);

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Make the directory.
  result = ps2netfs_request_mkdir(pathname, 0);

  // Close the connection.
  ps2netfs_close();

#ifdef __CHATTY__

  // Output the result.
  if (result < 0) { printf("Error!\n"); } else { printf("Done!\n"); }

  // Display the new directory.
  if (result >= 0) { usleep(1); ps2netfs_command_dir(hostname, pathname); }

#endif

  // End function.
  return 0;

 }

 int ps2netfs_command_mount(char *hostname, char *device, char *fsname) { int result = 0;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Mounting device (%s) as (%s)... ", device, fsname);

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Mount the device.
  result = ps2netfs_request_mount(device, fsname, MOUNT_READWRITE, "", 0);

  // Close the connection.
  ps2netfs_close();

#ifdef __CHATTY__

  // Output the result.
  if (result < 0) { printf("Error!\n"); } else { printf("Done!\n"); }

  // Display the new device.
  if (result >= 0) { char temp[256]; sprintf(temp, "%s/", device); usleep(1); ps2netfs_command_dir(hostname, temp); }

#endif

  // End function.
  return 0;

 }

 // UNDEFINED - FINISH ME
 int ps2netfs_command_rename(char *hostname, char *source, char *destination) { return -1; }

 int ps2netfs_command_rmdir(char *hostname, char *pathname) { int result = 0;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Removing old directory (%s)... ", pathname);

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Make the directory.
  result = ps2netfs_request_rmdir(pathname, 0);

  // Close the connection.
  ps2netfs_close();

#ifdef __CHATTY__

  // Output the result.
  if (result < 0) { printf("Error!\n"); } else { printf("Done!\n"); }

#endif

  // End function.
  return 0;

 }

 int ps2netfs_command_sync(char *hostname, char *device) { int result = 0;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Requesting a sync for device (%s)... ", device);

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Make the directory.
  result = ps2netfs_request_sync(device, 0);

  // Close the connection.
  ps2netfs_close();

#ifdef __CHATTY__

  // Output the result.
  if (result < 0) { printf("Error!\n"); } else { printf("Done!\n"); }

#endif

  // End function.
  return 0;

 }

 int ps2netfs_command_umount(char *hostname, char *device) { int result = 0;

#ifdef __CHATTY__

  // Tell the user about the command.
  printf("[***] Umounting device (%s)... ", device);

#endif

  // Open the connection.
  ps2netfs_connect(hostname);

  // Mount the device.
  result = ps2netfs_request_umount(device, 0);

  // Close the connection.
  ps2netfs_close();

#ifdef __CHATTY__

  // Output the result.
  if (result < 0) { printf("Error!\n"); } else { printf("Done!\n"); }

#endif

  // End function.
  return 0;

 }

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

 int ps2netfs_request_delete(char *pathname, int flags) {
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
  printf("[***] delete(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_mkdir(char *pathname, int flags) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8081);
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


 int ps2netfs_request_rmdir(char *pathname, int flags) {
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8091);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] rmdir(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

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

 // UNDEFINED - FINISH ME
 int ps2netfs_request_format(char *device, int flags) { return -1; }

 // UNDEFINED - FINISH ME
 int ps2netfs_request_rename(char *source, char *destination) { return -1; }

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

 int ps2netfs_request_mount(char *device, char *fsname, int flags, char *argv, int argc) {
  struct { int number; short length; char device[256], fsname[256]; int flags; char argv[256]; int argc; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8141);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  request.argc   = htonl(argc);
  if (device) { strncpy(request.device, device, 256); }
  if (fsname) { strncpy(request.fsname, fsname, 256); }
  if (argv) { strncpy(request.argv, argv, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] mount(\"%s\", \"%s\", %d) = %d\n", device, fsname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_umount(char *device, int flags) {
  struct { int number; short length; int flags; char device[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the request packet.
  request.number = htonl(0xBEEF8151);
  request.length = htons(sizeof(request));
  if (device) { strncpy(request.device, device, 256); }

  // Send the request and receive the response.
  ps2netfs_send(&request, sizeof(request));
  ps2netfs_recv(&response, sizeof(response));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] umount(\"%s\", %d) = %d\n", device, flags, ntohl(response.result));

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
  memcpy(devlist, response.devlist, 256);

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[***] devlist(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.count));

#endif

  // End function.
  return ntohl(response.count);

 }
