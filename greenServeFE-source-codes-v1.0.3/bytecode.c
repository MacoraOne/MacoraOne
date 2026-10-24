#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char*sd(const char*s){return strdup(s?s:"");}
static void*grow(void*p,size_t*n,size_t es){size_t c=*n?*n*2:8;void*q=realloc(p,c*es);if(!q)exit(2);*n=c;return q;}
GSCode*code_new(const char*n){GSCode*c=calloc(1,sizeof*c);if(!c)exit(2);c->refs=1;c->name=sd(n);return c;}
void code_retain(GSCode*c){if(c)c->refs++;}
void code_release(GSCode*c){if(!c||--c->refs)return;for(size_t i=0;i<c->const_count;i++)gs_release(c->constants[i]);for(size_t i=0;i<c->child_count;i++)code_release(c->children[i]);for(size_t i=0;i<c->name_count;i++)free(c->names[i]);free(c->name);free(c->code);free(c->constants);free(c->names);free(c->children);free(c);}
int code_emit(GSCode*c,GSOp op,int arg,int line){if(c->count==c->capacity)c->code=grow(c->code,&c->capacity,sizeof*c->code);c->code[c->count]=(GSInstr){(uint8_t)op,arg,line};return(int)c->count++;}
int code_add_const(GSCode*c,GSValue v){if(c->const_count==c->const_capacity)c->constants=grow(c->constants,&c->const_capacity,sizeof*c->constants);c->constants[c->const_count]=gs_copy(v);return(int)c->const_count++;}
int code_add_name(GSCode*c,const char*n){for(size_t i=0;i<c->name_count;i++)if(!strcmp(c->names[i],n))return(int)i;if(c->name_count==c->name_capacity)c->names=grow(c->names,&c->name_capacity,sizeof*c->names);c->names[c->name_count]=sd(n);return(int)c->name_count++;}
int code_add_child(GSCode*c,GSCode*x){if(c->child_count==c->child_capacity)c->children=grow(c->children,&c->child_capacity,sizeof*c->children);code_retain(x);c->children[c->child_count]=x;return(int)c->child_count++;}
void code_patch(GSCode*c,int at,int arg){if(at>=0&&(size_t)at<c->count)c->code[at].arg=arg;}
const char*op_name(GSOp o){static const char*n[]={"NOP","LOAD_CONST","LOAD_NAME","STORE_NAME","POP_TOP","ADD","SUB","MUL","DIV","MOD","NEG","NOT","EQ","NE","LT","LE","GT","GE","AND","OR","JUMP","JUMP_IF_FALSE","JUMP_IF_TRUE","CALL","RETURN","BUILD_ARRAY","BUILD_OBJECT","GET_INDEX","SET_INDEX","GET_MEMBER","SET_MEMBER","MAKE_FUNCTION","IMPORT","FROM_IMPORT","ITER_INIT","ITER_NEXT","HALT"};return o<sizeof(n)/sizeof(n[0])?n[o]:"UNKNOWN";}
void code_disassemble(const GSCode*c,const char*prefix){if(!c)return;printf("%sCode %s (params=%d)\n",prefix?prefix:"",c->name,c->param_count);for(size_t i=0;i<c->count;i++){GSInstr in=c->code[i];printf("%s%04zu %-18s %d\n",prefix?prefix:"",i,op_name((GSOp)in.op),in.arg);}for(size_t i=0;i<c->child_count;i++){printf("%schild %zu:\n",prefix?prefix:"",i);char b[128];snprintf(b,sizeof b,"%s  ",prefix?prefix:"");code_disassemble(c->children[i],b);}}
