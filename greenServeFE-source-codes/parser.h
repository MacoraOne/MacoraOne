#ifndef GSFE_PARSER_H
#define GSFE_PARSER_H
#include "lexer.h"
#include <stddef.h>
typedef enum{EX_NUMBER,EX_STRING,EX_BOOL,EX_NULL,EX_IDENTIFIER,EX_ARRAY,EX_OBJECT,EX_UNARY,EX_BINARY,EX_ASSIGN,EX_CALL,EX_INDEX,EX_MEMBER,EX_FUNCTION}ExprType;
typedef enum{ST_BLOCK,ST_EXPR,ST_DEFINE,ST_IF,ST_WHILE,ST_FOR,ST_FUNC,ST_RETURN,ST_BREAK,ST_CONTINUE,ST_IMPORT,ST_FROM_IMPORT}StmtType;
typedef struct Expr Expr;typedef struct Stmt Stmt;typedef struct{char*key;Expr*value;}ObjectEntry;typedef struct{char**names;size_t count;}NameList;
struct Expr{ExprType type;int line,column;union{double number;char*string;int boolean;char*identifier;struct{Expr**items;size_t count;}array;struct{ObjectEntry*items;size_t count;}object;struct{TokenType op;Expr*value;}unary;struct{TokenType op;Expr*left,*right;}binary;struct{Expr*target,*value;}assign;struct{Expr*callee;Expr**args;size_t count;}call;struct{Expr*object,*index;}index;struct{Expr*object;char*name;}member;struct{char**params;size_t param_count;Stmt*body;}function;}as;};
struct Stmt{StmtType type;int line,column;Stmt*next;union{struct{Stmt*first;}block;struct{Expr*expr;}expr;struct{char*name;Expr*value;}define;struct{Expr*condition;Stmt*then_branch,*else_branch;}if_stmt;struct{Expr*condition;Stmt*body;}while_stmt;struct{char*name;Expr*iterable;Stmt*body;}for_stmt;struct{char*name;char**params;size_t param_count;Stmt*body;}func;struct{Expr*value;}return_stmt;struct{char*module;}import_stmt;struct{char*module;NameList names;}from_import;}as;};
typedef struct{Stmt*program;int errors;}Program;Program parse_program(TokenList*);void free_program(Program*);void print_parse_errors(const Program*);
#endif
