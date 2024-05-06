#include "nes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define ENFORCE_SPEC

#define READ_SPECIFIC_BIT(flags, bit) ((flags >> bit) & 1)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const char *_err = NULL;
static uint8_t _has_err = 0;

static _nes_header_t _parse_header(uint8_t *bytes, size_t len);
static size_t _parse_trainer(nes_rom_t *rom, uint8_t *bytes, size_t len);
static size_t _parse_prg_rom(nes_rom_t *rom, uint8_t *bytes, size_t len);
static size_t _parse_chr_rom(nes_rom_t *rom, uint8_t *bytes, size_t len);

const char *nes_geterr(void)
{
    return _err;
}

nes_rom_t *nes_rom_from_content(uint8_t *bytes, size_t len)
{
    if (!bytes || len == 0)
    {
        _has_err = 1;
        _err = "There is no content to parse";
        return NULL;
    }

    nes_rom_t *rom = malloc(sizeof(nes_rom_t));
    if (!rom)
    {
        _has_err = 1;
        _err = "Failed to allocate memory for ROM";
        return NULL;
    }

    rom->header = _parse_header(bytes, len);
    if (_has_err)
    {
        free(rom);
        return NULL;
    }

    len -= sizeof(_nes_header_t);
    bytes += sizeof(_nes_header_t);

    size_t trainer_len = _parse_trainer(rom, bytes, len);
    if (_has_err)
    {
        free(rom);
        return NULL;
    }

    len -= trainer_len;
    bytes += trainer_len;

    size_t prg_rom_size = _parse_prg_rom(rom, bytes, len);

    if (_has_err)
    {
        free(rom);
        return NULL;
    }

    len -= prg_rom_size;
    bytes += prg_rom_size;

    // zero means we have no CHR ROM
    if (rom->header.chr_rom_size != 0)
    {
        size_t chr_rom_size = _parse_chr_rom(rom, bytes, len);
        if (_has_err)
        {
            free(rom);
            return NULL;
        }

        len -= chr_rom_size;
        bytes += chr_rom_size;
    }

    // if the 2 bit of flags7 is set, we have 8KB of PlayChoice INST-ROM
    if (READ_SPECIFIC_BIT(rom->header.flags_7, 1))
    {
        if (len < 8192)
        {
            _has_err = 1;
            _err = "Not enough bytes to read PlayChoice INST-ROM";
            free(rom);
            return NULL;
        }

        memcpy(rom->playchoice_inst_rom, bytes, 8192);

        len -= 8192;
        bytes += 8192;



        // PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing; see PC10 ROM-Images for details)
        if (len < 32)
        {
            _has_err = 1;
            _err = "Not enough bytes to read PlayChoice PROM";
            free(rom);
            return NULL;
        }

        memcpy(rom->playchoice_prom, bytes, 32);

        len -= 32;
        bytes += 32;
    }


    if (len > 0)
    {
       // the rest is supposed to be the title
       memcpy(rom->title, bytes, MIN(len, 127));
       // make sure it's null-terminated
       rom->title[MIN(len, 127)] = '\0';
    }

    return rom;
}

static _nes_header_t _parse_header(uint8_t *bytes, size_t len)
{
    _nes_header_t header = {0};
    // A header has 16 bytes, so if we have less than that, we know we can't parse a header
    // from it
    if (len < 16)
    {
       _has_err = 1;
       _err = "Content is too short to parse a header";
       return header;
    }

    // 4 magic bytes
    memcpy(header.magic, bytes, 4);
    uint8_t expected_magic[4] = {0x4E, 0x45, 0x53, 0x1A};

    if (memcmp(header.magic, expected_magic, 4) != 0)
    {
        _has_err = 1;
        _err = "Invalid magic bytes";
        return header;
    }

    // number of 16KB PRG ROM banks
    header.prg_rom_size = bytes[4];

    // number of 8KB CHR ROM banks
    header.chr_rom_size = bytes[5];

    // flags 6
    header.flags_6 = bytes[6];

    // flags 7
    header.flags_7 = bytes[7];

    // number of 8KB PRG RAM banks
    header.flags_8 = bytes[8];

    // flags 9
    header.flags_9 = bytes[9];

    // flags 10
    header.flags_10 = bytes[10];

#ifndef ENFORCE_SPEC
    // 11-15 are padding, but we can just copy them, just in case there is some data there
    memcpy(header.unused, bytes + 11, 5);
#else
    // the spec says that these bytes should be 0
    for (size_t i = 11; i < 16; i++)
    {
        if (bytes[i] != 0)
        {
            _has_err = 1;
            _err = "Invalid padding bytes";
            return header;
        }

        header.unused[i - 11] = bytes[i];
    }
#endif

    return header;
}

static size_t _parse_trainer(nes_rom_t *rom, uint8_t *bytes, size_t len)
{
    if (READ_SPECIFIC_BIT(rom->header.flags_6, 2))
    {
        printf("Trainer is enabled\n");
        // check if there is enough bytes to read the trainer
        if (len < 512)
        {
            _has_err = 1;
            _err = "Trainer is enabled, but there is not enough bytes to read it";
            return 0;
        }

        memcpy(rom->trainer, bytes, 512);

        return 512;
    }

    return 0;
}

static size_t _parse_prg_rom(nes_rom_t *rom, uint8_t *bytes, size_t len)
{
    // PRG ROM data (16384 * x bytes)
    size_t prg_rom_size = rom->header.prg_rom_size * 16384;
    if (len < prg_rom_size)
    {
        _has_err = 1;
        _err = "Not enough bytes to read PRG ROM data";
        return 0;
    }

    rom->prg_rom = malloc(prg_rom_size);
    if (!rom->prg_rom)
    {
        _has_err = 1;
        _err = "Failed to allocate memory for PRG ROM data";
        return 0;
    }

    memcpy(rom->prg_rom, bytes, prg_rom_size);

    rom->prg_rom_len = prg_rom_size;

    return prg_rom_size;
}

static size_t _parse_chr_rom(nes_rom_t *rom, uint8_t *bytes, size_t len)
{
    // CHR ROM data (8192 * y bytes)
    size_t chr_rom_size = rom->header.chr_rom_size * 8192;
    if (len < chr_rom_size)
    {
        _has_err = 1;
        _err = "Not enough bytes to read CHR ROM data";
        return 0;
    }

    rom->chr_rom = malloc(chr_rom_size);
    if (!rom->chr_rom)
    {
        _has_err = 1;
        _err = "Failed to allocate memory for CHR ROM data";
        return 0;
    }

    memcpy(rom->chr_rom, bytes, chr_rom_size);

    rom->chr_rom_len = chr_rom_size;

    return chr_rom_size;
}
