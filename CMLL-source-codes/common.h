#ifndef CMLL_COMMON_H
#define CMLL_COMMON_H

#include <stddef.h>

typedef struct {
    const char *filename;
    int line;
    int column;
} CMLLPosition;

typedef struct {
    int line;
    const char *text;
} CMLLSourceLine;

char *cmll_strdup(const char *text);

char *cmll_read_file(
    const char *filename,
    size_t *size
);

int cmll_write_file(
    const char *filename,
    const char *content
);

void cmll_error(
    CMLLPosition position,
    const char *source,
    const char *message
);

void cmll_error_simple(
    CMLLPosition position,
    const char *message
);

#endif
