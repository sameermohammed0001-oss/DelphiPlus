
#include <stddef.h>
#include "semantic.h"
#include "error.h"

#include <ctype.h>

static int same_name(const char *a, const char *b)
{
    if (a == NULL || b == NULL)
        return 0;

    while (*a && *b) {
        if (toupper((unsigned char)*a) !=
            toupper((unsigned char)*b))
            return 0;

        a++;
        b++;
    }

    return *a == *b;
}

static int is_declared(const Ast *list, const char *name)
{
    for (const Ast *node = list; node != NULL; node = node->next) {
        if (node->type == AST_DECLARE &&
            same_name(node->name, name))
            return 1;
    }

    return 0;
}

static int check_expression(
    const Expr *expr,
    const Ast *declarations)
{
    if (expr == NULL)
        return 1;

    int valid = 1;

    if (expr->type == EXPR_VARIABLE &&
        !is_declared(declarations, expr->text)) {
        error_set("Use of undeclared variable", 0, 0);
        valid = 0;
    }

    if (expr->left != NULL)
        valid &= check_expression(expr->left, declarations);

    if (expr->right != NULL)
        valid &= check_expression(expr->right, declarations);

    return valid;
}

static int check_statements(
    const Ast *statements,
    const Ast *declarations)
{
    int valid = 1;

    for (const Ast *node = statements;
         node != NULL;
         node = node->next) {

        if (node->type == AST_DECLARE && node->expr != NULL)
            valid &= check_expression(node->expr, declarations);

        if (node->type == AST_ASSIGN) {
            if (!is_declared(declarations, node->name)) {
                error_set("Assignment to undeclared variable", 0, 0);
                valid = 0;
            }

            valid &= check_expression(node->expr, declarations);
        }

        if (node->type == AST_PRINT)
            valid &= check_expression(node->expr, declarations);

        if (node->type == AST_IF) {
            valid &= check_expression(node->condition, declarations);
            valid &= check_statements(node->body, declarations);
            valid &= check_statements(node->else_body, declarations);
        }

        if (node->type == AST_WHILE) {
            valid &= check_expression(node->condition, declarations);
            valid &= check_statements(node->body, declarations);
        }

        if (node->type == AST_BLOCK)
            valid &= check_statements(node->body, declarations);
    }

    return valid;
}

int semantic_check(const Ast *program)
{
    if (program == NULL)
        return 0;

    return check_statements(program->body, program->body);
}
