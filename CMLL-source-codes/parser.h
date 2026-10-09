#ifndef CMLL_PARSER_H
#define CMLL_PARSER_H

#include "lexer.h"

typedef enum {
    NODE_PROGRAM,

    NODE_PROJECT,

    NODE_SET,
    NODE_ASSIGN,

    NODE_TASK,
    NODE_FUNCTION,

    NODE_IF,
    NODE_WHILE,
    NODE_FOR,

    NODE_RETURN,

    NODE_SAY,
    NODE_SHOW,
    NODE_RUN,

    NODE_READ_FILE,
    NODE_WRITE_FILE,
    NODE_FILE_EXISTS,

    NODE_LIST_URLS,
    NODE_ENV,

    NODE_HTTP_GET,

    NODE_EXPRESSION_STATEMENT,

    NODE_CALL,

    NODE_LITERAL,
    NODE_VARIABLE,

    NODE_ARRAY,
    NODE_OBJECT,
    NODE_PROPERTY,

    NODE_BINARY,
    NODE_UNARY,

    NODE_PIPE
} CMLLNodeType;

typedef struct CMLLNode CMLLNode;

struct CMLLNode {
    CMLLNodeType type;

    CMLLPosition position;

    char *value;
    char *value2;

    CMLLNode *left;
    CMLLNode *right;

    CMLLNode **children;
    size_t child_count;
    size_t child_capacity;
};

typedef struct {
    CMLLLexer lexer;

    CMLLToken current;
    CMLLToken previous;

    int failed;
} CMLLParser;

void cmll_parser_init(
    CMLLParser *parser,
    const char *source,
    const char *filename
);

CMLLNode *cmll_parse(
    CMLLParser *parser
);

void cmll_node_free(
    CMLLNode *node
);

#endif
