#include "parser.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void advance_parser(
    CMLLParser *parser
)
{
    cmll_token_free(
        &parser->previous
    );

    parser->previous =
        parser->current;

    parser->current =
        cmll_next_token(
            &parser->lexer
        );
}

static int check(
    CMLLParser *parser,
    CMLLTokenType type
)
{
    return parser->current.type == type;
}

static int match(
    CMLLParser *parser,
    CMLLTokenType type
)
{
    if (!check(parser, type))
        return 0;

    advance_parser(parser);

    return 1;
}

static void parser_error(
    CMLLParser *parser,
    const char *message
)
{
    cmll_error(
        parser->current.position,
        parser->lexer.source,
        message
    );

    parser->failed = 1;
}

static int expect(
    CMLLParser *parser,
    CMLLTokenType type,
    const char *message
)
{
    if (match(parser, type))
        return 1;

    parser_error(parser, message);

    return 0;
}

static CMLLNode *node(
    CMLLNodeType type,
    CMLLPosition position
)
{
    CMLLNode *result;

    result = calloc(
        1,
        sizeof(CMLLNode)
    );

    if (!result)
        return NULL;

    result->type = type;
    result->position = position;

    return result;
}

static void add_child(
    CMLLNode *parent,
    CMLLNode *child
)
{
    CMLLNode **expanded;
    size_t capacity;

    if (!child)
        return;

    if (
        parent->child_count >=
        parent->child_capacity
    ) {
        capacity =
            parent->child_capacity
                ? parent->child_capacity * 2
                : 4;

        expanded = realloc(
            parent->children,
            capacity * sizeof(CMLLNode *)
        );

        if (!expanded)
            return;

        parent->children = expanded;
        parent->child_capacity = capacity;
    }

    parent->children[
        parent->child_count++
    ] = child;
}

static CMLLNode *expression(
    CMLLParser *parser
);

static CMLLNode *statement(
    CMLLParser *parser
);

static void consume_separators(
    CMLLParser *parser
)
{
    while (
        match(parser, TOKEN_SEMICOLON)
    ) {
    }
}

static CMLLNode *primary(
    CMLLParser *parser
)
{
    CMLLNode *result;

    if (check(parser, TOKEN_NUMBER)) {
        result = node(
            NODE_LITERAL,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        result->value2 =
            cmll_strdup("number");

        advance_parser(parser);

        return result;
    }

    if (check(parser, TOKEN_STRING)) {
        result = node(
            NODE_LITERAL,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        result->value2 =
            cmll_strdup("string");

        advance_parser(parser);

        return result;
    }

    if (check(parser, TOKEN_TRUE) ||
        check(parser, TOKEN_FALSE)) {

        result = node(
            NODE_LITERAL,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                check(parser, TOKEN_TRUE)
                    ? "true"
                    : "false"
            );

        result->value2 =
            cmll_strdup("boolean");

        advance_parser(parser);

        return result;
    }

    if (match(parser, TOKEN_NULL)) {
        result = node(
            NODE_LITERAL,
            parser->previous.position
        );

        result->value =
            cmll_strdup("null");

        result->value2 =
            cmll_strdup("null");

        return result;
    }

    if (check(parser, TOKEN_IDENTIFIER)) {
        result = node(
            NODE_VARIABLE,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        return result;
    }

    if (match(parser, TOKEN_LPAREN)) {
        result = expression(parser);

        expect(
            parser,
            TOKEN_RPAREN,
            "expected ')' after expression"
        );

        return result;
    }

    if (match(parser, TOKEN_LBRACKET)) {
        result = node(
            NODE_ARRAY,
            parser->previous.position
        );

        while (
            !check(parser, TOKEN_RBRACKET) &&
            !check(parser, TOKEN_EOF)
        ) {
            add_child(
                result,
                expression(parser)
            );

            if (!match(parser, TOKEN_COMMA))
                break;
        }

        expect(
            parser,
            TOKEN_RBRACKET,
            "expected ']' after array"
        );

        return result;
    }

    if (match(parser, TOKEN_LBRACE)) {
        result = node(
            NODE_OBJECT,
            parser->previous.position
        );

        while (
            !check(parser, TOKEN_RBRACE) &&
            !check(parser, TOKEN_EOF)
        ) {
            CMLLNode *key;
            CMLLNode *value;

            if (
                !check(
                    parser,
                    TOKEN_IDENTIFIER
                ) &&
                !check(
                    parser,
                    TOKEN_STRING
                )
            ) {
                parser_error(
                    parser,
                    "expected object property name"
                );

                break;
            }

            key = node(
                NODE_LITERAL,
                parser->current.position
            );

            key->value =
                cmll_strdup(
                    parser->current.text
                );

            key->value2 =
                cmll_strdup("string");

            advance_parser(parser);

            if (!expect(
                parser,
                TOKEN_COLON,
                "expected ':' after object property"
            )) {
                cmll_node_free(key);
                break;
            }

            value = expression(parser);

            add_child(result, key);
            add_child(result, value);

            if (!match(parser, TOKEN_COMMA))
                break;
        }

        expect(
            parser,
            TOKEN_RBRACE,
            "expected '}' after object"
        );

        return result;
    }

    parser_error(
        parser,
        "expected an expression"
    );

    return NULL;
}

static CMLLNode *postfix(
    CMLLParser *parser
)
{
    CMLLNode *result;

    result = primary(parser);

    if (!result)
        return NULL;

    while (1) {
        if (match(parser, TOKEN_DOT)) {
            CMLLNode *property;

            if (
                !check(
                    parser,
                    TOKEN_IDENTIFIER
                )
            ) {
                parser_error(
                    parser,
                    "expected property name after '.'"
                );

                break;
            }

            property = node(
                NODE_PROPERTY,
                parser->previous.position
            );

            property->left = result;

            property->value =
                cmll_strdup(
                    parser->current.text
                );

            advance_parser(parser);

            result = property;

            continue;
        }

        if (match(parser, TOKEN_LPAREN)) {
            CMLLNode *call;
            CMLLNode *argument;

            call = node(
                NODE_CALL,
                result->position
            );

            call->left = result;

            while (
                !check(parser, TOKEN_RPAREN) &&
                !check(parser, TOKEN_EOF)
            ) {
                argument = expression(parser);

                add_child(
                    call,
                    argument
                );

                if (!match(parser, TOKEN_COMMA))
                    break;
            }

            expect(
                parser,
                TOKEN_RPAREN,
                "expected ')' after arguments"
            );

            result = call;

            continue;
        }

        break;
    }

    return result;
}

static CMLLNode *unary(
    CMLLParser *parser
)
{
    if (
        check(parser, TOKEN_NOT) ||
        check(parser, TOKEN_MINUS)
    ) {
        CMLLNode *result;

        result = node(
            NODE_UNARY,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        result->left =
            unary(parser);

        return result;
    }

    return postfix(parser);
}

static CMLLNode *multiplication(
    CMLLParser *parser
)
{
    CMLLNode *left;

    left = unary(parser);

    while (
        check(parser, TOKEN_STAR) ||
        check(parser, TOKEN_SLASH) ||
        check(parser, TOKEN_PERCENT)
    ) {
        CMLLNode *result;

        result = node(
            NODE_BINARY,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        result->left = left;
        result->right = unary(parser);

        left = result;
    }

    return left;
}

static CMLLNode *addition(
    CMLLParser *parser
)
{
    CMLLNode *left;

    left = multiplication(parser);

    while (
        check(parser, TOKEN_PLUS) ||
        check(parser, TOKEN_MINUS)
    ) {
        CMLLNode *result;

        result = node(
            NODE_BINARY,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        result->left = left;
        result->right =
            multiplication(parser);

        left = result;
    }

    return left;
}

static CMLLNode *comparison(
    CMLLParser *parser
)
{
    CMLLNode *left;

    left = addition(parser);

    while (
        check(parser, TOKEN_LESS) ||
        check(parser, TOKEN_LESS_EQUAL) ||
        check(parser, TOKEN_GREATER) ||
        check(parser, TOKEN_GREATER_EQUAL)
    ) {
        CMLLNode *result;

        result = node(
            NODE_BINARY,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        result->left = left;
        result->right =
            addition(parser);

        left = result;
    }

    return left;
}

static CMLLNode *equality(
    CMLLParser *parser
)
{
    CMLLNode *left;

    left = comparison(parser);

    while (
        check(parser, TOKEN_EQUAL_EQUAL) ||
        check(parser, TOKEN_NOT_EQUAL)
    ) {
        CMLLNode *result;

        result = node(
            NODE_BINARY,
            parser->current.position
        );

        result->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        result->left = left;
        result->right =
            comparison(parser);

        left = result;
    }

    return left;
}

static CMLLNode *logical_and(
    CMLLParser *parser
)
{
    CMLLNode *left;

    left = equality(parser);

    while (match(parser, TOKEN_AND)) {
        CMLLNode *result;

        result = node(
            NODE_BINARY,
            parser->previous.position
        );

        result->value =
            cmll_strdup("&&");

        result->left = left;
        result->right =
            equality(parser);

        left = result;
    }

    return left;
}

static CMLLNode *logical_or(
    CMLLParser *parser
)
{
    CMLLNode *left;

    left = logical_and(parser);

    while (match(parser, TOKEN_OR)) {
        CMLLNode *result;

        result = node(
            NODE_BINARY,
            parser->previous.position
        );

        result->value =
            cmll_strdup("||");

        result->left = left;
        result->right =
            logical_and(parser);

        left = result;
    }

    return left;
}

static CMLLNode *expression(
    CMLLParser *parser
)
{
    return logical_or(parser);
}

static CMLLNode *block(
    CMLLParser *parser
)
{
    CMLLNode *result;

    if (!expect(
        parser,
        TOKEN_LBRACE,
        "expected '{' to begin block"
    )) {
        return NULL;
    }

    result = node(
        NODE_PROGRAM,
        parser->previous.position
    );

    consume_separators(parser);

    while (
        !check(parser, TOKEN_RBRACE) &&
        !check(parser, TOKEN_EOF)
    ) {
        CMLLNode *child;

        child = statement(parser);

        if (child)
            add_child(result, child);

        consume_separators(parser);
    }

    expect(
        parser,
        TOKEN_RBRACE,
        "expected '}' to close block"
    );

    return result;
}

static CMLLNode *parse_set(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    if (
        !check(
            parser,
            TOKEN_IDENTIFIER
        )
    ) {
        parser_error(
            parser,
            "expected variable name after 'set'"
        );

        return NULL;
    }

    result = node(
        NODE_SET,
        parser->previous.position
    );

    result->value =
        cmll_strdup(
            parser->current.text
        );

    advance_parser(parser);

    if (!expect(
        parser,
        TOKEN_EQUALS,
        "expected '=' after variable name"
    )) {
        cmll_node_free(result);
        return NULL;
    }

    result->left =
        expression(parser);

    return result;
}

static CMLLNode *parse_if(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    result = node(
        NODE_IF,
        parser->previous.position
    );

    result->left =
        expression(parser);

    result->right =
        block(parser);

    if (match(parser, TOKEN_ELSE)) {
        result->value =
            cmll_strdup("else");

        result->children =
            malloc(sizeof(CMLLNode *));

        result->child_capacity = 1;

        if (check(parser, TOKEN_IF)) {
            result->children[0] =
                parse_if(parser);
        } else {
            result->children[0] =
                block(parser);
        }

        result->child_count = 1;
    }

    return result;
}

static CMLLNode *parse_while(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    result = node(
        NODE_WHILE,
        parser->previous.position
    );

    result->left =
        expression(parser);

    result->right =
        block(parser);

    return result;
}

static CMLLNode *parse_for(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    result = node(
        NODE_FOR,
        parser->previous.position
    );

    if (
        !check(
            parser,
            TOKEN_IDENTIFIER
        )
    ) {
        parser_error(
            parser,
            "expected loop variable after 'for'"
        );

        cmll_node_free(result);

        return NULL;
    }

    result->value =
        cmll_strdup(
            parser->current.text
        );

    advance_parser(parser);

    if (!expect(
        parser,
        TOKEN_IN,
        "expected 'in' after loop variable"
    )) {
        cmll_node_free(result);
        return NULL;
    }

    result->left =
        expression(parser);

    result->right =
        block(parser);

    return result;
}

static CMLLNode *parse_function(
    CMLLParser *parser,
    CMLLNodeType type
)
{
    CMLLNode *result;

    advance_parser(parser);

    if (
        !check(
            parser,
            TOKEN_IDENTIFIER
        )
    ) {
        parser_error(
            parser,
            "expected name after declaration"
        );

        return NULL;
    }

    result = node(
        type,
        parser->previous.position
    );

    result->value =
        cmll_strdup(
            parser->current.text
        );

    advance_parser(parser);

    if (!expect(
        parser,
        TOKEN_LPAREN,
        "expected '(' after name"
    )) {
        cmll_node_free(result);
        return NULL;
    }

    while (
        !check(parser, TOKEN_RPAREN) &&
        !check(parser, TOKEN_EOF)
    ) {
        CMLLNode *argument;

        if (
            !check(
                parser,
                TOKEN_IDENTIFIER
            )
        ) {
            parser_error(
                parser,
                "expected parameter name"
            );

            break;
        }

        argument = node(
            NODE_VARIABLE,
            parser->current.position
        );

        argument->value =
            cmll_strdup(
                parser->current.text
            );

        advance_parser(parser);

        add_child(
            result,
            argument
        );

        if (!match(parser, TOKEN_COMMA))
            break;
    }

    expect(
        parser,
        TOKEN_RPAREN,
        "expected ')' after parameters"
    );

    {
        CMLLNode *body;

        body = block(parser);

        add_child(
            result,
            body
        );
    }

    return result;
}

static CMLLNode *parse_return(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    result = node(
        NODE_RETURN,
        parser->previous.position
    );

    if (
        check(parser, TOKEN_SEMICOLON) ||
        check(parser, TOKEN_RBRACE) ||
        check(parser, TOKEN_EOF)
    ) {
        result->left = NULL;
    } else {
        result->left =
            expression(parser);
    }

    return result;
}

static CMLLNode *parse_run(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    result = node(
        NODE_RUN,
        parser->previous.position
    );

    result->left =
        expression(parser);

    while (match(parser, TOKEN_PIPE)) {
        CMLLNode *pipe;

        pipe = node(
            NODE_PIPE,
            parser->previous.position
        );

        pipe->left = result;

        pipe->right =
            expression(parser);

        result = pipe;
    }

    return result;
}

static CMLLNode *parse_read(
    CMLLParser *parser
)
{
    CMLLNode *result;
    CMLLNode *file;

    advance_parser(parser);

    if (!expect(
        parser,
        TOKEN_FILE,
        "expected 'file' after 'read'"
    )) {
        return NULL;
    }

    result = node(
        NODE_READ_FILE,
        parser->previous.position
    );

    file = expression(parser);

    result->left = file;

    if (!match(parser, TOKEN_IDENTIFIER) ||
        strcmp(
            parser->previous.text,
            "as"
        ) != 0) {

        parser_error(
            parser,
            "expected 'as' after file"
        );

        cmll_node_free(result);

        return NULL;
    }

    if (
        !check(
            parser,
            TOKEN_IDENTIFIER
        )
    ) {
        parser_error(
            parser,
            "expected variable name after 'as'"
        );

        cmll_node_free(result);

        return NULL;
    }

    result->value =
        cmll_strdup(
            parser->current.text
        );

    advance_parser(parser);

    return result;
}

static CMLLNode *parse_write(
    CMLLParser *parser
)
{
    CMLLNode *result;

    advance_parser(parser);

    if (!expect(
        parser,
        TOKEN_FILE,
        "expected 'file' after 'write'"
    )) {
        return NULL;
    }

    result = node(
        NODE_WRITE_FILE,
        parser->previous.position
    );

    result->left =
        expression(parser);

    if (!match(parser, TOKEN_FROM)) {
        parser_error(
            parser,
            "expected 'from' in file write"
        );

        cmll_node_free(result);

        return NULL;
    }

    result->right =
        expression(parser);

    return result;
}

static CMLLNode *parse_statement(
    CMLLParser *parser
)
{
    CMLLNode *result;

    if (check(parser, TOKEN_PROJECT)) {
        advance_parser(parser);

        result = node(
            NODE_PROJECT,
            parser->previous.position
        );

        result->left =
            expression(parser);

        return result;
    }

    if (check(parser, TOKEN_SET))
        return parse_set(parser);

    if (check(parser, TOKEN_IF))
        return parse_if(parser);

    if (check(parser, TOKEN_WHILE))
        return parse_while(parser);

    if (check(parser, TOKEN_FOR))
        return parse_for(parser);

    if (check(parser, TOKEN_FUNC))
        return parse_function(
            parser,
            NODE_FUNCTION
        );

    if (check(parser, TOKEN_TASK))
        return parse_function(
            parser,
            NODE_TASK
        );

    if (check(parser, TOKEN_RETURN))
        return parse_return(parser);

    if (check(parser, TOKEN_RUN))
        return parse_run(parser);

    if (check(parser, TOKEN_READ))
        return parse_read(parser);

    if (check(parser, TOKEN_WRITE))
        return parse_write(parser);

    if (check(parser, TOKEN_SAY)) {
        advance_parser(parser);

        result = node(
            NODE_SAY,
            parser->previous.position
        );

        result->left =
            expression(parser);

        return result;
    }

    if (check(parser, TOKEN_SHOW)) {
        advance_parser(parser);

        result = node(
            NODE_SHOW,
            parser->previous.position
        );

        result->left =
            expression(parser);

        return result;
    }

    if (check(parser, TOKEN_LIST)) {
        advance_parser(parser);

        if (!expect(
            parser,
            TOKEN_URLS,
            "expected 'urls' after 'list'"
        )) {
            return NULL;
        }

        if (!expect(
            parser,
            TOKEN_FROM,
            "expected 'from' after 'urls'"
        )) {
            return NULL;
        }

        result = node(
            NODE_LIST_URLS,
            parser->previous.position
        );

        result->left =
            expression(parser);

        return result;
    }

    if (check(parser, TOKEN_ENV)) {
        advance_parser(parser);

        result = node(
            NODE_ENV,
            parser->previous.position
        );

        result->left =
            expression(parser);

        return result;
    }

    if (check(parser, TOKEN_GET)) {
        advance_parser(parser);

        result = node(
            NODE_HTTP_GET,
            parser->previous.position
        );

        result->left =
            expression(parser);

        return result;
    }

    if (check(parser, TOKEN_IDENTIFIER)) {
        CMLLNode *left;

        left = expression(parser);

        if (match(parser, TOKEN_EQUALS)) {
            result = node(
                NODE_ASSIGN,
                left->position
            );

            result->left = left;
            result->right =
                expression(parser);

            return result;
        }

        result = node(
            NODE_EXPRESSION_STATEMENT,
            left->position
        );

        result->left = left;

        return result;
    }

    if (
        check(parser, TOKEN_SEMICOLON)
    ) {
        advance_parser(parser);
        return NULL;
    }

    parser_error(
        parser,
        "expected a statement"
    );

    advance_parser(parser);

    return NULL;
}

static CMLLNode *statement(
    CMLLParser *parser
)
{
    return parse_statement(parser);
}

void cmll_parser_init(
    CMLLParser *parser,
    const char *source,
    const char *filename
)
{
    memset(
        parser,
        0,
        sizeof(*parser)
    );

    cmll_lexer_init(
        &parser->lexer,
        source,
        filename
    );

    parser->current =
        cmll_next_token(
            &parser->lexer
        );
}

CMLLNode *cmll_parse(
    CMLLParser *parser
)
{
    CMLLNode *program;

    program = node(
        NODE_PROGRAM,
        parser->current.position
    );

    while (
        !check(parser, TOKEN_EOF)
    ) {
        CMLLNode *child;

        child = statement(parser);

        if (child)
            add_child(program, child);

        consume_separators(parser);
    }

    cmll_token_free(
        &parser->current
    );

    cmll_token_free(
        &parser->previous
    );

    if (parser->failed) {
        cmll_node_free(program);
        return NULL;
    }

    return program;
}

void cmll_node_free(
    CMLLNode *node
)
{
    size_t i;

    if (!node)
        return;

    free(node->value);
    free(node->value2);

    cmll_node_free(node->left);
    cmll_node_free(node->right);

    for (
        i = 0;
        i < node->child_count;
        i++
    ) {
        cmll_node_free(
            node->children[i]
        );
    }

    free(node->children);
    free(node);
}
