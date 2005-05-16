#ifndef __FSCLIENT__
#define __FSCLIENT__

 ////////////////////////
 // FSCLIENT FUNCTIONS //
 ////////////////////////

 int fsclient_copyfrom(char *source, char *destination);

 int fsclient_copyto(char *source, char *destination);

 int fsclient_delete(char *pathname);

 int fsclient_devlist(void);

 int fsclient_dir(char *pathname);

 int fsclient_mkdir(char *pathname);

 int fsclient_mount(char *device, char *fsname);

 int fsclient_rmdir(char *pathname);

 int fsclient_sync(char *device);

 int fsclient_umount(char *device);

#endif
