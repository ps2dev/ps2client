#ifndef __PS2LINK_H__
#define __PS2LINK_H__

 ///////////////////////
 // PS2LINK FUNCTIONS //
 ///////////////////////

 int ps2link_open(char *hostname);

 int ps2link_loop(void);

 int ps2link_shut(void);

 int ps2link_flags(int flags);

 int ps2link_slash(char *pathname);

 ///////////////////////////////
 // PS2LINK COMMAND FUNCTIONS //
 ///////////////////////////////

 int ps2link_cmd_reset(void);

 int ps2link_cmd_poweroff(void);

 int ps2link_cmd_execiop(char *pathname);

 int ps2link_cmd_execee(char *pathname);

 ///////////////////////////////
 // PS2LINK REQUEST FUNCTIONS //
 ///////////////////////////////

 int ps2link_req_open(void *request);

 int ps2link_req_close(void *request);

 int ps2link_req_read(void *request);

 int ps2link_req_write(void *request);

 int ps2link_req_lseek(void *request);

#endif
