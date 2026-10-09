#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char current(
    CMLLLexer *lexer
)
{
    if (lexer->index >= lexer->length)
        return '\0';

    return lexer->source[lexer->index];
}

static char peek(
    CMLLLexer *lexer
)
{
    if (lexer->index + 1 >= lexer->length)
        return '\0';

    return lexer->source[lexer->index + 1];
}

static void advance(
    CMLLLexer *lexer
)
{
    char c;

    c = current(lexer);

    if (!c)
        return;

    lexer->index++;

    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
}

static CMLLPosition position(
    CMLLLexer *lexer
)
{
    CMLLPosition result;

    result.filename = lexer->filename;
    result.line = lexer->line;
    result.column = lexer->column;

    return result;
}

static char *range(
    const char *source,
    size_t start,
    size_t end
)
{
    size_t length;
    char *result;

    length = end - start;

    result = malloc(length + 1);

    if (!result)
        return NULL;

    memcpy(
        result,
        source + start,
        length
    );

    result[length] = '\0';

    return result;
}

static CMLLToken token(
    CMLLTokenType type,
    char *text,
    CMLLPosition pos
)
{
    CMLLToken result;

    result.type = type;
    result.text = text;
    result.position = pos;

    return result;
}

static CMLLToken simple(
    CMLLLexer *lexer,
    CMLLTokenType type
)
{
    size_t start;
    CMLLPosition pos;

    start = lexer->index;
    pos = position(lexer);

    advance(lexer);

    return token(
        type,
        range(
            lexer->source,
            start,
            lexer->index
        ),
        pos
    );
}

static CMLLToken identifier(
    CMLLLexer *lexer
)
{
    size_t start;
    CMLLPosition pos;
    char *word;
    CMLLTokenType type;

    start = lexer->index;
    pos = position(lexer);

    while (
        isalnum((unsigned char)current(lexer)) ||
        current(lexer) == '_' ||
        current(lexer) == '-'
    ) {
        advance(lexer);
    }

    word = range(
        lexer->source,
        start,
        lexer->index
    );

    type = TOKEN_IDENTIFIER;

    if (strcmp(word, "project") == 0)
        type = TOKEN_PROJECT;
    else if (strcmp(word, "set") == 0)
        type = TOKEN_SET;
    else if (strcmp(word, "task") == 0)
        type = TOKEN_TASK;
    else if (strcmp(word, "if") == 0)
        type = TOKEN_IF;
    else if (strcmp(word, "else") == 0)
        type = TOKEN_ELSE;
    else if (strcmp(word, "while") == 0)
        type = TOKEN_WHILE;
    else if (strcmp(word, "for") == 0)
        type = TOKEN_FOR;
    else if (strcmp(word, "in") == 0)
        type = TOKEN_IN;
    else if (strcmp(word, "func") == 0)
        type = TOKEN_FUNC;
    else if (strcmp(word, "return") == 0)
        type = TOKEN_RETURN;
    else if (strcmp(word, "true") == 0)
        type = TOKEN_TRUE;
    else if (strcmp(word, "false") == 0)
        type = TOKEN_FALSE;
    else if (strcmp(word, "null") == 0)
        type = TOKEN_NULL;
    else if (strcmp(word, "say") == 0)
        type = TOKEN_SAY;
    else if (strcmp(word, "show") == 0)
        type = TOKEN_SHOW;
    else if (strcmp(word, "run") == 0)
        type = TOKEN_RUN;
    else if (strcmp(word, "read") == 0)
        type = TOKEN_READ;
    else if (strcmp(word, "write") == 0)
        type = TOKEN_WRITE;
    else if (strcmp(word, "file") == 0)
        type = TOKEN_FILE;
    else if (strcmp(word, "exists") == 0)
        type = TOKEN_EXISTS;
    else if (strcmp(word, "list") == 0)
        type = TOKEN_LIST;
    else if (strcmp(word, "urls") == 0)
        type = TOKEN_URLS;
    else if (strcmp(word, "from") == 0)
        type = TOKEN_FROM;
    else if (strcmp(word, "env") == 0)
        type = TOKEN_ENV;
    else if (strcmp(word, "get") == 0)
        type = TOKEN_GET;

    return token(
        type,
        word,
        pos
    );
}

static CMLLToken string_token(
    CMLLLexer *lexer
)
{
    CMLLPosition pos;
    size_t start;
    size_t capacity;
    size_t length;
    char *result;
    char quote;

    pos = position(lexer);
    quote = current(lexer);

    advance(lexer);

    capacity = 32;
    length = 0;

    result = malloc(capacity);

    if (!result)
        return token(
            TOKEN_EOF,
            NULL,
            pos
        );

    while (current(lexer) != '\0') {
        char c;

        c = current(lexer);

        if (c == quote) {
            advance(lexer);
            result[length] = '\0';

            return token(
                TOKEN_STRING,
                result,
                pos
            );
        }

        if (c == '\\') {
            advance(lexer);

            c = current(lexer);

            if (!c)
                break;

            switch (c) {
                case 'n':
                    c = '\n';
                    break;

                case 't':
                    c = '\t';
                    break;

                case 'r':
                    c = '\r';
                    break;

                case '\\':
                    c = '\\';
                    break;

                case '"':
                    c = '"';
                    break;

                case '\'':
                    c = '\'';
                    break;

                default:
                    break;
            }
        }

        if (length + 2 > capacity) {
            char *expanded;

            capacity *= 2;

            expanded = realloc(
                result,
                capacity
            );

            if (!expanded) {
                free(result);

                return token(
                    TOKEN_EOF,
                    NULL,
                    pos
                );
            }

            result = expanded;
        }

        result[length++] = c;

        advance(lexer);
    }

    free(result);

    cmll_error(
        pos,
        lexer->source,
        "unterminated string"
    );

    lexer->failed = 1;

    return token(
        TOKEN_EOF,
        NULL,
        pos
    );
}

void cmll_lexer_init(
    CMLLLexer *lexer,
    const char *source,
    const char *filename
)
{
    memset(
        lexer,
        0,
        sizeof(*lexer)
    );

    lexer->source = source;
    lexer->filename = filename;

    lexer->length = strlen(source);

    lexer->line = 1;
    lexer->column = 1;
}

CMLLToken cmll_next_token(
    CMLLLexer *lexer
)
{
    while (1) {
        char c;

        c = current(lexer);

        if (!c) {
            return token(
                TOKEN_EOF,
                cmll_strdup(""),
                position(lexer)
            );
        }

        if (
            c == ' ' ||
            c == '\t' ||
            c == '\r' ||
            c == '\n'
        ) {
            advance(lexer);
            continue;
        }

        if (c == '#') {
            while (
                current(lexer) &&
                current(lexer) != '\n'
            ) {
                advance(lexer);
            }

            continue;
        }

        break;
    }

    {
        char c;
        CMLLPosition pos;

        c = current(lexer);
        pos = position(lexer);

        if (
            isalpha((unsigned char)c) ||
            c == '_'
        ) {
            return identifier(lexer);
        }

        if (isdigit((unsigned char)c)) {
            size_t start;

            start = lexer->index;

            while (
                isdigit((unsigned char)current(lexer)) ||
                current(lexer) == '.'
            ) {
                advance(lexer);
            }

            return token(
                TOKEN_NUMBER,
                range(
                    lexer->source,
                    start,
                    lexer->index
                ),
                pos
            );
        }

        if (c == '"' || c == '\'')
            return string_token(lexer);

        if (c == '=' && peek(lexer) == '=') {
            advance(lexer);
            advance(lexer);

            return token(
                TOKEN_EQUAL_EQUAL,
                cmll_strdup("=="),
                pos
            );
        }

        if (c == '!' && peek(lexer) == '=') {
            advance(lexer);
            advance(lexer);

            return token(
                TOKEN_NOT_EQUAL,
                cmll_strdup("!="),
                pos
            );
        }

        if (c == '<' && peek(lexer) == '=') {
            advance(lexer);
            advance(lexer);

            return token(
                TOKEN_LESS_EQUAL,
                cmll_strdup("<="),
                pos
            );
        }

        if (c == '>' && peek(lexer) == '=') {
            advance(lexer);
            advance(lexer);

            return token(
                TOKEN_GREATER_EQUAL,
                cmll_strdup(">="),
                pos
            );
        }

        if (c == '&' && peek(lexer) == '&') {
            advance(lexer);
            advance(lexer);

            return token(
                TOKEN_AND,
                cmll_strdup("&&"),
                pos
            );
        }

        if (c == '|' && peek(lexer) == '|') {
            advance(lexer);
            advance(lexer);

            return token(
                TOKEN_OR,
                cmll_strdup("||"),
                pos
            );
        }

        switch (c) {
            case '{':
                return simple(lexer, TOKEN_LBRACE);

            case '}':
                return simple(lexer, TOKEN_RBRACE);

            case '[':
                return simple(lexer, TOKEN_LBRACKET);

            case ']':
                return simple(lexer, TOKEN_RBRACKET);

            case '(':
                return simple(lexer, TOKEN_LPAREN);

            case ')':
                return simple(lexer, TOKEN_RPAREN);

            case ',':
                return simple(lexer, TOKEN_COMMA);

            case ':':
                return simple(lexer, TOKEN_COLON);

            case '.':
                return simple(lexer, TOKEN_DOT);

            case ';':
                return simple(lexer, TOKEN_SEMICOLON);

            case '+':
                return simple(lexer, TOKEN_PLUS);

            case '-':
                return simple(lexer, TOKEN_MINUS);

            case '*':
                return simple(lexer, TOKEN_STAR);

            case '/':
                return simple(lexer, TOKEN_SLASH);

            case '%':
                return simple(lexer, TOKEN_PERCENT);

            case '=':
                return simple(lexer, TOKEN_EQUALS);

            case '<':
                return simple(lexer, TOKEN_LESS);

            case '>':
                return simple(lexer, TOKEN_GREATER);

            case '!':
                return simple(lexer, TOKEN_NOT);

            case '|':
                return simple(lexer, TOKEN_PIPE);

            default:
                {
                    char message[128];

                    snprintf(
                        message,
                        sizeof(message),
                        "unexpected character '%c'",
                        c
                    );

                    cmll_error(
                        pos,
                        lexer->source,
                        message
                    );

                    lexer->failed = 1;

                    advance(lexer);

                    return token(
                        TOKEN_EOF,
                        cmll_strdup(""),
                        pos
                    );
                }
        }
    }
}

void cmll_token_free(
    CMLLToken *token
)
{
    if (!token)
        return;

    free(token->text);
    token->text = NULL;
}

const char *cmll_token_name(
    CMLLTokenType type
)
{
    switch (type) {
        case TOKEN_EOF: return "end of file";
        case TOKEN_IDENTIFIER: return "identifier";
        case TOKEN_STRING: return "string";
        case TOKEN_NUMBER: return "number";

        case TOKEN_PROJECT: return "project";
        case TOKEN_SET: return "set";
        case TOKEN_TASK: return "task";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_FOR: return "for";
        case TOKEN_IN: return "in";
        case TOKEN_FUNC: return "func";
        case TOKEN_RETURN: return "return";

        case TOKEN_TRUE: return "true";
        case TOKEN_FALSE: return "false";
        case TOKEN_NULL: return "null";

        case TOKEN_SAY: return "say";
        case TOKEN_SHOW: return "show";
        case TOKEN_RUN: return "run";

        case TOKEN_READ: return "read";
        case TOKEN_WRITE: return "write";
        case TOKEN_FILE: return "file";
        case TOKEN_EXISTS: return "exists";

        case TOKEN_LIST: return "list";
        case TOKEN_URLS: return "urls";
        case TOKEN_FROM: return "from";

        case TOKEN_ENV: return "env";
        case TOKEN_GET: return "get";

        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";

        case TOKEN_COMMA: return ",";
        case TOKEN_COLON: return ":";
        case TOKEN_DOT: return ".";
        case TOKEN_SEMICOLON: return ";";

        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_PERCENT: return "%";

        case TOKEN_EQUALS: return "=";
        case TOKEN_EQUAL_EQUAL: return "==";
        case TOKEN_NOT_EQUAL: return "!=";

        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";

        case TOKEN_AND: return "&&";
        case TOKEN_OR: return "||";
        case TOKEN_NOT: return "!";

        case TOKEN_PIPE: return "|";
    }

    return "unknown";
}
