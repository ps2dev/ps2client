#ifndef __NETWORK_H__
#define __NETWORK_H__

 /////////////////////
 // NETWORK DEFINES //
 /////////////////////

 #define SOCKET_TCP	0x0000
 #define SOCKET_UDP	0x0001

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

 int network_connect(char *hostname, int port, int type);

 int network_listen(int port, int type);

 int network_send(int nd, void *buffer, int size);

 int network_wait(int timeout);

 int network_recv(int nd, void *buffer, int size);

 int network_recvfrom(int nd, void *buffer, int size);

 int network_close(int nd);

#endif
