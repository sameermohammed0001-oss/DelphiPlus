
#include "parser.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>

static TokenArray *tokens;
static size_t position;

static Token *current(void)
{
    if (position >= tokens->count)
        return &tokens->items[tokens->count - 1];

    return &tokens->items[position];
}

static Token *advance_token(void)
{
    Token *token = current();

    if (position + 1 < tokens->count)
        position++;

    return token;
}

static int accept(TokenType type)
{
    if (current()->type == type) {
        advance_token();
        return 1;
    }

    return 0;
}

static int expect(TokenType type, const char *message)
{
    if (accept(type))
        return 1;

    error_set(message, current()->line, current()->column);
    return 0;
}

static char *duplicate_string(const char *text)
{
    if (text == NULL)
        return NULL;

    size_t length = strlen(text) + 1;
    char *result = malloc(length);

    if (result != NULL)
        memcpy(result, text, length);

    return result;
}

static Expr *parse_expression(void);
static Ast *parse_statement(void);
static Ast *parse_block_body(void);
static Ast *parse_if_tail(void);

/* ---------- Expressions ---------- */

static Expr *parse_primary(void)
{
    Token *token = current();

    if (accept(TOK_NUMBER))
        return expr_new(EXPR_NUMBER, token->lexeme);

    if (accept(TOK_STRING)) {
        size_t length = strlen(token->lexeme);
        char *value = duplicate_string(token->lexeme);

        if (value == NULL)
            return NULL;

        if (length >= 2) {
            memmove(value, value + 1, length - 2);
            value[length - 2] = '\0';
        }

        Expr *expr = expr_new(EXPR_STRING, value);
        free(value);
        return expr;
    }

    if (accept(TOK_TRUE))
        return expr_new(EXPR_BOOL, "TRUE");

    if (accept(TOK_FALSE))
        return expr_new(EXPR_BOOL, "FALSE");

    if (accept(TOK_NULL))
        return expr_new(EXPR_NULL, "NULL");

    if (accept(TOK_IDENTIFIER))
        return expr_new(EXPR_VARIABLE, token->lexeme);

    if (accept(TOK_LPAREN)) {
        Expr *expr = parse_expression();
        expect(TOK_RPAREN, "Expected closing parenthesis");
        return expr;
    }

    error_set("Expected an expression",
              token->line, token->column);

    advance_token();
    return expr_new(EXPR_NULL, "NULL");
}

static Expr *parse_unary(void)
{
    if (accept(TOK_MINUS)) {
        Expr *expr = expr_new(EXPR_UNARY, "-");

        if (expr != NULL)
            expr->right = parse_unary();

        return expr;
    }

    if (accept(TOK_NOT)) {
        Expr *expr = expr_new(EXPR_UNARY, "NOT");

        if (expr != NULL)
            expr->right = parse_unary();

        return expr;
    }

    return parse_primary();
}

static Expr *parse_multiplication(void)
{
    Expr *left = parse_unary();

    while (current()->type == TOK_STAR ||
           current()->type == TOK_SLASH) {
        Token *op = advance_token();
        Expr *right = parse_unary();

        left = expr_binary(op->lexeme, left, right);
    }

    return left;
}

static Expr *parse_addition(void)
{
    Expr *left = parse_multiplication();

    while (current()->type == TOK_PLUS ||
           current()->type == TOK_MINUS ||
           current()->type == TOK_CONCAT) {
        Token *op = advance_token();
        Expr *right = parse_multiplication();

        left = expr_binary(op->lexeme, left, right);
    }

    return left;
}

static Expr *parse_comparison(void)
{
    Expr *left = parse_addition();

    while (current()->type == TOK_EQ ||
           current()->type == TOK_NE ||
           current()->type == TOK_LT ||
           current()->type == TOK_LE ||
           current()->type == TOK_GT ||
           current()->type == TOK_GE) {
        Token *op = advance_token();
        Expr *right = parse_addition();

        left = expr_binary(op->lexeme, left, right);
    }

    return left;
}

static Expr *parse_and(void)
{
    Expr *left = parse_comparison();

    while (accept(TOK_AND))
        left = expr_binary("AND", left, parse_comparison());

    return left;
}

static Expr *parse_expression(void)
{
    Expr *left = parse_and();

    while (accept(TOK_OR))
        left = expr_binary("OR", left, parse_and());

    return left;
}

/* ---------- Declarations ---------- */

static int is_datatype(TokenType type)
{
    return type == TOK_NUMBER_KW ||
           type == TOK_INTEGER ||
           type == TOK_PLS_INTEGER ||
           type == TOK_VARCHAR2 ||
           type == TOK_BOOLEAN;
}

static Ast *parse_declarations(void)
{
    Ast *list = NULL;

    while (current()->type == TOK_IDENTIFIER) {
        Token *name = advance_token();
        Ast *node = ast_new(AST_DECLARE);

        if (node == NULL)
            return list;

        node->name = duplicate_string(name->lexeme);

        if (!is_datatype(current()->type)) {
            error_set("Expected a supported datatype",
                      current()->line, current()->column);

            ast_append(&list, node);

            while (current()->type != TOK_SEMICOLON &&
                   current()->type != TOK_EOF) {
                advance_token();
            }

            accept(TOK_SEMICOLON);
            continue;
        }

        node->datatype =
            duplicate_string(advance_token()->lexeme);

        if (accept(TOK_LPAREN)) {
            while (current()->type != TOK_RPAREN &&
                   current()->type != TOK_EOF) {
                advance_token();
            }

            expect(TOK_RPAREN, "Expected ')' in datatype");
        }

        if (accept(TOK_ASSIGN) || accept(TOK_EQ))
            node->expr = parse_expression();

        expect(TOK_SEMICOLON,
               "Expected ';' after declaration");

        ast_append(&list, node);
    }

    return list;
}

/* ---------- IF / ELSIF / ELSE ---------- */

/*
 * Called after the IF token has been consumed.
 *
 * Grammar:
 * IF expression THEN
 *     statements
 * [ELSIF expression THEN
 *     statements]
 * [ELSE
 *     statements]
 * END IF;
 *
 * ELSIF is represented as a nested AST_IF node.
 */
static Ast *parse_if_tail(void)
{
    Ast *node = ast_new(AST_IF);

    if (node == NULL)
        return NULL;

    node->condition = parse_expression();

    expect(TOK_THEN, "Expected THEN after IF condition");

    node->body = parse_block_body();

    if (accept(TOK_ELSIF)) {
        node->else_body = parse_if_tail();
    } else if (accept(TOK_ELSE)) {
        node->else_body = parse_block_body();
    }

    return node;
}

static Ast *parse_if_statement(void)
{
    /* IF token has already been consumed. */
    Ast *node = parse_if_tail();

    expect(TOK_END, "Expected END before IF");
    expect(TOK_IF, "Expected IF after END");
    expect(TOK_SEMICOLON, "Expected ';' after END IF");

    return node;
}

/* ---------- General statements ---------- */

static Ast *parse_block_body(void)
{
    Ast *list = NULL;

    while (current()->type != TOK_END &&
           current()->type != TOK_ELSE &&
           current()->type != TOK_ELSIF &&
           current()->type != TOK_EXCEPTION &&
           current()->type != TOK_EOF) {
        size_t old_position = position;

        Ast *node = parse_statement();

        if (node != NULL)
            ast_append(&list, node);

        if (position == old_position) {
            error_set("Parser could not advance",
                      current()->line, current()->column);
            advance_token();
        }
    }

    return list;
}

static Ast *parse_statement(void)
{
    Token *token = current();

    if (accept(TOK_IF))
        return parse_if_statement();

    if (accept(TOK_DBMS_OUTPUT)) {
        expect(TOK_DOT, "Expected '.' after DBMS_OUTPUT");
        expect(TOK_PUT_LINE, "Expected PUT_LINE");
        expect(TOK_LPAREN, "Expected '(' after PUT_LINE");

        Ast *node = ast_new(AST_PRINT);

        if (node != NULL)
            node->expr = parse_expression();

        expect(TOK_RPAREN, "Expected ')' after expression");
        expect(TOK_SEMICOLON,
               "Expected ';' after output statement");

        return node;
    }

    if (accept(TOK_BEGIN)) {
        Ast *node = ast_new(AST_BLOCK);

        if (node != NULL)
            node->body = parse_block_body();

        expect(TOK_END, "Expected END");

        if (current()->type == TOK_IDENTIFIER)
            advance_token();

        expect(TOK_SEMICOLON, "Expected ';' after END");

        return node;
    }

    if (current()->type == TOK_IDENTIFIER) {
        Token *name = advance_token();

        if (accept(TOK_ASSIGN)) {
            Ast *node = ast_new(AST_ASSIGN);

            if (node != NULL) {
                node->name = duplicate_string(name->lexeme);
                node->expr = parse_expression();
            }

            expect(TOK_SEMICOLON,
                   "Expected ';' after assignment");

            return node;
        }

        error_set("Expected ':=' after variable name",
                  name->line, name->column);

        while (current()->type != TOK_SEMICOLON &&
               current()->type != TOK_EOF) {
            advance_token();
        }

        accept(TOK_SEMICOLON);
        return NULL;
    }

    if (current()->type == TOK_SELECT ||
        current()->type == TOK_INSERT ||
        current()->type == TOK_UPDATE ||
        current()->type == TOK_DELETE ||
        current()->type == TOK_CREATE ||
        current()->type == TOK_COMMIT ||
        current()->type == TOK_ROLLBACK) {
        char buffer[4096] = "";
        size_t used = 0;

        while (current()->type != TOK_SEMICOLON &&
               current()->type != TOK_EOF &&
               used < sizeof(buffer) - 128) {
            Token *part = advance_token();
            size_t length = strlen(part->lexeme);

            if (used + length + 2 >= sizeof(buffer))
                break;

            if (used > 0)
                buffer[used++] = ' ';

            memcpy(buffer + used, part->lexeme, length);
            used += length;
            buffer[used] = '\0';
        }

        expect(TOK_SEMICOLON,
               "Expected ';' after SQL statement");

        Ast *node = ast_new(AST_SQL);

        if (node != NULL)
            node->sql = duplicate_string(buffer);

        return node;
    }

    if (accept(TOK_NULL)) {
        expect(TOK_SEMICOLON, "Expected ';' after NULL");
        return ast_new(AST_EXPR_STMT);
    }

    error_set("Unsupported statement",
              token->line, token->column);

    advance_token();
    return NULL;
}

/* ---------- Program entry ---------- */

Ast *parser_parse(TokenArray *input)
{
    if (input == NULL || input->count == 0)
        return NULL;

    tokens = input;
    position = 0;

    Ast *program = ast_new(AST_BLOCK);

    if (program == NULL)
        return NULL;

    if (accept(TOK_DECLARE))
        program->body = parse_declarations();

    if (accept(TOK_BEGIN)) {
        Ast *body = parse_block_body();

        expect(TOK_END, "Expected END");

        if (current()->type == TOK_IDENTIFIER)
            advance_token();

        expect(TOK_SEMICOLON, "Expected ';' after END");

        if (program->body == NULL) {
            program->body = body;
        } else {
            Ast *last = program->body;

            while (last->next != NULL)
                last = last->next;

            last->next = body;
        }
    } else {
        error_set("Expected BEGIN block",
                  current()->line, current()->column);
    }

    if (current()->type != TOK_EOF)
        error_set("Unexpected tokens after program",
                  current()->line, current()->column);

    return program;
}
