#ifndef __PS2LINK_H__
#define __PS2LINK_H__

 ///////////////////////
 // PS2LINK FUNCTIONS //
 ///////////////////////

 int ps2link_mainloop(char *hostname);

 int ps2link_listenloop(void);

 int ps2link_fixflags(int flags);

 int ps2link_fixpathname(char *pathname);

 ///////////////////////////////
 // PS2LINK COMMAND FUNCTIONS //
 ///////////////////////////////

 int ps2link_send_command(char *hostname, void *packet, int size);

 int ps2link_command_reset(char *hostname);

 int ps2link_command_execiop(char *hostname, char *pathname);

 int ps2link_command_execee(char *hostname, char *pathname);

 int ps2link_command_poweroff(char *hostname);

 int ps2link_command_dumpmem(char *hostname, char *pathname, int offset, int size);

 int ps2link_command_startvu(char *hostname, int vpu);

 int ps2link_command_stopvu(char *hostname, int vpu);

 int ps2link_command_dumpreg(char *hostname, char *pathname, int type);

 int ps2link_command_gsexec(char *hostname, char *pathname, int size);

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_request_open(void *request);

 int ps2link_request_close(void *request);

 int ps2link_request_read(void *request);

 int ps2link_request_write(void *request);

 int ps2link_request_lseek(void *request);

 int ps2link_request_dopen(void *request);

 int ps2link_request_dclose(void *request);

 int ps2link_request_dread(void *request);

#endif
