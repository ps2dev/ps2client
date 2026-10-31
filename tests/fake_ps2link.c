/* Fake PS2Link server for the Linux network-namespace integration suite.
 * Commands: script-based execee, dumpreg handshake, UDP console, host: I/O.
 */
#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define CMD_EXECEE 0xBABE0203u
#define CMD_DUMPREG 0xBABE020Au
#define REQ_OPEN 0xBABE0111u
#define REQ_CLOSE 0xBABE0121u
#define REQ_READ 0xBABE0131u
#define REQ_WRITE 0xBABE0141u

struct fake {
    int listener, commands, console, client;
    struct sockaddr_in client_addr;
    pthread_mutex_t lock;
    int ignore_dumpreg;
};
static struct fake fake = {.client = -1, .lock = PTHREAD_MUTEX_INITIALIZER};

static uint32_t get32(const unsigned char *p)
{
    uint32_t n;
    memcpy(&n, p, 4);
    return ntohl(n);
}
static void put32(unsigned char *p, uint32_t n)
{
    n = htonl(n);
    memcpy(p, &n, 4);
}
static int recv_exact(int socket_fd, void *data, size_t length)
{
    size_t done = 0;
    while (done < length) {
        ssize_t received = recv(socket_fd, (char *)data + done, length - done, 0);
        if (received <= 0) return -1;
        done += (size_t)received;
    }
    return 0;
}
static int send_exact(int socket_fd, const void *data, size_t length)
{
    size_t done = 0;
    while (done < length) {
        ssize_t sent = send(socket_fd, (const char *)data + done, length - done, 0);
        if (sent <= 0) return -1;
        done += (size_t)sent;
    }
    return 0;
}
static int request_locked(uint32_t type, const unsigned char *payload,
                          size_t payload_len, const void *data, size_t length)
{
    unsigned char head[6];
    uint16_t packet_len = htons((uint16_t)(payload_len + 6));
    if (fake.client < 0) return -1;
    put32(head, type);
    memcpy(head + 4, &packet_len, 2);
    if (send_exact(fake.client, head, sizeof(head)) ||
        (payload_len && send_exact(fake.client, payload, payload_len)) ||
        (length && send_exact(fake.client, data, length))) return -1;
    return 0;
}
static int reply_locked(unsigned char *buffer, size_t length)
{
    return fake.client < 0 ? -1 : recv_exact(fake.client, buffer, length);
}
static int fake_open(const char *name, uint32_t flags)
{
    unsigned char payload[260] = {0}, reply[10];
    int result = -1;
    put32(payload, flags);
    snprintf((char *)payload + 4, 256, "%s", name);
    pthread_mutex_lock(&fake.lock);
    if (!request_locked(REQ_OPEN, payload, sizeof(payload), NULL, 0) &&
        !reply_locked(reply, sizeof(reply))) result = (int32_t)get32(reply + 6);
    pthread_mutex_unlock(&fake.lock);
    return result;
}
static void fake_close(int fd)
{
    unsigned char payload[4];
    put32(payload, (uint32_t)fd);
    pthread_mutex_lock(&fake.lock);
    if (!request_locked(REQ_CLOSE, payload, sizeof(payload), NULL, 0)) {
        unsigned char reply[10];
        reply_locked(reply, sizeof(reply));
    }
    pthread_mutex_unlock(&fake.lock);
}
static int fake_read(int fd, unsigned char *buffer, int max_size)
{
    unsigned char payload[8], reply[14];
    int count = -1;
    put32(payload, (uint32_t)fd);
    put32(payload + 4, (uint32_t)max_size);
    pthread_mutex_lock(&fake.lock);
    if (!request_locked(REQ_READ, payload, sizeof(payload), NULL, 0) &&
        !reply_locked(reply, sizeof(reply))) {
        count = (int32_t)get32(reply + 10);
        if (count < 0 || count > max_size ||
            (count && reply_locked(buffer, (size_t)count))) count = -1;
    }
    pthread_mutex_unlock(&fake.lock);
    return count;
}
static int fake_write(int fd, const unsigned char *data, int length)
{
    unsigned char payload[8], reply[10];
    int result = -1;
    put32(payload, (uint32_t)fd);
    put32(payload + 4, (uint32_t)length);
    pthread_mutex_lock(&fake.lock);
    if (!request_locked(REQ_WRITE, payload, sizeof(payload), data, length) &&
        !reply_locked(reply, sizeof(reply))) result = (int32_t)get32(reply + 6);
    pthread_mutex_unlock(&fake.lock);
    return result;
}
static unsigned char *load_file(const char *path, size_t *length)
{
    unsigned char *data = NULL;
    unsigned char chunk[128 * 1024];
    size_t allocated = 0;
    int fd = fake_open(path, 1), n;
    *length = 0;
    if (fd < 0) return NULL;
    while ((n = fake_read(fd, chunk, sizeof(chunk))) > 0) {
        if (*length + (size_t)n + 1 > allocated) {
            size_t target = (*length + (size_t)n + 1) * 2;
            unsigned char *grown = realloc(data, target);
            if (!grown) { free(data); data = NULL; break; }
            data = grown; allocated = target;
        }
        memcpy(data + *length, chunk, (size_t)n);
        *length += (size_t)n;
    }
    fake_close(fd);
    if (n < 0 || !data) { free(data); return NULL; }
    data[*length] = '\0';
    return data;
}
static void console(const void *data, size_t len)
{
    struct sockaddr_in peer;
    pthread_mutex_lock(&fake.lock);
    peer = fake.client_addr;
    pthread_mutex_unlock(&fake.lock);
    peer.sin_port = htons(0x4712);
    sendto(fake.console, data, len, 0, (struct sockaddr *)&peer, sizeof(peer));
}
static void line_out(const char *line)
{
    char text[2048];
    int n = snprintf(text, sizeof(text), "%s\n", line);
    if (n > 0) console(text, (size_t)n < sizeof(text) ? (size_t)n : sizeof(text) - 1);
}
static uint32_t crc32_bytes(const unsigned char *data, size_t length)
{
    uint32_t crc = ~0u;
    size_t i;
    int bit;
    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (crc & 1 ? 0xedb88320u : 0);
    }
    return ~crc;
}
static void sleep_seconds(double seconds)
{
    struct timespec time;
    if (seconds < 0) return;
    time.tv_sec = (time_t)seconds;
    time.tv_nsec = (long)((seconds - time.tv_sec) * 1e9);
    nanosleep(&time, NULL);
}
static void *run_script(void *context)
{
    char *script = context, *line, *next;
    for (line = script; line && *line; line = next) {
        char *arg;
        next = strchr(line, '\n');
        if (next) *next++ = '\0';
        arg = strchr(line, ' ');
        if (arg) *arg++ = '\0';
        else arg = line + strcspn(line, "\r\n");
        if (!strcmp(line, "print")) line_out(arg);
        else if (!strcmp(line, "raw")) {
            int count = 0;
            char ch = 0;
            if (sscanf(arg, "%d %c", &count, &ch) == 2 && count > 0 &&
                count <= 1600) {
                char bytes[1600];
                memset(bytes, ch, (size_t)count);
                console(bytes, (size_t)count);
            }
        } else if (!strcmp(line, "sleep")) sleep_seconds(strtod(arg, NULL));
        else if (!strcmp(line, "close")) {
            pthread_mutex_lock(&fake.lock);
            if (fake.client >= 0) {
                shutdown(fake.client, SHUT_RDWR);
                close(fake.client);
                fake.client = -1;
            }
            pthread_mutex_unlock(&fake.lock);
            break;
        } else if (!strcmp(line, "readback")) {
            size_t length = 0;
            unsigned char *data = load_file(arg, &length);
            if (data) {
                char report[128];
                snprintf(report, sizeof(report), "readback %zu %08x",
                         length, crc32_bytes(data, length));
                line_out(report);
                free(data);
            }
        }
        /* A script may end at the last line without a trailing newline. */
    }
    free(script);
    return NULL;
}
static void command_packet(unsigned char *packet, size_t length)
{
    uint32_t type;
    if (length < 10) return;
    type = get32(packet);
    if (type == CMD_DUMPREG && length >= 266) {
        int fd;
        if (fake.ignore_dumpreg > 0) { fake.ignore_dumpreg--; return; }
        fd = fake_open((char *)packet + 10, 0x0202);
        if (fd >= 0) {
            unsigned char zeros[8] = {0};
            fake_write(fd, zeros, sizeof(zeros));
            fake_close(fd);
        }
    } else if (type == CMD_EXECEE && length >= 266) {
        char args[256], *first, *path;
        char output[512] = "fake: args ";
        size_t size = 0;
        unsigned char *script;
        pthread_t worker;
        uint32_t count;
        uint32_t i;
        size_t offset;
        count = get32(packet + 6);
        memcpy(args, packet + 10, sizeof(args));
        args[255] = '\0';
        first = args;
        path = strncmp(first, "host:", 5) == 0 ? first + 5 : first;
        script = load_file(path, &size);
        if (!script) { line_out("fake: could not load script"); return; }
        offset = strnlen(first, sizeof(args)) + 1;
        for (i = 1; i < count && offset < 256; i++) {
            size_t used = strnlen(output, sizeof(output));
            size_t current = strnlen(args + offset, sizeof(args) - offset);
            snprintf(output + used, sizeof(output) - used, "%s%s",
                     i > 1 ? " " : "", args + offset);
            offset += current + 1;
        }
        line_out(output);
        if (!pthread_create(&worker, NULL, run_script, script))
            pthread_detach(worker);
        else free(script);
    }
}
static int bind_port(int type, const char *ip, uint16_t port)
{
    int fd = socket(AF_INET, type, 0), reuse = 1;
    struct sockaddr_in addr = {0};
    if (fd < 0) return -1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1 ||
        bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd); return -1;
    }
    return fd;
}
int main(int argc, char **argv)
{
    const char *ip = NULL;
    int i;
    fake.console = socket(AF_INET, SOCK_DGRAM, 0);
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--bind") && i + 1 < argc) ip = argv[++i];
        else if (!strcmp(argv[i], "--ignore-dumpreg") && i + 1 < argc)
            fake.ignore_dumpreg = atoi(argv[++i]);
        else { fprintf(stderr, "Unknown option: %s\n", argv[i]); return 2; }
    }
    if (!ip) { fprintf(stderr, "Usage: fake_ps2link --bind IPV4 [--ignore-dumpreg N]\n"); return 2; }
    fake.listener = bind_port(SOCK_STREAM, ip, 0x4711);
    fake.commands = bind_port(SOCK_DGRAM, ip, 0x4712);
    if (fake.listener < 0 || fake.commands < 0 || listen(fake.listener, 5) < 0) {
        perror("fake_ps2link bind/listen"); return 1;
    }
    for (;;) {
        fd_set set;
        int ready, maxfd = fake.listener > fake.commands ? fake.listener : fake.commands;
        FD_ZERO(&set); FD_SET(fake.listener, &set); FD_SET(fake.commands, &set);
        ready = select(maxfd + 1, &set, NULL, NULL, NULL);
        if (ready <= 0) continue;
        if (FD_ISSET(fake.listener, &set)) {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int client = accept(fake.listener, (struct sockaddr *)&peer, &len);
            if (client < 0) continue;
            pthread_mutex_lock(&fake.lock);
            if (fake.client >= 0) close(fake.client);
            fake.client = client;
            fake.client_addr = peer;
            pthread_mutex_unlock(&fake.lock);
            { int one = 1;
              setsockopt(client, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); }
        } else if (FD_ISSET(fake.commands, &set)) {
            unsigned char packet[2048];
            ssize_t n = recv(fake.commands, packet, sizeof(packet), 0);
            if (n > 0 && fake.client >= 0) command_packet(packet, (size_t)n);
        }
    }
}
