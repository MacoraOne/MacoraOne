#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Loop{int start;int*breaks;size_t nb,cap;struct Loop*parent;}Loop;
typedef struct{GSCode*code;Loop*loop;int errors;int in_function;}Compiler;
#define C_RED "\033[31m"
#define C_YELLOW "\033[33m"
#define C_RESET "\033[0m"
static int emit(Compiler*c,GSOp op,int arg,int line){return code_emit(c->code,op,arg,line);}
static void patch(Compiler*c,int at,int target){code_patch(c->code,at,target);}
static void err_at(Compiler*c,int line,int column,const char*m){fprintf(stderr,C_RED "I could not compile this code" C_RESET " " C_YELLOW "[CMP001]" C_RESET " at %d:%d\n",line,column);
    fprintf(stderr,"  " C_YELLOW "What went wrong:" C_RESET " %s\n",m);c->errors++;}
static void err(Compiler*c,Stmt*s,const char*m){err_at(c,s?s->line:0,s?s->column:0,m);}
static void expr(Compiler*,Expr*);static void stmt(Compiler*,Stmt*);static void compile_block(Compiler*,Stmt*);
static void compile_function_body(Compiler *parent, GSCode *child, char **params, size_t param_count, Stmt *body, int line){
    Compiler nested={0};
    nested.code=child;
    nested.loop=NULL;
    nested.errors=0;
    nested.in_function=1;
    nested.code->param_count=(int)param_count;
    for(size_t i=0;i<param_count;i++)code_add_name(nested.code,params[i]);
    if(body)compile_block(&nested,body->as.block.first);
    int k=code_add_const(nested.code,gs_null());
    emit(&nested,OP_LOAD_CONST,k,line);
    emit(&nested,OP_RETURN,0,line);
    parent->errors+=nested.errors;
}
static void expr(Compiler*c,Expr*e){
 if(!e)return;
 int k;
 switch(e->type){
 case EX_NUMBER:{GSValue v=gs_number(e->as.number);k=code_add_const(c->code,v);emit(c,OP_LOAD_CONST,k,e->line);break;}
 case EX_STRING:{GSValue v=gs_string(e->as.string);k=code_add_const(c->code,v);gs_release(v);emit(c,OP_LOAD_CONST,k,e->line);break;}
 case EX_BOOL:{GSValue v=gs_bool(e->as.boolean);k=code_add_const(c->code,v);emit(c,OP_LOAD_CONST,k,e->line);break;}
 case EX_NULL:k=code_add_const(c->code,gs_null());emit(c,OP_LOAD_CONST,k,e->line);break;
 case EX_IDENTIFIER:k=code_add_name(c->code,e->as.identifier);emit(c,OP_LOAD_NAME,k,e->line);break;
 case EX_ARRAY:for(size_t i=0;i<e->as.array.count;i++)expr(c,e->as.array.items[i]);emit(c,OP_BUILD_ARRAY,(int)e->as.array.count,e->line);break;
 case EX_OBJECT:for(size_t i=0;i<e->as.object.count;i++){GSValue key=gs_string(e->as.object.items[i].key);k=code_add_const(c->code,key);gs_release(key);emit(c,OP_LOAD_CONST,k,e->line);expr(c,e->as.object.items[i].value);}emit(c,OP_BUILD_OBJECT,(int)e->as.object.count,e->line);break;
 case EX_UNARY:expr(c,e->as.unary.value);if(e->as.unary.op==TOK_MINUS)emit(c,OP_NEG,0,e->line);else if(e->as.unary.op==TOK_BANG)emit(c,OP_NOT,0,e->line);break;
 case EX_BINARY:expr(c,e->as.binary.left);expr(c,e->as.binary.right);switch(e->as.binary.op){case TOK_PLUS:emit(c,OP_ADD,0,e->line);break;case TOK_MINUS:emit(c,OP_SUB,0,e->line);break;case TOK_STAR:emit(c,OP_MUL,0,e->line);break;case TOK_SLASH:emit(c,OP_DIV,0,e->line);break;case TOK_PERCENT:emit(c,OP_MOD,0,e->line);break;case TOK_EQUAL_EQUAL:emit(c,OP_EQ,0,e->line);break;case TOK_BANG_EQUAL:emit(c,OP_NE,0,e->line);break;case TOK_LESS:emit(c,OP_LT,0,e->line);break;case TOK_LESS_EQUAL:emit(c,OP_LE,0,e->line);break;case TOK_GREATER:emit(c,OP_GT,0,e->line);break;case TOK_GREATER_EQUAL:emit(c,OP_GE,0,e->line);break;case TOK_AND_AND:emit(c,OP_AND,0,e->line);break;case TOK_OR_OR:emit(c,OP_OR,0,e->line);break;default:emit(c,OP_NOP,0,e->line);break;}break;
 case EX_ASSIGN:{Expr*t=e->as.assign.target;if(t->type==EX_IDENTIFIER){expr(c,e->as.assign.value);emit(c,OP_STORE_NAME,code_add_name(c->code,t->as.identifier),e->line);}else if(t->type==EX_INDEX){expr(c,t->as.index.object);expr(c,t->as.index.index);expr(c,e->as.assign.value);emit(c,OP_SET_INDEX,0,e->line);}else if(t->type==EX_MEMBER){expr(c,t->as.member.object);GSValue key=gs_string(t->as.member.name);k=code_add_const(c->code,key);gs_release(key);emit(c,OP_LOAD_CONST,k,e->line);expr(c,e->as.assign.value);emit(c,OP_SET_MEMBER,0,e->line);}else{err_at(c,e->line,e->column,"invalid assignment target; expected a variable, index, or member target");}break;}
 case EX_CALL:expr(c,e->as.call.callee);for(size_t i=0;i<e->as.call.count;i++)expr(c,e->as.call.args[i]);emit(c,OP_CALL,(int)e->as.call.count,e->line);break;
 case EX_INDEX:expr(c,e->as.index.object);expr(c,e->as.index.index);emit(c,OP_GET_INDEX,0,e->line);break;
 case EX_MEMBER:expr(c,e->as.member.object);k=code_add_name(c->code,e->as.member.name);emit(c,OP_GET_MEMBER,k,e->line);break;
 case EX_FUNCTION:{GSCode *child=code_new("<anonymous>");compile_function_body(c,child,e->as.function.params,e->as.function.param_count,e->as.function.body,e->line);k=code_add_child(c->code,child);code_release(child);emit(c,OP_MAKE_FUNCTION,k,e->line);break;}
 }}
static void compile_block(Compiler*c,Stmt*s){for(;s;s=s->next)stmt(c,s);}
static void stmt(Compiler*c,Stmt*s){
 if(!s)return;
 switch(s->type){
 case ST_BLOCK:compile_block(c,s->as.block.first);break;
 case ST_EXPR:expr(c,s->as.expr.expr);emit(c,OP_POP_TOP,0,s->line);break;
 case ST_DEFINE:expr(c,s->as.define.value);emit(c,OP_STORE_NAME,code_add_name(c->code,s->as.define.name),s->line);emit(c,OP_POP_TOP,0,s->line);break;
 case ST_IF:{expr(c,s->as.if_stmt.condition);int jf=emit(c,OP_JUMP_IF_FALSE,0,s->line);stmt(c,s->as.if_stmt.then_branch);if(s->as.if_stmt.else_branch){int je=emit(c,OP_JUMP,0,s->line);patch(c,jf,(int)c->code->count);stmt(c,s->as.if_stmt.else_branch);patch(c,je,(int)c->code->count);}else patch(c,jf,(int)c->code->count);break;}
 case ST_WHILE:{int start=(int)c->code->count;Loop l={.start=start,.parent=c->loop};c->loop=&l;expr(c,s->as.while_stmt.condition);int jf=emit(c,OP_JUMP_IF_FALSE,0,s->line);stmt(c,s->as.while_stmt.body);emit(c,OP_JUMP,start,s->line);int end=(int)c->code->count;patch(c,jf,end);for(size_t i=0;i<l.nb;i++)patch(c,l.breaks[i],end);free(l.breaks);c->loop=l.parent;break;}
 case ST_FOR:{expr(c,s->as.for_stmt.iterable);emit(c,OP_ITER_INIT,0,s->line);int start=(int)c->code->count;Loop l={.start=start,.parent=c->loop};c->loop=&l;int next=emit(c,OP_ITER_NEXT,0,s->line);emit(c,OP_STORE_NAME,code_add_name(c->code,s->as.for_stmt.name),s->line);emit(c,OP_POP_TOP,0,s->line);stmt(c,s->as.for_stmt.body);emit(c,OP_JUMP,start,s->line);int end=(int)c->code->count;patch(c,next,end);emit(c,OP_POP_TOP,0,s->line);for(size_t i=0;i<l.nb;i++)patch(c,l.breaks[i],end);free(l.breaks);c->loop=l.parent;break;}
 case ST_FUNC:{GSCode *child=code_new(s->as.func.name);compile_function_body(c,child,s->as.func.params,s->as.func.param_count,s->as.func.body,s->line);int ci=code_add_child(c->code,child);code_release(child);emit(c,OP_MAKE_FUNCTION,ci,s->line);emit(c,OP_STORE_NAME,code_add_name(c->code,s->as.func.name),s->line);emit(c,OP_POP_TOP,0,s->line);break;}
 case ST_RETURN:if(!c->in_function){err(c,s,"return outside a function");break;}if(s->as.return_stmt.value)expr(c,s->as.return_stmt.value);else{int k=code_add_const(c->code,gs_null());emit(c,OP_LOAD_CONST,k,s->line);}emit(c,OP_RETURN,0,s->line);break;
 case ST_BREAK:if(!c->loop){err(c,s,"break outside loop");break;}{if(c->loop->nb==c->loop->cap){c->loop->cap=c->loop->cap?c->loop->cap*2:4;c->loop->breaks=realloc(c->loop->breaks,c->loop->cap*sizeof(int));}c->loop->breaks[c->loop->nb++]=emit(c,OP_JUMP,0,s->line);}break;
 case ST_CONTINUE:if(!c->loop){err(c,s,"continue outside loop");break;}emit(c,OP_JUMP,c->loop->start,s->line);break;
 case ST_IMPORT:{GSValue m=gs_string(s->as.import_stmt.module);int k=code_add_const(c->code,m);gs_release(m);emit(c,OP_IMPORT,k,s->line);break;}
 case ST_FROM_IMPORT:{GSValue m=gs_string(s->as.from_import.module);int k=code_add_const(c->code,m);gs_release(m);emit(c,OP_LOAD_CONST,k,s->line);for(size_t i=0;i<s->as.from_import.names.count;i++){GSValue n=gs_string(s->as.from_import.names.names[i]);int q=code_add_const(c->code,n);gs_release(n);emit(c,OP_LOAD_CONST,q,s->line);}emit(c,OP_FROM_IMPORT,(int)s->as.from_import.names.count,s->line);break;}
 }}
GSCode*compile_program(Program*p){Compiler c={0};c.code=code_new("<module>");compile_block(&c,p?p->program:NULL);if(!c.code->count||c.code->code[c.code->count-1].op!=OP_HALT){emit(&c,OP_LOAD_CONST,code_add_const(c.code,gs_null()),0);emit(&c,OP_HALT,0,0);}if(c.errors){fprintf(stderr,C_YELLOW "Compilation stopped" C_RESET ": %d problem%s found. The program was not executed.\n",c.errors,c.errors==1?"":"s");code_release(c.code);return NULL;}return c.code;
}
