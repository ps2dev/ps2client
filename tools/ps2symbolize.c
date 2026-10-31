/* Annotate ps2link EE/IOP exception logs with matching ELF debug symbols. */
#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <spawn.h>

extern char **environ;

struct module_map { char name[128]; const char *elf; };
struct symbolizer {
    const char *ee_elf;
    const char *ee_tool;
    const char *iop_tool;
    struct module_map modules[64];
    size_t module_count;
    int exception; /* 0 none, 1 EE, 2 IOP */
    int errors;
};

typedef int (*resolve_fn)(const char *, const char *, uint32_t,
                          char *, size_t, void *);

static int equals_ignore_case(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return !*a && !*b;
}

/* addr2line is invoked directly, without a shell or string interpolation. */
#ifndef PS2_SYMBOLIZE_NO_MAIN
static int resolve_symbol(const char *tool, const char *elf, uint32_t address,
                          char *output, size_t capacity, void *unused)
{
    int pipefd[2], status;
    pid_t pid;
    FILE *stream;
    char pc[16];
    char *const argv[] = {(char *)tool, "-f", "-C", "-p", "-e",
                          (char *)elf, pc, NULL};
    posix_spawn_file_actions_t actions;
    (void)unused;
    snprintf(pc, sizeof(pc), "0x%08x", address);
    if (pipe(pipefd) < 0) return -1;
    if (posix_spawn_file_actions_init(&actions) != 0) {
        close(pipefd[0]); close(pipefd[1]); return -1;
    }
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);
    status = posix_spawnp(&pid, tool, &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]);
    if (status != 0) { close(pipefd[0]); return -1; }
    stream = fdopen(pipefd[0], "r");
    if (!stream) { close(pipefd[0]); waitpid(pid, &status, 0); return -1; }
    if (!fgets(output, (int)capacity, stream)) output[0] = '\0';
    fclose(stream);
    if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status) || WEXITSTATUS(status))
        return -1;
    output[strcspn(output, "\r\n")] = '\0';
    if (!output[0] || strncmp(output, "??", 2) == 0) return 0;
    return 1;
}
#endif

static void annotate_line(struct symbolizer *config, const char *line,
                          FILE *out, resolve_fn resolve, void *context)
{
    const char *elf = NULL, *tool = NULL, *epc;
    uint32_t pc;
    char name[128] = "", symbol[1024];
    int value;

    fputs(line, out);
    if (strstr(line, "EE Exception handler:") != NULL) config->exception = 1;
    if (strstr(line, "IOP Exception handler:") != NULL) config->exception = 2;
    if (config->exception == 1 && config->ee_elf) {
        epc = strstr(line, "EPC ");
        if (!epc || sscanf(epc, "EPC %x", &pc) != 1) return;
        elf = config->ee_elf;
        tool = config->ee_tool;
        config->exception = 0;
    } else if (config->exception == 2) {
        size_t i;
        epc = strstr(line, "Module Name \"");
        if (!epc || sscanf(epc, "Module Name \"%127[^\"]\" Relative EPC %x",
                            name, &pc) != 2) return;
        for (i = 0; i < config->module_count; i++) {
            if (equals_ignore_case(name, config->modules[i].name)) {
                elf = config->modules[i].elf;
                break;
            }
        }
        tool = config->iop_tool;
        config->exception = 0;
    } else {
        return;
    }
    if (!elf) return;
    value = resolve(tool, elf, pc, symbol, sizeof(symbol), context);
    if (value > 0) {
        if (name[0]) fprintf(out, "  [IOP %s symbol] %s\n", name, symbol);
        else fprintf(out, "  [EE symbol] %s\n", symbol);
    } else if (value < 0) {
        fprintf(stderr, "ps2symbolize: cannot execute %s for %s\n", tool, elf);
        config->errors++;
    }
}

#ifndef PS2_SYMBOLIZE_NO_MAIN
static void usage(FILE *out)
{
    fputs("Usage: ps2symbolize [--ee ELF] [--iop MODULE=ELF]... "
          "[--ee-tool PATH] [--iop-tool PATH] [LOG|-]\n", out);
}

int main(int argc, char **argv)
{
    struct symbolizer config = {0};
    FILE *input = stdin;
    char line[8192];
    const char *log_path = NULL;
    int i;
    config.ee_tool = "mips64r5900el-ps2-elf-addr2line";
    config.iop_tool = "mipsel-none-elf-addr2line";
    setvbuf(stdout, NULL, _IOLBF, 0);
    for (i = 1; i < argc; i++) {
        if ((!strcmp(argv[i], "--ee") || !strcmp(argv[i], "--iop") ||
             !strcmp(argv[i], "--ee-tool") || !strcmp(argv[i], "--iop-tool")) &&
            i + 1 >= argc) { usage(stderr); return 2; }
        if (!strcmp(argv[i], "--ee")) config.ee_elf = argv[++i];
        else if (!strcmp(argv[i], "--ee-tool")) config.ee_tool = argv[++i];
        else if (!strcmp(argv[i], "--iop-tool")) config.iop_tool = argv[++i];
        else if (!strcmp(argv[i], "--iop")) {
            const char *pair = argv[++i];
            const char *sep = strchr(pair, '=');
            size_t length = sep ? (size_t)(sep - pair) : 0;
            struct module_map *entry;
            if (!length || length >= sizeof(config.modules[0].name) ||
                !sep[1] || config.module_count == 64) { usage(stderr); return 2; }
            entry = &config.modules[config.module_count++];
            memcpy(entry->name, pair, length);
            entry->name[length] = '\0';
            entry->elf = sep + 1;
        } else if (!strcmp(argv[i], "--help")) { usage(stdout); return 0; }
        else if (!log_path && (argv[i][0] != '-' || !strcmp(argv[i], "-"))) {
            log_path = argv[i];
        } else { usage(stderr); return 2; }
    }
    if (!config.ee_elf && !config.module_count) { usage(stderr); return 2; }
    if (log_path && strcmp(log_path, "-")) {
        input = fopen(log_path, "r");
        if (!input) { perror(log_path); return 1; }
    }
    while (fgets(line, sizeof(line), input)) {
        annotate_line(&config, line, stdout, resolve_symbol, NULL);
    }
    if (input != stdin) fclose(input);
    return config.errors ? 1 : 0;
}
#endif
