/* Portable POSIX ps2client smoke tests: no actual PS2 or namespaces needed. */
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static const char *client;
static int failures;
static void check(int ok, const char *name)
{
    printf("%s %s\n", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}
static void pause_ms(long ms)
{
    struct timespec ts = {ms / 1000, (ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}
static pid_t spawn(char *const arguments[], int output_fd, const char *hostname)
{
    pid_t pid = fork();
    if (pid == 0) {
        if (output_fd >= 0) {
            dup2(output_fd, STDOUT_FILENO);
            dup2(output_fd, STDERR_FILENO);
        }
        if (hostname) setenv("PS2HOSTNAME", hostname, 1);
        execv(client, arguments);
        _exit(127);
    }
    return pid;
}
static int status_code(pid_t pid)
{
    int status;
    if (pid < 0 || waitpid(pid, &status, 0) < 0 || !WIFEXITED(status))
        return -1;
    return WEXITSTATUS(status);
}
static int run(char *const args[], const char *hostname, char *text,
               size_t capacity)
{
    char filename[] = "/tmp/ps2client-smoke-XXXXXX";
    int fd = mkstemp(filename), rc;
    ssize_t n;
    pid_t pid;
    if (fd < 0) return -1;
    unlink(filename);
    pid = spawn(args, fd, hostname);
    rc = status_code(pid);
    lseek(fd, 0, SEEK_SET);
    n = read(fd, text, capacity - 1); /* Flawfinder: ignore */
    text[n < 0 ? 0 : (size_t)n] = '\0';
    close(fd);
    return rc;
}
static void test_console(void)
{
    struct sockaddr_in addr = {0};
    char filename[] = "/tmp/ps2console-XXXXXX", text[4096];
    char *const args[] = {(char *)client, "-h", "1.1.l.1", "-t", "3",
                           "-e", "smoke-console-line", "console", NULL};
    int fd = mkstemp(filename), sock = socket(AF_INET, SOCK_DGRAM, 0), i, rc;
    pid_t pid;
    ssize_t n;
    if (fd < 0 || sock < 0) { check(0, "console setup"); return; }
    unlink(filename);
    pid = spawn(args, fd, NULL);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0x4712);
    addr.sin_addr.s_addr = htonl(0x7f000001u);
    pause_ms(100);
    for (i = 0; i < 15; i++) {
        sendto(sock, "smoke-console-line\n", 19, 0,
               (struct sockaddr *)&addr, sizeof(addr));
        pause_ms(45);
    }
    close(sock);
    rc = status_code(pid);
    lseek(fd, 0, SEEK_SET);
    n = read(fd, text, sizeof(text) - 1); /* Flawfinder: ignore */
    text[n < 0 ? 0 : (size_t)n] = '\0';
    close(fd);
    check(rc == 0 && strstr(text, "smoke-console-line"),
          "UDP-only console logs with invalid PS2 hostname");
}
static void test_reset(void)
{
    struct sockaddr_in addr = {0};
    char result[1024];
    unsigned char packet[32];
    char *const args[] = {(char *)client, "-h", "127.0.0.1", "reset", NULL};
    int fd = socket(AF_INET, SOCK_DGRAM, 0), rc;
    struct timeval tv = {2, 0};
    ssize_t n;
    if (fd < 0) { check(0, "reset socket"); return; }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0x4712);
    addr.sin_addr.s_addr = htonl(0x7f000001u);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        check(0, "reset bind"); close(fd); return;
    }
    rc = run(args, NULL, result, sizeof(result));
    n = recv(fd, packet, sizeof(packet), 0);
    close(fd);
    check(rc == 0 && n == 6 && packet[0] == 0xba && packet[1] == 0xbe &&
          packet[2] == 0x02 && packet[3] == 0x01 &&
          packet[4] == 0 && packet[5] == 6,
          "reset uses command UDP without TCP");
}
int main(int argc, char **argv)
{
    char text[4096];
    int rc;
    char *invalid_opt[] = {NULL, "-h", "1.1.l.1", "ping", NULL};
    char *invalid_env[] = {NULL, "ping", NULL};
    char *listen_args[] = {NULL, "-h", "1.1.l.1", "-t", "1", "listen", NULL};
    if (argc != 2) { fprintf(stderr, "Usage: %s /path/to/ps2client\n", argv[0]); return 2; }
    client = argv[1];
    invalid_opt[0] = (char *)client;
    invalid_env[0] = (char *)client;
    listen_args[0] = (char *)client;
    rc = run(invalid_opt, NULL, text, sizeof(text));
    check(rc == 69 && strstr(text, "1.1.l.1"), "invalid CLI hostname");
    rc = run(invalid_env, "1.1.l.1", text, sizeof(text));
    check(rc == 69 && strstr(text, "1.1.l.1"), "invalid environment hostname");
    rc = run(listen_args, NULL, text, sizeof(text));
    check(rc == 69, "legacy listen requires host: service");
    test_console();
    test_reset();
    return failures ? 1 : 0;
}
