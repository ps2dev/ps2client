#ifndef PS2CLIENT_RESETCHECK_H
#define PS2CLIENT_RESETCHECK_H

/* Opt-in host-only reset observer, compatible with existing PS2Link. */
int ps2client_resetcheck(const char *executable, const char *hostname,
                         int timeout_seconds);

#endif
