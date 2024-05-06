#pragma once

/**
    The .NES file format (file name suffix .nes) is the de facto standard for distribution of NES binary programs,
    with use even in licensed emulators such as commercialized PocketNES and Wii Virtual Console.
    It is often called the iNES format, as it was created by Marat Fayzullin for an emulator called iNES.
    The format was later extended with NES 2.0 to fix many of its shortcomings.

    Name of file format:
        This file format is commonly referred to as the iNES file format/iNES header format.
        The file extension is .nes, so it is sometimes referred to as the .nes file format,
        and files in it as .nes files.

        Now that the NES 2.0 file format exists, which uses the same .nes extension, a .nes file/the .nes file format could mean the
        iNES file format or NES 2.0 format, so the full format names should be used where the differences in the formats are relevant,
        like specifications or format support.

    iNES file format:
        An iNES file consists of the following sections, in order:

        1. Header (16 bytes)
        2. Trainer, if present (0 or 512 bytes)
        3. PRG ROM data (16384 * x bytes)
        4. CHR ROM data, if present (8192 * y bytes)
        5. PlayChoice INST-ROM, if present (0 or 8192 bytes)
        6. PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing; see PC10 ROM-Images for details)

        Note: Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.

        The format of the header is as follows:

        Bytes | Description
        0-3   | Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
        4     | Size of PRG ROM in 16 KB units
        5     | Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
        6     | Flags 6 - Mapper, mirroring, battery, trainer
        7     | Flags 7 - Mapper, VS/Playchoice, NES 2.0
        8     | Flags 8 - PRG-RAM size (rarely used extension)
        9     | Flags 9 - TV system (rarely used extension)
        10    | Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension)
        11-15 | Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)
*/

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t magic[4];
    uint8_t prg_rom_size;
    uint8_t chr_rom_size;
    uint8_t flags_6;
    uint8_t flags_7;
    uint8_t flags_8;
    uint8_t flags_9;
    uint8_t flags_10;
    uint8_t unused[5];
} _nes_header_t;

typedef struct
{
    _nes_header_t header;
    uint8_t trainer[512];

    uint8_t *prg_rom;
    size_t prg_rom_len;

    uint8_t *chr_rom;
    size_t chr_rom_len;

    uint8_t playchoice_inst_rom[8192];
    uint8_t playchoice_prom[32];

    uint8_t title[128];
} nes_rom_t;

const char *nes_geterr(void);
nes_rom_t *nes_rom_from_content(uint8_t *bytes, size_t len);
