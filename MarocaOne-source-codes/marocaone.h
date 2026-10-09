#ifndef MAROCAONE_H
#define MAROCAONE_H

#include <stddef.h>

#define MAROCAONE_VERSION "1.0.0"
#define MAROCAONE_NAME "MarocaOne"

typedef enum {
    MAROCAONE_JAVA = 1,
    MAROCAONE_KOTLIN = 2
} MarocaLanguage;

typedef struct {
    const char *name;
    const char *url;
} MarocaRepository;

int maroca_run(const char *command, int verbose);
int maroca_command(const char *command, char *output, size_t output_size);
int maroca_exists(const char *path);
int maroca_is_directory(const char *path);
int maroca_mkdir_p(const char *path);
int maroca_copy_tree(const char *src, const char *dst);
int maroca_remove_tree(const char *path);
int maroca_validate_name(const char *name);
int maroca_open_url(const char *url);
int maroca_authenticate(void);
int maroca_install(void);
int maroca_uninstall(void);
int maroca_create(const char *app_name);
int maroca_push(const char *app_name, const char *repo_name);
int maroca_build(const char *app_name, const char *mode);
void maroca_help(const char *program);
void maroca_version(void);

#endif
