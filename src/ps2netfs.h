#ifndef __PS2NETFS_H__
#define __PS2NETFS_H__

 // Open flags.
 #define OPEN_READ		0x0001
 #define OPEN_WRITE		0x0002
 #define OPEN_NONBLOCK		0x0010
 #define OPEN_APPEND		0x0100
 #define OPEN_CREATE		0x0200
 #define OPEN_TRUNCATE		0x0400

 // Lseek flags.
 #define LSEEK_SET		0x0000
 #define LSEEK_CURRENT		0x0001
 #define LSEEK_END		0x0002

 // Mount flags.
 #define MOUNT_READWRITE	0x0000
 #define MOUNT_READONLY		0x0001

 // Dread structure.
 typedef struct { int mode, attr, size; char ctime[8], atime[8], mtime[8]; int hisize; char name[256]; } DIRENT;

 ////////////////////////
 // PS2NETFS FUNCTIONS //
 ////////////////////////

 int ps2netfs_connect(char *hostname);

 int ps2netfs_disconnect(void);

 ////////////////////////////////
 // PS2NETFS COMMAND FUNCTIONS //
 ////////////////////////////////

 #define PS2NETFS_COMMAND_OPEN		0xBEEF8011
 #define PS2NETFS_COMMAND_CLOSE		0xBEEF8021
 #define PS2NETFS_COMMAND_READ		0xBEEF8031
 #define PS2NETFS_COMMAND_WRITE		0xBEEF8041
 #define PS2NETFS_COMMAND_LSEEK		0xBEEF8051
 #define PS2NETFS_COMMAND_IOCTL		0xBEEF8061
 #define PS2NETFS_COMMAND_DELETE	0xBEEF8071
 #define PS2NETFS_COMMAND_MKDIR		0xBEEF8081
 #define PS2NETFS_COMMAND_RMDIR		0xBEEF8091
 #define PS2NETFS_COMMAND_DOPEN		0xBEEF80A1
 #define PS2NETFS_COMMAND_DCLOSE	0xBEEF80B1
 #define PS2NETFS_COMMAND_DREAD		0xBEEF80C1
 #define PS2NETFS_COMMAND_GETSTAT	0xBEEF80D1
 #define PS2NETFS_COMMAND_CHSTAT	0xBEEF80E1
 #define PS2NETFS_COMMAND_FORMAT	0xBEEF80F1
 #define PS2NETFS_COMMAND_RENAME	0xBEEF8111
 #define PS2NETFS_COMMAND_CHDIR		0xBEEF8121
 #define PS2NETFS_COMMAND_SYNC		0xBEEF8031
 #define PS2NETFS_COMMAND_MOUNT		0xBEEF8041
 #define PS2NETFS_COMMAND_UMOUNT	0xBEEF8051
 #define PS2NETFS_COMMAND_LSEEK64	0xBEEF8161
 #define PS2NETFS_COMMAND_DEVCTL	0xBEEF8171
 #define PS2NETFS_COMMAND_SYMLINK	0xBEEF8181
 #define PS2NETFS_COMMAND_READLINK	0xBEEF8191
 #define PS2NETFS_COMMAND_IOCTL2	0xBEEF81A1
 #define PS2NETFS_COMMAND_INFO		0xBEEF8F01
 #define PS2NETFS_COMMAND_FSTYPE	0xBEEF8F11
 #define PS2NETFS_COMMAND_DEVLIST	0xBEEF8F21

 int ps2netfs_open(char *pathname, int flags);

 int ps2netfs_close(int fd);

 int ps2netfs_read(int fd, void *buffer, int size);

 int ps2netfs_write(int fd, void *buffer, int size);

 int ps2netfs_lseek(int fd, int offset, int whence);

 // ioctl - unimplemented

 int ps2netfs_delete(char *pathname, int flags);

 int ps2netfs_mkdir(char *pathname, int flags);

 int ps2netfs_rmdir(char *pathname, int flags);

 int ps2netfs_dopen(char *pathname, int flags);

 int ps2netfs_dclose(int dd);

 int ps2netfs_dread(int dd, DIRENT *dirent);

 // getstat - unimplemented

 // chstat - unimplemented

 // format  - unimplemented

 // rename  - unimplemented

 // chdir - unimplemented

 int ps2netfs_sync(char *pathname, int flags);

 int ps2netfs_mount(char *device, char *fsname, int flags, char *argv, int argc);

 int ps2netfs_umount(char *device, int flags);

 // lseek64 - unimplemented

 // devctl - unimplemented

 // symlink - unimplemented

 // readlink - unimplemented

 // ioctl2 - unimplemented

 // info - unimplemented

 // fstype - unimplemented

 int ps2netfs_devlist(char *pathname, int flags, char *devlist);

#endif
