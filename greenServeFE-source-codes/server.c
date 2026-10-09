#include "module.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <curl/curl.h>
#include <errno.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

static const GSNativeAPI *api;

static char *dup_string(const char *text);
static int path_match(const char *pattern, const char *path, GSValue *params);
static GSValue query_object(const char *query);

typedef struct { char *data; size_t length, capacity; } Buffer;

static int buf_init(Buffer *b)
{
    b->capacity = 1024;
    b->length = 0;
    b->data = malloc(b->capacity);
    if (!b->data) return 0;
    b->data[0] = 0;
    return 1;
}

static int buf_reserve(Buffer *b, size_t extra)
{
    if (extra > SIZE_MAX - b->length - 1) return 0;
    size_t need = b->length + extra + 1;
    if (need <= b->capacity) return 1;
    size_t cap = b->capacity;
    while (cap < need) {
        if (cap > SIZE_MAX / 2) { cap = need; break; }
        cap *= 2;
    }
    char *p = realloc(b->data, cap);
    if (!p) return 0;
    b->data = p;
    b->capacity = cap;
    return 1;
}

static int buf_write(Buffer *b, const void *p, size_t n)
{
    if (!buf_reserve(b, n)) return 0;
    memcpy(b->data + b->length, p, n);
    b->length += n;
    b->data[b->length] = 0;
    return 1;
}

static int buf_puts(Buffer *b, const char *s)
{
    return buf_write(b, s ? s : "", strlen(s ? s : ""));
}

static void buf_free(Buffer *b)
{
    free(b->data);
    b->data = NULL;
    b->length = b->capacity = 0;
}

static GSValue object_get(GSValue object, const char *name)
{
    return object.type == V_OBJECT ? api->object_get(object, name) : api->null();
}

static const char *option_string(GSValue options, const char *name)
{
    static char value[4096];
    GSValue v = object_get(options, name);
    if (v.type != V_STRING) {
        if (v.type != V_NULL) api->release(v);
        return NULL;
    }
    snprintf(value, sizeof value, "%s", api->cstring(v));
    api->release(v);
    return value;
}

static long option_number(GSValue options, const char *name, long fallback)
{
    GSValue v = object_get(options, name);
    long result = fallback;
    if (v.type == V_NUMBER) result = (long)api->number_value(v);
    if (v.type != V_NULL) api->release(v);
    return result;
}

typedef struct {
    Buffer body;
    GSValue headers;
} CurlResult;

static size_t curl_body(void *ptr, size_t size, size_t count, void *userdata)
{
    Buffer *body = userdata;
    size_t n = size * count;
    return buf_write(body, ptr, n) ? n : 0;
}

static size_t curl_headers(void *ptr, size_t size, size_t count, void *userdata)
{
    CurlResult *result = userdata;
    size_t n = size * count;
    char *colon = memchr(ptr, ':', n);
    if (!colon || result->headers.type != V_OBJECT) return n;

    size_t key_len = (size_t)(colon - (char *)ptr);
    while (key_len && (((char *)ptr)[key_len - 1] == ' ' ||
                       ((char *)ptr)[key_len - 1] == '\t')) key_len--;

    char key[256];
    if (key_len >= sizeof key) key_len = sizeof key - 1;
    memcpy(key, ptr, key_len);
    key[key_len] = 0;

    char *value = colon + 1;
    size_t value_len = n - (size_t)(value - (char *)ptr);
    while (value_len && (*value == ' ' || *value == '\t')) {
        value++;
        value_len--;
    }
    while (value_len &&
           (value[value_len - 1] == '\r' ||
            value[value_len - 1] == '\n' ||
            value[value_len - 1] == ' ' ||
            value[value_len - 1] == '\t')) {
        value_len--;
    }

    char *copy = malloc(value_len + 1);
    if (!copy) return n;
    memcpy(copy, value, value_len);
    copy[value_len] = 0;

    GSValue text = api->string(copy);
    api->object_set(&result->headers, key, text);
    api->release(text);
    free(copy);
    return n;
}

static GSValue http_request(GSValue *args, size_t argc)
{
    if (argc < 2 ||
        args[0].type != V_STRING ||
        args[1].type != V_STRING) return api->null();

    const char *method = api->cstring(args[0]);
    const char *url = api->cstring(args[1]);
    const char *body = NULL;
    GSValue headers = api->null();

    if (argc >= 3 && args[2].type == V_STRING) body = api->cstring(args[2]);
    if (argc >= 4 && args[3].type == V_OBJECT) headers = args[3];

    CURL *curl = curl_easy_init();
    if (!curl) return api->null();

    CurlResult result;
    if (!buf_init(&result.body)) {
        curl_easy_cleanup(curl);
        return api->null();
    }
    result.headers = api->object();

    struct curl_slist *header_list = NULL;
    if (headers.type == V_OBJECT) {
        for (size_t i = 0; i < api->object_count(headers); i++) {
            const char *key = api->object_key(headers, i);
            GSValue value = api->object_get(headers, key);
            if (value.type == V_STRING) {
                char line[8192];
                snprintf(line, sizeof line, "%s: %s", key, api->cstring(value));
                header_list = curl_slist_append(header_list, line);
            }
            api->release(value);
        }
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_body);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_headers);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &result);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "greenServeFE/1.0");
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    if (header_list) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    if (body) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    CURLcode code = curl_easy_perform(curl);
    long status = 0;
    char *content_type = NULL;
    char *effective_url = NULL;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective_url);

    GSValue output = api->object();
    GSValue value = api->number((double)status);
    api->object_set(&output, "status", value);
    api->release(value);

    value = api->string(result.body.data ? result.body.data : "");
    api->object_set(&output, "body", value);
    api->release(value);

    value = api->string(content_type ? content_type : "");
    api->object_set(&output, "contentType", value);
    api->release(value);

    value = api->string(effective_url ? effective_url : url);
    api->object_set(&output, "url", value);
    api->release(value);

    api->object_set(&output, "headers", result.headers);

    value = api->string(code == CURLE_OK ? "" : curl_easy_strerror(code));
    api->object_set(&output, "error", value);
    api->release(value);

    api->release(result.headers);
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    buf_free(&result.body);
    return output;
}

static GSValue http_get(GSValue *args, size_t argc)
{
    if (argc < 1) return api->null();
    GSValue method = api->string("GET");
    GSValue result = http_request((GSValue[]){method, args[0]}, 2);
    api->release(method);
    return result;
}

static GSValue https_get(GSValue *args, size_t argc)
{
    return http_get(args, argc);
}

static GSValue http_post(GSValue *args, size_t argc)
{
    if (argc < 2) return api->null();
    GSValue method = api->string("POST");
    GSValue result = http_request((GSValue[]){method, args[0], args[1]}, 3);
    api->release(method);
    return result;
}

static int read_file(const char *path, Buffer *buffer)
{
    FILE *file = fopen(path, "rb");
    if (!file) return 0;

    char data[8192];
    size_t count;
    while ((count = fread(data, 1, sizeof data, file)) != 0) {
        if (!buf_write(buffer, data, count)) {
            fclose(file);
            return 0;
        }
    }

    int ok = !ferror(file);
    fclose(file);
    return ok;
}

static GSValue file_read(GSValue *args, size_t argc)
{
    if (argc < 1 || args[0].type != V_STRING) return api->null();

    Buffer buffer;
    if (!buf_init(&buffer)) return api->null();

    if (!read_file(api->cstring(args[0]), &buffer)) {
        buf_free(&buffer);
        return api->null();
    }

    GSValue result = api->string(buffer.data);
    buf_free(&buffer);
    return result;
}

static GSValue file_write(GSValue *args, size_t argc)
{
    if (argc < 2 ||
        args[0].type != V_STRING ||
        args[1].type != V_STRING) return api->boolean(0);

    FILE *file = fopen(api->cstring(args[0]), "wb");
    if (!file) return api->boolean(0);

    const char *text = api->cstring(args[1]);
    size_t length = strlen(text);
    int ok = fwrite(text, 1, length, file) == length;
    fclose(file);
    return api->boolean(ok);
}

static GSValue template_lookup(GSValue data, const char *key)
{
    if (data.type != V_OBJECT || !key || !*key) return api->null();
    GSValue direct = api->object_get(data, key);
    if (direct.type != V_NULL) return direct;

    char *copy = dup_string(key);
    if (!copy) return api->null();
    GSValue current = api->copy(data);
    char *save = NULL;
    char *part = strtok_r(copy, ".", &save);
    while (part) {
        if (current.type != V_OBJECT) {
            if (current.type != V_NULL) api->release(current);
            free(copy);
            return api->null();
        }
        GSValue next = api->object_get(current, part);
        api->release(current);
        current = next;
        if (current.type == V_NULL) break;
        part = strtok_r(NULL, ".", &save);
    }
    free(copy);
    return current;
}

static int render_template(Buffer *out, const char *template, GSValue data)
{
    const char *cursor = template;

    while (*cursor) {
        const char *start = strstr(cursor, "{{");
        if (!start) return buf_puts(out, cursor);

        if (!buf_write(out, cursor, (size_t)(start - cursor))) return 0;

        const char *end = strstr(start + 2, "}}");
        if (!end) return buf_puts(out, start);

        size_t length = (size_t)(end - start - 2);
        while (length &&
               (start[2 + length - 1] == ' ' ||
                start[2 + length - 1] == '\t')) length--;

        size_t offset = 0;
        while (offset < length &&
               (start[2 + offset] == ' ' ||
                start[2 + offset] == '\t')) offset++;

        char key[512];
        size_t key_length = length - offset;
        if (key_length >= sizeof key) key_length = sizeof key - 1;
        memcpy(key, start + 2 + offset, key_length);
        key[key_length] = 0;

        GSValue value = template_lookup(data, key);
        if (value.type != V_NULL) {
            if (!buf_puts(out, api->cstring(value))) {
                api->release(value);
                return 0;
            }
            api->release(value);
        }

        cursor = end + 2;
    }

    return 1;
}

static GSValue render(GSValue *args, size_t argc)
{
    if (argc < 2 ||
        args[0].type != V_STRING ||
        args[1].type != V_OBJECT) return api->null();

    Buffer output;
    if (!buf_init(&output)) return api->null();

    if (!render_template(&output, api->cstring(args[0]), args[1])) {
        buf_free(&output);
        return api->null();
    }

    GSValue result = api->string(output.data);
    buf_free(&output);
    return result;
}

static GSValue url_encode(GSValue *args, size_t argc)
{
    if (argc < 1 || args[0].type != V_STRING) return api->null();

    CURL *curl = curl_easy_init();
    if (!curl) return api->null();

    char *encoded = curl_easy_escape(curl, api->cstring(args[0]), 0);
    GSValue result = api->string(encoded ? encoded : "");
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

static GSValue json_escape(GSValue *args, size_t argc)
{
    if (argc < 1) return api->string("");

    const char *text = api->cstring(args[0]);
    Buffer output;
    if (!buf_init(&output)) return api->null();

    for (; *text; text++) {
        switch (*text) {
            case '"': if (!buf_puts(&output, "\\\"")) goto fail; break;
            case '\\': if (!buf_puts(&output, "\\\\")) goto fail; break;
            case '\n': if (!buf_puts(&output, "\\n")) goto fail; break;
            case '\r': if (!buf_puts(&output, "\\r")) goto fail; break;
            case '\t': if (!buf_puts(&output, "\\t")) goto fail; break;
            default: if (!buf_write(&output, text, 1)) goto fail;
        }
    }

    {
        GSValue result = api->string(output.data);
        buf_free(&output);
        return result;
    }

fail:
    buf_free(&output);
    return api->null();
}

static const char *mime_type(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (!strcasecmp(ext, ".html") || !strcasecmp(ext, ".htm")) return "text/html; charset=utf-8";
    if (!strcasecmp(ext, ".css")) return "text/css; charset=utf-8";
    if (!strcasecmp(ext, ".js")) return "application/javascript; charset=utf-8";
    if (!strcasecmp(ext, ".json")) return "application/json; charset=utf-8";
    if (!strcasecmp(ext, ".txt")) return "text/plain; charset=utf-8";
    if (!strcasecmp(ext, ".svg")) return "image/svg+xml";
    if (!strcasecmp(ext, ".png")) return "image/png";
    if (!strcasecmp(ext, ".jpg") || !strcasecmp(ext, ".jpeg")) return "image/jpeg";
    if (!strcasecmp(ext, ".gif")) return "image/gif";
    return "application/octet-stream";
}

static int send_all(int fd, const void *data, size_t length)
{
    const char *p = data;
    while (length) {
        ssize_t written = send(fd, p, length, 0);
        if (written <= 0) return 0;
        p += written;
        length -= (size_t)written;
    }
    return 1;
}


typedef struct {
    char *method;
    char *path;
    char *query;
    char *body;
    GSValue headers;
} HTTPRequest;

typedef struct {
    char *method;
    char *path;
    GSValue handler;
    int prefix;
    size_t order;
} Route;

static Route *routes;
static size_t route_count;
static size_t route_capacity;
static size_t route_sequence;

static void route_free(Route *route)
{
    if (!route) return;
    free(route->method);
    free(route->path);
    if (route->handler.type != V_NULL) api->release(route->handler);
    route->method = NULL;
    route->path = NULL;
    route->handler = api->null();
}

static void routes_clear(void)
{
    for (size_t i = 0; i < route_count; i++) route_free(&routes[i]);
    free(routes);
    routes = NULL;
    route_count = 0;
    route_capacity = 0;
}

static int routes_reserve(size_t extra)
{
    if (extra > SIZE_MAX - route_count) return 0;
    size_t need = route_count + extra;
    if (need <= route_capacity) return 1;

    size_t cap = route_capacity ? route_capacity : 32;
    while (cap < need) {
        if (cap > SIZE_MAX / 2) {
            cap = need;
            break;
        }
        cap *= 2;
    }

    Route *next = realloc(routes, cap * sizeof(*next));
    if (!next) return 0;
    routes = next;
    route_capacity = cap;
    return 1;
}

static char *dup_string(const char *text)
{
    if (!text) text = "";
    size_t length = strlen(text);
    char *copy = malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, text, length + 1);
    return copy;
}

static int route_add(const char *method, const char *path, GSValue handler, int prefix)
{
    if (!method || !path || !*path || handler.type == V_NULL) return 0;
    if (!routes_reserve(1)) return 0;

    Route *route = &routes[route_count];
    memset(route, 0, sizeof(*route));

    route->method = dup_string(method);
    route->path = dup_string(path);
    route->handler = api->copy(handler);
    route->prefix = prefix;
    route->order = route_sequence++;

    if (!route->method || !route->path) {
        route_free(route);
        return 0;
    }

    route_count++;
    return 1;
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static char *url_decode_copy(const char *text)
{
    if (!text) return dup_string("");
    size_t length = strlen(text);
    char *out = malloc(length + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < length; i++) {
        if (text[i] == '+') {
            out[j++] = ' ';
        } else if (text[i] == '%' && i + 2 < length) {
            int hi = hex_value(text[i + 1]);
            int lo = hex_value(text[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out[j++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                out[j++] = text[i];
            }
        } else {
            out[j++] = text[i];
        }
    }
    out[j] = 0;
    return out;
}

static int path_prefix_match(const char *prefix, const char *path)
{
    if (!prefix || !path) return 0;
    if (!strcmp(prefix, "/") || !strcmp(prefix, "*")) return 1;
    size_t n = strlen(prefix);
    if (strncmp(prefix, path, n)) return 0;
    return path[n] == 0 || path[n] == '/';
}

static int route_specificity(const Route *route, const char *path)
{
    if (!route || !path) return -1;
    if (route->prefix) {
        if (!path_prefix_match(route->path, path)) return -1;
        return 100 + (int)strlen(route->path);
    }
    if (!path_match(route->path, path, NULL)) return -1;
    int score = 0;
    if (!strcmp(route->path, path)) return 1000000;
    const char *p = route->path;
    while (*p) {
        if (*p == ':') score += 100;
        else if (*p == '*') score += 1;
        else score += 1000;
        p++;
    }
    return score;
}

static int path_match(const char *pattern, const char *path, GSValue *params)
{
    if (!pattern || !path) return 0;

    if (!strcmp(pattern, path)) return 1;

    char *p = dup_string(pattern);
    char *u = dup_string(path);
    if (!p || !u) {
        free(p);
        free(u);
        return 0;
    }

    char *save_p = NULL;
    char *save_u = NULL;
    char *pp = strtok_r(p, "/", &save_p);
    char *uu = strtok_r(u, "/", &save_u);

    while (pp && uu) {
        if (pp[0] == ':') {
            if (params && (*params).type == V_OBJECT) {
                char *decoded = url_decode_copy(uu);
                GSValue value = api->string(decoded ? decoded : uu);
                api->object_set(params, pp + 1, value);
                free(decoded);
                api->release(value);
            }
        } else if (!strcmp(pp, "*")) {
            if (params && (*params).type == V_OBJECT) {
                char *decoded = url_decode_copy(uu);
                GSValue value = api->string(decoded ? decoded : uu);
                api->object_set(params, "wildcard", value);
                free(decoded);
                api->release(value);
            }
            free(p);
            free(u);
            return 1;
        } else if (strcmp(pp, uu)) {
            free(p);
            free(u);
            return 0;
        }

        pp = strtok_r(NULL, "/", &save_p);
        uu = strtok_r(NULL, "/", &save_u);
    }

    int matched = !pp && !uu;
    if (pp && !strcmp(pp, "*")) {
        matched = 1;
        if (params && (*params).type == V_OBJECT) {
            char *decoded = url_decode_copy(uu ? uu : "");
            GSValue value = api->string(decoded ? decoded : (uu ? uu : ""));
            api->object_set(params, "wildcard", value);
            free(decoded);
            api->release(value);
        }
    }

    free(p);
    free(u);
    return matched;
}

static Route *find_route(const char *method, const char *path, GSValue *params)
{
    Route *best = NULL;
    int best_score = -1;
    size_t best_order = SIZE_MAX;

    for (size_t i = 0; i < route_count; i++) {
        if (strcasecmp(routes[i].method, method) && strcmp(routes[i].method, "*")) continue;
        int score = route_specificity(&routes[i], path);
        if (score < 0) continue;
        if (score > best_score || (score == best_score && routes[i].order < best_order)) {
            best = &routes[i];
            best_score = score;
            best_order = routes[i].order;
        }
    }

    if (best && params && params->type == V_OBJECT && !best->prefix)
        path_match(best->path, path, params);

    if (!best && !strcasecmp(method, "HEAD")) {
        for (size_t i = 0; i < route_count; i++) {
            if (strcasecmp(routes[i].method, "GET")) continue;
            int score = route_specificity(&routes[i], path);
            if (score < 0) continue;
            if (score > best_score || (score == best_score && routes[i].order < best_order)) {
                best = &routes[i];
                best_score = score;
                best_order = routes[i].order;
            }
        }
        if (best && params && params->type == V_OBJECT && !best->prefix)
            path_match(best->path, path, params);
    }

    return best;
}

static int path_has_route(const char *path)
{
    for (size_t i = 0; i < route_count; i++) {
        if (route_specificity(&routes[i], path) >= 0) return 1;
    }
    return 0;
}

static GSValue register_route(GSValue *args, size_t argc, const char *method)
{
    if (argc < 2 || args[0].type != V_STRING) return api->boolean(0);
    return api->boolean(route_add(method, api->cstring(args[0]), args[1], 0));
}

static GSValue route_use(GSValue *args, size_t argc)
{
    if (argc < 2 || args[0].type != V_STRING) return api->boolean(0);
    return api->boolean(route_add("*", api->cstring(args[0]), args[1], 1));
}

static GSValue route_get(GSValue *args, size_t argc)
{
    return register_route(args, argc, "GET");
}

static GSValue route_post(GSValue *args, size_t argc)
{
    return register_route(args, argc, "POST");
}

static GSValue route_put(GSValue *args, size_t argc)
{
    return register_route(args, argc, "PUT");
}

static GSValue route_patch(GSValue *args, size_t argc)
{
    return register_route(args, argc, "PATCH");
}

static GSValue route_delete(GSValue *args, size_t argc)
{
    return register_route(args, argc, "DELETE");
}

static GSValue route_options(GSValue *args, size_t argc)
{
    return register_route(args, argc, "OPTIONS");
}

static GSValue route_any(GSValue *args, size_t argc)
{
    return register_route(args, argc, "*");
}

static GSValue route_clear(GSValue *args, size_t argc)
{
    (void)args;
    (void)argc;
    routes_clear();
    return api->boolean(1);
}

static GSValue request_object(const HTTPRequest *request, GSValue params)
{
    GSValue result = api->object();
    if (!request) return result;

    GSValue value = api->string(request->method ? request->method : "");
    api->object_set(&result, "method", value);
    api->release(value);
    value = api->string(request->path ? request->path : "");
    api->object_set(&result, "path", value);
    api->release(value);
    value = api->string(request->query ? request->query : "");
    api->object_set(&result, "queryString", value);
    api->release(value);
    value = query_object(request->query);
    api->object_set(&result, "query", value);
    api->release(value);
    value = api->string(request->body ? request->body : "");
    api->object_set(&result, "body", value);
    api->release(value);
    api->object_set(&result, "headers", request->headers);
    if (params.type == V_OBJECT) api->object_set(&result, "params", params);
    return result;
}

static void request_free(HTTPRequest *request)
{
    if (!request) return;
    free(request->method);
    free(request->path);
    free(request->query);
    free(request->body);
    if (request->headers.type != V_NULL) api->release(request->headers);
    memset(request, 0, sizeof(*request));
}

static char *trim_copy(const char *start, size_t length)
{
    while (length && (*start == ' ' || *start == '\t' ||
                      *start == '\r' || *start == '\n')) {
        start++;
        length--;
    }

    while (length && (start[length - 1] == ' ' ||
                      start[length - 1] == '\t' ||
                      start[length - 1] == '\r' ||
                      start[length - 1] == '\n')) {
        length--;
    }

    char *copy = malloc(length + 1);
    if (!copy) return NULL;
    memcpy(copy, start, length);
    copy[length] = 0;
    return copy;
}

static int parse_http_request(Buffer *raw, HTTPRequest *request)
{
    memset(request, 0, sizeof(*request));
    request->headers = api->object();

    char *headers_end = strstr(raw->data, "\r\n\r\n");
    if (!headers_end) {
        request_free(request);
        return 0;
    }

    size_t header_bytes = (size_t)(headers_end - raw->data);
    char *header_copy = malloc(header_bytes + 1);
    if (!header_copy) {
        request_free(request);
        return 0;
    }

    memcpy(header_copy, raw->data, header_bytes);
    header_copy[header_bytes] = 0;

    char *line_end = strstr(header_copy, "\r\n");
    if (!line_end) {
        free(header_copy);
        request_free(request);
        return 0;
    }

    char *line = header_copy;
    *line_end = 0;

    char method[32];
    char target[8192];
    char version[32];

    if (sscanf(line, "%31s %8191s %31s", method, target, version) != 3) {
        free(header_copy);
        request_free(request);
        return 0;
    }

    request->method = dup_string(method);
    if (!request->method) {
        free(header_copy);
        request_free(request);
        return 0;
    }

    char *question = strchr(target, '?');
    if (question) {
        *question = 0;
        request->path = dup_string(target);
        request->query = dup_string(question + 1);
    } else {
        request->path = dup_string(target);
        request->query = dup_string("");
    }

    if (!request->path || !request->query) {
        free(header_copy);
        request_free(request);
        return 0;
    }

    size_t content_length = 0;
    char *header_line = line_end + 2;

    while (*header_line) {
        char *next = strstr(header_line, "\r\n");
        if (next) *next = 0;

        char *colon = strchr(header_line, ':');
        if (colon) {
            char *key = trim_copy(header_line, (size_t)(colon - header_line));
            char *value = trim_copy(colon + 1, strlen(colon + 1));

            if (key && value) {
                GSValue text = api->string(value);
                api->object_set(&request->headers, key, text);
                api->release(text);

                if (!strcasecmp(key, "Content-Length")) {
                    content_length = (size_t)strtoull(value, NULL, 10);
                }
            }

            free(key);
            free(value);
        }

        if (!next) break;
        header_line = next + 2;
    }

    const char *body_start = headers_end + 4;
    size_t available = raw->length - (size_t)(body_start - raw->data);

    if (content_length > 16 * 1024 * 1024) {
        free(header_copy);
        request_free(request);
        return 0;
    }

    request->body = malloc(content_length + 1);
    if (!request->body) {
        free(header_copy);
        request_free(request);
        return 0;
    }

    size_t body_available = available < content_length
        ? available
        : content_length;

    if (body_available)
        memcpy(request->body, body_start, body_available);

    request->body[body_available] = 0;

    free(header_copy);
    return 1;
}

static size_t request_content_length(HTTPRequest *request)
{
    GSValue value = api->object_get(request->headers, "Content-Length");
    size_t length = 0;

    if (value.type == V_STRING) {
        length = (size_t)strtoull(api->cstring(value), NULL, 10);
    }

    if (value.type != V_NULL) api->release(value);
    return length;
}

static int receive_request(int fd, SSL *ssl, int tls, Buffer *request)
{
    char data[16384];
    size_t expected = 0;
    int headers_seen = 0;

    for (;;) {
        int count = tls
            ? SSL_read(ssl, data, (int)sizeof data)
            : (int)recv(fd, data, sizeof data, 0);

        if (count <= 0) return 0;

        if (!buf_write(request, data, (size_t)count)) return 0;

        if (!headers_seen) {
            char *headers_end = strstr(request->data, "\r\n\r\n");
            if (headers_end) {
                headers_seen = 1;

                HTTPRequest parsed;
                if (!parse_http_request(request, &parsed)) return 0;

                expected = request_content_length(&parsed);
                request_free(&parsed);

                size_t header_length =
                    (size_t)(headers_end - request->data) + 4;
                if (request->length - header_length >= expected) return 1;
            }
        } else {
            char *headers_end = strstr(request->data, "\r\n\r\n");
            if (!headers_end) return 0;

            size_t header_length =
                (size_t)(headers_end - request->data) + 4;

            if (request->length - header_length >= expected) return 1;
        }

        if (request->length > 16 * 1024 * 1024) return 0;
    }
}

static int ssl_send_all(SSL *ssl, const void *data, size_t length)
{
    const char *p = data;

    while (length) {
        int chunk = length > 1024 * 1024 ? 1024 * 1024 : (int)length;
        int written = SSL_write(ssl, p, chunk);
        if (written <= 0) return 0;

        p += written;
        length -= (size_t)written;
    }

    return 1;
}

static int send_response(
    int fd,
    SSL *ssl,
    int tls,
    int status,
    const char *content_type,
    const char *body,
    size_t length,
    int head,
    GSValue headers
)
{
    const char *reason =
        status == 200 ? "OK" :
        status == 201 ? "Created" :
        status == 204 ? "No Content" :
        status == 301 ? "Moved Permanently" :
        status == 302 ? "Found" :
        status == 304 ? "Not Modified" :
        status == 400 ? "Bad Request" :
        status == 401 ? "Unauthorized" :
        status == 403 ? "Forbidden" :
        status == 404 ? "Not Found" :
        status == 405 ? "Method Not Allowed" :
        status == 409 ? "Conflict" :
        status == 422 ? "Unprocessable Entity" :
        status == 429 ? "Too Many Requests" :
        status == 500 ? "Internal Server Error" :
        status == 502 ? "Bad Gateway" :
        status == 503 ? "Service Unavailable" :
        "Response";

    if (!content_type || !*content_type)
        content_type = "text/plain; charset=utf-8";

    Buffer header;
    if (!buf_init(&header)) return 0;

    char line[1024];
    int n = snprintf(
        line,
        sizeof line,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Server: greenServeFE/1.0\r\n"
        "Connection: close\r\n",
        status,
        reason,
        content_type,
        length
    );

    if (n < 0 || !buf_write(&header, line, (size_t)n)) {
        buf_free(&header);
        return 0;
    }

    if (headers.type == V_OBJECT) {
        for (size_t i = 0; i < api->object_count(headers); i++) {
            const char *key = api->object_key(headers, i);
            GSValue value = api->object_get(headers, key);

            if (key && value.type == V_STRING) {
                n = snprintf(
                    line,
                    sizeof line,
                    "%s: %s\r\n",
                    key,
                    api->cstring(value)
                );

                if (n > 0) buf_write(&header, line, (size_t)n);
            }

            if (value.type != V_NULL) api->release(value);
        }
    }

    if (!buf_puts(&header, "\r\n")) {
        buf_free(&header);
        return 0;
    }

    int ok;

    if (tls) {
        ok = ssl_send_all(ssl, header.data, header.length);
        if (ok && !head && length)
            ok = ssl_send_all(ssl, body, length);
    } else {
        ok = send_all(fd, header.data, header.length);
        if (ok && !head && length)
            ok = send_all(fd, body, length);
    }

    buf_free(&header);
    return ok;
}

static GSValue query_object(const char *query)
{
    GSValue result = api->object();
    if (!query || !*query) return result;

    char *copy = dup_string(query);
    if (!copy) return result;

    char *save = NULL;
    char *item = strtok_r(copy, "&", &save);

    while (item) {
        char *equals = strchr(item, '=');

        if (equals) {
            *equals = 0;
            char *key = url_decode_copy(item);
            char *decoded = url_decode_copy(equals + 1);
            GSValue value = api->string(decoded ? decoded : equals + 1);
            api->object_set(&result, key ? key : item, value);
            free(key);
            free(decoded);
            api->release(value);
        } else {
            char *key = url_decode_copy(item);
            GSValue value = api->string("");
            api->object_set(&result, key ? key : item, value);
            free(key);
            api->release(value);
        }

        item = strtok_r(NULL, "&", &save);
    }

    free(copy);
    return result;
}

static int path_is_safe(const char *path)
{
    if (!path || !*path) return 0;
    if (strstr(path, "..")) return 0;
    if (strchr(path, '\\')) return 0;
    return 1;
}


typedef struct {
    int status;
    const char *content_type;
    char content_type_storage[256];
    GSValue headers;
    Buffer *body;
    const char *root;
    int ended;
} GSResponseState;

static GSResponseState *active_response;

static int path_is_safe(const char *path);
static const char *mime_type(const char *path);
static int read_file(const char *path, Buffer *buffer);
static int render_template(Buffer *out, const char *template, GSValue data);

static GSResponseState *response_state(void)
{
    return active_response;
}

static GSValue response_status(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state) return api->null();
    if (argc >= 1 && args[0].type == V_NUMBER)
        state->status = (int)api->number_value(args[0]);
    return api->boolean(1);
}

static GSValue response_set(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 2 ||
        args[0].type != V_STRING || args[1].type != V_STRING)
        return api->boolean(0);
    api->object_set(&state->headers, api->cstring(args[0]), args[1]);
    return api->boolean(1);
}

static GSValue response_type(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1 || args[0].type != V_STRING)
        return api->boolean(0);
    snprintf(
        state->content_type_storage,
        sizeof state->content_type_storage,
        "%s",
        api->cstring(args[0])
    );
    state->content_type = state->content_type_storage;
    return api->boolean(1);
}

static int json_write_value(Buffer *out, GSValue value)
{
    switch (value.type) {
        case V_NULL:
            return buf_puts(out, "null");
        case V_BOOL:
            return buf_puts(out, value.as.boolean ? "true" : "false");
        case V_NUMBER: {
            char number[128];
            snprintf(number, sizeof number, "%.15g", value.as.number);
            return buf_puts(out, number);
        }
        case V_STRING: {
            if (!buf_puts(out, "\"")) return 0;
            const char *text = api->cstring(value);
            for (; *text; text++) {
                switch (*text) {
                    case '"': if (!buf_puts(out, "\\\"")) return 0; break;
                    case '\\': if (!buf_puts(out, "\\\\")) return 0; break;
                    case '\n': if (!buf_puts(out, "\\n")) return 0; break;
                    case '\r': if (!buf_puts(out, "\\r")) return 0; break;
                    case '\t': if (!buf_puts(out, "\\t")) return 0; break;
                    default: if (!buf_write(out, text, 1)) return 0; break;
                }
            }
            return buf_puts(out, "\"");
        }
        case V_ARRAY: {
            if (!buf_puts(out, "[")) return 0;
            for (size_t i = 0; i < api->array_count(value); i++) {
                if (i && !buf_puts(out, ",")) return 0;
                GSValue item = api->array_get(value, i);
                int ok = json_write_value(out, item);
                if (item.type != V_NULL) api->release(item);
                if (!ok) return 0;
            }
            return buf_puts(out, "]");
        }
        case V_OBJECT: {
            if (!buf_puts(out, "{")) return 0;
            for (size_t i = 0; i < api->object_count(value); i++) {
                if (i && !buf_puts(out, ",")) return 0;
                const char *key = api->object_key(value, i);
                if (!buf_puts(out, "\"")) return 0;
                for (const char *p = key ? key : ""; *p; p++) {
                    if (*p == '"' || *p == '\\') {
                        if (!buf_write(out, "\\", 1)) return 0;
                    }
                    if (!buf_write(out, p, 1)) return 0;
                }
                if (!buf_puts(out, "\":")) return 0;
                GSValue item = api->object_get(value, key);
                int ok = json_write_value(out, item);
                if (item.type != V_NULL) api->release(item);
                if (!ok) return 0;
            }
            return buf_puts(out, "}");
        }
        default:
            return buf_puts(out, "null");
    }
}

static GSValue response_send(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1) return api->boolean(0);
    if (args[0].type == V_STRING) {
        if (!buf_puts(state->body, api->cstring(args[0]))) return api->boolean(0);
    } else {
        if (!json_write_value(state->body, args[0])) return api->boolean(0);
        if (!state->content_type)
            state->content_type = "application/json; charset=utf-8";
    }
    state->ended = 1;
    return api->boolean(1);
}

static GSValue response_json(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1) return api->boolean(0);
    if (!json_write_value(state->body, args[0])) return api->boolean(0);
    state->content_type = "application/json; charset=utf-8";
    state->ended = 1;
    return api->boolean(1);
}

static GSValue response_html(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1) return api->boolean(0);
    if (!buf_puts(state->body, api->cstring(args[0]))) return api->boolean(0);
    state->content_type = "text/html; charset=utf-8";
    state->ended = 1;
    return api->boolean(1);
}

static GSValue response_redirect(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1 || args[0].type != V_STRING)
        return api->boolean(0);
    GSValue location = api->string(api->cstring(args[0]));
    api->object_set(&state->headers, "Location", location);
    api->release(location);
    if (state->status == 200) state->status = 302;
    state->ended = 1;
    return api->boolean(1);
}

static GSValue response_send_file(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1 || args[0].type != V_STRING || !state->root)
        return api->boolean(0);
    const char *file = api->cstring(args[0]);
    if (!path_is_safe(file)) return api->boolean(0);

    char full[8192];
    int n = snprintf(full, sizeof full, "%s/%s", state->root, file);
    if (n < 0 || (size_t)n >= sizeof full) return api->boolean(0);

    Buffer file_body;
    if (!buf_init(&file_body) || !read_file(full, &file_body)) {
        if (file_body.data) buf_free(&file_body);
        return api->boolean(0);
    }

    int ok = buf_write(state->body, file_body.data, file_body.length);
    if (!state->content_type) state->content_type = mime_type(full);
    buf_free(&file_body);
    if (ok) state->ended = 1;
    return api->boolean(ok);
}

static GSValue response_render(GSValue *args, size_t argc)
{
    GSResponseState *state = response_state();
    if (!state || argc < 1 || args[0].type != V_STRING || !state->root)
        return api->boolean(0);

    const char *file = api->cstring(args[0]);
    if (!path_is_safe(file)) return api->boolean(0);

    char full[8192];
    int n = snprintf(full, sizeof full, "%s/%s", state->root, file);
    if (n < 0 || (size_t)n >= sizeof full) return api->boolean(0);

    Buffer source;
    Buffer rendered;
    if (!buf_init(&source) ||
        !read_file(full, &source) ||
        !buf_init(&rendered)) {
        if (source.data) buf_free(&source);
        return api->boolean(0);
    }

    GSValue data = argc >= 2 && args[1].type == V_OBJECT
        ? args[1]
        : api->object();

    int ok = render_template(&rendered, source.data, data);
    if (ok) {
        buf_free(state->body);
        *state->body = rendered;
        state->content_type = "text/html; charset=utf-8";
        state->ended = 1;
    } else {
        buf_free(&rendered);
    }

    buf_free(&source);
    if (argc < 2 || args[1].type != V_OBJECT) api->release(data);
    return api->boolean(ok);
}

static GSValue make_response_object(void)
{
    GSValue result = api->object();
    GSValue fn;

    fn = api->native(response_status); api->object_set(&result, "status", fn); api->release(fn);
    fn = api->native(response_set); api->object_set(&result, "set", fn); api->release(fn);
    fn = api->native(response_type); api->object_set(&result, "type", fn); api->release(fn);
    fn = api->native(response_send); api->object_set(&result, "send", fn); api->release(fn);
    fn = api->native(response_json); api->object_set(&result, "json", fn); api->release(fn);
    fn = api->native(response_html); api->object_set(&result, "html", fn); api->release(fn);
    fn = api->native(response_redirect); api->object_set(&result, "redirect", fn); api->release(fn);
    fn = api->native(response_send_file); api->object_set(&result, "sendFile", fn); api->release(fn);
    fn = api->native(response_render); api->object_set(&result, "render", fn); api->release(fn);
    return result;
}

static void apply_response_properties(GSValue response, GSResponseState *state)
{
    if (!state || response.type != V_OBJECT) return;

    GSValue value = api->object_get(response, "status");
    if (value.type == V_NUMBER)
        state->status = (int)api->number_value(value);
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(response, "body");
    if (value.type == V_STRING) {
        buf_free(state->body);
        if (!buf_init(state->body)) {
            api->release(value);
            return;
        }
        if (!buf_puts(state->body, api->cstring(value))) {
            api->release(value);
            return;
        }
    }
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(response, "contentType");
    if (value.type == V_STRING) {
        snprintf(state->content_type_storage, sizeof state->content_type_storage, "%s", api->cstring(value));
        state->content_type = state->content_type_storage;
    }
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(response, "headers");
    if (value.type == V_OBJECT) {
        api->release(state->headers);
        state->headers = api->copy(value);
    }
    if (value.type != V_NULL) api->release(value);

    if (state->body->length > 0) state->ended = 1;
}

static int invoke_route_handler(
    GSValue handler,
    const HTTPRequest *request,
    GSValue params,
    const char *root,
    Buffer *body,
    int *status,
    const char **content_type,
    GSValue *response_headers
)
{
    if (handler.type != V_FUNCTION || !api->call) return 0;

    GSResponseState state;
    memset(&state, 0, sizeof state);
    state.status = *status;
    state.body = body;
    state.content_type = *content_type;
    state.headers = api->object();
    state.root = root;

    GSValue req = request_object(request, params);
    GSValue res = make_response_object();
    GSValue argv[2] = { req, res };
    GSValue result = api->null();

    GSResponseState *previous = active_response;
    active_response = &state;
    int ok = api->call(api->context, handler, argv, 2, &result);
    apply_response_properties(res, &state);
    active_response = previous;

    api->release(req);
    api->release(res);

    int had_result = result.type != V_NULL;
    if (had_result && !state.ended) {
        if (result.type == V_STRING) {
            buf_puts(body, api->cstring(result));
        } else {
            json_write_value(body, result);
            if (!state.content_type)
                state.content_type = "application/json; charset=utf-8";
        }
    }

    if (result.type != V_NULL) api->release(result);

    *status = state.status;
    *content_type = state.content_type;
    api->release(*response_headers);
    *response_headers = state.headers;

    return ok && (state.ended || body->length > 0 || had_result);
}

static int read_route_handler(
    GSValue handler,
    const char *root,
    const HTTPRequest *request,
    GSValue params,
    Buffer *body,
    int *status,
    const char **content_type,
    GSValue *response_headers
)
{
    if (handler.type == V_FUNCTION) {
        return invoke_route_handler(
            handler, request, params, root, body, status,
            content_type, response_headers
        );
    }

    if (handler.type != V_OBJECT) {
        if (handler.type == V_STRING) {
            return buf_puts(body, api->cstring(handler));
        }
        return 0;
    }

    GSValue value = api->object_get(handler, "status");
    if (value.type == V_NUMBER) *status = (int)api->number_value(value);
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(handler, "headers");
    if (value.type == V_OBJECT) {
        *response_headers = api->copy(value);
    }
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(handler, "contentType");
    if (value.type == V_STRING) {
        static char type_storage[256];
        snprintf(
            type_storage,
            sizeof type_storage,
            "%s",
            api->cstring(value)
        );
        *content_type = type_storage;
    }
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(handler, "redirect");
    if (value.type == V_STRING) {
        GSValue redirect_headers = api->object_get(*response_headers, "Location");
        if (redirect_headers.type == V_NULL) {
            GSValue location = api->string(api->cstring(value));
            api->object_set(response_headers, "Location", location);
            api->release(location);
        }
        if (redirect_headers.type != V_NULL) api->release(redirect_headers);
        *status = *status == 200 ? 302 : *status;
    }
    if (value.type != V_NULL) api->release(value);

    value = api->object_get(handler, "body");
    if (value.type == V_STRING) {
        int ok = buf_puts(body, api->cstring(value));
        api->release(value);
        if (!ok) return 0;
    } else if (value.type != V_NULL) {
        api->release(value);
    }

    value = api->object_get(handler, "file");
    if (value.type == V_STRING) {
        const char *file = api->cstring(value);
        char full[8192];
        snprintf(full, sizeof full, "%s/%s", root, file);

        if (!path_is_safe(file)) {
            api->release(value);
            return 0;
        }

        Buffer file_body;
        if (!buf_init(&file_body) || !read_file(full, &file_body)) {
            if (file_body.data) buf_free(&file_body);
            api->release(value);
            return 0;
        }

        int ok = buf_write(body, file_body.data, file_body.length);
        if (!*content_type) *content_type = mime_type(full);
        buf_free(&file_body);
        api->release(value);
        if (!ok) return 0;
    } else if (value.type != V_NULL) {
        api->release(value);
    }

    value = api->object_get(handler, "template");
    if (value.type == V_STRING) {
        const char *file = api->cstring(value);
        char full[8192];

        if (!path_is_safe(file)) {
            api->release(value);
            return 0;
        }

        snprintf(full, sizeof full, "%s/%s", root, file);

        Buffer source;
        Buffer rendered;
        GSValue template_data = api->object();

        GSValue request_data = request_object(request, params);
        api->object_set(&template_data, "request", request_data);
        api->release(request_data);
        if (request) {
            GSValue value = api->string(request->method ? request->method : "");
            api->object_set(&template_data, "method", value);
            api->release(value);
            value = api->string(request->path ? request->path : "");
            api->object_set(&template_data, "path", value);
            api->release(value);
            value = api->string(request->query ? request->query : "");
            api->object_set(&template_data, "queryString", value);
            api->release(value);
            value = query_object(request->query);
            api->object_set(&template_data, "query", value);
            api->release(value);
            value = api->string(request->body ? request->body : "");
            api->object_set(&template_data, "body", value);
            api->release(value);
            api->object_set(&template_data, "headers", request->headers);
            api->object_set(&template_data, "params", params);
        }

        GSValue configured_data = api->object_get(handler, "data");
        if (configured_data.type == V_OBJECT) {
            for (size_t i = 0; i < api->object_count(configured_data); i++) {
                const char *key = api->object_key(configured_data, i);
                GSValue configured_value = api->object_get(configured_data, key);
                api->object_set(&template_data, key, configured_value);
                if (configured_value.type != V_NULL) api->release(configured_value);
            }
        }
        if (configured_data.type != V_NULL)
            api->release(configured_data);

        if (params.type == V_OBJECT) {
            for (size_t i = 0; i < api->object_count(params); i++) {
                const char *key = api->object_key(params, i);
                GSValue parameter = api->object_get(params, key);
                api->object_set(&template_data, key, parameter);
                if (parameter.type != V_NULL) api->release(parameter);
            }
        }

        if (!buf_init(&source) ||
            !read_file(full, &source) ||
            !buf_init(&rendered) ||
            !render_template(&rendered, source.data, template_data)) {
            if (source.data) buf_free(&source);
            if (rendered.data) buf_free(&rendered);
            api->release(template_data);
            api->release(value);
            return 0;
        }

        api->release(template_data);
        buf_free(body);
        *body = rendered;

        if (!*content_type)
            *content_type = "text/html; charset=utf-8";

        buf_free(&source);
    }

    if (!*content_type) {
        *content_type = "text/plain; charset=utf-8";
    }

    return 1;
}

static int serve_static(
    const char *root,
    const char *index,
    const char *path,
    Buffer *body,
    const char **content_type
)
{
    if (!path_is_safe(path)) return 404;

    const char *relative = path;

    if (relative[0] == '/') relative++;

    if (!*relative) relative = index;

    char full_path[8192];
    int n = snprintf(
        full_path,
        sizeof full_path,
        "%s/%s",
        root,
        relative
    );

    if (n < 0 || (size_t)n >= sizeof full_path) return 404;

    struct stat file_info;

    if (stat(full_path, &file_info) < 0 ||
        !S_ISREG(file_info.st_mode)) {
        return 404;
    }

    if (!read_file(full_path, body)) return 500;

    *content_type = mime_type(full_path);
    return 200;
}

static int handle_connection(
    int client,
    SSL *ssl,
    int tls,
    const char *root,
    const char *index,
    GSValue options
)
{
    Buffer raw;
    if (!buf_init(&raw)) return 0;

    if (!receive_request(client, ssl, tls, &raw)) {
        buf_free(&raw);
        return 0;
    }

    HTTPRequest request;
    if (!parse_http_request(&raw, &request)) {
        send_response(
            client,
            ssl,
            tls,
            400,
            "text/plain; charset=utf-8",
            "bad request",
            11,
            0,
            api->null()
        );
        buf_free(&raw);
        return 0;
    }

    int head = !strcasecmp(request.method, "HEAD");
    int method_allowed = request.method && *request.method;

    if (!method_allowed) {
        GSValue headers = api->object();
        GSValue allow = api->string(
            "GET, HEAD, POST, PUT, PATCH, DELETE, OPTIONS"
        );
        api->object_set(&headers, "Allow", allow);
        api->release(allow);

        send_response(
            client, ssl, tls, 405,
            "text/plain; charset=utf-8",
            "method not allowed",
            18,
            head,
            headers
        );

        api->release(headers);
        request_free(&request);
        buf_free(&raw);
        return 1;
    }

    GSValue params = api->object();
    Route *route = find_route(request.method, request.path, &params);

    Buffer response_body;
    if (!buf_init(&response_body)) {
        api->release(params);
        request_free(&request);
        buf_free(&raw);
        return 0;
    }

    int status = 404;
    const char *content_type = NULL;
    GSValue response_headers = api->object();

    if (route) {
        status = 200;

        if (!read_route_handler(
                route->handler,
                root,
                &request,
                params,
                &response_body,
                &status,
                &content_type,
                &response_headers)) {
            status = 500;
            buf_free(&response_body);
            buf_init(&response_body);
            buf_puts(&response_body, "route handler failed");
        }
    } else {
        GSValue static_routes = object_get(options, "static");
        int static_enabled = static_routes.type == V_NULL ||
                             api->truthy(static_routes);
        if (static_routes.type != V_NULL) api->release(static_routes);

        if (static_enabled &&
            (!strcasecmp(request.method, "GET") ||
             !strcasecmp(request.method, "HEAD"))) {
            status = serve_static(
                root,
                index,
                request.path,
                &response_body,
                &content_type
            );
        }

        if (status == 404) {
            if (path_has_route(request.path)) {
                status = 405;
                buf_free(&response_body);
                buf_init(&response_body);
                buf_puts(&response_body, "method not allowed");
                content_type = "text/plain; charset=utf-8";
            }

            const char *fallback = option_string(options, "notFound");
            if (status == 405) {
                const char *allow = "GET, HEAD, POST, PUT, PATCH, DELETE, OPTIONS";
                GSValue allow_header = api->string(allow);
                api->object_set(&response_headers, "Allow", allow_header);
                api->release(allow_header);
            } else if (fallback) {
                buf_puts(&response_body, fallback);
                content_type = "text/plain; charset=utf-8";
            } else {
                buf_puts(&response_body, "not found");
                content_type = "text/plain; charset=utf-8";
            }
        }
    }

    GSValue cors = object_get(options, "cors");
    if (cors.type != V_NULL && api->truthy(cors)) {
        GSValue origin = api->string("*");
        api->object_set(&response_headers, "Access-Control-Allow-Origin", origin);
        api->release(origin);

        origin = api->string("GET, HEAD, POST, PUT, PATCH, DELETE, OPTIONS");
        api->object_set(&response_headers, "Access-Control-Allow-Methods", origin);
        api->release(origin);

        origin = api->string("Content-Type, Authorization");
        api->object_set(&response_headers, "Access-Control-Allow-Headers", origin);
        api->release(origin);
    }
    if (cors.type != V_NULL) api->release(cors);

    if (!strcasecmp(request.method, "OPTIONS") && !route) {
        status = 204;
        buf_free(&response_body);
        buf_init(&response_body);
        response_body.length = 0;
        response_body.data[0] = 0;
    }

    send_response(
        client,
        ssl,
        tls,
        status,
        content_type,
        response_body.data ? response_body.data : "",
        response_body.length,
        head || status == 204,
        response_headers
    );

    api->release(response_headers);
    api->release(params);
    request_free(&request);
    buf_free(&response_body);
    buf_free(&raw);
    return 1;
}

static GSValue serve(GSValue *args, size_t argc)
{
    if (argc < 1 || args[0].type != V_STRING)
        return api->boolean(0);

    const char *root = api->cstring(args[0]);

    long port = argc >= 2 && args[1].type == V_NUMBER
        ? (long)api->number_value(args[1])
        : 8080;

    GSValue options = argc >= 3 && args[2].type == V_OBJECT
        ? args[2]
        : api->null();

    const char *protocol = option_string(options, "protocol");
    const char *certificate = option_string(options, "cert");
    const char *key = option_string(options, "key");
    const char *index = option_string(options, "index");
    const char *host = option_string(options, "host");

    if (!index) index = "index.html";
    if (!host) host = "0.0.0.0";

    int tls = protocol &&
        (!strcasecmp(protocol, "https") ||
         !strcasecmp(protocol, "tls"));

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) return api->boolean(0);

    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in address;
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    if (!strcasecmp(host, "0.0.0.0") || !strcasecmp(host, "*")) {
        address.sin_addr.s_addr = htonl(INADDR_ANY);
    } else if (inet_pton(AF_INET, host, &address.sin_addr) != 1) {
        close(server);
        return api->boolean(0);
    }
    address.sin_port = htons((uint16_t)port);

    long backlog = option_number(options, "backlog", 1024);
    if (backlog < 1) backlog = 1;

    if (bind(server, (struct sockaddr *)&address, sizeof address) < 0 ||
        listen(server, (int)backlog) < 0) {
        close(server);
        return api->boolean(0);
    }

    SSL_CTX *context = NULL;

    if (tls) {
        if (!certificate || !key) {
            close(server);
            return api->boolean(0);
        }

        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        context = SSL_CTX_new(TLS_server_method());

        if (!context ||
            SSL_CTX_use_certificate_file(
                context,
                certificate,
                SSL_FILETYPE_PEM
            ) <= 0 ||
            SSL_CTX_use_PrivateKey_file(
                context,
                key,
                SSL_FILETYPE_PEM
            ) <= 0 ||
            !SSL_CTX_check_private_key(context)) {
            if (context) SSL_CTX_free(context);
            close(server);
            return api->boolean(0);
        }
    }

    signal(SIGCHLD, SIG_IGN);

    long max_requests = option_number(options, "max_requests", 0);
    long served = 0;

    for (;;) {
        if (max_requests > 0 && served >= max_requests) break;

        int client = accept(server, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) continue;
            break;
        }

        pid_t child = fork();

        if (child < 0) {
            close(client);
            continue;
        }

        if (child == 0) {
            close(server);

            SSL *ssl = NULL;

            if (tls) {
                ssl = SSL_new(context);
                if (!ssl) {
                    close(client);
                    _exit(1);
                }

                SSL_set_fd(ssl, client);

                if (SSL_accept(ssl) <= 0) {
                    SSL_free(ssl);
                    close(client);
                    _exit(1);
                }
            }

            int ok = handle_connection(
                client,
                ssl,
                tls,
                root,
                index,
                options
            );

            if (ssl) {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }

            close(client);
            _exit(ok ? 0 : 1);
        }

        close(client);
        served++;
    }

    if (context) SSL_CTX_free(context);
    close(server);

    routes_clear();
    return api->number((double)served);
}

static GSValue http_method(
    GSValue *args,
    size_t argc,
    const char *method
)
{
    if (argc < 1 || args[0].type != V_STRING)
        return api->null();

    GSValue method_value = api->string(method);
    GSValue result;

    if (argc >= 3)
        result = http_request(
            (GSValue[]){
                method_value,
                args[0],
                args[1],
                args[2]
            },
            4
        );
    else if (argc >= 2)
        result = http_request(
            (GSValue[]){
                method_value,
                args[0],
                args[1]
            },
            3
        );
    else
        result = http_request(
            (GSValue[]){
                method_value,
                args[0]
            },
            2
        );

    api->release(method_value);
    return result;
}

static GSValue http_put(GSValue *args, size_t argc)
{
    return http_method(args, argc, "PUT");
}

static GSValue http_patch(GSValue *args, size_t argc)
{
    return http_method(args, argc, "PATCH");
}

static GSValue http_delete(GSValue *args, size_t argc)
{
    return http_method(args, argc, "DELETE");
}

static GSValue http_options(GSValue *args, size_t argc)
{
    return http_method(args, argc, "OPTIONS");
}

GSValue greenServeFE_module_init(const GSNativeAPI *native_api)
{
    api = native_api;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    GSValue module = api->object();
    GSValue fn;

    fn = api->native(http_request);
    api->object_set(&module, "request", fn);
    api->release(fn);

    fn = api->native(http_get);
    api->object_set(&module, "http_get", fn);
    api->release(fn);

    fn = api->native(https_get);
    api->object_set(&module, "https_get", fn);
    api->release(fn);

    fn = api->native(http_post);
    api->object_set(&module, "http_post", fn);
    api->release(fn);

    fn = api->native(http_put);
    api->object_set(&module, "http_put", fn);
    api->release(fn);

    fn = api->native(http_patch);
    api->object_set(&module, "http_patch", fn);
    api->release(fn);

    fn = api->native(http_delete);
    api->object_set(&module, "http_delete", fn);
    api->release(fn);

    fn = api->native(http_options);
    api->object_set(&module, "http_options", fn);
    api->release(fn);

    fn = api->native(file_read);
    api->object_set(&module, "read_file", fn);
    api->release(fn);

    fn = api->native(file_write);
    api->object_set(&module, "write_file", fn);
    api->release(fn);

    fn = api->native(render);
    api->object_set(&module, "render", fn);
    api->release(fn);

    fn = api->native(url_encode);
    api->object_set(&module, "url_encode", fn);
    api->release(fn);

    fn = api->native(json_escape);
    api->object_set(&module, "json_escape", fn);
    api->release(fn);

    fn = api->native(route_get);
    api->object_set(&module, "get", fn);
    api->release(fn);

    fn = api->native(route_post);
    api->object_set(&module, "post", fn);
    api->release(fn);

    fn = api->native(route_put);
    api->object_set(&module, "put", fn);
    api->release(fn);

    fn = api->native(route_patch);
    api->object_set(&module, "patch", fn);
    api->release(fn);

    fn = api->native(route_delete);
    api->object_set(&module, "delete", fn);
    api->release(fn);

    fn = api->native(route_options);
    api->object_set(&module, "options", fn);
    api->release(fn);

    fn = api->native(route_any);
    api->object_set(&module, "any", fn);
    api->release(fn);

    fn = api->native(route_use);
    api->object_set(&module, "use", fn);
    api->release(fn);

    fn = api->native(route_clear);
    api->object_set(&module, "clear_routes", fn);
    api->release(fn);

    fn = api->native(serve);
    api->object_set(&module, "serve", fn);
    api->release(fn);

    return module;
}
