#ifndef __UTILITY_H__
#define __UTILITY_H__

 #define PACKED __attribute__((packed))

 ///////////////////////
 // UTILITY FUNCTIONS //
 ///////////////////////

 int fix_flags(int flags);

 int fix_pathname(char *pathname);

 // Pack argv as null-terminated strings into destination; -1 if they do not fit.
 int fix_argv(char *destination, int size, char **argv);

 // Monotonic time in milliseconds.
 long long time_ms(void);

 void sleep_ms(int ms);

 int print_usage(void);

#endif
