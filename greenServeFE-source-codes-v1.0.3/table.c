#include "module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const GSNativeAPI *api;

typedef struct { char *data; size_t length, capacity; } Buffer;

static int init_buffer(Buffer *b)
{
    b->capacity = 1024;
    b->length = 0;
    b->data = malloc(b->capacity);
    if (!b->data) return 0;
    b->data[0] = 0;
    return 1;
}

static int append(Buffer *b, const char *s)
{
    size_t n = strlen(s ? s : "");
    if (n > SIZE_MAX - b->length - 1) return 0;
    size_t need = b->length + n + 1;
    if (need > b->capacity) {
        size_t cap = b->capacity;
        while (cap < need) cap *= 2;
        char *p = realloc(b->data, cap);
        if (!p) return 0;
        b->data = p;
        b->capacity = cap;
    }
    memcpy(b->data + b->length, s ? s : "", n);
    b->length += n;
    b->data[b->length] = 0;
    return 1;
}

static int putc_buffer(Buffer *b, char c)
{
    char s[2] = {c, 0};
    return append(b, s);
}

static void free_buffer(Buffer *b)
{
    free(b->data);
}

static GSValue get(GSValue object, const char *key)
{
    return object.type == V_OBJECT ? api->object_get(object, key) : api->null();
}

static long option_number(GSValue options, const char *key, long fallback)
{
    GSValue value = get(options, key);
    long result = value.type == V_NUMBER
        ? (long)api->number_value(value)
        : fallback;
    if (value.type != V_NULL) api->release(value);
    return result;
}

static int option_bool(GSValue options, const char *key, int fallback)
{
    GSValue value = get(options, key);
    if (value.type == V_NULL) return fallback;
    int result = api->truthy(value);
    api->release(value);
    return result;
}

static int width_for(GSValue options, size_t column, size_t fallback)
{
    GSValue widths = get(options, "widths");
    if (widths.type != V_ARRAY || column >= api->array_count(widths)) {
        if (widths.type != V_NULL) api->release(widths);
        return (int)fallback;
    }

    GSValue value = api->array_get(widths, column);
    int result = value.type == V_NUMBER
        ? (int)api->number_value(value)
        : (int)fallback;
    api->release(value);
    api->release(widths);
    return result > 0 ? result : (int)fallback;
}

static const char *alignment_for(GSValue options, size_t column)
{
    static char value[16];
    GSValue align = get(options, "align");

    if (align.type == V_STRING) {
        snprintf(value, sizeof value, "%s", api->cstring(align));
        api->release(align);
        return value;
    }

    if (align.type == V_ARRAY && column < api->array_count(align)) {
        GSValue item = api->array_get(align, column);
        if (item.type == V_STRING) {
            snprintf(value, sizeof value, "%s", api->cstring(item));
            api->release(item);
            api->release(align);
            return value;
        }
        api->release(item);
    }

    if (align.type != V_NULL) api->release(align);
    return "left";
}

static GSValue cell(GSValue rows, size_t row, size_t column)
{
    if (rows.type != V_ARRAY || row >= api->array_count(rows))
        return api->string("");

    GSValue current = api->array_get(rows, row);
    if (current.type != V_ARRAY || column >= api->array_count(current)) {
        api->release(current);
        return api->string("");
    }

    GSValue result = api->array_get(current, column);
    api->release(current);
    return result;
}

static GSValue header(GSValue headers, size_t column)
{
    if (headers.type != V_ARRAY || column >= api->array_count(headers))
        return api->string("");
    return api->array_get(headers, column);
}

static size_t columns(GSValue headers, GSValue rows)
{
    size_t result = headers.type == V_ARRAY ? api->array_count(headers) : 0;

    if (rows.type == V_ARRAY) {
        for (size_t i = 0; i < api->array_count(rows); i++) {
            GSValue row = api->array_get(rows, i);
            if (row.type == V_ARRAY && api->array_count(row) > result)
                result = api->array_count(row);
            api->release(row);
        }
    }

    return result;
}

static int pad(
    Buffer *b,
    const char *text,
    size_t width,
    const char *alignment,
    int padding
)
{
    size_t length = strlen(text ? text : "");
    if (length > width) width = length;

    size_t extra = width - length;
    size_t left = 0;

    if (!strcmp(alignment, "right")) left = extra;
    else if (!strcmp(alignment, "center")) left = extra / 2;

    size_t right = extra - left;

    for (int i = 0; i < padding; i++) if (!putc_buffer(b, ' ')) return 0;
    for (size_t i = 0; i < left; i++) if (!putc_buffer(b, ' ')) return 0;
    if (!append(b, text ? text : "")) return 0;
    for (size_t i = 0; i < right; i++) if (!putc_buffer(b, ' ')) return 0;
    for (int i = 0; i < padding; i++) if (!putc_buffer(b, ' ')) return 0;

    return 1;
}

static int border(Buffer *b, const size_t *widths, size_t count, int padding)
{
    if (!putc_buffer(b, '+')) return 0;

    for (size_t i = 0; i < count; i++) {
        size_t width = widths[i] + (size_t)(padding * 2);
        for (size_t j = 0; j < width; j++)
            if (!putc_buffer(b, '-')) return 0;
        if (!putc_buffer(b, '+')) return 0;
    }

    return putc_buffer(b, '\n');
}

static GSValue render(GSValue *args, size_t argc)
{
    if (argc < 2 || args[0].type != V_ARRAY || args[1].type != V_ARRAY)
        return api->null();

    GSValue headers = args[0];
    GSValue rows = args[1];
    GSValue options = argc >= 3 && args[2].type == V_OBJECT
        ? args[2]
        : api->null();

    size_t count = columns(headers, rows);
    if (!count) return api->string("");

    size_t *widths = calloc(count, sizeof *widths);
    if (!widths) return api->null();

    int padding = (int)option_number(options, "padding", 1);
    if (padding < 0) padding = 0;

    for (size_t column = 0; column < count; column++) {
        GSValue h = header(headers, column);
        widths[column] = strlen(api->cstring(h));
        api->release(h);

        if (rows.type == V_ARRAY) {
            for (size_t row = 0; row < api->array_count(rows); row++) {
                GSValue value = cell(rows, row, column);
                size_t length = strlen(api->cstring(value));
                if (length > widths[column]) widths[column] = length;
                api->release(value);
            }
        }

        widths[column] = (size_t)width_for(options, column, widths[column] ? widths[column] : 1);
    }

    Buffer out;
    if (!init_buffer(&out)) {
        free(widths);
        return api->null();
    }

    GSValue title = get(options, "title");
    if (title.type == V_STRING) {
        if (!append(&out, api->cstring(title)) || !putc_buffer(&out, '\n')) {
            api->release(title);
            free_buffer(&out);
            free(widths);
            return api->null();
        }
    }
    if (title.type != V_NULL) api->release(title);

    GSValue border_style = get(options, "border");
    int compact = border_style.type == V_STRING &&
                  !strcmp(api->cstring(border_style), "compact");
    if (border_style.type != V_NULL) api->release(border_style);

    int show_header = option_bool(options, "header", 1);

    if (!compact && !border(&out, widths, count, padding)) goto fail;

    if (show_header) {
        if (!putc_buffer(&out, '|')) goto fail;
        for (size_t column = 0; column < count; column++) {
            GSValue h = header(headers, column);
            if (!pad(&out, api->cstring(h), widths[column],
                     alignment_for(options, column), padding)) {
                api->release(h);
                goto fail;
            }
            api->release(h);
            if (!putc_buffer(&out, '|')) goto fail;
        }
        if (!putc_buffer(&out, '\n')) goto fail;
        if (!compact && !border(&out, widths, count, padding)) goto fail;
    }

    for (size_t row = 0; row < api->array_count(rows); row++) {
        if (!putc_buffer(&out, '|')) goto fail;

        for (size_t column = 0; column < count; column++) {
            GSValue value = cell(rows, row, column);
            if (!pad(&out, api->cstring(value), widths[column],
                     alignment_for(options, column), padding)) {
                api->release(value);
                goto fail;
            }
            api->release(value);
            if (!putc_buffer(&out, '|')) goto fail;
        }

        if (!putc_buffer(&out, '\n')) goto fail;
    }

    if (!compact && !border(&out, widths, count, padding)) goto fail;

    {
        GSValue result = api->string(out.data);
        free_buffer(&out);
        free(widths);
        return result;
    }

fail:
    free_buffer(&out);
    free(widths);
    return api->null();
}

static int csv_value(Buffer *out, const char *text)
{
    if (!text) text = "";
    int quoted = strchr(text, ',') || strchr(text, '"') ||
                 strchr(text, '\n') || strchr(text, '\r');

    if (!quoted) return append(out, text);
    if (!putc_buffer(out, '"')) return 0;

    for (const char *p = text; *p; p++) {
        if (*p == '"' && !putc_buffer(out, '"')) return 0;
        if (!putc_buffer(out, *p)) return 0;
    }

    return putc_buffer(out, '"');
}

static GSValue csv(GSValue *args, size_t argc)
{
    if (argc < 2 || args[0].type != V_ARRAY || args[1].type != V_ARRAY)
        return api->null();

    size_t count = columns(args[0], args[1]);
    Buffer out;
    if (!init_buffer(&out)) return api->null();

    for (size_t c = 0; c < count; c++) {
        GSValue h = header(args[0], c);
        if (!csv_value(&out, api->cstring(h))) {
            api->release(h);
            goto fail;
        }
        api->release(h);
        if (c + 1 < count && !putc_buffer(&out, ',')) goto fail;
    }

    if (!putc_buffer(&out, '\n')) goto fail;

    for (size_t r = 0; r < api->array_count(args[1]); r++) {
        for (size_t c = 0; c < count; c++) {
            GSValue value = cell(args[1], r, c);
            if (!csv_value(&out, api->cstring(value))) {
                api->release(value);
                goto fail;
            }
            api->release(value);
            if (c + 1 < count && !putc_buffer(&out, ',')) goto fail;
        }
        if (!putc_buffer(&out, '\n')) goto fail;
    }

    {
        GSValue result = api->string(out.data);
        free_buffer(&out);
        return result;
    }

fail:
    free_buffer(&out);
    return api->null();
}

static GSValue markdown(GSValue *args, size_t argc)
{
    if (argc < 2 || args[0].type != V_ARRAY || args[1].type != V_ARRAY)
        return api->null();

    size_t count = columns(args[0], args[1]);
    Buffer out;
    if (!init_buffer(&out)) return api->null();

    if (!putc_buffer(&out, '|')) goto fail;
    for (size_t c = 0; c < count; c++) {
        GSValue h = header(args[0], c);
        if (!append(&out, " ") ||
            !append(&out, api->cstring(h)) ||
            !append(&out, " |")) {
            api->release(h);
            goto fail;
        }
        api->release(h);
    }

    if (!putc_buffer(&out, '\n') || !putc_buffer(&out, '|')) goto fail;

    for (size_t c = 0; c < count; c++)
        if (!append(&out, " --- |")) goto fail;

    if (!putc_buffer(&out, '\n')) goto fail;

    for (size_t r = 0; r < api->array_count(args[1]); r++) {
        if (!putc_buffer(&out, '|')) goto fail;

        for (size_t c = 0; c < count; c++) {
            GSValue value = cell(args[1], r, c);
            if (!append(&out, " ") ||
                !append(&out, api->cstring(value)) ||
                !append(&out, " |")) {
                api->release(value);
                goto fail;
            }
            api->release(value);
        }

        if (!putc_buffer(&out, '\n')) goto fail;
    }

    {
        GSValue result = api->string(out.data);
        free_buffer(&out);
        return result;
    }

fail:
    free_buffer(&out);
    return api->null();
}

static GSValue row_count(GSValue *args, size_t argc)
{
    if (argc != 1 || args[0].type != V_ARRAY) return api->number(0);
    return api->number((double)api->array_count(args[0]));
}

static GSValue column_count(GSValue *args, size_t argc)
{
    if (argc != 1 || args[0].type != V_ARRAY) return api->number(0);
    return api->number((double)api->array_count(args[0]));
}

GSValue greenServeFE_module_init(const GSNativeAPI *native_api)
{
    api = native_api;
    GSValue module = api->object();

    api->object_set(&module, "render", api->native(render));
    api->object_set(&module, "csv", api->native(csv));
    api->object_set(&module, "markdown", api->native(markdown));
    api->object_set(&module, "row_count", api->native(row_count));
    api->object_set(&module, "column_count", api->native(column_count));

    return module;
}
