
#ifndef _WIN32
 #include <errno.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <signal.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <sys/time.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
#endif

 #include <string.h>

 #include "network.h"

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

 int network_startup(void) {
#ifdef _WIN32
  WSADATA wsaData;

  // Start up winsock.
  if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) { return -1; }

#else
  // A send on a connection the ps2 has closed must fail, not kill us.
  signal(SIGPIPE, SIG_IGN);
#endif
  // End function.
  return 0;
 }

 static int network_set_blocking(int sock, int blocking) {
#ifdef _WIN32
  u_long mode = blocking ? 0 : 1;

  return ioctlsocket(sock, FIONBIO, &mode) == 0 ? 0 : -1;
#else
  int flags = fcntl(sock, F_GETFL, 0); if (flags < 0) { return -1; }

  if (blocking) { flags &= ~O_NONBLOCK; } else { flags |= O_NONBLOCK; }

  return fcntl(sock, F_SETFL, flags);
#endif
 }

 int network_connect(char *hostname, int port, int type) {

  // Connect without a timeout.
  return network_connect_timeout(hostname, port, type, -1);

 }

 int network_connect_timeout(char *hostname, int port, int type, int timeout_ms) { int sock = -1;
  struct sockaddr_in sockaddr; struct hostent *host;

  // Resolve the hostname.
  host = gethostbyname(hostname); if (host == NULL || host->h_addr == NULL) { return -1; }

  // Populate the sockaddr structure.
  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  sockaddr.sin_addr = *(struct in_addr *)host->h_addr;

  // Open the socket.
  sock = socket(AF_INET, type, 0); if (sock < 0) { return -1; }

  // Disable Nagle on the request channel: ps2link's fileio is a synchronous
  // sequence of small request/response exchanges, and Nagle on this side
  // colliding with the PS2's delayed ACKs stalls every exchange ~200ms
  // (observed ~4 KB/s file serving; with TCP_NODELAY it is wire-speed).
  if (type == SOCK_STREAM) {
   int nodelay = 1;
   setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&nodelay, sizeof(nodelay));
  }

  // Without a timeout, connect the socket and wait as long as it takes.
  if (timeout_ms < 0) {
   if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0) { network_disconnect(sock); return -2; }
   return sock;
  }

  // Else, start a non-blocking connect and wait for it until the timeout.
  if (network_set_blocking(sock, 0) < 0) { network_disconnect(sock); return -1; }
  if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0) {
   fd_set wfds, efds; struct timeval tv; int error = 0; socklen_t len = sizeof(error);

#ifdef _WIN32
   if (WSAGetLastError() != WSAEWOULDBLOCK) { network_disconnect(sock); return -2; }
#else
   if (errno != EINPROGRESS) { network_disconnect(sock); return -2; }
#endif

   FD_ZERO(&wfds); FD_SET(sock, &wfds);
   FD_ZERO(&efds); FD_SET(sock, &efds);
   tv.tv_sec = timeout_ms / 1000; tv.tv_usec = (timeout_ms % 1000) * 1000;

   // Timed out, or the connect failed.
   if (select(sock + 1, NULL, &wfds, &efds, &tv) <= 0) { network_disconnect(sock); return -2; }
   if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&error, &len) < 0 || error != 0) { network_disconnect(sock); return -2; }
  }
  if (network_set_blocking(sock, 1) < 0) { network_disconnect(sock); return -1; }

  // Return the socket.
  return sock;

 }

 int network_listen(int port, int type) { int sock = -1;
  struct sockaddr_in sockaddr;

  // Populate the sockaddr structure.
  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Create the socket.
  sock = socket(AF_INET, type, 0); if (sock < 0) { return -1; }

  // Bind the socket.
  if (bind(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr)) < 0) { network_disconnect(sock); return -2; }

  // Return the socket.
  return sock;

 }

 int network_send(int sock, void *buffer, int size) { int total = 0;

  // Keep sending data until it has all been sent.
  while (total < size) {
   int sent = send(sock, &((char *)buffer)[total], size - total, 0);

   // The connection is gone.
   if (sent <= 0) {
#ifndef _WIN32
    if (sent < 0 && errno == EINTR) { continue; }
#endif
    return -1;
   }

   total += sent;
  }

  // Return the total bytes sent.
  return total;

 }

 int network_wait(int sock, int timeout) {
  fd_set nfds; struct timeval tv;

  // Initialize the rdfs structure.
  FD_ZERO(&nfds); FD_SET(sock, &nfds);

  // Populate the tv structure.
  tv.tv_sec = timeout; tv.tv_usec = 0;

  // No timeout was specified, so wait forever.
  if (timeout < 0) { return select(sock + 1, &nfds, NULL, NULL, NULL); }

  // A timeout was specified, so wait until the time has elapsed.
  else { return select(sock + 1, &nfds, NULL, NULL, &tv); }

  // End function.
  return 0;

 }

 int network_receive(int sock, void *buffer, int size) {

  // Receive the data from the socket.
  return recvfrom(sock, buffer, size, 0, NULL, NULL);

 }

 int network_receive_all(int sock, void *buffer, int size) { int total = 0;

  // Receive the data from the socket.
  while (total < size) {
   int received = recvfrom(sock, &((char *)buffer)[total], size - total, 0, NULL, NULL);

   // The connection was closed (0) or failed (<0).
   if (received <= 0) {
#ifndef _WIN32
    if (received < 0 && errno == EINTR) { continue; }
#endif
    return -1;
   }

   total += received;
  }

  // Return the total bytes received.
  return total;

 }

 int network_disconnect(int sock) {

  // Close the socket.
#ifdef _WIN32
  if (closesocket(sock) < 0) { return -1; }
#else
  if (close(sock) < 0) { return -1; }
#endif

  // End function.
  return 0;

 }
