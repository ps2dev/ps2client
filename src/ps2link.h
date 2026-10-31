#ifndef __PS2LINK_H__
#define __PS2LINK_H__

 ///////////////////////
 // PS2LINK FUNCTIONS //
 ///////////////////////

 // Exit statuses of ps2client (sysexits.h values where one fits).
 #define PS2CLIENT_EXIT_OK		0
 #define PS2CLIENT_EXIT_USAGE		64
 #define PS2CLIENT_EXIT_NOCONNECT	69	// ps2link is not answering
 #define PS2CLIENT_EXIT_CONNLOST	74	// ps2link closed the connection
 #define PS2CLIENT_EXIT_PORTBUSY	75	// the console port is in use
 #define PS2CLIENT_EXIT_TIMEOUT		124	// idle timeout before the -e marker

 // What ps2link_connect sets up.
 #define PS2LINK_CONNECT_CONSOLE	1	// receive the console output
 #define PS2LINK_CONNECT_REQUEST	2	// serve host: file requests
 #define PS2LINK_CONNECT_COMMAND	4	// send commands to ps2link

 // Exit the main loop when a console line contains marker (NULL: never).
 void ps2link_set_marker(char *marker);

 // Returns 0, or PS2CLIENT_EXIT_PORTBUSY / PS2CLIENT_EXIT_NOCONNECT.
 int ps2link_connect(char *hostname, int what, int timeout_ms);

 // Wait until ps2link answers a command through its host: requests, which
 // shows that the EE side, the IOP side and the request connection all work.
 // Returns the round trip time in ms, or -1 on timeout.
 int ps2link_handshake(int timeout_ms);

 // Returns the exit status for ps2client.
 int ps2link_mainloop(int timeout);

 int ps2link_disconnect(void);

 ///////////////////////////////
 // PS2LINK COMMAND FUNCTIONS //
 ///////////////////////////////

 #define PS2LINK_COMMAND_RESET		0xBABE0201
 #define PS2LINK_COMMAND_EXECIOP	0xBABE0202
 #define PS2LINK_COMMAND_EXECEE		0xBABE0203
 #define PS2LINK_COMMAND_POWEROFF	0xBABE0204
 #define PS2LINK_COMMAND_SCRDUMP	0xBABE0205
 #define PS2LINK_COMMAND_NETDUMP	0xBABE0206
 #define PS2LINK_COMMAND_DUMPMEM	0xBABE0207
 #define PS2LINK_COMMAND_STARTVU	0xBABE0208
 #define PS2LINK_COMMAND_STOPVU		0xBABE0209
 #define PS2LINK_COMMAND_DUMPREG	0xBABE020A
 #define PS2LINK_COMMAND_GSEXEC		0xBABE020B
 #define PS2LINK_COMMAND_WRITEMEM	0xBABE020C
 #define PS2LINK_COMMAND_IOPEXCEP	0xBABE020D

 int ps2link_command_reset(void);

 // Returns -1 if the arguments do not fit in the command packet.
 int ps2link_command_execiop(int argc, char **argv);

 int ps2link_command_execee(int argc, char **argv);

 int ps2link_command_poweroff(void);

 int ps2link_command_scrdump(void);

 int ps2link_command_netdump(void);

 int ps2link_command_dumpmem(unsigned int offset, unsigned int size, char *pathname);

 int ps2link_command_startvu(int vu);

 int ps2link_command_stopvu(int vu);

 int ps2link_command_dumpreg(int type, char *pathname);

 int ps2link_command_gsexec(unsigned short size, char *pathname);

 int ps2link_command_writemem(unsigned int offset, unsigned int size, char *pathname);

 int ps2link_command_iopexcep(void);

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 #define PS2LINK_REQUEST_OPEN		0xBABE0111
 #define PS2LINK_REQUEST_CLOSE		0xBABE0121
 #define PS2LINK_REQUEST_READ		0xBABE0131
 #define PS2LINK_REQUEST_WRITE		0xBABE0141
 #define PS2LINK_REQUEST_LSEEK		0xBABE0151
 #define PS2LINK_REQUEST_OPENDIR	0xBABE0161
 #define PS2LINK_REQUEST_CLOSEDIR	0xBABE0171
 #define PS2LINK_REQUEST_READDIR	0xBABE0181
 #define PS2LINK_REQUEST_REMOVE		0xBABE0191
 #define PS2LINK_REQUEST_MKDIR		0xBABE01A1
 #define PS2LINK_REQUEST_RMDIR		0xBABE01B1
 #define PS2LINK_REQUEST_GETSTAT	0xBABE01C1

 int ps2link_request_open(void *packet);

 int ps2link_request_close(void *packet);

 int ps2link_request_read(void *packet);

 int ps2link_request_write(void *packet);

 int ps2link_request_lseek(void *packet);

 int ps2link_request_opendir(void *packet);

 int ps2link_request_closedir(void *packet);

 int ps2link_request_readdir(void *packet);

 int ps2link_request_remove(void *packet);

 int ps2link_request_mkdir(void *packet);

 int ps2link_request_rmdir(void *packet);

 int ps2link_request_getstat(void *packet);

 ////////////////////////////////
 // PS2LINK RESPONSE FUNCTIONS //
 ////////////////////////////////

 #define PS2LINK_RESPONSE_OPEN		0xBABE0112
 #define PS2LINK_RESPONSE_CLOSE		0xBABE0122
 #define PS2LINK_RESPONSE_READ		0xBABE0132
 #define PS2LINK_RESPONSE_WRITE		0xBABE0142
 #define PS2LINK_RESPONSE_LSEEK		0xBABE0152
 #define PS2LINK_RESPONSE_OPENDIR	0xBABE0162
 #define PS2LINK_RESPONSE_CLOSEDIR	0xBABE0172
 #define PS2LINK_RESPONSE_READDIR	0xBABE0182
 #define PS2LINK_RESPONSE_REMOVE	0xBABE0192
 #define PS2LINK_RESPONSE_MKDIR		0xBABE01A2
 #define PS2LINK_RESPONSE_RMDIR		0xBABE01B2
 #define PS2LINK_RESPONSE_GETSTAT	0xBABE01C2

 int ps2link_response_open(int result);

 int ps2link_response_close(int result);

 int ps2link_response_read(int result, int size);

 int ps2link_response_write(int result);

 int ps2link_response_lseek(int result);

 int ps2link_response_opendir(int result);

 int ps2link_response_closedir(int result);

 int ps2link_response_readdir(int result, unsigned int mode, unsigned int attr, unsigned int size, unsigned char *ctime, unsigned char *atime, unsigned char *mtime, unsigned int hisize, char *name);

 int ps2link_response_remove(int result);

 int ps2link_response_mkdir(int result);

 int ps2link_response_rmdir(int result);

 int ps2link_response_getstat(
   int result,
   unsigned int mode,
   unsigned int attr,
   unsigned int size,
   unsigned char *ctime,
   unsigned char *atime,
   unsigned char *mtime,
   unsigned int hisize
);

 //////////////////////////////
 // PS2LINK THREAD FUNCTIONS //
 //////////////////////////////

 void *ps2link_thread_console(void *thread_id);

 void *ps2link_thread_request(void *thread_id);

#endif
