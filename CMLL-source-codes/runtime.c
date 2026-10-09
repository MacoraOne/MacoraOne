#include "runtime.h"
#include "common.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <sys/wait.h>

static CMLLValue value_null(void)
{
    CMLLValue value;

    memset(&value, 0, sizeof(value));

    value.type = VALUE_NULL;

    return value;
}

static CMLLValue value_number(
    double number
)
{
    CMLLValue value;

    memset(&value, 0, sizeof(value));

    value.type = VALUE_NUMBER;
    value.number = number;

    return value;
}

static CMLLValue value_boolean(
    int boolean
)
{
    CMLLValue value;

    memset(&value, 0, sizeof(value));

    value.type = VALUE_BOOLEAN;
    value.boolean = boolean != 0;

    return value;
}

static CMLLValue value_string(
    const char *string
)
{
    CMLLValue value;

    memset(&value, 0, sizeof(value));

    value.type = VALUE_STRING;
    value.string =
        cmll_strdup(
            string ? string : ""
        );

    return value;
}

static CMLLValue value_array(void)
{
    CMLLValue value;

    memset(&value, 0, sizeof(value));

    value.type = VALUE_ARRAY;

    return value;
}

static CMLLValue value_object(void)
{
    CMLLValue value;

    memset(&value, 0, sizeof(value));

    value.type = VALUE_OBJECT;

    return value;
}

static void value_free(
    CMLLValue *value
)
{
    size_t i;

    if (!value)
        return;

    free(value->string);

    for (
        i = 0;
        i < value->array.count;
        i++
    ) {
        value_free(
            value->array.items[i]
        );

        free(
            value->array.items[i]
        );
    }

    free(value->array.items);

    for (
        i = 0;
        i < value->object.count;
        i++
    ) {
        free(
            value->object.entries[i].name
        );

        value_free(
            value->object.entries[i].value
        );

        free(
            value->object.entries[i].value
        );
    }

    free(value->object.entries);

    memset(
        value,
        0,
        sizeof(*value)
    );

    value->type = VALUE_NULL;
}

static CMLLValue value_copy(
    const CMLLValue *source
)
{
    CMLLValue result;
    size_t i;

    if (!source)
        return value_null();

    memset(&result, 0, sizeof(result));

    result.type = source->type;
    result.number = source->number;
    result.boolean = source->boolean;

    if (source->string)
        result.string =
            cmll_strdup(
                source->string
            );

    for (
        i = 0;
        i < source->array.count;
        i++
    ) {
        CMLLValue *copy;

        copy = malloc(sizeof(CMLLValue));

        if (!copy)
            continue;

        *copy =
            value_copy(
                source->array.items[i]
            );

        if (
            result.array.count >=
            result.array.capacity
        ) {
            size_t capacity;

            capacity =
                result.array.capacity
                    ? result.array.capacity * 2
                    : 4;

            result.array.items =
                realloc(
                    result.array.items,
                    capacity *
                        sizeof(CMLLValue *)
                );

            result.array.capacity =
                capacity;
        }

        result.array.items[
            result.array.count++
        ] = copy;
    }

    for (
        i = 0;
        i < source->object.count;
        i++
    ) {
        CMLLObjectEntry *entry;

        if (
            result.object.count >=
            result.object.capacity
        ) {
            size_t capacity;

            capacity =
                result.object.capacity
                    ? result.object.capacity * 2
                    : 4;

            result.object.entries =
                realloc(
                    result.object.entries,
                    capacity *
                        sizeof(CMLLObjectEntry)
                );

            result.object.capacity =
                capacity;
        }

        entry =
            &result.object.entries[
                result.object.count++
            ];

        entry->name =
            cmll_strdup(
                source->object.entries[i].name
            );

        entry->value =
            malloc(sizeof(CMLLValue));

        *entry->value =
            value_copy(
                source->object.entries[i].value
            );
    }

    result.function =
        source->function;

    return result;
}

static int truthy(
    const CMLLValue *value
)
{
    if (!value)
        return 0;

    switch (value->type) {
        case VALUE_NULL:
            return 0;

        case VALUE_BOOLEAN:
            return value->boolean;

        case VALUE_NUMBER:
            return value->number != 0.0;

        case VALUE_STRING:
            return value->string &&
                   value->string[0] != '\0';

        case VALUE_ARRAY:
            return value->array.count > 0;

        case VALUE_OBJECT:
            return value->object.count > 0;

        default:
            return 1;
    }
}

static char *value_stringify(
    const CMLLValue *value
)
{
    char buffer[128];
    size_t i;
    size_t length;
    char *result;

    if (!value)
        return cmll_strdup("null");

    switch (value->type) {
        case VALUE_NULL:
            return cmll_strdup("null");

        case VALUE_BOOLEAN:
            return cmll_strdup(
                value->boolean
                    ? "true"
                    : "false"
            );

        case VALUE_NUMBER:
            snprintf(
                buffer,
                sizeof(buffer),
                "%.15g",
                value->number
            );

            return cmll_strdup(buffer);

        case VALUE_STRING:
            return cmll_strdup(
                value->string
                    ? value->string
                    : ""
            );

        case VALUE_ARRAY:
            length = 3;

            for (
                i = 0;
                i < value->array.count;
                i++
            ) {
                char *part;

                part =
                    value_stringify(
                        value->array.items[i]
                    );

                length += strlen(part) + 2;

                free(part);
            }

            result = malloc(length);

            if (!result)
                return cmll_strdup("");

            strcpy(result, "[");

            for (
                i = 0;
                i < value->array.count;
                i++
            ) {
                char *part;

                part =
                    value_stringify(
                        value->array.items[i]
                    );

                strcat(result, part);

                if (
                    i + 1 <
                    value->array.count
                ) {
                    strcat(result, ", ");
                }

                free(part);
            }

            strcat(result, "]");

            return result;

        case VALUE_OBJECT:
            length = 3;

            for (
                i = 0;
                i < value->object.count;
                i++
            ) {
                char *part;

                part =
                    value_stringify(
                        value->object.entries[i].value
                    );

                length +=
                    strlen(
                        value->object.entries[i].name
                    ) +
                    strlen(part) +
                    5;

                free(part);
            }

            result = malloc(length);

            if (!result)
                return cmll_strdup("");

            strcpy(result, "{");

            for (
                i = 0;
                i < value->object.count;
                i++
            ) {
                char *part;

                part =
                    value_stringify(
                        value->object.entries[i].value
                    );

                strcat(
                    result,
                    value->object.entries[i].name
                );

                strcat(result, ": ");
                strcat(result, part);

                if (
                    i + 1 <
                    value->object.count
                ) {
                    strcat(result, ", ");
                }

                free(part);
            }

            strcat(result, "}");

            return result;

        case VALUE_FUNCTION:
            return cmll_strdup("<function>");
    }

    return cmll_strdup("null");
}

static CMLLEnvironment *environment_new(
    CMLLEnvironment *parent
)
{
    CMLLEnvironment *environment;

    environment =
        calloc(
            1,
            sizeof(CMLLEnvironment)
        );

    if (!environment)
        return NULL;

    environment->parent = parent;

    return environment;
}

static void environment_free(
    CMLLEnvironment *environment
)
{
    size_t i;

    if (!environment)
        return;

    for (
        i = 0;
        i < environment->count;
        i++
    ) {
        free(environment->names[i]);

        value_free(
            &environment->values[i]
        );
    }

    free(environment->names);
    free(environment->values);

    free(environment);
}

static int environment_find_local(
    CMLLEnvironment *environment,
    const char *name
)
{
    size_t i;

    for (
        i = 0;
        i < environment->count;
        i++
    ) {
        if (
            strcmp(
                environment->names[i],
                name
            ) == 0
        ) {
            return (int)i;
        }
    }

    return -1;
}

static CMLLEnvironment *environment_owner(
    CMLLEnvironment *environment,
    const char *name
)
{
    while (environment) {
        if (
            environment_find_local(
                environment,
                name
            ) >= 0
        ) {
            return environment;
        }

        environment =
            environment->parent;
    }

    return NULL;
}

static void environment_set(
    CMLLEnvironment *environment,
    const char *name,
    CMLLValue value
)
{
    int index;

    index =
        environment_find_local(
            environment,
            name
        );

    if (index >= 0) {
        value_free(
            &environment->values[index]
        );

        environment->values[index] =
            value;

        return;
    }

    if (
        environment->count >=
        environment->capacity
    ) {
        size_t capacity;

        capacity =
            environment->capacity
                ? environment->capacity * 2
                : 8;

        environment->names =
            realloc(
                environment->names,
                capacity * sizeof(char *)
            );

        environment->values =
            realloc(
                environment->values,
                capacity *
                    sizeof(CMLLValue)
            );

        environment->capacity =
            capacity;
    }

    environment->names[
        environment->count
    ] = cmll_strdup(name);

    environment->values[
        environment->count++
    ] = value;
}

static int environment_get(
    CMLLEnvironment *environment,
    const char *name,
    CMLLValue *result
)
{
    CMLLEnvironment *owner;
    int index;

    owner =
        environment_owner(
            environment,
            name
        );

    if (!owner)
        return 0;

    index =
        environment_find_local(
            owner,
            name
        );

    *result =
        value_copy(
            &owner->values[index]
        );

    return 1;
}

static int environment_assign(
    CMLLEnvironment *environment,
    const char *name,
    CMLLValue value
)
{
    CMLLEnvironment *owner;
    int index;

    owner =
        environment_owner(
            environment,
            name
        );

    if (!owner)
        return 0;

    index =
        environment_find_local(
            owner,
            name
        );

    value_free(
        &owner->values[index]
    );

    owner->values[index] = value;

    return 1;
}

static CMLLValue eval(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    CMLLNode *node
);

static int execute(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    CMLLNode *node
);

static CMLLValue eval_binary(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    CMLLNode *node
)
{
    CMLLValue left;
    CMLLValue right;
    CMLLValue result;

    left =
        eval(
            runtime,
            environment,
            node->left
        );

    right =
        eval(
            runtime,
            environment,
            node->right
        );

    result = value_null();

    if (
        strcmp(node->value, "&&") == 0
    ) {
        result =
            value_boolean(
                truthy(&left) &&
                truthy(&right)
            );
    } else if (
        strcmp(node->value, "||") == 0
    ) {
        result =
            value_boolean(
                truthy(&left) ||
                truthy(&right)
            );
    } else if (
        strcmp(node->value, "+") == 0
    ) {
        if (
            left.type == VALUE_NUMBER &&
            right.type == VALUE_NUMBER
        ) {
            result =
                value_number(
                    left.number +
                    right.number
                );
        } else {
            char *a;
            char *b;
            size_t length;

            a = value_stringify(&left);
            b = value_stringify(&right);

            length =
                strlen(a) +
                strlen(b) +
                1;

            result.string =
                malloc(length);

            if (result.string) {
                strcpy(result.string, a);
                strcat(result.string, b);
            }

            result.type = VALUE_STRING;

            free(a);
            free(b);
        }
    } else if (
        strcmp(node->value, "-") == 0
    ) {
        result =
            value_number(
                left.number -
                right.number
            );
    } else if (
        strcmp(node->value, "*") == 0
    ) {
        result =
            value_number(
                left.number *
                right.number
            );
    } else if (
        strcmp(node->value, "/") == 0
    ) {
        if (right.number == 0.0) {
            cmll_error(
                node->position,
                runtime->source,
                "division by zero"
            );

            runtime->failed = 1;
        } else {
            result =
                value_number(
                    left.number /
                    right.number
                );
        }
    } else if (
        strcmp(node->value, "%") == 0
    ) {
        result =
            value_number(
                fmod(
                    left.number,
                    right.number
                )
            );
    } else if (
        strcmp(node->value, "<") == 0
    ) {
        result =
            value_boolean(
                left.number <
                right.number
            );
    } else if (
        strcmp(node->value, "<=") == 0
    ) {
        result =
            value_boolean(
                left.number <=
                right.number
            );
    } else if (
        strcmp(node->value, ">") == 0
    ) {
        result =
            value_boolean(
                left.number >
                right.number
            );
    } else if (
        strcmp(node->value, ">=") == 0
    ) {
        result =
            value_boolean(
                left.number >=
                right.number
            );
    } else if (
        strcmp(node->value, "==") == 0
    ) {
        if (
            left.type == VALUE_NUMBER &&
            right.type == VALUE_NUMBER
        ) {
            result =
                value_boolean(
                    left.number ==
                    right.number
                );
        } else {
            char *a;
            char *b;

            a = value_stringify(&left);
            b = value_stringify(&right);

            result =
                value_boolean(
                    strcmp(a, b) == 0
                );

            free(a);
            free(b);
        }
    } else if (
        strcmp(node->value, "!=") == 0
    ) {
        if (
            left.type == VALUE_NUMBER &&
            right.type == VALUE_NUMBER
        ) {
            result =
                value_boolean(
                    left.number !=
                    right.number
                );
        } else {
            char *a;
            char *b;

            a = value_stringify(&left);
            b = value_stringify(&right);

            result =
                value_boolean(
                    strcmp(a, b) != 0
                );

            free(a);
            free(b);
        }
    }

    value_free(&left);
    value_free(&right);

    return result;
}

static CMLLValue eval(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    CMLLNode *node
)
{
    CMLLValue value;

    if (!node)
        return value_null();

    switch (node->type) {
        case NODE_LITERAL:
            if (
                strcmp(
                    node->value2,
                    "number"
                ) == 0
            ) {
                return value_number(
                    strtod(
                        node->value,
                        NULL
                    )
                );
            }

            if (
                strcmp(
                    node->value2,
                    "boolean"
                ) == 0
            ) {
                return value_boolean(
                    strcmp(
                        node->value,
                        "true"
                    ) == 0
                );
            }

            if (
                strcmp(
                    node->value2,
                    "null"
                ) == 0
            ) {
                return value_null();
            }

            return value_string(
                node->value
            );

        case NODE_VARIABLE:
            if (
                environment_get(
                    environment,
                    node->value,
                    &value
                )
            ) {
                return value;
            }

            cmll_error(
                node->position,
                runtime->source,
                "unknown variable"
            );

            runtime->failed = 1;

            return value_null();

        case NODE_ARRAY:
            {
                CMLLValue array;
                size_t i;

                array = value_array();

                for (
                    i = 0;
                    i < node->child_count;
                    i++
                ) {
                    CMLLValue *item;

                    item =
                        malloc(
                            sizeof(CMLLValue)
                        );

                    if (!item)
                        continue;

                    *item =
                        eval(
                            runtime,
                            environment,
                            node->children[i]
                        );

                    if (
                        array.array.count >=
                        array.array.capacity
                    ) {
                        size_t capacity;

                        capacity =
                            array.array.capacity
                                ? array.array.capacity * 2
                                : 4;

                        array.array.items =
                            realloc(
                                array.array.items,
                                capacity *
                                    sizeof(CMLLValue *)
                            );

                        array.array.capacity =
                            capacity;
                    }

                    array.array.items[
                        array.array.count++
                    ] = item;
                }

                return array;
            }

        case NODE_OBJECT:
            {
                CMLLValue object;
                size_t i;

                object = value_object();

                for (
                    i = 0;
                    i + 1 < node->child_count;
                    i += 2
                ) {
                    CMLLValue key;
                    CMLLValue property;

                    key =
                        eval(
                            runtime,
                            environment,
                            node->children[i]
                        );

                    property =
                        eval(
                            runtime,
                            environment,
                            node->children[i + 1]
                        );

                    if (
                        object.object.count >=
                        object.object.capacity
                    ) {
                        size_t capacity;

                        capacity =
                            object.object.capacity
                                ? object.object.capacity * 2
                                : 4;

                        object.object.entries =
                            realloc(
                                object.object.entries,
                                capacity *
                                    sizeof(CMLLObjectEntry)
                            );

                        object.object.capacity =
                            capacity;
                    }

                    object.object.entries[
                        object.object.count
                    ].name =
                        value_stringify(&key);

                    object.object.entries[
                        object.object.count
                    ].value =
                        malloc(
                            sizeof(CMLLValue)
                        );

                    *object.object.entries[
                        object.object.count
                    ].value = property;

                    object.object.count++;

                    value_free(&key);
                }

                return object;
            }

        case NODE_PROPERTY:
            {
                CMLLValue object;
                size_t i;

                object =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                if (
                    object.type !=
                    VALUE_OBJECT
                ) {
                    cmll_error(
                        node->position,
                        runtime->source,
                        "property access requires an object"
                    );

                    value_free(&object);
                    runtime->failed = 1;

                    return value_null();
                }

                for (
                    i = 0;
                    i < object.object.count;
                    i++
                ) {
                    if (
                        strcmp(
                            object.object.entries[i].name,
                            node->value
                        ) == 0
                    ) {
                        value =
                            value_copy(
                                object.object.entries[i].value
                            );

                        value_free(&object);

                        return value;
                    }
                }

                value_free(&object);

                return value_null();
            }

        case NODE_BINARY:
            return eval_binary(
                runtime,
                environment,
                node
            );

        case NODE_UNARY:
            {
                CMLLValue operand;

                operand =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                if (
                    strcmp(node->value, "!") == 0
                ) {
                    value =
                        value_boolean(
                            !truthy(&operand)
                        );
                } else {
                    value =
                        value_number(
                            -operand.number
                        );
                }

                value_free(&operand);

                return value;
            }

        case NODE_CALL:
            {
                CMLLValue function;

                function =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                if (
                    function.type !=
                    VALUE_FUNCTION
                ) {
                    cmll_error(
                        node->position,
                        runtime->source,
                        "value is not callable"
                    );

                    value_free(&function);
                    runtime->failed = 1;

                    return value_null();
                }

                {
                    CMLLNode *declaration;
                    CMLLEnvironment *call_environment;
                    size_t parameter_count;
                    size_t i;
                    int result;

                    declaration =
                        function.function.declaration;

                    call_environment =
                        environment_new(
                            function.function.closure
                        );

                    parameter_count =
                        declaration->child_count - 1;

                    if (
                        node->child_count !=
                        parameter_count
                    ) {
                        cmll_error(
                            node->position,
                            runtime->source,
                            "incorrect number of arguments"
                        );

                        environment_free(
                            call_environment
                        );

                        value_free(&function);

                        runtime->failed = 1;

                        return value_null();
                    }

                    for (
                        i = 0;
                        i < parameter_count;
                        i++
                    ) {
                        CMLLValue argument;

                        argument =
                            eval(
                                runtime,
                                environment,
                                node->children[i]
                            );

                        environment_set(
                            call_environment,
                            declaration->children[i]->value,
                            argument
                        );
                    }

                    runtime->return_active = 0;

                    result =
                        execute(
                            runtime,
                            call_environment,
                            declaration->children[
                                parameter_count
                            ]
                        );

                    (void)result;

                    value =
                        value_copy(
                            &runtime->return_value
                        );

                    value_free(
                        &runtime->return_value
                    );

                    runtime->return_active = 0;

                    environment_free(
                        call_environment
                    );

                    value_free(&function);

                    return value;
                }
            }

        default:
            break;
    }

    return value_null();
}

static int execute_block(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    CMLLNode *block
)
{
    size_t i;

    for (
        i = 0;
        i < block->child_count;
        i++
    ) {
        if (
            execute(
                runtime,
                environment,
                block->children[i]
            ) != 0
        ) {
            return 1;
        }

        if (
            runtime->return_active ||
            runtime->failed
        ) {
            return 0;
        }
    }

    return 0;
}

static int execute_command(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    const char *command
)
{
    int status;

    status = system(command);

    if (status == -1) {
        cmll_error(
            (CMLLPosition){
                runtime->filename,
                1,
                1
            },
            runtime->source,
            "failed to start process"
        );

        return 1;
    }

#ifndef _WIN32
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
#endif

    (void)environment;

    return status;
}

static int execute(
    CMLLRuntime *runtime,
    CMLLEnvironment *environment,
    CMLLNode *node
)
{
    if (!node)
        return 0;

    switch (node->type) {
        case NODE_PROGRAM:
            return execute_block(
                runtime,
                environment,
                node
            );

        case NODE_PROJECT:
            {
                CMLLValue value;
                char *text;

                value =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                text =
                    value_stringify(&value);

                printf(
                    "Project: %s\n",
                    text
                );

                free(text);
                value_free(&value);

                return 0;
            }

        case NODE_SET:
            {
                CMLLValue value;

                value =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                environment_set(
                    environment,
                    node->value,
                    value
                );

                return 0;
            }

        case NODE_ASSIGN:
            {
                CMLLValue value;

                value =
                    eval(
                        runtime,
                        environment,
                        node->right
                    );

                if (
                    node->left->type ==
                    NODE_VARIABLE
                ) {
                    if (
                        !environment_assign(
                            environment,
                            node->left->value,
                            value
                        )
                    ) {
                        environment_set(
                            environment,
                            node->left->value,
                            value
                        );
                    }

                    return 0;
                }

                value_free(&value);

                cmll_error(
                    node->position,
                    runtime->source,
                    "assignment target must be a variable"
                );

                runtime->failed = 1;

                return 1;
            }

        case NODE_IF:
            {
                CMLLValue condition;

                condition =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                if (truthy(&condition)) {
                    value_free(&condition);

                    return execute(
                        runtime,
                        environment,
                        node->right
                    );
                }

                value_free(&condition);

                if (
                    node->child_count > 0
                ) {
                    return execute(
                        runtime,
                        environment,
                        node->children[0]
                    );
                }

                return 0;
            }

        case NODE_WHILE:
            {
                int guard = 0;

                while (!runtime->failed) {
                    CMLLValue condition;

                    condition =
                        eval(
                            runtime,
                            environment,
                            node->left
                        );

                    if (!truthy(&condition)) {
                        value_free(&condition);
                        break;
                    }

                    value_free(&condition);

                    execute(
                        runtime,
                        environment,
                        node->right
                    );

                    if (
                        runtime->return_active
                    ) {
                        break;
                    }

                    guard++;

                    if (guard > 1000000) {
                        cmll_error(
                            node->position,
                            runtime->source,
                            "loop exceeded one million iterations"
                        );

                        runtime->failed = 1;

                        break;
                    }
                }

                return runtime->failed;
            }

        case NODE_FOR:
            {
                CMLLValue collection;
                size_t i;

                collection =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                if (
                    collection.type !=
                    VALUE_ARRAY
                ) {
                    cmll_error(
                        node->position,
                        runtime->source,
                        "'for' requires an array"
                    );

                    value_free(&collection);

                    runtime->failed = 1;

                    return 1;
                }

                for (
                    i = 0;
                    i < collection.array.count;
                    i++
                ) {
                    CMLLValue item;

                    item =
                        value_copy(
                            collection.array.items[i]
                        );

                    environment_set(
                        environment,
                        node->value,
                        item
                    );

                    execute(
                        runtime,
                        environment,
                        node->right
                    );

                    if (
                        runtime->return_active ||
                        runtime->failed
                    ) {
                        break;
                    }
                }

                value_free(&collection);

                return runtime->failed;
            }

        case NODE_RETURN:
            {
                if (node->left) {
                    runtime->return_value =
                        eval(
                            runtime,
                            environment,
                            node->left
                        );
                } else {
                    runtime->return_value =
                        value_null();
                }

                runtime->return_active = 1;

                return 0;
            }

        case NODE_FUNCTION:
        case NODE_TASK:
            {
                CMLLValue function;

                memset(
                    &function,
                    0,
                    sizeof(function)
                );

                function.type =
                    VALUE_FUNCTION;

                function.function.declaration =
                    node;

                function.function.closure =
                    environment;

                environment_set(
                    environment,
                    node->value,
                    function
                );

                return 0;
            }

        case NODE_SAY:
        case NODE_SHOW:
            {
                CMLLValue value;
                char *text;

                value =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                text =
                    value_stringify(&value);

                printf(
                    "%s\n",
                    text
                );

                free(text);
                value_free(&value);

                return 0;
            }

        case NODE_RUN:
            {
                CMLLValue command;
                char *text;
                int status;

                command =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                text =
                    value_stringify(&command);

                printf(
                    "→ %s\n",
                    text
                );

                status =
                    execute_command(
                        runtime,
                        environment,
                        text
                    );

                free(text);
                value_free(&command);

                if (status != 0) {
                    char message[128];

                    snprintf(
                        message,
                        sizeof(message),
                        "process exited with status %d",
                        status
                    );

                    cmll_error(
                        node->position,
                        runtime->source,
                        message
                    );

                    runtime->failed = 1;

                    return 1;
                }

                return 0;
            }

        case NODE_READ_FILE:
            {
                CMLLValue filename;
                char *name;
                char *content;

                filename =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                name =
                    value_stringify(&filename);

                content =
                    cmll_read_file(
                        name,
                        NULL
                    );

                if (!content) {
                    cmll_error(
                        node->position,
                        runtime->source,
                        "unable to read file"
                    );

                    free(name);
                    value_free(&filename);

                    runtime->failed = 1;

                    return 1;
                }

                environment_set(
                    environment,
                    node->value,
                    value_string(content)
                );

                free(content);
                free(name);
                value_free(&filename);

                return 0;
            }

        case NODE_WRITE_FILE:
            {
                CMLLValue filename;
                CMLLValue content;
                char *name;
                char *text;

                filename =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                content =
                    eval(
                        runtime,
                        environment,
                        node->right
                    );

                name =
                    value_stringify(&filename);

                text =
                    value_stringify(&content);

                if (
                    !cmll_write_file(
                        name,
                        text
                    )
                ) {
                    cmll_error(
                        node->position,
                        runtime->source,
                        "unable to write file"
                    );

                    runtime->failed = 1;
                }

                free(name);
                free(text);

                value_free(&filename);
                value_free(&content);

                return runtime->failed;
            }

        case NODE_FILE_EXISTS:
            return 0;

        case NODE_LIST_URLS:
            {
                CMLLValue source;
                char *text;
                const char *cursor;

                source =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                text =
                    value_stringify(&source);

                cursor = text;

                while (*cursor) {
                    const char *start;

                    if (
                        strncmp(
                            cursor,
                            "http://",
                            7
                        ) == 0 ||
                        strncmp(
                            cursor,
                            "https://",
                            8
                        ) == 0
                    ) {
                        start = cursor;

                        while (
                            *cursor &&
                            !isspace(
                                (unsigned char)*cursor
                            ) &&
                            *cursor != '"' &&
                            *cursor != '\'' &&
                            *cursor != ')' &&
                            *cursor != ']'
                        ) {
                            cursor++;
                        }

                        printf(
                            "%.*s\n",
                            (int)(cursor - start),
                            start
                        );

                        continue;
                    }

                    cursor++;
                }

                free(text);
                value_free(&source);

                return 0;
            }

        case NODE_ENV:
            {
                CMLLValue name;
                char *key;
                const char *value;

                name =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                key =
                    value_stringify(&name);

                value =
                    getenv(key);

                if (value)
                    printf(
                        "%s\n",
                        value
                    );

                free(key);
                value_free(&name);

                return 0;
            }

        case NODE_EXPRESSION_STATEMENT:
            {
                CMLLValue value;

                value =
                    eval(
                        runtime,
                        environment,
                        node->left
                    );

                value_free(&value);

                return 0;
            }

        default:
            return 0;
    }
}

void cmll_runtime_init(
    CMLLRuntime *runtime,
    CMLLNode *program,
    const char *filename,
    const char *source
)
{
    memset(
        runtime,
        0,
        sizeof(*runtime)
    );

    runtime->program = program;
    runtime->filename = filename;
    runtime->source = source;

    runtime->global =
        environment_new(NULL);

    runtime->return_value =
        value_null();
}

int cmll_runtime_execute(
    CMLLRuntime *runtime
)
{
    size_t i;

    if (!runtime->program)
        return 1;

    /*
     * Declarations are registered first.
     * This allows functions/tasks to be called
     * before their declaration in the source.
     */
    for (
        i = 0;
        i < runtime->program->child_count;
        i++
    ) {
        CMLLNode *child;

        child =
            runtime->program->children[i];

        if (
            child->type == NODE_FUNCTION ||
            child->type == NODE_TASK
        ) {
            execute(
                runtime,
                runtime->global,
                child
            );
        }
    }

    /*
     * Execute ordinary top-level statements.
     */
    for (
        i = 0;
        i < runtime->program->child_count;
        i++
    ) {
        CMLLNode *child;

        child =
            runtime->program->children[i];

        if (
            child->type == NODE_FUNCTION ||
            child->type == NODE_TASK
        ) {
            continue;
        }

        execute(
            runtime,
            runtime->global,
            child
        );

        if (runtime->failed)
            break;
    }

    return runtime->failed ? 1 : 0;
}

void cmll_runtime_free(
    CMLLRuntime *runtime
)
{
    if (!runtime)
        return;

    value_free(
        &runtime->return_value
    );

    environment_free(
        runtime->global
    );

    runtime->global = NULL;
}
