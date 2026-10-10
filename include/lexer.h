
#ifndef DELPHIPLUS_LEXER_H
#define DELPHIPLUS_LEXER_H

#include <stddef.h>

typedef enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_BEGIN,
    TOK_END,
    TOK_DECLARE,
    TOK_VAR,
    TOK_NUMBER_KW,
    TOK_VARCHAR2,
    TOK_INTEGER,
    TOK_PLS_INTEGER,
    TOK_BOOLEAN,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_ELSIF,
    TOK_LOOP,
    TOK_WHILE,
    TOK_FOR,
    TOK_IN,
    TOK_REVERSE,
    TOK_EXIT,
    TOK_WHEN,
    TOK_OTHERS,
    TOK_EXCEPTION,
    TOK_IS,
    TOK_AS,
    TOK_NULL,
    TOK_TRUE,
    TOK_FALSE,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_SELECT,
    TOK_FROM,
    TOK_WHERE,
    TOK_INSERT,
    TOK_INTO,
    TOK_VALUES,
    TOK_UPDATE,
    TOK_SET,
    TOK_DELETE,
    TOK_CREATE,
    TOK_TABLE,
    TOK_COMMIT,
    TOK_ROLLBACK,
    TOK_DBMS_OUTPUT,
    TOK_PUT_LINE,
    TOK_ASSIGN,
    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_DOT,
    TOK_SEMICOLON,
    TOK_COLON,
    TOK_CONCAT,
    TOK_PERCENT,
    TOK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

int lexer_tokenize(const char *source, TokenArray *out);
void lexer_free(TokenArray *tokens);
const char *token_type_name(TokenType type);

#endif
