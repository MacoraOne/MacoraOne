#ifndef MACORAONE_H
#define MACORAONE_H

#include <stddef.h>

#define MACORAONE_VERSION "1.0.0"
#define MACORAONE_NAME "MacoraOne"

typedef enum {
    MACORAONE_JAVA = 1,
    MACORAONE_KOTLIN = 2
} MacoraLanguage;

typedef struct {
    const char *name;
    const char *url;
} MacoraRepository;

int macora_run(const char *command, int verbose);
int macora_command(const char *command, char *output, size_t output_size);
int macora_exists(const char *path);
int macora_is_directory(const char *path);
int macora_mkdir_p(const char *path);
int macora_copy_tree(const char *src, const char *dst);
int macora_remove_tree(const char *path);
int macora_validate_name(const char *name);
int macora_open_url(const char *url);
int macora_authenticate(void);
int macora_install(void);
int macora_uninstall(void);
int macora_create(const char *app_name);
int macora_push(const char *app_name, const char *repo_name);
int macora_build(const char *app_name, const char *mode);
void macora_help(const char *program);
void macora_version(void);

#endif
