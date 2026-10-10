
#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct {
    const char *word;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    {"BEGIN", TOK_BEGIN},
    {"END", TOK_END},
    {"DECLARE", TOK_DECLARE},
    {"VAR", TOK_VAR},
    {"NUMBER", TOK_NUMBER_KW},
    {"VARCHAR2", TOK_VARCHAR2},
    {"INTEGER", TOK_INTEGER},
    {"PLS_INTEGER", TOK_PLS_INTEGER},
    {"BOOLEAN", TOK_BOOLEAN},
    {"IF", TOK_IF},
    {"THEN", TOK_THEN},
    {"ELSE", TOK_ELSE},
    {"ELSIF", TOK_ELSIF},
    {"LOOP", TOK_LOOP},
    {"WHILE", TOK_WHILE},
    {"FOR", TOK_FOR},
    {"IN", TOK_IN},
    {"REVERSE", TOK_REVERSE},
    {"EXIT", TOK_EXIT},
    {"WHEN", TOK_WHEN},
    {"OTHERS", TOK_OTHERS},
    {"EXCEPTION", TOK_EXCEPTION},
    {"IS", TOK_IS},
    {"AS", TOK_AS},
    {"NULL", TOK_NULL},
    {"TRUE", TOK_TRUE},
    {"FALSE", TOK_FALSE},
    {"AND", TOK_AND},
    {"OR", TOK_OR},
    {"NOT", TOK_NOT},
    {"SELECT", TOK_SELECT},
    {"FROM", TOK_FROM},
    {"WHERE", TOK_WHERE},
    {"INSERT", TOK_INSERT},
    {"INTO", TOK_INTO},
    {"VALUES", TOK_VALUES},
    {"UPDATE", TOK_UPDATE},
    {"SET", TOK_SET},
    {"DELETE", TOK_DELETE},
    {"CREATE", TOK_CREATE},
    {"TABLE", TOK_TABLE},
    {"COMMIT", TOK_COMMIT},
    {"ROLLBACK", TOK_ROLLBACK},
    {"DBMS_OUTPUT", TOK_DBMS_OUTPUT},
    {"PUT_LINE", TOK_PUT_LINE}
};

static char *copy_range(const char *s, size_t length)
{
    char *result = malloc(length + 1);

    if (result == NULL)
        return NULL;

    memcpy(result, s, length);
    result[length] = '\0';

    return result;
}

static int push_token(
    TokenArray *tokens,
    TokenType type,
    const char *text,
    size_t length,
    int line,
    int column)
{
    if (tokens->count == tokens->capacity) {
        size_t new_capacity =
            tokens->capacity == 0 ? 64 : tokens->capacity * 2;

        Token *new_items = realloc(
            tokens->items,
            new_capacity * sizeof(Token)
        );

        if (new_items == NULL)
            return 0;

        tokens->items = new_items;
        tokens->capacity = new_capacity;
    }

    char *lexeme = copy_range(text, length);

    if (lexeme == NULL)
        return 0;

    Token token = {
        .type = type,
        .lexeme = lexeme,
        .line = line,
        .column = column
    };

    tokens->items[tokens->count++] = token;
    return 1;
}

static TokenType find_keyword(const char *word)
{
    size_t count = sizeof(keywords) / sizeof(keywords[0]);

    for (size_t i = 0; i < count; i++) {
        if (strcasecmp(word, keywords[i].word) == 0)
            return keywords[i].type;
    }

    return TOK_IDENTIFIER;
}

int lexer_tokenize(const char *source, TokenArray *out)
{
    if (source == NULL || out == NULL)
        return 0;

    memset(out, 0, sizeof(*out));

    size_t i = 0;
    int line = 1;
    int column = 1;

    while (source[i] != '\0') {
        char c = source[i];

        if (isspace((unsigned char)c)) {
            if (c == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }

            i++;
            continue;
        }

        /* Single-line comment */
        if (c == '-' && source[i + 1] == '-') {
            while (source[i] != '\0' && source[i] != '\n') {
                i++;
                column++;
            }
            continue;
        }

        /* Block comment */
        if (c == '/' && source[i + 1] == '*') {
            i += 2;
            column += 2;

            while (source[i] != '\0' &&
                   !(source[i] == '*' && source[i + 1] == '/')) {
                if (source[i] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                i++;
            }

            if (source[i] != '\0') {
                i += 2;
                column += 2;
            }
            continue;
        }

        size_t start = i;
        int start_line = line;
        int start_column = column;
        TokenType type = TOK_UNKNOWN;

        if (isalpha((unsigned char)c) || c == '_') {
            i++;
            column++;

            while (isalnum((unsigned char)source[i]) ||
                   source[i] == '_' ||
                   source[i] == '$' ||
                   source[i] == '#') {
                i++;
                column++;
            }

            char *word = copy_range(source + start, i - start);

            if (word == NULL) {
                lexer_free(out);
                return 0;
            }

            type = find_keyword(word);
            free(word);
        } else if (isdigit((unsigned char)c)) {
            i++;
            column++;

            while (isdigit((unsigned char)source[i]))
            {
                i++;
                column++;
            }

            if (source[i] == '.' &&
                isdigit((unsigned char)source[i + 1])) {
                i++;
                column++;

                while (isdigit((unsigned char)source[i])) {
                    i++;
                    column++;
                }
            }

            type = TOK_NUMBER;
        } else if (c == '\'') {
            i++;
            column++;

            while (source[i] != '\0') {
                if (source[i] == '\'' && source[i + 1] == '\'') {
                    i += 2;
                    column += 2;
                    continue;
                }

                if (source[i] == '\'') {
                    i++;
                    column++;
                    break;
                }

                if (source[i] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }

                i++;
            }

            type = TOK_STRING;
        } else {
            char next = source[i + 1];

            if (c == ':' && next == '=') {
                type = TOK_ASSIGN;
                i += 2;
                column += 2;
            } else if (c == '<' && next == '=') {
                type = TOK_LE;
                i += 2;
                column += 2;
            } else if (c == '>' && next == '=') {
                type = TOK_GE;
                i += 2;
                column += 2;
            } else if ((c == '<' && next == '>') ||
                       (c == '!' && next == '=')) {
                type = TOK_NE;
                i += 2;
                column += 2;
            } else if (c == '|' && next == '|') {
                type = TOK_CONCAT;
                i += 2;
                column += 2;
            } else {
                i++;
                column++;

                switch (c) {
                    case '=': type = TOK_EQ; break;
                    case '<': type = TOK_LT; break;
                    case '>': type = TOK_GT; break;
                    case '+': type = TOK_PLUS; break;
                    case '-': type = TOK_MINUS; break;
                    case '*': type = TOK_STAR; break;
                    case '/': type = TOK_SLASH; break;
                    case '(': type = TOK_LPAREN; break;
                    case ')': type = TOK_RPAREN; break;
                    case ',': type = TOK_COMMA; break;
                    case '.': type = TOK_DOT; break;
                    case ';': type = TOK_SEMICOLON; break;
                    case ':': type = TOK_COLON; break;
                    case '%': type = TOK_PERCENT; break;
                    default: type = TOK_UNKNOWN; break;
                }
            }
        }

        if (!push_token(
                out, type, source + start, i - start,
                start_line, start_column)) {
            lexer_free(out);
            return 0;
        }
    }

    if (!push_token(out, TOK_EOF, "", 0, line, column)) {
        lexer_free(out);
        return 0;
    }

    return 1;
}

void lexer_free(TokenArray *tokens)
{
    if (tokens == NULL)
        return;

    for (size_t i = 0; i < tokens->count; i++)
        free(tokens->items[i].lexeme);

    free(tokens->items);
    memset(tokens, 0, sizeof(*tokens));
}

const char *token_type_name(TokenType type)
{
    static const char *names[] = {
        "EOF", "IDENTIFIER", "NUMBER", "STRING",
        "BEGIN", "END", "DECLARE", "VAR", "NUMBER_KW",
        "VARCHAR2", "INTEGER", "PLS_INTEGER", "BOOLEAN",
        "IF", "THEN", "ELSE", "ELSIF", "LOOP", "WHILE",
        "FOR", "IN", "REVERSE", "EXIT", "WHEN", "OTHERS",
        "EXCEPTION", "IS", "AS", "NULL", "TRUE", "FALSE",
        "AND", "OR", "NOT", "SELECT", "FROM", "WHERE",
        "INSERT", "INTO", "VALUES", "UPDATE", "SET", "DELETE",
        "CREATE", "TABLE", "COMMIT", "ROLLBACK", "DBMS_OUTPUT",
        "PUT_LINE", "ASSIGN", "EQ", "NE", "LT", "LE", "GT",
        "GE", "PLUS", "MINUS", "STAR", "SLASH", "LPAREN",
        "RPAREN", "COMMA", "DOT", "SEMICOLON", "COLON",
        "CONCAT", "PERCENT", "UNKNOWN"
    };

    size_t count = sizeof(names) / sizeof(names[0]);

    if ((unsigned)type >= count)
        return "UNKNOWN";

    return names[type];
}
