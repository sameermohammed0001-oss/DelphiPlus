
#include "semantic.h"
#include "error.h"

#include <ctype.h>
#include <string.h>

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

        if (node->body != NULL &&
            is_declared(node->body, name))
            return 1;
    }

    return 0;
}

static int check_expression(const Expr *expr, const Ast *declarations)
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

int semantic_check(const Ast *program)
{
    if (program == NULL)
        return 0;

    int valid = 1;
    const Ast *root = program->body;

    for (const Ast *node = root; node != NULL; node = node->next) {
        if (node->expr != NULL)
            valid &= check_expression(node->expr, root);

        if (node->condition != NULL)
            valid &= check_expression(node->condition, root);
    }

    return valid;
}
