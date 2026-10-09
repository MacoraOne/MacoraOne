#ifndef CMLL_RUNTIME_H
#define CMLL_RUNTIME_H

#include "parser.h"

typedef enum {
    VALUE_NULL,
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_BOOLEAN,
    VALUE_ARRAY,
    VALUE_OBJECT,
    VALUE_FUNCTION
} CMLLValueType;

typedef struct CMLLValue CMLLValue;

typedef struct {
    char *name;
    CMLLValue *value;
} CMLLObjectEntry;

typedef struct {
    CMLLValue **items;
    size_t count;
    size_t capacity;
} CMLLArray;

typedef struct {
    CMLLObjectEntry *entries;
    size_t count;
    size_t capacity;
} CMLLObject;

typedef struct CMLLEnvironment CMLLEnvironment;

typedef struct {
    CMLLNode *declaration;
    CMLLEnvironment *closure;
} CMLLFunction;

struct CMLLValue {
    CMLLValueType type;

    double number;
    int boolean;

    char *string;

    CMLLArray array;
    CMLLObject object;

    CMLLFunction function;
};

struct CMLLEnvironment {
    CMLLEnvironment *parent;

    char **names;
    CMLLValue *values;

    size_t count;
    size_t capacity;
};

typedef struct {
    CMLLNode *program;

    const char *filename;
    const char *source;

    CMLLEnvironment *global;

    int failed;
    int return_active;

    CMLLValue return_value;
} CMLLRuntime;

void cmll_runtime_init(
    CMLLRuntime *runtime,
    CMLLNode *program,
    const char *filename,
    const char *source
);

int cmll_runtime_execute(
    CMLLRuntime *runtime
);

void cmll_runtime_free(
    CMLLRuntime *runtime
);

#endif
