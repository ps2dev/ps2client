/* Native Windows command and UDP tests without a PS2 or Python. */
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONSOLE_PORT 0x4712

static int failures;

static void check(int passed, const char *name)
{
    printf("%s %s\n", passed ? "PASS" : "FAIL", name);
    failures += !passed;
}

static int launch(const char *arguments, PROCESS_INFORMATION *process)
{
    STARTUPINFOA startup = {0};
    char command[1024];
    startup.cb = sizeof(startup);
    memset(process, 0, sizeof(*process));
    snprintf(command, sizeof(command), "\"bin\\ps2client.exe\" %s", arguments);
    return CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW,
                          NULL, NULL, &startup, process) != 0;
}

static int finish(PROCESS_INFORMATION *process, DWORD timeout)
{
    DWORD status = (DWORD)-1;
    if (WaitForSingleObject(process->hProcess, timeout) != WAIT_OBJECT_0) {
        TerminateProcess(process->hProcess, 1);
        WaitForSingleObject(process->hProcess, 5000);
    } else {
        GetExitCodeProcess(process->hProcess, &status);
    }
    CloseHandle(process->hThread);
    CloseHandle(process->hProcess);
    return (int)status;
}

static int run(const char *arguments)
{
    PROCESS_INFORMATION process;
    if (!launch(arguments, &process)) return -1;
    return finish(&process, 7000);
}

static void test_console(void)
{
    PROCESS_INFORMATION process;
    struct sockaddr_in address = {0};
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int i;
    if (sock == INVALID_SOCKET) { check(0, "UDP console setup"); return; }
    if (!launch("-h 1.1.l.1 -t 3 -e smoke-console-line console", &process)) {
        closesocket(sock);
        check(0, "UDP console launch");
        return;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(0x7f000001UL);
    address.sin_port = htons(CONSOLE_PORT);
    Sleep(150);
    for (i = 0; i < 10; i++) {
        sendto(sock, "smoke-console-line\n", 19, 0,
               (struct sockaddr *)&address, sizeof(address));
        Sleep(40);
    }
    closesocket(sock);
    check(finish(&process, 5000) == 0,
          "UDP-only console accepts messages without resolving host");
}

static void test_reset(void)
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in address = {0};
    unsigned char packet[64];
    DWORD timeout = 3000;
    int bytes, status;
    if (sock == INVALID_SOCKET) { check(0, "UDP reset setup"); return; }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(0x7f000001UL);
    address.sin_port = htons(CONSOLE_PORT);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               (const char *)&timeout, sizeof(timeout));
    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) != 0) {
        closesocket(sock);
        check(0, "UDP reset bind");
        return;
    }
    status = run("-h 127.0.0.1 reset");
    bytes = recv(sock, (char *)packet, sizeof(packet), 0);
    closesocket(sock);
    check(status == 0 && bytes == 6 &&
          packet[0] == 0xba && packet[1] == 0xbe &&
          packet[2] == 0x02 && packet[3] == 0x01 &&
          packet[4] == 0x00 && packet[5] == 0x06,
          "reset uses UDP with the expected command packet");
}

int main(void)
{
    WSADATA sockets;
    if (WSAStartup(MAKEWORD(2, 2), &sockets) != 0) return 1;

    check(run("-h 1.1.l.1 ping") == 69, "invalid hostname");
    check(run("-h 1.1.l.1 -t 1 listen") == 69,
          "legacy listen requires a resolvable host");
    check(run("invalid-command") == 64, "unknown command");
    check(run("dumpmem 0x100000") == 64, "missing arguments");
    check(run("-e") == 64, "missing marker");
    SetEnvironmentVariableA("PS2HOSTNAME", "1.1.l.1");
    check(run("ping") == 69, "invalid PS2HOSTNAME environment setting");
    SetEnvironmentVariableA("PS2HOSTNAME", NULL);
    test_console();
    test_reset();

    WSACleanup();
    return failures ? 1 : 0;
}
