
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/time.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include "network.h"

 int sock[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

 fd_set rfds;

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

 int network_connect(char *hostname, int port, int type) { int loop0, nd = -1;
  struct sockaddr_in sockaddr; struct hostent *hostent;

  // Find an available network descriptor, aborting it none is found.
  for(loop0=0;loop0<10;loop0++) { if (sock[loop0] <= 0) { nd = loop0; } }
  if (nd == -1) { return -1; }

  // Set the host information.
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port   = htons(port);
  hostent = gethostbyname(hostname);
  sockaddr.sin_addr   = *(struct in_addr *)hostent->h_addr;

  // Create and connect the socket.
  if (type == SOCKET_TCP) { sock[nd] = socket(AF_INET, SOCK_STREAM, 0); }
  if (type == SOCKET_UDP) { sock[nd] = socket(AF_INET, SOCK_DGRAM,  0); }

  // Connect the socket to the remote host.
  connect(sock[nd], (struct sockaddr *)&sockaddr, sizeof(struct sockaddr));

  // End function.
  return nd;

 }

 int network_listen(int port, int type) { int loop0, nd = -1;
  struct sockaddr_in sockaddr;

  // Find an available network descriptor, aborting if none is found.
  for(loop0=0;loop0<10;loop0++) { if (sock[loop0] <= 0) { nd = loop0; } }
  if (nd == -1) { return -1; }

  // Set the sockaddr information.
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port   = htons(port);
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Create the socket.
  if (type == SOCKET_TCP) { sock[nd] = socket(AF_INET, SOCK_STREAM, 0); }
  if (type == SOCKET_UDP) { sock[nd] = socket(AF_INET, SOCK_DGRAM,  0); }

  // Bind to the socket.
  bind(sock[nd], (struct sockaddr *)&sockaddr, sizeof(struct sockaddr));

  // Listen for data on the socket.
  listen(sock[nd], 10);

  // End function.
  return nd;

 }

 int network_send(int nd, void *buffer, int size) {

  // Send data to the network.
  return send(sock[nd], buffer, size, 0);

 }

 int network_wait(int timeout) { int result = 0;
  int loop0; struct timeval tv;

  // Clear the watch set.
  FD_ZERO(&rfds);

  // Add the sockets to the watch set.
  for(loop0=0;loop0<10;loop0++) { if (sock[loop0] > 0) { FD_SET(sock[loop0], &rfds); } }

  // Set the timeout value.
  tv.tv_sec = timeout; tv.tv_usec = 0;

  // No timeout specified, wait forever.
  if (timeout < 0) { result = select(FD_SETSIZE, &rfds, NULL, NULL, NULL); }

  // Timeout specified, wait until time has elapsed.
  else { result = select(FD_SETSIZE, &rfds, NULL, NULL, &tv); }

  // End function.
  return result;

 }

 int network_recv(int nd, void *buffer, int size) {

  // Clear the buffer.
  memset(buffer, 0, size);

  // Receive some data from the network.
  if (FD_ISSET(sock[nd], &rfds)) { return recv(sock[nd], buffer, size, 0); }

  // End function.
  return 0;

 }

 int network_recvfrom(int nd, void *buffer, int size) {

  // Clear the buffer.
  memset(buffer, 0, size);

  // Receive data from the network.
  if (FD_ISSET(sock[nd], &rfds)) { return recvfrom(sock[nd], buffer, size, 0, NULL, NULL); }

  // End function.
  return 0;

 }

 int network_disconnect(int nd) { int result = 0;

  // Close the socket.
  result = close(sock[nd]);

  // Clear the descriptor.
  sock[nd] = -1;

  // End function.
  return result;

 }
