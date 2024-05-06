#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    uint8_t *data;
    size_t size;
} bytes_t;

bytes_t *getfcontent(const char *path);
const char *file_geterr(void);
