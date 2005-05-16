#ifndef __UTILITY_H__
#define __UTILITY_H__

 #define PACKED __attribute__((packed))

 ///////////////////////
 // UTILITY FUNCTIONS //
 ///////////////////////

 int fix_flags(int flags);

 int fix_pathname(char *pathname);

 int fix_argv(char *destination, char **argv);

 int print_usage(void);

#endif
