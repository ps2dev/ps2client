#ifndef __PS2LINK_H__
#define __PS2LINK_H__

 ////////////////////////////
 // PS2LINK MAIN FUNCTIONS //
 ////////////////////////////

 int ps2link_connect(char *hostname);

 int ps2link_mainloop(int timeout);

 int ps2link_disconnect(void);

 ///////////////////////////////
 // PS2LINK TOOLBOX FUNCTIONS //
 ///////////////////////////////

 int ps2link_fixflags(int flags);

 int ps2link_fixpathname(char *pathname);

 ///////////////////////////////
 // PS2LINK COMMAND FUNCTIONS //
 ///////////////////////////////

 int ps2link_send_command(char *hostname, void *packet, int size);

 int ps2link_command_reset(char *hostname);

 int ps2link_command_execiop(char *hostname, int timeout, int argc, char *argv);

 int ps2link_command_execee(char *hostname, int timeout, int argc, char *argv);

 int ps2link_command_poweroff(char *hostname);

 int ps2link_command_dumpmem(char *hostname, int timeout, char *pathname, int offset, int size);

 int ps2link_command_startvu(char *hostname, int vu);

 int ps2link_command_stopvu(char *hostname, int vu);

 int ps2link_command_dumpreg(char *hostname, int timeout, char *pathname, int type);

 int ps2link_command_gsexec(char *hostname, int timeout, char *pathname);

 int ps2link_command_listen(char *hostname, int timeout);

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
