#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *cmll_strdup(const char *text)
{
    size_t length;
    char *copy;

    if (!text)
        return NULL;

    length = strlen(text);

    copy = malloc(length + 1);

    if (!copy)
        return NULL;

    memcpy(copy, text, length + 1);

    return copy;
}

char *cmll_read_file(
    const char *filename,
    size_t *size
)
{
    FILE *file;
    long length;
    char *buffer;
    size_t count;

    file = fopen(filename, "rb");

    if (!file)
        return NULL;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    length = ftell(file);

    if (length < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);

    buffer = malloc((size_t)length + 1);

    if (!buffer) {
        fclose(file);
        return NULL;
    }

    count = fread(
        buffer,
        1,
        (size_t)length,
        file
    );

    fclose(file);

    if (count != (size_t)length) {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';

    if (size)
        *size = (size_t)length;

    return buffer;
}

int cmll_write_file(
    const char *filename,
    const char *content
)
{
    FILE *file;

    file = fopen(filename, "wb");

    if (!file)
        return 0;

    if (content)
        fputs(content, file);

    fclose(file);

    return 1;
}

static const char *source_line(
    const char *source,
    int target_line
)
{
    const char *cursor;
    int line;

    if (!source)
        return NULL;

    cursor = source;
    line = 1;

    while (*cursor && line < target_line) {
        if (*cursor == '\n')
            line++;

        cursor++;
    }

    if (line != target_line)
        return NULL;

    return cursor;
}

void cmll_error(
    CMLLPosition position,
    const char *source,
    const char *message
)
{
    const char *line;
    const char *end;
    int i;

    fprintf(
        stderr,
        "\nCMLL error\n"
        "  --> %s:%d:%d\n",
        position.filename
            ? position.filename
            : "<source>",
        position.line,
        position.column
    );

    line = source_line(
        source,
        position.line
    );

    if (!line) {
        fprintf(
            stderr,
            "  | %s\n\n",
            message
        );
        return;
    }

    end = line;

    while (*end && *end != '\n')
        end++;

    fprintf(
        stderr,
        "  |\n"
        "  | %.*s\n",
        (int)(end - line),
        line
    );

    fprintf(stderr, "  | ");

    for (i = 1; i < position.column; i++)
        fputc(' ', stderr);

    fprintf(
        stderr,
        "^\n"
        "  = %s\n\n",
        message
    );
}

void cmll_error_simple(
    CMLLPosition position,
    const char *message
)
{
    fprintf(
        stderr,
        "CMLL error at %s:%d:%d: %s\n",
        position.filename
            ? position.filename
            : "<source>",
        position.line,
        position.column,
        message
    );
}
