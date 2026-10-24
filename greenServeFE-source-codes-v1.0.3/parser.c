#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TokenList *T; static size_t pos; static int errors;
static Token *cur(void){return &T->items[pos];}
static int check(TokenType t){return cur()->type==t;}
static Token *advance_tok(void){return &T->items[pos++];}
static int match(TokenType t){if(check(t)){pos++;return 1;}return 0;}
#define C_RED "\033[31m"
#define C_YELLOW "\033[33m"
#define C_CYAN "\033[36m"
#define C_DIM "\033[2m"
#define C_RESET "\033[0m"
static void error_here(const char *m){
    fprintf(stderr,C_RED "I found a syntax problem" C_RESET " " C_DIM "[PAR001]" C_RESET " at %d:%d\n",cur()->line,cur()->column);
    fprintf(stderr,"  " C_YELLOW "What I expected:" C_RESET " %s\n",m);
    fprintf(stderr,"  " C_CYAN "What I found:" C_RESET " %s",token_type_name(cur()->type));
    if(cur()->lexeme&&*cur()->lexeme)fprintf(stderr," " C_DIM "(%s)" C_RESET,cur()->lexeme);
    fprintf(stderr,"\n");
    errors++;
    while(!check(TOK_EOF)&&!check(TOK_SEMICOLON)&&!check(TOK_RBRACE))pos++;
    if(check(TOK_SEMICOLON))pos++;
}
static Token *expect(TokenType t,const char *m){if(check(t))return advance_tok();error_here(m);return NULL;}
static char *dupstr(const char *s){return s?strdup(s):NULL;}
static Expr *expr_new(ExprType t, Token *tok){Expr *e=calloc(1,sizeof(*e));e->type=t;e->line=tok?tok->line:0;e->column=tok?tok->column:0;return e;}
static Stmt *stmt_new(StmtType t, Token *tok){Stmt *s=calloc(1,sizeof(*s));s->type=t;s->line=tok?tok->line:0;s->column=tok?tok->column:0;return s;}
static void append_stmt(Stmt **head,Stmt **tail,Stmt *s){if(!s)return;if(!*head)*head=s;else(*tail)->next=s;*tail=s;}

static Expr *parse_expression(void);
static Stmt *parse_block_after_open(void);
static void free_stmt(Stmt *s);

static Expr *parse_function_expression(Token *t){
    Expr *e=expr_new(EX_FUNCTION,t);
    expect(TOK_LPAREN,"expected '(' after func");
    while(!check(TOK_RPAREN)&&!check(TOK_EOF)){
        Token *p=expect(TOK_IDENTIFIER,"expected parameter name");
        if(p){
            e->as.function.params=realloc(e->as.function.params,(e->as.function.param_count+1)*sizeof(char*));
            e->as.function.params[e->as.function.param_count++]=dupstr(p->lexeme);
        }
        if(!match(TOK_COMMA))break;
    }
    expect(TOK_RPAREN,"expected ')' after parameters");
    expect(TOK_LBRACE,"expected '{' before function body");
    if(check(TOK_RBRACE)){
        advance_tok();
        e->as.function.body=stmt_new(ST_BLOCK,t);
    }else{
        e->as.function.body=parse_block_after_open();
    }
    return e;
}

static Expr *parse_primary(void){
    Token *t=cur();
    if(match(TOK_NUMBER)){Expr *e=expr_new(EX_NUMBER,t);e->as.number=t->number;return e;}
    if(match(TOK_STRING)){Expr *e=expr_new(EX_STRING,t);e->as.string=dupstr(t->lexeme);return e;}
    if(match(TOK_TRUE)||match(TOK_FALSE)){Expr *e=expr_new(EX_BOOL,t);e->as.boolean=(t->type==TOK_TRUE);return e;}
    if(match(TOK_NULL))return expr_new(EX_NULL,t);
    if(match(TOK_IDENTIFIER)){Expr *e=expr_new(EX_IDENTIFIER,t);e->as.identifier=dupstr(t->lexeme);return e;}
    if(match(TOK_FUNC))return parse_function_expression(t);
    if(match(TOK_LPAREN)){Expr *e=parse_expression();expect(TOK_RPAREN,"expected ')' ");return e;}
    if(match(TOK_LBRACKET)){
        Expr *e=expr_new(EX_ARRAY,t); while(!check(TOK_RBRACKET)&&!check(TOK_EOF)){Expr *x=parse_expression();if(x){e->as.array.items=realloc(e->as.array.items,(e->as.array.count+1)*sizeof(Expr*));e->as.array.items[e->as.array.count++]=x;}if(!match(TOK_COMMA))break;}expect(TOK_RBRACKET,"expected ']' ");return e;
    }
    if(match(TOK_LBRACE)){
        Expr *e=expr_new(EX_OBJECT,t); while(!check(TOK_RBRACE)&&!check(TOK_EOF)){
            Token *k=cur(); if(!(check(TOK_IDENTIFIER)||check(TOK_STRING))){error_here("expected object key");break;}advance_tok();expect(TOK_COLON,"expected ':' after object key");Expr *v=parse_expression();e->as.object.items=realloc(e->as.object.items,(e->as.object.count+1)*sizeof(ObjectEntry));e->as.object.items[e->as.object.count].key=dupstr(k->lexeme);e->as.object.items[e->as.object.count].value=v;e->as.object.count++;if(!match(TOK_COMMA))break;
        } expect(TOK_RBRACE,"expected '}' ");return e;
    }
    error_here("expected expression"); return expr_new(EX_NULL,t);
}
static Expr *parse_postfix(void){
    Expr *e=parse_primary();
    for(;;){
        if(match(TOK_LPAREN)){
            Expr *n=calloc(1,sizeof(*n));n->type=EX_CALL;n->line=e->line;n->column=e->column;n->as.call.callee=e;
            while(!check(TOK_RPAREN)&&!check(TOK_EOF)){Expr *a=parse_expression();n->as.call.args=realloc(n->as.call.args,(n->as.call.count+1)*sizeof(Expr*));n->as.call.args[n->as.call.count++]=a;if(!match(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')' after arguments");e=n;
        } else if(match(TOK_LBRACKET)){Expr *n=calloc(1,sizeof(*n));n->type=EX_INDEX;n->line=e->line;n->column=e->column;n->as.index.object=e;n->as.index.index=parse_expression();expect(TOK_RBRACKET,"expected ']'");e=n;
        } else if(match(TOK_DOT)){Token *name=expect(TOK_IDENTIFIER,"expected member name");if(!name)break;Expr *n=calloc(1,sizeof(*n));n->type=EX_MEMBER;n->line=e->line;n->column=e->column;n->as.member.object=e;n->as.member.name=dupstr(name->lexeme);e=n;
        } else break;
    } return e;
}
static Expr *parse_unary(void){if(check(TOK_BANG)||check(TOK_MINUS)||check(TOK_PLUS)){Token *t=advance_tok();Expr *e=expr_new(EX_UNARY,t);e->as.unary.op=t->type;e->as.unary.value=parse_unary();return e;}return parse_postfix();}
static Expr *binary_level(Expr *(*next)(void),TokenType a,TokenType b){Expr *e=next();while(check(a)||check(b)){Token *t=advance_tok();Expr *r=next();Expr *n=expr_new(EX_BINARY,t);n->as.binary.op=t->type;n->as.binary.left=e;n->as.binary.right=r;e=n;}return e;}
static Expr *mul(void){return binary_level(parse_unary,TOK_STAR,TOK_SLASH);}
static Expr *add(void){Expr *e=mul();while(check(TOK_PLUS)||check(TOK_MINUS)||check(TOK_PERCENT)){Token *t=advance_tok();Expr *r=mul();Expr *n=expr_new(EX_BINARY,t);n->as.binary.op=t->type;n->as.binary.left=e;n->as.binary.right=r;e=n;}return e;}
static Expr *cmp(void){Expr *e=add();while(check(TOK_LESS)||check(TOK_LESS_EQUAL)||check(TOK_GREATER)||check(TOK_GREATER_EQUAL)){Token *t=advance_tok();Expr *r=add();Expr *n=expr_new(EX_BINARY,t);n->as.binary.op=t->type;n->as.binary.left=e;n->as.binary.right=r;e=n;}return e;}
static Expr *eq(void){Expr *e=cmp();while(check(TOK_EQUAL_EQUAL)||check(TOK_BANG_EQUAL)){Token *t=advance_tok();Expr *r=cmp();Expr *n=expr_new(EX_BINARY,t);n->as.binary.op=t->type;n->as.binary.left=e;n->as.binary.right=r;e=n;}return e;}
static Expr *land(void){Expr *e=eq();while(match(TOK_AND_AND)){Token *t=&T->items[pos-1];Expr *r=eq();Expr *n=expr_new(EX_BINARY,t);n->as.binary.op=t->type;n->as.binary.left=e;n->as.binary.right=r;e=n;}return e;}
static Expr *lor(void){Expr *e=land();while(match(TOK_OR_OR)){Token *t=&T->items[pos-1];Expr *r=land();Expr *n=expr_new(EX_BINARY,t);n->as.binary.op=t->type;n->as.binary.left=e;n->as.binary.right=r;e=n;}return e;}
static Expr *parse_expression(void){Expr *e=lor();if(match(TOK_EQUAL)){Token *t=&T->items[pos-1];Expr *v=parse_expression();Expr *n=expr_new(EX_ASSIGN,t);n->as.assign.target=e;n->as.assign.value=v;return n;}return e;}

static Stmt *parse_statement(void);
static Stmt *parse_block_after_open(void){Stmt *s=stmt_new(ST_BLOCK,NULL),*tail=NULL;while(!check(TOK_RBRACE)&&!check(TOK_EOF)){size_t before=pos;Stmt *x=parse_statement();append_stmt(&s->as.block.first,&tail,x);if(pos==before){error_here("parser made no progress");if(pos==before&& !check(TOK_EOF))pos++;}}if(check(TOK_RBRACE))advance_tok();else error_here("expected '}'");return s;}
static Stmt *parse_statement(void){
    Token *t=cur();
    if(match(TOK_SEMICOLON))return NULL;
    if(match(TOK_LBRACE))return parse_block_after_open();
    if(match(TOK_DEFINE)){Token *n=expect(TOK_IDENTIFIER,"expected variable name");expect(TOK_EQUAL,"expected '=' after variable name");Stmt *s=stmt_new(ST_DEFINE,t);s->as.define.name=dupstr(n?n->lexeme:"");s->as.define.value=parse_expression();expect(TOK_SEMICOLON,"expected ';'");return s;}
    if(match(TOK_IF)){expect(TOK_LPAREN,"expected '(' after if");Expr *c=parse_expression();expect(TOK_RPAREN,"expected ')' after condition");Stmt *s=stmt_new(ST_IF,t);s->as.if_stmt.condition=c;s->as.if_stmt.then_branch=parse_statement();if(match(TOK_ELSE))s->as.if_stmt.else_branch=parse_statement();return s;}
    if(match(TOK_WHILE)){expect(TOK_LPAREN,"expected '(' after while");Expr *c=parse_expression();expect(TOK_RPAREN,"expected ')' after condition");Stmt *s=stmt_new(ST_WHILE,t);s->as.while_stmt.condition=c;s->as.while_stmt.body=parse_statement();return s;}
    if(match(TOK_FOR)){
        int paren=match(TOK_LPAREN);Token *n=expect(TOK_IDENTIFIER,"expected loop variable");expect(TOK_IN,"expected 'in' in for loop");Expr *it=parse_expression();if(paren)expect(TOK_RPAREN,"expected ')' after for clause");Stmt *s=stmt_new(ST_FOR,t);s->as.for_stmt.name=dupstr(n?n->lexeme:"");s->as.for_stmt.iterable=it;s->as.for_stmt.body=parse_statement();return s;
    }
    if(match(TOK_FUNC)){
        Token *n=expect(TOK_IDENTIFIER,"expected function name");expect(TOK_LPAREN,"expected '(' after function name");Stmt *s=stmt_new(ST_FUNC,t);s->as.func.name=dupstr(n?n->lexeme:"");while(!check(TOK_RPAREN)&&!check(TOK_EOF)){Token *p=expect(TOK_IDENTIFIER,"expected parameter name");if(p){s->as.func.params=realloc(s->as.func.params,(s->as.func.param_count+1)*sizeof(char*));s->as.func.params[s->as.func.param_count++]=dupstr(p->lexeme);}if(!match(TOK_COMMA))break;}expect(TOK_RPAREN,"expected ')' after parameters");expect(TOK_LBRACE,"expected '{' before function body");s->as.func.body=parse_block_after_open();return s;
    }
    if(match(TOK_RETURN)){Stmt *s=stmt_new(ST_RETURN,t);if(!check(TOK_SEMICOLON))s->as.return_stmt.value=parse_expression();expect(TOK_SEMICOLON,"expected ';'");return s;}
    if(match(TOK_BREAK)){Stmt *s=stmt_new(ST_BREAK,t);expect(TOK_SEMICOLON,"expected ';'");return s;}
    if(match(TOK_CONTINUE)){Stmt *s=stmt_new(ST_CONTINUE,t);expect(TOK_SEMICOLON,"expected ';'");return s;}
    if(match(TOK_IMPORT)){Token *m=expect(TOK_IDENTIFIER,"expected module name after import");Stmt *s=stmt_new(ST_IMPORT,t);s->as.import_stmt.module=dupstr(m?m->lexeme:"");expect(TOK_SEMICOLON,"expected ';'");return s;}
    if(match(TOK_FROM)){Token *m=expect(TOK_IDENTIFIER,"expected module name after from");expect(TOK_IMPORT,"expected import");Stmt *s=stmt_new(ST_FROM_IMPORT,t);s->as.from_import.module=dupstr(m?m->lexeme:"");while(!check(TOK_SEMICOLON)&&!check(TOK_EOF)){Token *n=expect(TOK_IDENTIFIER,"expected imported name");if(n){s->as.from_import.names.names=realloc(s->as.from_import.names.names,(s->as.from_import.names.count+1)*sizeof(char*));s->as.from_import.names.names[s->as.from_import.names.count++]=dupstr(n->lexeme);}if(!match(TOK_COMMA))break;}expect(TOK_SEMICOLON,"expected ';'");return s;}
    Stmt *s=stmt_new(ST_EXPR,t);s->as.expr.expr=parse_expression();expect(TOK_SEMICOLON,"expected ';'");return s;
}
Program parse_program(TokenList *tokens){T=tokens;pos=0;errors=0;Program p={0};Stmt *tail=NULL;while(!check(TOK_EOF)){size_t before=pos;Stmt *s=parse_statement();append_stmt(&p.program,&tail,s);if(pos==before){error_here("parser made no progress");if(pos==before&&!check(TOK_EOF))pos++;}}p.errors=errors;return p;}

static void free_expr(Expr *e){if(!e)return;switch(e->type){case EX_STRING:case EX_IDENTIFIER:free(e->as.string);break;case EX_ARRAY:for(size_t i=0;i<e->as.array.count;i++)free_expr(e->as.array.items[i]);free(e->as.array.items);break;case EX_OBJECT:for(size_t i=0;i<e->as.object.count;i++){free(e->as.object.items[i].key);free_expr(e->as.object.items[i].value);}free(e->as.object.items);break;case EX_UNARY:free_expr(e->as.unary.value);break;case EX_BINARY:free_expr(e->as.binary.left);free_expr(e->as.binary.right);break;case EX_ASSIGN:free_expr(e->as.assign.target);free_expr(e->as.assign.value);break;case EX_CALL:free_expr(e->as.call.callee);for(size_t i=0;i<e->as.call.count;i++)free_expr(e->as.call.args[i]);free(e->as.call.args);break;case EX_INDEX:free_expr(e->as.index.object);free_expr(e->as.index.index);break;case EX_MEMBER:free_expr(e->as.member.object);free(e->as.member.name);break;case EX_FUNCTION:for(size_t i=0;i<e->as.function.param_count;i++)free(e->as.function.params[i]);free(e->as.function.params);free_stmt(e->as.function.body);break;default:break;}free(e);}
static void free_stmt(Stmt *s){while(s){Stmt *n=s->next;switch(s->type){case ST_BLOCK:free_stmt(s->as.block.first);break;case ST_EXPR:free_expr(s->as.expr.expr);break;case ST_DEFINE:free(s->as.define.name);free_expr(s->as.define.value);break;case ST_IF:free_expr(s->as.if_stmt.condition);free_stmt(s->as.if_stmt.then_branch);free_stmt(s->as.if_stmt.else_branch);break;case ST_WHILE:free_expr(s->as.while_stmt.condition);free_stmt(s->as.while_stmt.body);break;case ST_FOR:free(s->as.for_stmt.name);free_expr(s->as.for_stmt.iterable);free_stmt(s->as.for_stmt.body);break;case ST_FUNC:free(s->as.func.name);for(size_t i=0;i<s->as.func.param_count;i++)free(s->as.func.params[i]);free(s->as.func.params);free_stmt(s->as.func.body);break;case ST_RETURN:free_expr(s->as.return_stmt.value);break;case ST_IMPORT:free(s->as.import_stmt.module);break;case ST_FROM_IMPORT:free(s->as.from_import.module);for(size_t i=0;i<s->as.from_import.names.count;i++)free(s->as.from_import.names.names[i]);free(s->as.from_import.names.names);break;default:break;}free(s);n= n; s=n;}}
void free_program(Program *p){free_stmt(p->program);p->program=NULL;}
void print_parse_errors(const Program *p){if(p->errors)fprintf(stderr,C_YELLOW "I found %d syntax problem%s" C_RESET ".\n",p->errors,p->errors==1?"":"s");}
