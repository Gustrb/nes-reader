#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "file.h"
#include "nes.h"

#define INVALID_USAGE 1
#define FAILED_READ_FILE 2

int main(int argc, const char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return INVALID_USAGE;
    }

    bytes_t *fcontent = getfcontent(argv[1]);
    if (!fcontent)
    {
        fprintf(stderr, "Failed to read file: %s -> %s\n", argv[1], file_geterr());
        return FAILED_READ_FILE;
    }

    nes_rom_t *rom = nes_rom_from_content(fcontent->data, fcontent->size);
    if (!rom)
    {
        fprintf(stderr, "Failed to parse ROM: %s\n", nes_geterr());
        return EXIT_FAILURE;
    }

    printf("%s", rom->title);

    return EXIT_SUCCESS;
}
