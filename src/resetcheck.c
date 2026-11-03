#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <process.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "ps2link.h"
#include "resetcheck.h"
#include "utility.h"

/* Separate processes avoid changing ps2client's legacy network/thread state. */
static int run_command(const char *executable, const char *hostname,
                       const char *name, int quiet)
{
    char *const args[] = {
        (char *)executable, "-h", (char *)hostname, "-t", "1",
        (char *)name, NULL
    };
#ifdef _WIN32
    intptr_t status;
    (void)quiet;
    status = _spawnvp(_P_WAIT, executable, (const char *const *)args);
    return status < 0 ? -1 : (int)status;
#else
    pid_t pid = fork();
    int status;
    if (pid < 0) return -1;
    if (pid == 0) {
        if (quiet) {
            int devnull = open("/dev/null", O_WRONLY);
            if (devnull >= 0) {
                dup2(devnull, STDOUT_FILENO);
                dup2(devnull, STDERR_FILENO);
                close(devnull);
            }
        }
        execvp(executable, args);
        _exit(127);
    }
    do {
        if (waitpid(pid, &status, 0) >= 0) break;
        if (errno != EINTR) return -1;
    } while (1);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

int ps2client_resetcheck(const char *executable, const char *hostname,
                         int timeout_seconds)
{
    long long deadline;
    int saw_outage = 0;
    int status;

    if (timeout_seconds < 1 || timeout_seconds > 120) {
        fprintf(stderr, "resetcheck requires a -t timeout between 1 and 120 seconds.\n");
        return PS2CLIENT_EXIT_USAGE;
    }

    fprintf(stderr, "resetcheck: checking the EE/IOP round trip before RESET...\n");
    if (run_command(executable, hostname, "ping", 1) != PS2CLIENT_EXIT_OK) {
        fprintf(stderr, "resetcheck: PS2Link is not responding; RESET was NOT sent.\n");
        return PS2CLIENT_EXIT_NOCONNECT;
    }

    fprintf(stderr, "resetcheck: sending one legacy UDP RESET request...\n");
    status = run_command(executable, hostname, "reset", 1);
    if (status != PS2CLIENT_EXIT_OK) {
        fprintf(stderr, "resetcheck: failed to send RESET (%d).\n", status);
        return PS2CLIENT_EXIT_NOCONNECT;
    }

    /* A UDP send has no delivery acknowledgment. Observing a loss and
     * restoration of the EE host: round trip is consistent with a restart,
     * not proof that the reset command was processed. */
    deadline = time_ms() + (long long)timeout_seconds * 1000;
    while (time_ms() < deadline) {
        status = run_command(executable, hostname, "ping", 1);
        if (status != PS2CLIENT_EXIT_OK) {
            if (!saw_outage) {
                saw_outage = 1;
                fprintf(stderr, "resetcheck: PS2Link round trip went offline.\n");
            }
        } else if (saw_outage) {
            fprintf(stderr, "resetcheck: PS2Link round trip recovered.\n");
            fprintf(stderr, "resetcheck: recovery observed (not a reset acknowledgment).\n");
            return PS2CLIENT_EXIT_OK;
        }
        if (time_ms() < deadline) sleep_ms(150);
    }
    if (!saw_outage)
        fprintf(stderr, "resetcheck: no offline transition observed; reset UNCONFIRMED.\n");
    else
        fprintf(stderr, "resetcheck: PS2Link did not recover; reset UNCONFIRMED.\n");
    return PS2CLIENT_EXIT_TIMEOUT;
}
