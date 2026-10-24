#ifndef GSFE_BYTECODE_H
#define GSFE_BYTECODE_H
#include <stddef.h>
#include <stdint.h>
#include "value.h"
typedef enum{OP_NOP,OP_LOAD_CONST,OP_LOAD_NAME,OP_STORE_NAME,OP_POP_TOP,OP_ADD,OP_SUB,OP_MUL,OP_DIV,OP_MOD,OP_NEG,OP_NOT,OP_EQ,OP_NE,OP_LT,OP_LE,OP_GT,OP_GE,OP_AND,OP_OR,OP_JUMP,OP_JUMP_IF_FALSE,OP_JUMP_IF_TRUE,OP_CALL,OP_RETURN,OP_BUILD_ARRAY,OP_BUILD_OBJECT,OP_GET_INDEX,OP_SET_INDEX,OP_GET_MEMBER,OP_SET_MEMBER,OP_MAKE_FUNCTION,OP_IMPORT,OP_FROM_IMPORT,OP_ITER_INIT,OP_ITER_NEXT,OP_HALT}GSOp;
typedef struct{uint8_t op;int32_t arg;int line;}GSInstr;
struct GSCode{size_t refs;char*name;GSInstr*code;size_t count,capacity;GSValue*constants;size_t const_count,const_capacity;char**names;size_t name_count,name_capacity;GSCode**children;size_t child_count,child_capacity;int param_count;};
GSCode*code_new(const char*);void code_retain(GSCode*);void code_release(GSCode*);int code_emit(GSCode*,GSOp,int,int);int code_add_const(GSCode*,GSValue);int code_add_name(GSCode*,const char*);int code_add_child(GSCode*,GSCode*);void code_patch(GSCode*,int,int);const char*op_name(GSOp);void code_disassemble(const GSCode*,const char*);
#endif
