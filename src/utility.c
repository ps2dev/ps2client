
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "utility.h"

///////////////////////
// UTILITY FUNCTIONS //
///////////////////////

int fix_flags(int flags)
{
    int result = 0;

    // Fix the flags.
    if (flags & 0x0001) {
        result |= O_RDONLY;
    }
    if (flags & 0x0002) {
        result |= O_WRONLY | O_TRUNC;
    }  // FIXME: Truncate is needed for some programs.
#ifndef _WIN32
    if (flags & 0x0010) {
        result |= O_NONBLOCK;
    }
#endif
    if (flags & 0x0100) {
        result |= O_APPEND;
    }
    if (flags & 0x0200) {
        result |= O_CREAT;
    }
    if (flags & 0x0400) {
        result |= O_TRUNC;
    }

#ifdef _WIN32

    // Binary mode file access.
    result |= O_BINARY;

#endif

    // End function.
    return result;
}

int fix_pathname(char *pathname)
{
    int loop0 = 0;

    // If empty, set a pathname default.
    if (pathname[0] == 0) {
        strcpy(pathname, ".");
    }

    // Convert \ to / for unix compatibility.
    for (loop0 = 0; loop0 < strlen(pathname); loop0++) {
        if (pathname[loop0] == '\\') {
            pathname[loop0] = '/';
        }
    }

    // If a leading slash is found...
    if ((pathname[0] == '/') && (pathname[1] != 0)) {

        // Kill off the leading slash.
        for (loop0 = 0; loop0 < strlen(pathname) - 1; loop0++) {
            pathname[loop0] = pathname[loop0 + 1];
        }

        // Terminate the pathname with a null.
        pathname[loop0] = 0;
    }

    // End function.
    return 0;
}

int fix_argv(char *destination, char **argv)
{
    int loop0 = 0;

    // For each argv...
    for (loop0 = 0; argv[loop0]; loop0++) {

        // Copy the argv to the destination.
        memcpy(destination, argv[loop0], strlen(argv[loop0]));

        // Increment the destination pointer.
        destination += strlen(argv[loop0]);

        // Null-terminate the argv.
        *destination = 0;

        // Increment the destination pointer.
        destination += 1;
    }

    // End function.
    return 0;
}

int print_usage(void)
{

    // Print out the ps2client usage string.
    printf("\n");
    printf(" Usage: ps2client [-h hostname] [-t timeout] <command> [arguments]\n\n");
    printf(" Available commands:\n\n");
    printf("   reset\n");
    printf("   execiop <filename> [arguments]\n");
    printf("   execee <filename> [arguments]\n");
    printf("   poweroff\n");
    printf("   scrdump\n");
    printf("   netdump\n");
    printf("   dumpmem <offset> <size> <filename>\n");
    printf("   startvu <vu>\n");
    printf("   stopvu <vu>\n");
    printf("   dumpreg <type> <filename>\n");
    printf("   gsexec <size> <filename>\n");
    printf("   writemem <offset> <size> <filename>\n");
    printf("   iopexcep\n");
    printf("   listen\n\n");

    // Print out the fsclient usage string.
    printf(" Usage: fsclient [-h hostname] <command> [arguments]\n\n");
    printf(" Available commands:\n\n");
    printf("   copyfrom <source> <destination>\n");
    printf("   copyto <source> <destination>\n");
    printf("   delete <pathname>\n");
    printf("   devlist\n");
    printf("   dir <pathname>\n");
    printf("   mkdir <pathname>\n");
    printf("   mount <device> <fsname>\n");
    printf("   rmdir <pathname>\n");
    printf("   sync <device>\n");
    printf("   umount <device>\n\n");

    // End function.
    return 0;
}
