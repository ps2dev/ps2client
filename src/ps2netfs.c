
 #include <stdio.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <netinet/in.h>
 #include "network.h"
 #include "ps2netfs.h"

 int request_sock;

 /////////////////////////////
 // PS2NETFS MAIN FUNCTIONS //
 /////////////////////////////

 int ps2netfs_connect(char *hostname) {

  // Connect to request.
  request_sock = network_connect(hostname, 0x4713, SOCKET_TCP);
  if (request_sock < 0) { printf("[ERROR] Connect to request failed. (%d)\n", request_sock); return -1; }

  // End function.
  return 0;

 }

 int ps2netfs_send_request(void *buffer, int size) { int result = 0;

  // Send the request data.
  result = network_sendall(request_sock, buffer, size);
  if (result < 0) { printf("[ERROR] Send request data failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int ps2netfs_recv_response(void *buffer, int size) { int result = 0;

  // Wait for the data.
  result = network_wait(10);
  if (result < 1) { printf("[ERROR] Wait for response data failed. (%d)\n", result); return -1; }

  // Receive the response data.
  result = network_recvall(request_sock, buffer, size);
  if (result < 0) { printf("[ERROR] Receive response data failed. (%d)\n", result); return -2; }

  // End function.
  return 0;

 }

 int ps2netfs_disconnect(void) { int result = 0;

  // Disconnect from request.
  result = network_disconnect(request_sock);
  if (result < 0) { printf("[ERROR] Disconnect from request failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 ////////////////////////////////
 // PS2NETFS COMMAND FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_command_copyfrom(char *hostname, char *source, char *destination) { int result = 0;
  int fd0, fd1, size, total = 0; char buffer[28000];

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Open the source file.
  fd0 = ps2netfs_request_open(source, OPEN_READ);
  if (fd0 < 0) { printf("[ERROR] Open source file failed. (%d)\n", fd0); return -2; }

  // Get the source file size.
  size = ps2netfs_request_lseek(fd0, 0, LSEEK_END); ps2netfs_request_lseek(fd0, 0, LSEEK_SET);
  if (size < 0) { printf("[ERROR] Get source file size failed. (%d)\n", size); return -3; }

  // Open the destination file.
  fd1 = open(destination, O_RDWR | O_CREAT, 0644);
  if (fd0 < 1) { printf("[ERROR] Open destination file failed. (%d)\n", fd1); return -4; }

  // Output the display header.
  printf("\n [%s --> %s]\n\n  Progress: ", source, destination);

  // Repeat until the copy is finished.
  while (total < size) { printf("#");

   // Read the source data.
   result = ps2netfs_request_read(fd0, buffer, sizeof(buffer));
   if (result < 0) { printf("[ERROR] Read source data failed. (%d)\n", result); return -5; }

   // Write the destination data.
   result = write(fd1, buffer, result);
   if (result < 0) { printf("[ERROR] Write destination data failed. (%d)\n", result); return -6; }

   // Increment the counter.
   total += result;

  }

  // Output the display footer.
  printf("\n\n [%d Bytes Copied]\n\n", total);

  // Close the source file.
  result = ps2netfs_request_close(fd0);
  if (result < 0) { printf("[ERROR] Close source file failed. (%d)\n", result); return -7; }

  // Close the destination file.
  result = close(fd1);
  if (result < 0) { printf("[ERROR] Close destination file failed. (%d)\n", result); return -8; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -9; }

  // End function.
  return 0;

 }

 int ps2netfs_command_copyto(char *hostname, char *source, char *destination) { int result = 0;
  int fd0, fd1, size, total = 0; char buffer[28000];

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Open the source file.
  fd0 = open(source, O_RDONLY);
  if (fd0 < 0) { printf("[ERROR] Open source file failed. (%d)\n", fd0); return -2; }

  // Get the source file size.
  size = lseek(fd0, 0, SEEK_END); lseek(fd0, 0, SEEK_SET);
  if (size < 0) { printf("[ERROR] Get source file size failed. (%d)\n", size); return -3; }

  // Open the destination file.
  fd1 = ps2netfs_request_open(destination, OPEN_WRITE | OPEN_CREATE);
  if (fd0 < 1) { printf("[ERROR] Open destination file failed. (%d)\n", fd1); return -4; }

  // Output the display header.
  printf("\n [%s --> %s]\n\n  Progress: ", source, destination);

  // Repeat until the copy is finished.
  while (total < size) { printf("#");

   // Read the source data.
   result = read(fd0, buffer, sizeof(buffer));
   if (result < 0) { printf("[ERROR] Read source data failed. (%d)\n", result); return -5; }

   // Write the destination data.
   result = ps2netfs_request_write(fd1, buffer, result);
   if (result < 0) { printf("[ERROR] Write destination data failed. (%d)\n", result); return -6; }

   // Increment the counter.
   total += result;

  }

  // Output the display footer.
  printf("\n\n [%d Bytes Copied]\n\n", total);

  // Close the source file.
  result = close(fd0);
  if (result < 0) { printf("[ERROR] Close source file failed. (%d)\n", result); return -7; }

  // Close the destination file.
  result = ps2netfs_request_close(fd1);
  if (result < 0) { printf("[ERROR] Close destination file failed. (%d)\n", result); return -8; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -9; }

  // End function.
  return 0;

 }

 int ps2netfs_command_delete(char *hostname, char *pathname) { int result = 0;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Delete the file.
  result = ps2netfs_request_delete(pathname, 0);
  if (result < 0) { printf("[ERROR] Delete file failed. (%d)\n", result); return -2; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2netfs_command_devlist(char *hostname) { int result = 0;
  int loop0, devcount; char devlist[256], *temp = devlist;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Get the device listing.
  devcount = ps2netfs_request_devlist("", 0, devlist);
  if (devcount < 0) { printf("[ERROR] Get device listing failed. (%d)\n", result); return -2; }

  // Output the display header.
  printf("\n [Active Devices]\n\n");

  // Output each available device.
  for(loop0=0;loop0<devcount;loop0++) {

   // Output the device description.
   if (!strcmp(temp, "rom"))   { printf("  rom    - Onboard rom bios chip.\n");           } else
   if (!strcmp(temp, "cdrom")) { printf("  cdrom  - Standard cd/dvd device. (cdrom:)\n"); } else
   if (!strcmp(temp, "host"))  { printf("  host   - Host file system. (host:)\n");        } else
   if (!strcmp(temp, "mc"))    { printf("  mc     - Memory card driver. (mc0: mc1:)\n");  } else
   if (!strcmp(temp, "hdd"))   { printf("  hdd    - Internal HDD unit. (hdd0:)\n");       } else
   if (!strcmp(temp, "pfs"))   { printf("  pfs    - Playstation File System. (pfs0:)\n"); } else
   if (!strcmp(temp, "dev9x")) { printf("  dev9x  - Expansion port device driver.\n");    } else { printf("  %s\n", temp); }

   // Increment temp.
   temp += strlen(temp) + 1;

  }

  // Output the display footer.
  printf("\n [%d Devices Found]\n\n", devcount);

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2netfs_command_dir(char *hostname, char *pathname) { int result = 0;
  int dd, files = 0, size = 0; DIRENT dirent;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Open the directory.
  dd = ps2netfs_request_dopen(pathname, 0);
  if (dd < 0) { printf("[ERROR] Open directory failed. (%d)\n", dd); return -2; }

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
  result = ps2netfs_request_dclose(dd);
  if (result < 0) { printf("[ERROR] Close directory failed. (%d)\n", result); return -3; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -4; }

  // End function.
  return 0;

 }

 int ps2netfs_command_format(char *hostname, char *device) { printf("[ERROR] Not finished!\n"); return -1; }

 int ps2netfs_command_mkdir(char *hostname, char *pathname) { int result = 0;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Make the directory.
  result = ps2netfs_request_mkdir(pathname, 0);
  if (result < 0) { printf("[ERROR] Make directory failed. (%d)\n", result); return -2; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2netfs_command_mount(char *hostname, char *device, char *fsname) { int result = 0;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Mount the device.
  result = ps2netfs_request_mount(device, fsname, MOUNT_READWRITE, "", 0);
  if (result < 0) { printf("[ERROR] Mount device failed. (%d)\n", result); return -2; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2netfs_command_rename(char *hostname, char *source, char *destination) { printf("[ERROR] Not finished!\n"); return -1; }

 int ps2netfs_command_rmdir(char *hostname, char *pathname) { int result = 0;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Remove the directory.
  result = ps2netfs_request_rmdir(pathname, 0);
  if (result < 0) { printf("[ERROR] Remove directory failed. (%d)\n", result); return -2; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2netfs_command_sync(char *hostname, char *device) { int result = 0;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Sync the device.
  result = ps2netfs_request_sync(device, 0);
  if (result < 0) { printf("[ERROR] Sync device failed. (%d)\n", result); return -2; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 int ps2netfs_command_umount(char *hostname, char *device) { int result = 0;

  // Connect to ps2netfs.
  result = ps2netfs_connect(hostname);
  if (result < 0) { printf("[ERROR] Connect to ps2netfs failed. (%d)\n", result); return -1; }

  // Umount the device.
  result = ps2netfs_request_umount(device, 0);
  if (result < 0) { printf("[ERROR] Umount device failed. (%d)\n", result); return -2; }

  // Disconnect from ps2netfs.
  result = ps2netfs_disconnect();
  if (result < 0) { printf("[ERROR] Disconnect from ps2netfs failed. (%d)\n", result); return -3; }

  // End function.
  return 0;

 }

 ////////////////////////////////
 // PS2NETFS REQUEST FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_request_open(char *pathname, int flags) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the open request.
  request.number = htonl(0xBEEF8011);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strcpy(request.pathname, pathname); }

  // Send the open request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send open request failed. (%d)\n", result); return -1; }

  // Receive the open response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive open response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] open(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_close(int fd) { int result = 0;
  struct { int number; short length; int fd; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the close request.
  request.number = htonl(0xBEEF8021);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);

  // Send the open request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send close request failed. (%d)\n", result); return -1; }

  // Receive the open response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive close response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] close(%d) = %d\n", fd, ntohl(response.result));

#endif

  // End function,
  return ntohl(response.result);

 }

 int ps2netfs_request_read(int fd, void *buffer, int size) { int result = 0;
  struct { int number; short length; int fd, size; } __attribute__((packed))  request;
  struct { int number; short length; int result, size; } __attribute__((packed)) response;

  // Build the read request.
  request.number = htonl(0xBEEF8031);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);
  request.size   = htonl(size);

  // Send the read request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send read request failed. (%d)\n", result); return -1; }

// sleep(2);

  // Receive the read response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive read response failed. (%d)\n", result); return -2; }

  // Receive the read response data.
  result = ps2netfs_recv_response(buffer, ntohl(response.result));
  if (result < 0) { printf("[ERROR] Receive read response data failed. (%d)\n", result); return -3; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] read(%d, buffer, %d) = %d\n", fd, size, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_write(int fd, void *buffer, int size) { int result = 0;
  struct { int number; short length; int fd, size; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the write request.
  request.number = htonl(0xBEEF8041);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);
  request.size   = htonl(size);

  // Send the write request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send write request failed. (%d)\n", result); return -1; }

  // Send the write request data.
  result = ps2netfs_send_request(buffer, size);
  if (result < 0) { printf("[ERROR] Send write request data failed. (%d)\n", result); return -2; }

  // Receive the write response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive write response failed. (%d)\n", result); return -3; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] write(%d, buffer, %d) = %d\n", fd, size, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_lseek(int fd, int offset, int whence) { int result = 0;
  struct { int number; short length; int fd, offset, whence; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the lseek request.
  request.number = htonl(0xBEEF8051);
  request.length = htons(sizeof(request));
  request.fd     = htonl(fd);
  request.offset = htonl(offset);
  request.whence = htonl(whence);

  // Send the lseek request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send lseek request failed. (%d)\n", result); return -1; }

  // Receive the lseek response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive lseek response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] lseek(%d, %d, %d) = %d\n", fd, offset, whence, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_delete(char *pathname, int flags) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the delete request.
  request.number = htonl(0xBEEF8071);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the delete request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send delete request failed. (%d)\n", result); return -1; }

  // Receive the delete response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive delete response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] delete(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_mkdir(char *pathname, int flags) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the mkdir request.
  request.number = htonl(0xBEEF8081);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the mkdir request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send mkdir request failed. (%d)\n", result); return -1; }

  // Receive the mkdir response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive mkdir response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] mkdir(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }


 int ps2netfs_request_rmdir(char *pathname, int flags) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the rmdir request.
  request.number = htonl(0xBEEF8091);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the rmdir request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send rmdir request failed. (%d)\n", result); return -1; }

  // Receive the rmdir response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive rmdir response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] rmdir(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_dopen(char *pathname, int flags) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the dopen request.
  request.number = htonl(0xBEEF80A1);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the dopen request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send dopen request failed. (%d)\n", result); return -1; }

  // Receive the dopen response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive dopen response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] dopen(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_dclose(int dd) { int result = 0;
  struct { int number; short length; int dd; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the dclose request.
  request.number = htonl(0xBEEF80B1);
  request.length = htons(sizeof(request));
  request.dd     = htonl(dd);

  // Send the dclose request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send dclose request failed. (%d)\n", result); return -1; }

  // Receive the dclose response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive dclose response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] dclose(%d) = %d\n", dd, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_dread(int dd, DIRENT *dirent) { int result = 0;
  struct { int number; short length; int dd; } __attribute__((packed)) request;
  struct { int number; short length; int result; DIRENT dirent; } __attribute__((packed)) response;

  // Build the dread request.
  request.number = htonl(0xBEEF80C1);
  request.length = htons(sizeof(request));
  request.dd     = htonl(dd);

  // Send the dread request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send dread request failed. (%d)\n", result); return -1; }

  // Receive the dread response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive dread response failed. (%d)\n", result); return -2; }

  // Copy over the directory entry.
  memcpy(dirent, &response.dirent, sizeof(DIRENT));

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] dread(%d, dirent) = %d\n", dd, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_format(char *device, int flags) { printf("[ERROR] Unfinished!\n"); return -1; }

 int ps2netfs_request_rename(char *source, char *destination) { printf("[ERROR] Unfinished!\n"); return -1; }

 int ps2netfs_request_sync(char *device, int flags) { int result = 0;
  struct { int number; short length; int flags; char device[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the sync request.
  request.number = htonl(0xBEEF8131);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (device) { strncpy(request.device, device, 256); }

  // Send the sync request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send sync request failed. (%d)\n", result); return -1; }

  // Receive the sync response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive sync response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] sync(\"%s\", %d) = %d\n", device, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_mount(char *device, char *fsname, int flags, char *argv, int argc) { int result = 0;
  struct { int number; short length; char device[256], fsname[256]; int flags; char argv[256]; int argc; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the mount request.
  request.number = htonl(0xBEEF8141);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  request.argc   = htonl(argc);
  if (device) { strncpy(request.device, device, 256); }
  if (fsname) { strncpy(request.fsname, fsname, 256); }
  if (argv)   { strncpy(request.argv, argv, 256); }

  // Send the mount request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send mount request failed. (%d)\n", result); return -1; }

  // Receive the mount response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive mount response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] mount(\"%s\", \"%s\", %d) = %d\n", device, fsname, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_umount(char *device, int flags) { int result = 0;
  struct { int number; short length; int flags; char device[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; } __attribute__((packed)) response;

  // Build the umount request.
  request.number = htonl(0xBEEF8151);
  request.length = htons(sizeof(request));
  if (device) { strncpy(request.device, device, 256); }

  // Send the umount request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send umount request failed. (%d)\n", result); return -1; }

  // Receive the umount response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive umount response failed. (%d)\n", result); return -2; }

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] umount(\"%s\", %d) = %d\n", device, flags, ntohl(response.result));

#endif

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_request_devlist(char *pathname, int flags, char *devlist) { int result = 0;
  struct { int number; short length; int flags; char pathname[256]; } __attribute__((packed)) request;
  struct { int number; short length; int result; int count; char devlist[256]; } __attribute__((packed)) response;

  // Build the devlist request.
  request.number = htonl(0xBEEF8F21);
  request.length = htons(sizeof(request));
  request.flags  = htonl(flags);
  if (pathname) { strncpy(request.pathname, pathname, 256); }

  // Send the devlist request.
  result = ps2netfs_send_request(&request, sizeof(request));
  if (result < 0) { printf("[ERROR] Send devlist request failed. (%d)\n", result); return -1; }

  // Receive the devlist response.
  result = ps2netfs_recv_response(&response, sizeof(response));
  if (result < 0) { printf("[ERROR] Receive devlist response failed. (%d)\n", result); return -2; }

  // Copy over the device list.
  memcpy(devlist, response.devlist, 256);

#ifdef __DEBUG__

  // Tell the user about the request and its result.
  printf("[DEBUG] devlist(\"%s\", %d) = %d\n", pathname, flags, ntohl(response.count));

#endif

  // End function.
  return ntohl(response.count);

 }
