#ifndef __PS2NETFS_H__
#define __PS2NETFS_H__

 //////////////////////
 // PS2NETFS DEFINES //
 //////////////////////

 #define OPEN_READ		0x0001
 #define OPEN_WRITE		0x0002
 #define OPEN_NONBLOCK		0x0010
 #define OPEN_APPEND		0x0100
 #define OPEN_CREATE		0x0200
 #define OPEN_TRUNCATE		0x0400

 #define LSEEK_SET		0x0000
 #define LSEEK_CURRENT		0x0001
 #define LSEEK_END		0x0002

 #define MOUNT_READWRITE	0x0000
 #define MOUNT_READONLY		0x0001

 typedef struct { int mode, attr, size; char ctime[8], atime[8], mtime[8]; int hisize; char name[256]; } DIRENT;

 ////////////////////////
 // PS2NETFS FUNCTIONS //
 ////////////////////////

 int ps2netfs_connect(char *hostname);

 int ps2netfs_send(void *buffer, int size);

 int ps2netfs_recv(void *buffer, int size);

 int ps2netfs_close(void);

 ////////////////////////////////
 // PS2NETFS COMMAND FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_command_ps2copyfrom(char *hostname, char *source, char *destination);

 int ps2netfs_command_ps2copyto(char *hostname, char *source, char *destination);

 int ps2netfs_command_ps2delete(char *hostname, char *pathname);

 int ps2netfs_command_ps2devlist(char *hostname);

 int ps2netfs_command_ps2dir(char *hostname, char *pathname);

 int ps2netfs_command_ps2format(char *hostname, char *device);

 int ps2netfs_command_ps2mkdir(char *hostname, char *pathname);

 int ps2netfs_command_ps2mount(char *hostname, char *pathname, char *device);

 int ps2netfs_command_ps2rename(char *hostname, char *source, char *destination);

 int ps2netfs_command_ps2rmdir(char *hostname, char *pathname);

 int ps2netfs_command_ps2sync(char *hostname, char *device);

 int ps2netfs_command_ps2umount(char *hostname, char *device);

 ////////////////////////////////
 // PS2NETFS REQUEST FUNCTIONS //
 ////////////////////////////////

 int ps2netfs_request_open(char *pathname, int flags);

 int ps2netfs_request_close(int fd);

 int ps2netfs_request_read(int fd, void *buffer, int size);

 int ps2netfs_request_write(int fd, void *buffer, int size);

 int ps2netfs_request_lseek(int fd, int offset, int whence);

 int ps2netfs_request_ioctl(void); // UNKNOWN

 int ps2netfs_request_delete(char *pathname, int flags);

 int ps2netfs_request_mkdir(char *pathname, int flags);

 int ps2netfs_request_rmdir(char *pathname, int flags);

 int ps2netfs_request_dopen(char *pathname, int flags);

 int ps2netfs_request_dclose(int dd);

 int ps2netfs_request_dread(int dd, DIRENT *dirent);

 int ps2netfs_request_getstat(void); // UNKNOWN

 int ps2netfs_request_chstat(void); // UNKNOWN

 int ps2netfs_request_format(char *device, int flags);

 int ps2netfs_request_rename(char *source, char *destination);

 int ps2netfs_request_chdir(void); // UNKNOWN

 int ps2netfs_request_sync(char *pathname, int flags);

 int ps2netfs_request_mount(char *pathname, char *device, int flags, char *argv, int argc);

 int ps2netfs_request_umount(char *device, int flags);

 int ps2netfs_request_lseek64(void); // UNKNOWN

 int ps2netfs_request_devctl(void); // UNKNOWN

 int ps2netfs_request_symlink(void); // UNKNOWN

 int ps2netfs_request_readlink(void); // UNKNOWN

 int ps2netfs_request_ioctl2(void); // UNKNOWN

 int ps2netfs_request_info(void); // UNKNOWN

 int ps2netfs_request_fstype(void); // UNKNOWN

 int ps2netfs_request_devlist(char *pathname, int flags, char *devlist);

#endif
