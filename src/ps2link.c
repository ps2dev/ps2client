
 #include <stdio.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <dirent.h>
 #include <sys/stat.h>
 #include <sys/poll.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <time.h>

 #include "types.h"
 #include "network.h"
 #include "ps2link.h"

 int sock_cmd, sock_req, sock_log, sock_tmp;

 u08 buffer[65536];

 ////////////////////////////
 // PS2LINK MAIN FUNCTIONS //
 ////////////////////////////

 int ps2link_open(char *hostname) {

  // Open a command packet socket.
  network_open(&sock_cmd, hostname, 0x4712, 1); if (sock_cmd == 0) { return -1; }

  // Open a request packet socket.
  network_open(&sock_req, hostname, 0x4711, 0); if (sock_req == 0) { return -1; }

  // Open a command packet socket.
  network_serv(&sock_log, 0x4712, 1); if (sock_log == 0) { return -1; }

  // End function.
  return 0;

 }

 int ps2link_loop(void) { struct pollfd ufsd[2];
  struct { u32 cmd; u16 len; u08 data[65544]; } __attribute__((packed)) request;

  // Preparing the pool's arguments.
  ufsd[0].fd = sock_req; ufsd[1].fd = sock_log;
  ufsd[0].events = ufsd[1].events = POLLIN;

  // Main request parsing and log reading loop.
  while(1) {

   // Wait until something happens.
   poll(ufsd, 2, -1);

   // Read in the next request packet.
   if (network_recv(&sock_req, &request, sizeof(request)) > 0) {

    // Perform the request, if it is a valid command.    
    if (ntohl(request.cmd) == 0xBABE0111) { ps2link_req_open  ((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0121) { ps2link_req_close ((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0131) { ps2link_req_read  ((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0141) { ps2link_req_write ((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0151) { ps2link_req_lseek ((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0161) { ps2link_req_dopen ((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0171) { ps2link_req_dclose((void *)&request); } else
    if (ntohl(request.cmd) == 0xBABE0181) { ps2link_req_dread ((void *)&request); }

   }

   // Output log text if received.
   while (network_recvfrom(&sock_log, buffer, sizeof(buffer)) > 0) {

    // Print out the log text.
    printf("[LOG] %s", buffer);

   }

  }

  // End function.
  return 0;

 }

 int ps2link_shut(void) {

  // Close the command packet socket.
  network_shut(&sock_cmd);

  // Close the receive packet socket.
  network_shut(&sock_req);

  // End function.
  return 0;

 }

 int ps2link_fixflags(int flags) { int retval = 0;

  // Convert the read and write flags.
  if ((flags & 0x03) == 0x0001) { retval |= O_RDONLY; }
  if ((flags & 0x03) == 0x0002) { retval |= O_WRONLY; }
  if ((flags & 0x03) == 0x0003) { retval |= O_RDWR;   }

  // Convert the other flags.
  if (flags & 0x0010) { retval |= O_NONBLOCK; }
  if (flags & 0x0100) { retval |= O_APPEND;   }
  if (flags & 0x0200) { retval |= O_CREAT;    }
  if (flags & 0x0400) { retval |= O_TRUNC;    }

  // End function.
  return retval;

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

 int ps2link_cmd_reset(void) {
  struct { u32 cmd; u16 len; } __attribute__((packed)) packet;

  // Tell the user what we're doing.
  printf("[***] Sending reset command...\n");

  // Build the reset command packet.
  packet.cmd = htonl(0xBABE0201);
  packet.len = htons(sizeof(packet));

  // Send the reset command packet.
  return network_send(&sock_cmd, &packet, sizeof(packet));

 }

 int ps2link_cmd_poweroff(void) {
  struct { u32 cmd; u16 len; } __attribute__((packed)) packet;

  // Tell the user what we're doing.
  printf("[***] Sending poweroff command...\n");

  // Build the poweroff command packet.
  packet.cmd = htonl(0xBABE0204);
  packet.len = htons(sizeof(packet));

  // Send the poweroff command packet.
  return network_send(&sock_cmd, &packet, sizeof(packet));

 }

 int ps2link_cmd_execiop(char *pathname) {
  struct { u32 cmd; u16 len; int argc; char argv[256]; } __attribute__((packed)) packet;

  // Tell the user what we're doing.
  printf("[***] Sending execiop command...\n");

  // Build the execiop command packet.
  packet.cmd = htonl(0xBABE0202);
  packet.len = htons(sizeof(packet));
  packet.argc = htonl(1); sprintf(packet.argv, "%s", pathname);

  // Send the execiop command packet.
  network_send(&sock_cmd, &packet, sizeof(packet));

  // Start the request parsing loop.
  return ps2link_loop();

 }

 int ps2link_cmd_execee(char *pathname) {
  struct { u32 cmd; u16 len; int argc; char argv[256]; } __attribute__((packed)) packet;

  // Tell the user what we're doing.
  printf("[***] Sending execee command...\n");

  // Build the execee command packet.
  packet.cmd = htonl(0xBABE0203);
  packet.len = htons(sizeof(packet));
  packet.argc = htonl(1); sprintf(packet.argv, "%s", pathname);

  // Send the execee command packet.
  network_send(&sock_cmd, &packet, sizeof(packet));

  // Start the request parsing loop.
  return ps2link_loop();

 }

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_req_open(void *request) {
  struct { u32 cmd; u16 len; int ret; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int flags; char path[256]; } __attribute__((packed)) *temp = request;

  // Fix the pathname.
  ps2link_fixpathname(temp->path);

  // Build the response packet.
  packet.cmd = htonl(0xBABE0112);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl(open(temp->path, ps2link_fixflags(ntohl(temp->flags)), 0644));

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] open(\"host0:%s\", %d) = %d\n", temp->path, ntohl(temp->flags), ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_close(void *request) {
  struct { u32 cmd; u16 len; int ret; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int fd; } __attribute__((packed)) *temp = request;

  // Build the response packet.
  packet.cmd = htonl(0xBABE0122);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl(close(ntohl(temp->fd)));

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] close(%d) = %d\n", ntohl(temp->fd), ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_read(void *request) {
  struct { u32 cmd; u16 len; int ret; int nbytes; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int fd; int nbytes; } __attribute__((packed)) *temp = request;

  // Build the response packet.
  packet.cmd = htonl(0xBABE0132);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl(read(ntohl(temp->fd), buffer, ntohl(temp->nbytes)));
  packet.nbytes = packet.ret;

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Send the requested read data.
  network_send(&sock_req, buffer, ntohl(packet.ret));

  // Tell the user about the request and its result.
  printf("[***] read(%d, buffer, %d) = %d\n", ntohl(temp->fd), ntohl(temp->nbytes), ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_write(void *request) {
  struct { u32 cmd; u16 len; int ret; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int fd; int nbytes; u08 data[65536]; } __attribute__((packed)) *temp = request;

  // Build the response packet.
  packet.cmd = htonl(0xBABE0142);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl(write(ntohl(temp->fd), temp->data, ntohl(temp->nbytes)));

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] write(%d, buffer, %d) = %d\n", ntohl(temp->fd), ntohl(temp->nbytes), ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_lseek(void *request) {
  struct { u32 cmd; u16 len; int ret; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int fd; int offset; int whence; } __attribute__((packed)) *temp = request;

  // Build the response packet.
  packet.cmd = htonl(0xBABE0152);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl(lseek(ntohl(temp->fd), ntohl(temp->offset), ntohl(temp->whence)));

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] lseek(%d, %d, %d) = %d\n", ntohl(temp->fd), ntohl(temp->offset), ntohl(temp->whence), ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_dopen(void *request) {
  struct { u32 cmd; u16 len; int ret; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int flags; char pathname[256]; } __attribute__((packed)) *temp = request;

  // Fix the pathname.
  ps2link_fixpathname(temp->pathname);

  // Build the response packet.
  packet.cmd = htonl(0xBABE0162);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl((int)opendir(temp->pathname));

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] dopen(\"host0:%s\") = %d\n", temp->pathname, ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_dread(void *request) { struct dirent *direptr; struct stat stats; struct tm loctime;
  struct { u32 cmd; u16 len; int ret; u32 mode, attr, size; u08 ctime[8], atime[8], mtime[8]; u32 hisize; u08 name[256]; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int fd; } __attribute__((packed)) *temp = request;

  // Build the response packet.
  packet.cmd = htonl(0xBABE0182);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl((int)direptr = readdir((DIR *)ntohl(temp->fd)));

  // If an entry was returned, add the entry information to the response packet.
  if (direptr != 0) {

   // Fetch the stats for the entry.
   stat(direptr->d_name, &stats);

   // Convert and add the mode.
   packet.mode = (stats.st_mode & 0x07);
   if (S_ISDIR(stats.st_mode)) { packet.mode |= 0x20; }
   if (S_ISLNK(stats.st_mode)) { packet.mode |= 0x08; }
   if (S_ISREG(stats.st_mode)) { packet.mode |= 0x10; }
   packet.mode = htonl(packet.mode);

   // Add the attributes.
   packet.attr = htonl(0);

   // Add the size.
   packet.size = htonl(stats.st_size);

   // Convert and add the creation time.
   if (localtime_r(&(stats.st_ctime), &loctime)) {
     packet.ctime[6] = (u08)loctime.tm_year;
     packet.ctime[5] = (u08)loctime.tm_mon + 1;
     packet.ctime[4] = (u08)loctime.tm_mday;
     packet.ctime[3] = (u08)loctime.tm_hour;
     packet.ctime[2] = (u08)loctime.tm_min;
     packet.ctime[1] = (u08)loctime.tm_sec;
   }

   // Convert and add the access time.
   if (localtime_r(&(stats.st_atime), &loctime)) {
     packet.atime[6] = (u08)loctime.tm_year;
     packet.atime[5] = (u08)loctime.tm_mon + 1;
     packet.atime[4] = (u08)loctime.tm_mday;
     packet.atime[3] = (u08)loctime.tm_hour;
     packet.atime[2] = (u08)loctime.tm_min;
     packet.atime[1] = (u08)loctime.tm_sec;
   }

   // Convert and add the modification time.
   if (localtime_r(&(stats.st_mtime), &loctime)) {
     packet.mtime[6] = (u08)loctime.tm_year;
     packet.mtime[5] = (u08)loctime.tm_mon + 1;
     packet.mtime[4] = (u08)loctime.tm_mday;
     packet.mtime[3] = (u08)loctime.tm_hour;
     packet.mtime[2] = (u08)loctime.tm_min;
     packet.mtime[1] = (u08)loctime.tm_sec;
   }

   // Add the hsize. (what is this?)
   packet.hisize = htonl(0);

   // Add the name.
   strncpy(packet.name, direptr->d_name, 256);

  }

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] dread(%d) = %d\n", ntohl(temp->fd), ntohl(packet.ret));

  // End function.
  return 0;

 }

 int ps2link_req_dclose(void *request) {
  struct { u32 cmd; u16 len; int ret; } __attribute__((packed)) packet;
  struct { u32 cmd; u16 len; int fd; } __attribute__((packed)) *temp = request;

  // Build the response packet.
  packet.cmd = htonl(0xBABE0172);
  packet.len = htons(sizeof(packet));
  packet.ret = htonl(closedir((DIR *)ntohl(temp->fd)));

  // Send the response packet.
  network_send(&sock_req, &packet, sizeof(packet));

  // Tell the user about the request and its result.
  printf("[***] dclose(%d) = %d\n", ntohl(temp->fd), ntohl(packet.ret));

  // End function.
  return 0;

 }
