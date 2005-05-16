
 #include <stdio.h>
 #include "utility.h"
 #include "network.h"
 #include "ps2netfs.h"

 int ps2netfs_socket = -1;

 ////////////////////////
 // PS2NETFS FUNCTIONS //
 ////////////////////////

 int ps2netfs_connect(char *hostname) {

  // Connect to the ps2netfs server.
  if ((ps2netfs_socket = network_connect(hostname, 0x4713, SOCK_STREAM)) < 0) { return -1; }

  // End function.
  return 0;

 }

 int ps2netfs_disconnect(void) {

  // Disconnect from request.
  if (network_disconnect(ps2netfs_socket) < 0) { return -1; }

  // End function.
  return 0;

 }

 ////////////////////////////////
 // PS2NETFS COMMAND FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_command_open(char *pathname, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_OPEN);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (pathname) { strcpy(command.pathname, pathname); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_close(int fd) {
  struct { unsigned int number; unsigned short length; int fd; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_CLOSE);
  command.length = htons(sizeof(command));
  command.fd     = htonl(fd);

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_read(int fd, void *buffer, int size) {
  struct { unsigned int number; unsigned short length; int fd, size; } PACKED  command;
  struct { unsigned int number; unsigned short length; int result, size; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_READ);
  command.length = htons(sizeof(command));
  command.fd     = htonl(fd);
  command.size   = htonl(size);

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // Receive the response data.
  if (network_receive_all(ps2netfs_socket, buffer, ntohl(response.result)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_write(int fd, void *buffer, int size) {
  struct { unsigned int number; unsigned short length; int fd; int size; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_WRITE);
  command.length = htons(sizeof(command));
  command.fd     = htonl(fd);
  command.size   = htonl(size);

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Send the command data.
  if (network_send(ps2netfs_socket, buffer, size) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_lseek(int fd, int offset, int whence) {
  struct { unsigned int number; unsigned short length; int fd; int offset; int whence; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_LSEEK);
  command.length = htons(sizeof(command));
  command.fd     = htonl(fd);
  command.offset = htonl(offset);
  command.whence = htonl(whence);

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 // ioctl - unimplemented

 int ps2netfs_command_delete(char *pathname, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_DELETE);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_mkdir(char *pathname, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_MKDIR);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_rmdir(char *pathname, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_RMDIR);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_dopen(char *pathname, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_DOPEN);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_dclose(int dd) {
  struct { unsigned int number; unsigned short length; int dd; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_DCLOSE);
  command.length = htons(sizeof(command));
  command.dd     = htonl(dd);

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_dread(int dd, DIRENT *dirent) {
  struct { unsigned int number; unsigned short length; int dd; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; DIRENT dirent; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_DREAD);
  command.length = htons(sizeof(command));
  command.dd     = htonl(dd);

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // Save the response data.
  memcpy(dirent, &response.dirent, sizeof(DIRENT));

  // End function.
  return ntohl(response.result);

 }

 // getstat - unimplemented

 // chstat - unimplemented

 // format - unimplemented

 // rename - unimplemented

 // chdir - unimplemented

 int ps2netfs_command_sync(char *device, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char device[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_SYNC);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (device) { strncpy(command.device, device, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_mount(char *device, char *fsname, int flags, char *argv, int argc) {
  struct { unsigned int number; unsigned short length; char device[256], fsname[256]; int flags; char argv[256]; int argc; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_MOUNT);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  command.argc   = htonl(argc);
  if (device) { strncpy(command.device, device, 256); }
  if (fsname) { strncpy(command.fsname, fsname, 256); }
  if (argv)   { strncpy(command.argv, argv, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 int ps2netfs_command_umount(char *device, int flags) {
  struct { unsigned int number; unsigned short length; int flags; char device[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_UMOUNT);
  command.length = htons(sizeof(command));
  if (device) { strncpy(command.device, device, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // End function.
  return ntohl(response.result);

 }

 // lseek64 - unimplemented

 // devctl - unimplemented

 // symlink - unimplemented

 // readlink - unimplemented

 // ioctl2 - unimplemented

 // info - unimplemented

 // fstype - unimplemented

 int ps2netfs_command_devlist(char *pathname, int flags, char *devlist) {
  struct { unsigned int number; unsigned short length; int flags; char pathname[256]; } PACKED command;
  struct { unsigned int number; unsigned short length; int result; int count; char devlist[256]; } PACKED response;

  // Build the command packet.
  command.number = htonl(PS2NETFS_COMMAND_DEVLIST);
  command.length = htons(sizeof(command));
  command.flags  = htonl(flags);
  if (pathname) { strncpy(command.pathname, pathname, 256); }

  // Send the command packet.
  if (network_send(ps2netfs_socket, &command, sizeof(command)) < 0) { return -1; }

  // Receive the response packet.
  if (network_receive_all(ps2netfs_socket, &response, sizeof(response)) < 0) { return -1; }

  // Save the response data.
  memcpy(devlist, response.devlist, 256);

  // End function.
  return ntohl(response.result);

 }
