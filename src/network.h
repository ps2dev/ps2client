#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef _WIN32
 #include <winsock2.h>
 #include <ws2tcpip.h>
 #include <windows.h>
#else
 #include <sys/socket.h>
#endif

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

 int network_startup(void);

 int network_connect(char *hostname, int port, int type);

 // Like network_connect, but give up after timeout_ms (-1: no timeout).
 // Returns -1 if the hostname does not resolve, -2 if the connect fails.
 int network_connect_timeout(char *hostname, int port, int type, int timeout_ms);

 // Returns -2 if the port is in use.
 int network_listen(int port, int type);

 int network_send(int sock, void *buffer, int size);

 int network_wait(int sock, int timeout);

 int network_receive(int sock, void *buffer, int size);

 // Returns -1 if the connection was closed or failed before size bytes came in.
 int network_receive_all(int sock, void *buffer, int size);

 int network_disconnect(int sock);

#endif
