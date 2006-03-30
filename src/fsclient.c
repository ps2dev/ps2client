
 #include <stdio.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include "utility.h"
 #include "ps2netfs.h"
 #include "network.h"

 #include "fsclient.h"

 char hostname[256] = { "192.168.0.10" };

 int main(int argc, char **argv, char **env) { int loop0 = 0;

  // Turn off stdout buffering.
  setbuf(stdout, NULL);

  // Parse the environment list for optional arguments.
  for (loop0=0; env[loop0]; loop0++) {

   // A hostname has been specified...
   if (strncmp(env[loop0], "PS2HOSTNAME", 11) == 0) { strncpy(hostname, &env[loop0][12], sizeof(hostname)); }

  }

  // Check the number of arguments.
  if (argc < 2) { print_usage(); return -1; }

  // Parse the argument list for optional arguments.
  for (loop0=1; argv[loop0]; loop0++) {

   // If an optional hostname has been specified...
   if (strncmp(argv[loop0], "-h", 2) == 0) { loop0++;

    // Check to make sure the hostname value was actually supplied.
    if (argc == loop0) { printf("Error: No hostname was supplied the '-h' option.\n"); print_usage(); return -1; }

    // Set the hostname to the supplied value.
    strncpy(hostname, argv[loop0], sizeof(hostname));

   }

   // Else, the end of the options has been reached...
   else { break; }

  }

  // Increment the argument counters past any optional arguments.
  loop0++; argc -= loop0; argv += loop0;

  // Check to make sure a command was actually supplied.
  if (argc < 0) { printf("Error: No command was supplied.\n"); print_usage(); return -1; }

#ifdef _WIN32

  // Startup network, under windows.
  if (network_startup() < 0) { printf("Error: Could not start up winsock.\n"); return 1; }

#endif

  // Connect to the ps2netfs server.
  if (ps2netfs_connect(hostname) < 0) { printf("Error: Could not connect to the ps2netfs server. (%s)\n", hostname); return -1; }

  // Perform the requested command.
  if (strcmp(argv[-1], "copyfrom") == 0) { return fsclient_copyfrom(argv[0], argv[1]); } else
  if (strcmp(argv[-1], "copyto")   == 0) { return fsclient_copyto(argv[0], argv[1]);   } else
  if (strcmp(argv[-1], "delete")   == 0) { return fsclient_delete(argv[0]);            } else
  if (strcmp(argv[-1], "devlist")  == 0) { return fsclient_devlist();                  } else
  if (strcmp(argv[-1], "dir")      == 0) { return fsclient_dir(argv[0]);               } else
  if (strcmp(argv[-1], "mkdir")    == 0) { return fsclient_mkdir(argv[0]);             } else
  if (strcmp(argv[-1], "mount")    == 0) { return fsclient_mount(argv[0], argv[1]);    } else
  if (strcmp(argv[-1], "rmdir")    == 0) { return fsclient_rmdir(argv[0]);             } else
  if (strcmp(argv[-1], "sync")     == 0) { return fsclient_sync(argv[0]);              } else
  if (strcmp(argv[-1], "umount")   == 0) { return fsclient_umount(argv[0]);            } else

  // An unknown command was requested.
  { printf("Error: Unknown command requested. (%s)\n", argv[-1]); print_usage(); return -1; }

  // Disconnect from the ps2netfs server.
  if (ps2netfs_disconnect() < 0) { printf("Error: Could not disconnect from the ps2netfs server. (%s)\n", hostname); return -1; }

  // End program.
  return 0;

 }

 ////////////////////////
 // FSCLIENT FUNCTIONS //
 ////////////////////////

 int fsclient_copyfrom(char *source, char *destination) { int result = 0;
  int fd0, fd1, size, total = 0; char buffer[28000];

  // Open the source file.
  fd0 = ps2netfs_command_open(source, OPEN_READ);
  if (fd0 < 0) { printf("Error: Open source file failed. (%d)\n", fd0); return -1; }

  // Get the source file size.
  size = ps2netfs_command_lseek(fd0, 0, LSEEK_END); ps2netfs_command_lseek(fd0, 0, LSEEK_SET);
  if (size < 0) { printf("Error: Get source file size failed. (%d)\n", size); return -1; }

  // Open the destination file.
  fd1 = open(destination, O_RDWR|O_CREAT|O_BINARY, 0644);
  if (fd0 < 1) { printf("Error: Open destination file failed. (%d)\n", fd1); return -1; }

  // Output the display header.
  printf("\n [%s --> %s]\n\n  Progress: ", source, destination);

  // Repeat until the copy is finished.
  while (total < size) { printf("#");

   // Read the source data.
   result = ps2netfs_command_read(fd0, buffer, sizeof(buffer));
   if (result < 0) { printf("Error: Read source data failed. (%d)\n", result); return -1; }

   // Write the destination data.
   result = write(fd1, buffer, result);
   if (result < 0) { printf("Error: Write destination data failed. (%d)\n", result); return -1; }

   // Increment the counter.
   total += result;

  }

  // Output the display footer.
  printf("\n\n [%d Bytes Copied]\n\n", total);

  // Close the source file.
  result = ps2netfs_command_close(fd0);
  if (result < 0) { printf("Error: Close source file failed. (%d)\n", result); return -1; }

  // Close the destination file.
  result = close(fd1);
  if (result < 0) { printf("Error: Close destination file failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_copyto(char *source, char *destination) { int result = 0;
  int fd0, fd1, size, total = 0; char buffer[28000];

  // Open the source file.
  fd0 = open(source, O_RDONLY|O_BINARY);
  if (fd0 < 0) { printf("Error: Open source file failed. (%d)\n", fd0); return -1; }

  // Get the source file size.
  size = lseek(fd0, 0, SEEK_END); lseek(fd0, 0, SEEK_SET);
  if (size < 0) { printf("Error: Get source file size failed. (%d)\n", size); return -1; }

  // Open the destination file.
  fd1 = ps2netfs_command_open(destination, OPEN_WRITE | OPEN_CREATE);
  if (fd0 < 1) { printf("Error: Open destination file failed. (%d)\n", fd1); return -1; }

  // Output the display header.
  printf("\n [%s --> %s]\n\n  Progress: ", source, destination);

  // Repeat until the copy is finished.
  while (total < size) { printf("#");

   // Read the source data.
   result = read(fd0, buffer, sizeof(buffer));
   if (result < 0) { printf("Error: Read source data failed. (%d)\n", result); return -1; }

   // Write the destination data.
   result = ps2netfs_command_write(fd1, buffer, result);
   if (result < 0) { printf("Error: Write destination data failed. (%d)\n", result); return -1; }

   // Increment the counter.
   total += result;

  }

  // Output the display footer.
  printf("\n\n [%d Bytes Copied]\n\n", total);

  // Close the source file.
  result = close(fd0);
  if (result < 0) { printf("Error: Close source file failed. (%d)\n", result); return -1; }

  // Close the destination file.
  result = ps2netfs_command_close(fd1);
  if (result < 0) { printf("Error: Close destination file failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_delete(char *pathname) { int result = 0;

  // Delete the file.
  result = ps2netfs_command_delete(pathname, 0);
  if (result < 0) { printf("Error: Delete file failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_devlist(void) { int result = 0;
  int loop0, devcount; char devlist[256], *temp = devlist;

  // Get the device listing.
  devcount = ps2netfs_command_devlist("", 0, devlist);
  if (devcount < 0) { printf("Error: Get device listing failed. (%d)\n", result); return -1; }

  // Output the display header.
  printf("\n [Active Devices]\n\n");

  // Output each available device.
  for(loop0=0;loop0<devcount;loop0++) {

   // Output the device description.
   if (!strcmp(temp, "rom"))   { printf("  rom    - Onboard rom device.\n");              } else
   if (!strcmp(temp, "cdrom")) { printf("  cdrom  - Standard cd/dvd device. (cdrom:)\n"); } else
   if (!strcmp(temp, "host"))  { printf("  host   - Host file system. (host:)\n");        } else
   if (!strcmp(temp, "mc"))    { printf("  mc     - Memory card driver. (mc0: mc1:)\n");  } else
   if (!strcmp(temp, "hdd"))   { printf("  hdd    - Internal HDD unit.\n");               } else
   if (!strcmp(temp, "pfs"))   { printf("  pfs    - Playstation File System.\n");         } else
   if (!strcmp(temp, "dev9x")) { printf("  dev9x  - Blah blah blah.\n");                  } else

   // 
   { printf("  %s\n", temp); }

   // Increment temp.
   temp += strlen(temp) + 1;

  }

  // Output the display footer.
  printf("\n [%d Devices Found]\n\n", devcount);

  // End function.
  return 0;

 }

 int fsclient_dir(char *pathname) { int result = 0;
  int dd, files = 0, size = 0; DIRENT dirent;

  // Open the directory.
  dd = ps2netfs_command_dopen(pathname, 0);
  if (dd < 0) { printf("Error: Open directory failed. (%d)\n", dd); return -1; }

  // Output the display header.
  printf("\n [Contents of %s]\n\n", pathname);

  // Read in each of the entries.
  while (ps2netfs_command_dread(dd, &dirent) > 0) { printf("  ");

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
  result = ps2netfs_command_dclose(dd);
  if (result < 0) { printf("Error: Close directory failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_mkdir(char *pathname) { int result = 0;

  // Make the directory.
  result = ps2netfs_command_mkdir(pathname, 0);
  if (result < 0) { printf("Error: Make directory failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_mount(char *device, char *fsname) { int result = 0;

  // Mount the device.
  result = ps2netfs_command_mount(device, fsname, MOUNT_READWRITE, "", 0);
  if (result < 0) { printf("Error: Mount device failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_rmdir(char *pathname) { int result = 0;

  // Remove the directory.
  result = ps2netfs_command_rmdir(pathname, 0);
  if (result < 0) { printf("Error: Remove directory failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_sync(char *device) { int result = 0;

  // Sync the device.
  result = ps2netfs_command_sync(device, 0);
  if (result < 0) { printf("Error: Sync device failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }

 int fsclient_umount(char *device) { int result = 0;

  // Umount the device.
  result = ps2netfs_command_umount(device, 0);
  if (result < 0) { printf("Error: Umount device failed. (%d)\n", result); return -1; }

  // End function.
  return 0;

 }
