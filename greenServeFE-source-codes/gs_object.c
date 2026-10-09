#define _POSIX_C_SOURCE 200809L
#include "module.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const GSNativeAPI *A;

typedef struct {
    const char *s;
    size_t p, n;
    int line, col;
    char error[256];
} Parser;

static void perr(Parser *p, const char *msg) {
    if (!p->error[0])
        snprintf(p->error, sizeof p->error, "line %d:%d: %s", p->line, p->col, msg);
}
static int peek(Parser *p) {
    return p->p < p->n ? (unsigned char)p->s[p->p] : 0;
}
static int take(Parser *p) {
    int c = peek(p);
    if (!c) return 0;
    p->p++;
    if (c == '\n') { p->line++; p->col = 1; }
    else p->col++;
    return c;
}
static void skip_ws(Parser *p) {
    for (;;) {
        while (isspace(peek(p))) take(p);
        if (peek(p) == '#') {
            while (peek(p) && peek(p) != '\n') take(p);
            continue;
        }
        if (peek(p) == '/' && p->p + 1 < p->n && p->s[p->p + 1] == '/') {
            while (peek(p) && peek(p) != '\n') take(p);
            continue;
        }
        break;
    }
}
static int eat(Parser *p, int c) {
    skip_ws(p);
    if (peek(p) == c) { take(p); return 1; }
    return 0;
}
static char *dup_range(const char *s, size_t n) {
    char *x = malloc(n + 1);
    if (!x) exit(2);
    memcpy(x, s, n);
    x[n] = 0;
    return x;
}
static char *word(Parser *p) {
    skip_ws(p);
    size_t start = p->p;
    while (isalnum(peek(p)) || peek(p) == '_' || peek(p) == '-' ||
           peek(p) == '/' || peek(p) == '.')
        take(p);
    return p->p == start ? NULL : dup_range(p->s + start, p->p - start);
}
static char *quoted(Parser *p) {
    skip_ws(p);
    int q = peek(p);
    if (q != '"' && q != '\'') return NULL;
    take(p);
    size_t cap = 64, n = 0;
    char *out = malloc(cap);
    if (!out) exit(2);

    while (peek(p) && peek(p) != q) {
        int c = take(p);
        if (c == '\\') {
            c = take(p);
            if (c == 'n') c = '\n';
            else if (c == 'r') c = '\r';
            else if (c == 't') c = '\t';
            else if (c == '"' || c == '\'' || c == '\\') {}
        }
        if (n + 2 >= cap) {
            cap *= 2;
            out = realloc(out, cap);
            if (!out) exit(2);
        }
        out[n++] = (char)c;
    }
    if (!eat(p, q)) {
        free(out);
        perr(p, "unterminated string");
        return NULL;
    }
    out[n] = 0;
    return out;
}

static GSValue parse_value(Parser *p);

static int consume_property_end(Parser *p) {
    skip_ws(p);
    if (peek(p) == ';') { take(p); return 1; }
    return 0;
}

static GSValue parse_object_body(Parser *p, int opener, int closer) {
    GSValue o = A->object();
    if (!eat(p, opener)) {
        perr(p, "expected object opener");
        return o;
    }
    for (;;) {
        skip_ws(p);
        if (eat(p, closer)) break;
        if (!peek(p)) {
            perr(p, closer == '}' ? "expected '}'" : "expected ')'");
            break;
        }
        char *key = quoted(p);
        if (!key) key = word(p);
        if (!key) {
            perr(p, "expected a field name");
            break;
        }
        skip_ws(p);
        if (!(peek(p) == '=' && p->p + 1 < p->n && p->s[p->p + 1] == '>')) {
            free(key);
            perr(p, "expected '=>'");
            break;
        }
        take(p);
        take(p);
        GSValue value = parse_value(p);
        A->object_set(&o, key, value);
        A->release(value);
        free(key);
        if (p->error[0]) break;
        if (!consume_property_end(p)) {
            skip_ws(p);
            if (peek(p) != closer) {
                perr(p, "expected ';' after field");
                break;
            }
        }
    }
    return o;
}

static GSValue parse_object(Parser *p) {
    return parse_object_body(p, '{', '}');
}

static GSValue parse_record(Parser *p) {
    return parse_object_body(p, '(', ')');
}

static GSValue parse_array(Parser *p) {
    GSValue a = A->array();
    if (!eat(p, '[')) {
        perr(p, "expected '['");
        return a;
    }
    for (;;) {
        skip_ws(p);
        if (eat(p, ']')) break;
        if (!peek(p)) {
            perr(p, "expected ']'");
            break;
        }
        if (!eat(p, '|')) {
            perr(p, "expected '|' before sequence item");
            break;
        }
        GSValue value = parse_value(p);
        A->array_push(&a, value);
        A->release(value);
        if (p->error[0]) break;
        if (!consume_property_end(p)) {
            skip_ws(p);
            if (peek(p) != ']') {
                perr(p, "expected ';' after sequence item");
                break;
            }
        }
    }
    return a;
}

static GSValue parse_value(Parser *p) {
    skip_ws(p);
    int c = peek(p);
    if (c == '{') return parse_object(p);
    if (c == '(') return parse_record(p);
    if (c == '[') return parse_array(p);

    if (c == '"' || c == '\'') {
        char *s = quoted(p);
        GSValue v = A->string(s ? s : "");
        free(s);
        return v;
    }

    char *w = word(p);
    if (!w) {
        perr(p, "expected a value");
        return A->null();
    }
    if (!strcmp(w, "true")) { free(w); return A->boolean(1); }
    if (!strcmp(w, "false")) { free(w); return A->boolean(0); }
    if (!strcmp(w, "null")) { free(w); return A->null(); }

    char *end = NULL;
    double number = strtod(w, &end);
    if (end && *end == 0 && end != w) {
        free(w);
        return A->number(number);
    }

    GSValue v = A->string(w);
    free(w);
    return v;
}

static GSValue parse_text(const char *text) {
    Parser p = { text, 0, strlen(text), 1, 1, {0} };
    skip_ws(&p);

    char *root = word(&p);
    if (!root) {
        perr(&p, "expected a GS Object root name");
        return A->null();
    }
    skip_ws(&p);
    if (peek(&p) != '{') {
        free(root);
        perr(&p, "expected '{' after root name");
        return A->null();
    }
    free(root);

    GSValue value = parse_object(&p);
    skip_ws(&p);
    if (!p.error[0] && p.p < p.n)
        perr(&p, "unexpected content after root object");
    if (p.error[0]) {
        fprintf(stderr, "gs_object: %s\n", p.error);
        A->release(value);
        return A->null();
    }
    return value;
}

static void indent(FILE *f, int depth) {
    for (int i = 0; i < depth; i++) fputs("  ", f);
}

static int simple_key(const char *s) {
    if (!s || !*s) return 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++)
        if (!(isalnum(*p) || *p == '_' || *p == '-' || *p == '.')) return 0;
    return 1;
}

static void print_string(FILE *f, const char *s) {
    fputc('"', f);
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        if (*p == '\\') fputs("\\\\", f);
        else if (*p == '"') fputs("\\\"", f);
        else if (*p == '\n') fputs("\\n", f);
        else if (*p == '\r') fputs("\\r", f);
        else if (*p == '\t') fputs("\\t", f);
        else fputc(*p, f);
    }
    fputc('"', f);
}

static void write_value(FILE *f, GSValue v, int depth);

static void write_fields(FILE *f, GSValue v, int depth) {
    for (size_t i = 0; i < A->object_count(v); i++) {
        const char *key = A->object_key(v, i);
        indent(f, depth);
        if (simple_key(key)) fputs(key, f);
        else print_string(f, key ? key : "");
        fputs(" => ", f);
        GSValue value = A->object_get(v, key ? key : "");
        write_value(f, value, depth);
        A->release(value);
        fputs(";\n", f);
    }
}

static void write_object(FILE *f, GSValue v, int depth) {
    fputs("{\n", f);
    write_fields(f, v, depth + 1);
    indent(f, depth);
    fputc('}', f);
}

static void write_value(FILE *f, GSValue v, int depth) {
    switch (v.type) {
        case V_NULL: fputs("null", f); break;
        case V_BOOL: fputs(v.as.boolean ? "true" : "false", f); break;
        case V_NUMBER: {
            char b[64];
            snprintf(b, sizeof b, "%.15g", v.as.number);
            fputs(b, f);
            break;
        }
        case V_STRING: print_string(f, A->cstring(v)); break;
        case V_OBJECT: write_object(f, v, depth); break;
        case V_ARRAY:
            fputs("[\n", f);
            for (size_t i = 0; i < A->array_count(v); i++) {
                indent(f, depth + 1);
                fputs("| ", f);
                GSValue item = A->array_get(v, i);
                write_value(f, item, depth + 1);
                A->release(item);
                fputs(";\n", f);
            }
            indent(f, depth);
            fputc(']', f);
            break;
        default: fputs("null", f); break;
    }
}

static char *read_all(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    rewind(f);
    char *s = malloc((size_t)n + 1);
    if (!s) exit(2);
    size_t got = fread(s, 1, (size_t)n, f);
    fclose(f);
    s[got] = 0;
    return s;
}
static GSValue fn_read(GSValue *a, size_t n) {
    if (!n) return A->null();
    char *text = read_all(A->cstring(a[0]));
    if (!text) return A->null();
    GSValue value = parse_text(text);
    free(text);
    return value;
}
static GSValue fn_parse(GSValue *a, size_t n) {
    return n ? parse_text(A->cstring(a[0])) : A->null();
}
static GSValue fn_write(GSValue *a, size_t n) {
    if (n < 2 || a[1].type != V_OBJECT) return A->boolean(0);
    FILE *f = fopen(A->cstring(a[0]), "wb");
    if (!f) return A->boolean(0);

    fputs("# greenServe Object (.gsob)\n", f);
    fputs("# GSOB: => fields, ; termination, ( ) records, and | sequences.\n\n", f);
    fputs("object {\n", f);
    write_fields(f, a[1], 1);
    fputs("}\n", f);

    int ok = !ferror(f);
    if (fclose(f) != 0) ok = 0;
    return A->boolean(ok);
}
static GSValue fn_text(GSValue *a, size_t n) {
    if (!n || a[0].type != V_OBJECT) return A->string("");
    FILE *f = tmpfile();
    if (!f) return A->string("");

    fputs("object {\n", f);
    write_fields(f, a[0], 1);
    fputs("}\n", f);
    fflush(f);

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return A->string(""); }
    long nbytes = ftell(f);
    if (nbytes < 0) { fclose(f); return A->string(""); }
    rewind(f);

    char *buf = malloc((size_t)nbytes + 1);
    if (!buf) exit(2);
    size_t got = fread(buf, 1, (size_t)nbytes, f);
    fclose(f);
    buf[got] = 0;

    GSValue result = A->string(buf);
    free(buf);
    return result;
}
static GSValue fn_exists(GSValue *a, size_t n) {
    if (!n) return A->boolean(0);
    FILE *f = fopen(A->cstring(a[0]), "rb");
    if (!f) return A->boolean(0);
    fclose(f);
    return A->boolean(1);
}

GSValue greenServeFE_module_init(const GSNativeAPI *api) {
    A = api;
    GSValue module = A->object();
    GSValue fn;

    fn = A->native(fn_read);
    A->object_set(&module, "read", fn);
    A->object_set(&module, "load", fn);
    A->release(fn);

    fn = A->native(fn_parse);
    A->object_set(&module, "parse", fn);
    A->release(fn);

    fn = A->native(fn_write);
    A->object_set(&module, "write", fn);
    A->object_set(&module, "save", fn);
    A->release(fn);

    fn = A->native(fn_text);
    A->object_set(&module, "text", fn);
    A->object_set(&module, "format", fn);
    A->release(fn);

    fn = A->native(fn_exists);
    A->object_set(&module, "exists", fn);
    A->release(fn);

    fn = A->string("1.0.0");
    A->object_set(&module, "version", fn);
    A->release(fn);

    return module;
}
