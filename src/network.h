#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef _WIN32
 #include <winsock2.h>
#else
 #include <sys/socket.h>
#endif

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

#ifdef _WIN32
 int network_startup(void);
#endif

 int network_connect(char *hostname, int port, int type);

 int network_listen(int port, int type);

 int network_accept(int sock);

 int network_send(int sock, void *buffer, int size);

 int network_wait(int sock, int timeout);

 int network_receive(int sock, void *buffer, int size);

 int network_receive_all(int sock, void *buffer, int size);

 int network_disconnect(int sock);

#endif
