
 #include <stdio.h>
 #include <string.h>
#ifdef _WIN32
 #include <windows.h>
 #include <winsock2.h>
#else
 #include <stdlib.h>
 #include <netdb.h>
 #include <unistd.h>
 #include <sys/time.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
#endif
 #include "network.h"

 int sock[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

 fd_set rfds;

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

#ifdef _WIN32
 int network_startup() {
  WSADATA wsaData;
  
  if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) { printf("[ERROR] Unable to startup winsock\n"); return -1; }
  
  return 0;
 }
#endif

 int network_connect(char *hostname, int port, int type) { int result = 0;
  struct sockaddr_in sockaddr; int loop0, nd = -1;

  // Allocate the network descriptor.
  for(loop0=0;loop0<10;loop0++) { if (sock[loop0] <= 0) { nd = loop0; } }
  if (nd < 0) { printf("[ERROR] Unable to allocate network descriptor. (%d)\n", nd); return -1; }

  // Set the sockaddr information.
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  sockaddr.sin_addr = *(struct in_addr *)gethostbyname(hostname)->h_addr;

  // Create the socket.
  if (type == SOCKET_TCP) { sock[nd] = socket(AF_INET, SOCK_STREAM, 0); }
  if (type == SOCKET_UDP) { sock[nd] = socket(AF_INET, SOCK_DGRAM,  0); }
  if (sock[nd] < 0) { printf("[ERROR] Create socket failed. (%d)\n", result); return -2; }

  // Connect the socket.
  result = connect(sock[nd], (struct sockaddr *)&sockaddr, sizeof(struct sockaddr));
  if (result < 0) { printf("[ERROR] Connect socket failed. (%d)\n", result); return -3; }

  // End function.
  return nd;

 }

 int network_listen(int port, int type) { int result = 0;
  struct sockaddr_in sockaddr; int loop0, nd = -1;

  // Find an available network descriptor.
  for(loop0=0;loop0<10;loop0++) { if (sock[loop0] <= 0) { nd = loop0; } }
  if (nd < 0) { printf("[ERROR] Unable to allocate network descriptor. (%d)\n", nd); return -1; }

  // Set the sockaddr information.
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Create the socket.
  if (type == SOCKET_TCP) { sock[nd] = socket(AF_INET, SOCK_STREAM, 0); }
  if (type == SOCKET_UDP) { sock[nd] = socket(AF_INET, SOCK_DGRAM,  0); }
  if (sock[nd] < 0) { printf("[ERROR] Create socket failed. (%d)\n", result); return -2; }

  // Bind the socket.
  result = bind(sock[nd], (struct sockaddr *)&sockaddr, sizeof(struct sockaddr));
  if (result < 0) { printf("[ERROR] Bind socket failed. (%d)\n", result); return -3; }

  // End function.
  return nd;

 }

 int network_send(int nd, void *buffer, int size) { int result = 0;

  // Send the data.
  result = send(sock[nd], buffer, size, 0);
  if (result < 0) { printf("[ERROR] Send data failed. (%d)\n", result); return -1; }

  // End function.
  return result;

 }

 int network_sendall(int nd, void *buffer, int size) { int result = 0;
  int total = 0; char *temp = buffer;

  // Repeat until all the data has been sent.
  while (total < size) {

   // Send all the data.
   result = send(sock[nd], temp, size - total, 0);
   if (result < 0) { printf("[ERROR] Error sending all data. (%d)\n", result); return -1; }

   // Increment the counters.
   if (result > 0) { total += result; temp += result; }

  }

  // End function.
  return total;

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

 int network_recv(int nd, void *buffer, int size) { int result = 0;

  // Clear the buffer.
  memset(buffer, 0, size);

  // Check to see if data is available.
  if (!FD_ISSET(sock[nd], &rfds)) { return 0; }

  // Read the data.
  result = recv(sock[nd], buffer, size, 0);
  if (result < 0) { printf("[ERROR] Read data failed. (%d)\n", result); return -1; }

  // End function.
  return result;

 }

 int network_recvall(int nd, void *buffer, int size) { int result = 0;
  int total = 0; char *temp = buffer;

  // Clear the buffer.
  memset(buffer, 0, size);

  // Check to see if data is available.
  if (!FD_ISSET(sock[nd], &rfds)) { return 0; }

  // Repeat until all the data has been read.
  while (total < size) {

   // Read all the data.
   result = recv(sock[nd], temp, size - total, 0);
   if (result < 0) { printf("[ERROR] Error reading all data. (%d)\n", result); return -1; }

   // Increment the counters.
   if (result > 0) { total += result; temp += result; }

  }

  // End function.
  return total;

 }

 int network_recvfrom(int nd, void *buffer, int size) { int result = 0;

  // Clear the buffer.
  memset(buffer, 0, size);

  // Check to see if data is available.
  if (!FD_ISSET(sock[nd], &rfds)) { return 0; }

  // Read the data.
  result = recvfrom(sock[nd], buffer, size, 0, NULL, NULL);
  if (result < 0) { printf("[ERROR] Read data failure. (%d)\n", result); return -1; }

  // End function.
  return result;

 }

 int network_recvallfrom(int nd, void *buffer, int size) { int result = 0;
  int total = 0; char *temp = buffer;

  // Clear the buffer.
  memset(buffer, 0, size);

  // Check to see if data is available.
  if (!FD_ISSET(sock[nd], &rfds)) { return 0; }

  // Repeat until all the data has been read.
  while (total < size) {

   // Read all the data.
   result = recvfrom(sock[nd], temp, size - total, 0, NULL, NULL);
   if (result < 0) { printf("[ERROR] Error reading all data. (%d)\n", result); return -1; }

   // Increment the counters.
   if (result > 0) { total += result; temp += result; }

  }

  // End function.
  return total;

 }

 int network_disconnect(int nd) { int result = 0;

  // Close the socket.
#ifdef _WIN32
  result = closesocket(sock[nd]);
#else
  result = close(sock[nd]);
#endif
  if (result < 0) { printf("[ERROR] Close socket failed. (%d)\n", result); return -1; }

  // Clear the descriptor.
  sock[nd] = -1;

  // End function.
  return result;

 }
