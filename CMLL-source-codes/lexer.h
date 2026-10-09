#ifndef CMLL_LEXER_H
#define CMLL_LEXER_H

#include "common.h"

typedef enum {
    TOKEN_EOF,

    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    TOKEN_PROJECT,
    TOKEN_SET,
    TOKEN_TASK,

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,

    TOKEN_FUNC,
    TOKEN_RETURN,

    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    TOKEN_SAY,
    TOKEN_SHOW,
    TOKEN_RUN,

    TOKEN_READ,
    TOKEN_WRITE,
    TOKEN_FILE,
    TOKEN_EXISTS,

    TOKEN_LIST,
    TOKEN_URLS,
    TOKEN_FROM,

    TOKEN_ENV,
    TOKEN_GET,

    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LPAREN,
    TOKEN_RPAREN,

    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_SEMICOLON,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    TOKEN_EQUALS,
    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,

    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    TOKEN_PIPE
} CMLLTokenType;

typedef struct {
    CMLLTokenType type;
    char *text;
    CMLLPosition position;
} CMLLToken;

typedef struct {
    const char *source;
    const char *filename;

    size_t length;
    size_t index;

    int line;
    int column;

    int failed;
} CMLLLexer;

void cmll_lexer_init(
    CMLLLexer *lexer,
    const char *source,
    const char *filename
);

CMLLToken cmll_next_token(
    CMLLLexer *lexer
);

void cmll_token_free(
    CMLLToken *token
);

const char *cmll_token_name(
    CMLLTokenType type
);

#endif
