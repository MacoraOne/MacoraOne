#ifndef GSFE_MODULE_H
#define GSFE_MODULE_H

#include <stddef.h>
#include <stdint.h>
#include "value.h"

#define GSFE_MODULE_ABI_VERSION 1u
#define GSFE_MODULE_INIT_SYMBOL "greenServeFE_module_init"

typedef struct GSNativeAPI {
    uint32_t abi_version;
    GSValue (*null)(void);
    GSValue (*number)(double);
    GSValue (*boolean)(int);
    GSValue (*string)(const char *);
    GSValue (*object)(void);
    GSValue (*array)(void);
    GSValue (*native)(GSNativeFn);
    GSValue (*copy)(GSValue);
    void (*retain)(GSValue);
    void (*release)(GSValue);
    void (*array_push)(GSValue *, GSValue);
    size_t (*array_count)(GSValue);
    GSValue (*array_get)(GSValue, size_t);
    int (*array_set)(GSValue *, size_t, GSValue);
    size_t (*object_count)(GSValue);
    const char *(*object_key)(GSValue, size_t);
    void (*object_set)(GSValue *, const char *, GSValue);
    GSValue (*object_get)(GSValue, const char *);
    const char *(*cstring)(GSValue);
    int (*truthy)(GSValue);
    double (*number_value)(GSValue);
    void (*print)(FILE *, GSValue);
    void *context;
    int (*call)(void *, GSValue, GSValue *, size_t, GSValue *);
} GSNativeAPI;

typedef GSValue (*GSModuleInit)(const GSNativeAPI *api);

#endif
