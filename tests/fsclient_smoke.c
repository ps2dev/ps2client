/* Fake ps2netfs loopback tests for short transfers and malformed replies. */
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#define PORT 0x4713
#define OPEN 0xBEEF8011u
#define CLOSE 0xBEEF8021u
#define READ 0xBEEF8031u
#define WRITE 0xBEEF8041u
#define LSEEK 0xBEEF8051u

struct server {
    int listener;
    int mode; /* 1 short reads, 2 short writes, 3 malformed read */
    int errors;
    size_t position, bytes_received;
    char received[128];
};
static const char from_content[] = "partial-read-content";
static const char to_content[] = "partial-write-content";

static uint32_t get32(const unsigned char *p)
{
    uint32_t v;
    memcpy(&v, p, sizeof(v));
    return ntohl(v);
}
static void set32(unsigned char *p, uint32_t v)
{
    v = htonl(v);
    memcpy(p, &v, sizeof(v));
}
static int read_all(int s, void *buffer, size_t size)
{
    size_t done = 0;
    while (done < size) {
        ssize_t n = recv(s, (char *)buffer + done, size - done, 0);
        if (n <= 0) return -1;
        done += (size_t)n;
    }
    return 0;
}
static int send_all(int s, const void *buffer, size_t size)
{
    size_t done = 0;
    while (done < size) {
        ssize_t n = send(s, (const char *)buffer + done, size - done, 0);
        if (n <= 0) return -1;
        done += (size_t)n;
    }
    return 0;
}
static void *serve(void *context)
{
    struct server *state = context;
    unsigned char hdr[6], packet[1024], reply[14];
    int client = accept(state->listener, NULL, NULL);
    if (client < 0) { state->errors++; return NULL; }
    for (;;) {
        uint32_t command, result = 0;
        uint16_t length;
        const char *data = NULL;
        size_t data_len = 0;
        if (read_all(client, hdr, sizeof(hdr)) < 0) break;
        command = get32(hdr);
        memcpy(&length, hdr + 4, 2); length = ntohs(length);
        if (length < 6 || length - 6 > sizeof(packet) ||
            read_all(client, packet, length - 6) < 0) {
            state->errors++; break;
        }
        if (command == OPEN) {
            if (state->mode == 2 && !(get32(packet) & 0x0400)) state->errors++;
            result = 3;
        } else if (command == CLOSE) {
            result = 0;
        } else if (command == LSEEK) {
            uint32_t whence = get32(packet + 8);
            state->position = whence == 2 ? (state->mode == 3 ? 1 :
                                             sizeof(from_content) - 1) :
                              (size_t)get32(packet + 4);
            result = (uint32_t)state->position;
        } else if (command == READ) {
            size_t requested = get32(packet + 4);
            if (state->mode == 3) {
                result = 4096; /* Deliberately inconsistent with request. */
            } else {
                size_t remaining = (sizeof(from_content) - 1) - state->position;
                data_len = remaining < requested ? remaining : requested;
                if (data_len > 4) data_len = 4;
                data = from_content + state->position;
                state->position += data_len;
                result = (uint32_t)data_len;
            }
        } else if (command == WRITE) {
            unsigned char buffer[28000];
            size_t count = get32(packet + 4);
            if (count > sizeof(buffer) || read_all(client, buffer, count) < 0) {
                state->errors++; break;
            }
            result = count > 4 ? 4 : (uint32_t)count;
            if (state->bytes_received + result > sizeof(state->received)) {
                state->errors++; break;
            }
            memcpy(state->received + state->bytes_received, buffer, result);
            state->bytes_received += result;
        } else { state->errors++; break; }
        set32(reply, command + 1);
        { uint16_t rlen = htons(command == READ ? 14 : 10);
          memcpy(reply + 4, &rlen, 2); }
        set32(reply + 6, result);
        if (command == READ) set32(reply + 10, result);
        if (send_all(client, reply, command == READ ? 14 : 10) < 0 ||
            (data_len && send_all(client, data, data_len) < 0)) {
            state->errors++; break;
        }
    }
    close(client);
    return NULL;
}
static int start_server(struct server *state, pthread_t *thread)
{
    struct sockaddr_in addr = {0};
    int reuse = 1;
    state->listener = socket(AF_INET, SOCK_STREAM, 0);
    if (state->listener < 0) return -1;
    setsockopt(state->listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(0x7f000001u);
    if (bind(state->listener, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
        listen(state->listener, 1) < 0 || pthread_create(thread, NULL, serve, state)) {
        close(state->listener); return -1;
    }
    return 0;
}
static int run_client(const char *executable, const char *command,
                      const char *source, const char *dest)
{
    int status;
    pid_t pid = fork();
    if (pid < 0) return -1;
    if (!pid) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        execl(executable, executable, "-h", "127.0.0.1",
              command, source, dest, (char *)NULL);
        _exit(127);
    }
    if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status)) return -1;
    return WEXITSTATUS(status);
}
static int test_case(const char *exe, int mode)
{
    struct server state = {0};
    pthread_t worker;
    char name[] = "/tmp/fsclient-test-XXXXXX";
    int fd, rc, failed = 0;
    state.mode = mode;
    fd = mkstemp(name);
    if (fd < 0) return 1;
    if (mode == 2) {
        if (write(fd, to_content, sizeof(to_content) - 1) !=
            (ssize_t)(sizeof(to_content) - 1)) failed = 1;
    } else {
        write(fd, "old contents longer than result", 31);
    }
    close(fd);
    if (start_server(&state, &worker)) {
        perror("start fake ps2netfs"); unlink(name); return 1;
    }
    rc = mode == 2 ? run_client(exe, "copyto", name, "mass:/file") :
                     run_client(exe, "copyfrom", "mass:/file", name);
    pthread_join(worker, NULL);
    close(state.listener);
    if (state.errors) failed = 1;
    if (mode == 1) {
        fd = open(name, O_RDONLY);
        if (fd < 0) failed = 1;
        else {
            char result[128];
            ssize_t n = read(fd, result, sizeof(result)); /* Flawfinder: ignore */
            if (n != (ssize_t)(sizeof(from_content) - 1) ||
                memcmp(result, from_content, sizeof(from_content) - 1)) failed = 1;
            close(fd);
        }
        if (rc != 0) failed = 1;
    } else if (mode == 2) {
        if (rc != 0 || state.bytes_received != sizeof(to_content) - 1 ||
            memcmp(state.received, to_content, sizeof(to_content) - 1)) failed = 1;
    } else if (rc == 0) failed = 1;
    unlink(name);
    printf("%s %s\n", failed ? "FAIL" : "PASS",
           mode == 1 ? "short remote reads" : mode == 2 ?
           "short remote writes" : "oversized remote read rejection");
    return failed;
}
int main(int argc, char **argv)
{
    int failures = 0, mode;
    if (argc != 2) { fprintf(stderr, "Usage: %s fsclient\n", argv[0]); return 2; }
    for (mode = 1; mode <= 3; mode++) failures += test_case(argv[1], mode);
    return failures ? 1 : 0;
}
