
 #include <stdio.h>
 #include <netdb.h>
 #include <fcntl.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>

 #include "types.h"
 #include "network.h"
 #include "ps2link.h"

 #ifndef MSG_DONTWAIT
 #define MSG_DONTWAIT	0x40
 #endif

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

 int network_open(int *sock, char *addr, int port, int type) {
  struct sockaddr_in sockaddr; struct hostent *hostent;

  // Create a network socket of the type requested.
  if (type == 0x00) { *sock = socket(AF_INET, SOCK_STREAM, 0); } else
  if (type == 0x01) { *sock = socket(AF_INET, SOCK_DGRAM,  0); } else { return -1; }

  // Get the host information.
  hostent = gethostbyname(addr);

  // Fill the socket address structure.
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr = *(struct in_addr *)hostent->h_addr;
  sockaddr.sin_port = htons(port);

  // Open the network connection.
  return connect(*sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr));

 }

 int network_serv(int *sock, int port, int type) {
  struct sockaddr_in sockaddr;

  // Create a network socket of the type requested.
  if (type == 0x00) { *sock = socket(AF_INET, SOCK_STREAM, 0); } else
  if (type == 0x01) { *sock = socket(AF_INET, SOCK_DGRAM,  0); } else { return -1; }

  // Turn off socket blocking.
  fcntl(*sock, F_SETFL, O_NONBLOCK);

  // Fill in the socket address structure.
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sockaddr.sin_port = htons(port);

  // Bind to the network address.
  bind(*sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr));

  // Start listening for data.
  return listen(*sock, 10);

 }

 int network_send(int *sock, void *data, int size) {

  // Send the network data.
  return send(*sock, data, size, 0);

 }

 int network_recv(int *sock, void *data, int size) {

  // Clear the packet.
  memset(data, 0, size);

  // Receive some network data.
  return recv(*sock, data, size, MSG_DONTWAIT);

 }

 int network_recvfrom(int *sock, void *data, int size) { int result = 0;
  struct sockaddr_in sockaddr; int length;

  // Clear the packet.
  memset(data, 0, size);

  // Receive some network data.
  result = recvfrom(*sock, data, size, 0, (struct sockaddr *)&sockaddr, (socklen_t *)&length);

  // End function.
  return result;

 }

 int network_shut(int *sock) {

  // Close the network socket.
  return close(*sock);

 }
