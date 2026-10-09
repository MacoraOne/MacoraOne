#ifndef GSFE_VALUE_H
#define GSFE_VALUE_H
#include <stddef.h>
#include <stdio.h>
typedef struct GSValue GSValue;typedef struct GSArray GSArray;typedef struct GSObject GSObject;typedef struct GSEnvironment GSEnvironment;typedef struct GSCode GSCode;typedef GSValue(*GSNativeFn)(GSValue*,size_t);typedef struct GSIterator GSIterator;
typedef enum{V_NULL,V_NUMBER,V_BOOL,V_STRING,V_ARRAY,V_OBJECT,V_FUNCTION,V_NATIVE,V_ITER}GSValueType;
struct GSValue{GSValueType type;union{double number;int boolean;struct{char*data;size_t refs;}string;GSArray*array;GSObject*object;struct{GSCode*code;GSEnvironment*closure;}function;GSNativeFn native;GSIterator*iter;}as;};
struct GSIterator{size_t refs,index;GSValue iterable;};struct GSArray{size_t refs,count,capacity;GSValue*items;};typedef struct{char*key;GSValue value;}GSProperty;struct GSObject{size_t refs,count,capacity;GSProperty*items;};typedef struct GSBinding{char*name;GSValue value;struct GSBinding*next;}GSBinding;struct GSEnvironment{size_t refs;GSEnvironment*parent;GSBinding*bindings;};
const char *gs_type_name(GSValueType);
GSValue gs_null(void);GSValue gs_number(double);GSValue gs_bool(int);GSValue gs_string(const char*);GSValue gs_array(void);GSValue gs_object(void);GSValue gs_native(GSNativeFn);GSValue gs_function(GSCode*,GSEnvironment*);GSValue gs_iterator(GSValue);
void gs_retain(GSValue);void gs_release(GSValue);GSValue gs_copy(GSValue);int gs_truthy(GSValue);double gs_number_value(GSValue);const char*gs_cstring(GSValue);void gs_array_push(GSValue*,GSValue);size_t gs_array_count(GSValue);GSValue gs_array_get(GSValue,size_t);int gs_array_set(GSValue*,size_t,GSValue);size_t gs_object_count(GSValue);const char*gs_object_key(GSValue,size_t);GSValue gs_index(GSValue,GSValue);int gs_set_index(GSValue,GSValue,GSValue);void gs_object_set(GSValue*,const char*,GSValue);GSValue gs_object_get(GSValue,const char*);
GSEnvironment*env_new(GSEnvironment*);void env_retain(GSEnvironment*);void env_release(GSEnvironment*);void env_define(GSEnvironment*,const char*,GSValue);int env_set(GSEnvironment*,const char*,GSValue);int env_get(GSEnvironment*,const char*,GSValue*);void gs_print(FILE*,GSValue);int gs_equal(GSValue,GSValue);GSValue gs_binary(int,GSValue,GSValue);
#endif
