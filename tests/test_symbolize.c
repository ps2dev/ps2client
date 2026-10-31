/* Unit tests for EE/IOP exception log parsing. */
#define PS2_SYMBOLIZE_NO_MAIN
#include "../tools/ps2symbolize.c"

static uint32_t last_address;
static const char *last_elf;
static int calls;
static int fake_resolver(const char *tool, const char *elf, uint32_t address,
                         char *output, size_t capacity, void *context)
{
    (void)tool; (void)context;
    last_elf = elf; last_address = address; calls++;
    snprintf(output, capacity, "updateSprites at renderer.c:284");
    return 1;
}

static void test_symbolizer(void)
{
    struct symbolizer config = {0};
    FILE *output = tmpfile();
    char text[1024];
    size_t length;
    if (!output) { perror("tmpfile"); exit(1); }
    config.ee_tool = "ee-addr2line";
    config.iop_tool = "iop-addr2line";
    config.ee_elf = "game.elf";
    config.module_count = 1;
    strcpy(config.modules[0].name, "mydriver");
    config.modules[0].elf = "driver.elf";
    annotate_line(&config, "EE Exception handler: Address load exception\n",
                  output, fake_resolver, NULL);
    annotate_line(&config, "Cause 00000010 EPC 00123456\n",
                  output, fake_resolver, NULL);
    if (calls != 1 || last_address != 0x123456 ||
        strcmp(last_elf, "game.elf")) exit(1);
    annotate_line(&config, "IOP Exception handler: Address store\n",
                  output, fake_resolver, NULL);
    annotate_line(&config, "Module Name \"mydriver\" Relative EPC 00000064\n",
                  output, fake_resolver, NULL);
    if (calls != 2 || last_address != 100 ||
        strcmp(last_elf, "driver.elf")) exit(1);
    rewind(output);
    length = fread(text, 1, sizeof(text) - 1, output);
    text[length] = 0;
    if (!strstr(text, "[EE symbol]") || !strstr(text, "[IOP mydriver symbol]"))
        exit(1);
    fclose(output);
    puts("PASS EE/IOP symbolization, original log preservation");
}

int main(void) { test_symbolizer(); return 0; }
