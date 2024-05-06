#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "file.h"

static const char *_err = NULL;
static uint8_t _has_err = 0;

const char *file_geterr(void)
{
    return _err;
}

static uint64_t _getfsize(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        _err = "Failed to open file";
        _has_err = 1;
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) < 0)
    {
        _err = "Failed to seek file";
        _has_err = 1;

        fclose(fp);
        return 0;
    }

    int64_t size = ftell(fp);

    if (size < 0)
    {
        _err = "Failed to get file size";
        _has_err = 1;

        fclose(fp);
        return 0;
    }

    if (fclose(fp) != 0)
    {
        _err = "Failed to close file";
        _has_err = 1;

        return 0;
    }

    return (uint64_t) size;
}

bytes_t *getfcontent(const char *filename)
{
    uint64_t size = _getfsize(filename);

    if (_has_err)
    {
        return NULL;
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        _err = "Failed to open file";
        _has_err = 1;

        return NULL;
    }

    uint8_t *content = (uint8_t *) malloc(size);
    if (!content)
    {
        _err = "Failed to allocate memory";
        _has_err = 1;

        fclose(fp);
        return NULL;
    }

    if (fread(content, 1, size, fp) != size)
    {
        _err = "Failed to read file";
        _has_err = 1;

        fclose(fp);
        free(content);
        return NULL;
    }


    if (fclose(fp) != 0)
    {
        _has_err = 1;
        _err = "Failed to close file";

        free(content);
        return NULL;
    }

    bytes_t *b = malloc(sizeof(bytes_t));
    if (!b)
    {
        _err = "Failed to allocate memory";
        _has_err = 1;

        free(content);
        return NULL;
    }

    b->data = content;
    b->size = size;

    return b;
}
