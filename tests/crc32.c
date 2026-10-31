/* Print IEEE/zlib CRC32 of a file for the fake-PS2Link regression suite. */
#include <stdint.h>
#include <stdio.h>
int main(int argc, char **argv)
{
    FILE *input;
    int ch, bit;
    uint32_t crc = ~0u;
    if (argc != 2 || !(input = fopen(argv[1], "rb"))) return 1;
    while ((ch = fgetc(input)) != EOF) { /* Flawfinder: ignore */
        crc ^= (unsigned char)ch;
        for (bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (crc & 1 ? 0xedb88320u : 0);
    }
    fclose(input);
    printf("%08x\n", ~crc);
    return 0;
}
