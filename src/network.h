#ifndef __NETWORK_H__
#define __NETWORK_H__

 ///////////////////////
 // NETWORK FUNCTIONS //
 ///////////////////////

 int network_open(int *sock, char *addr, int port, int type);

 int network_serv(int *sock, int port, int type);

 int network_send(int *sock, void *data, int size);

 int network_recv(int *sock, void *data, int size);

 int network_recvfrom(int *sock, void *data, int size);

 int network_shut(int *sock);

#endif
