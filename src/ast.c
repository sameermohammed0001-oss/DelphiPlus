
#include "ast.h"

#include <stdlib.h>
#include <string.h>

static char *duplicate_string(const char *text)
{
    if (text == NULL)
        return NULL;

    size_t length = strlen(text) + 1;
    char *copy = malloc(length);

    if (copy != NULL)
        memcpy(copy, text, length);

    return copy;
}

Expr *expr_new(ExprType type, const char *text)
{
    Expr *expr = calloc(1, sizeof(*expr));

    if (expr == NULL)
        return NULL;

    expr->type = type;
    expr->text = duplicate_string(text);

    return expr;
}

Expr *expr_binary(const char *op, Expr *left, Expr *right)
{
    Expr *expr = expr_new(EXPR_BINARY, NULL);

    if (expr == NULL) {
        expr_free(left);
        expr_free(right);
        return NULL;
    }

    if (op != NULL)
        strncpy(expr->op, op, sizeof(expr->op) - 1);

    expr->left = left;
    expr->right = right;

    return expr;
}

Ast *ast_new(AstType type)
{
    Ast *node = calloc(1, sizeof(*node));

    if (node != NULL)
        node->type = type;

    return node;
}

void ast_append(Ast **list, Ast *node)
{
    if (list == NULL || node == NULL)
        return;

    if (*list == NULL) {
        *list = node;
        return;
    }

    Ast *current = *list;

    while (current->next != NULL)
        current = current->next;

    current->next = node;
}

void expr_free(Expr *expr)
{
    if (expr == NULL)
        return;

    expr_free(expr->left);
    expr_free(expr->right);

    free(expr->text);
    free(expr);
}

void ast_free(Ast *node)
{
    while (node != NULL) {
        Ast *next = node->next;

        free(node->name);
        free(node->datatype);
        free(node->sql);

        expr_free(node->expr);
        expr_free(node->condition);

        ast_free(node->body);
        ast_free(node->else_body);

        free(node);
        node = next;
    }
}
