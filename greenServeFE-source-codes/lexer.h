#ifndef GSFE_LEXER_H
#define GSFE_LEXER_H
#include <stddef.h>
typedef enum { TOK_EOF,TOK_ERROR,TOK_IDENTIFIER,TOK_NUMBER,TOK_STRING,TOK_DEFINE,TOK_IF,TOK_ELSE,TOK_FOR,TOK_IN,TOK_WHILE,TOK_FUNC,TOK_RETURN,TOK_BREAK,TOK_CONTINUE,TOK_IMPORT,TOK_FROM,TOK_TRUE,TOK_FALSE,TOK_NULL,TOK_LPAREN,TOK_RPAREN,TOK_LBRACE,TOK_RBRACE,TOK_LBRACKET,TOK_RBRACKET,TOK_COMMA,TOK_DOT,TOK_COLON,TOK_SEMICOLON,TOK_PLUS,TOK_MINUS,TOK_STAR,TOK_SLASH,TOK_PERCENT,TOK_BANG,TOK_EQUAL,TOK_EQUAL_EQUAL,TOK_BANG_EQUAL,TOK_LESS,TOK_LESS_EQUAL,TOK_GREATER,TOK_GREATER_EQUAL,TOK_AND_AND,TOK_OR_OR } TokenType;
typedef struct { TokenType type; char *lexeme; double number; int line,column; } Token;
typedef struct { Token *items; size_t count,capacity; } TokenList;
TokenList lex_source(const char *source); void free_tokens(TokenList *tokens); const char *token_type_name(TokenType type);
#endif
