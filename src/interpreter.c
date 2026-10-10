
#include "interpreter.h"
#include "sql_executor.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Variable {
    char *name;
    char *value;
    struct Variable *next;
} Variable;

static Variable *variables = NULL;

static char *copy_string(const char *text)
{
    if (text == NULL)
        text = "NULL";

    size_t length = strlen(text) + 1;
    char *copy = malloc(length);

    if (copy != NULL)
        memcpy(copy, text, length);

    return copy;
}

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

static Variable *find_variable(const char *name)
{
    for (Variable *v = variables; v != NULL; v = v->next) {
        if (same_name(v->name, name))
            return v;
    }

    return NULL;
}

static void set_variable(const char *name, const char *value)
{
    Variable *v = find_variable(name);

    if (v == NULL) {
        v = calloc(1, sizeof(*v));

        if (v == NULL) {
            error_set("Out of memory creating variable", 0, 0);
            return;
        }

        v->name = copy_string(name);
        v->next = variables;
        variables = v;
    }

    free(v->value);
    v->value = copy_string(value);
}

static void clear_variables(void)
{
    while (variables != NULL) {
        Variable *next = variables->next;

        free(variables->name);
        free(variables->value);
        free(variables);

        variables = next;
    }
}

static int truthy(const char *value)
{
    return same_name(value, "TRUE") ||
           (value != NULL && atoi(value) != 0);
}

static char *evaluate(const Expr *expr)
{
    if (expr == NULL)
        return copy_string("NULL");

    switch (expr->type) {
        case EXPR_NUMBER:
        case EXPR_STRING:
        case EXPR_BOOL:
        case EXPR_NULL:
            return copy_string(expr->text);

        case EXPR_VARIABLE: {
            Variable *v = find_variable(expr->text);

            if (v == NULL) {
                error_set("Runtime: variable is not defined", 0, 0);
                return copy_string("NULL");
            }

            return copy_string(v->value);
        }

        case EXPR_UNARY: {
            char *value = evaluate(expr->right);

            if (strcmp(expr->op, "-") == 0) {
                char result[80];

                snprintf(result, sizeof(result), "%.10g",
                         -atof(value));

                free(value);
                return copy_string(result);
            }

            if (strcmp(expr->op, "NOT") == 0) {
                int result = !truthy(value);
                free(value);

                return copy_string(result ? "TRUE" : "FALSE");
            }

            return value;
        }

        case EXPR_BINARY:
            break;
    }

    char *left = evaluate(expr->left);
    char *right = evaluate(expr->right);
    char result[512] = "NULL";

    if (strcmp(expr->op, "||") == 0) {
        snprintf(result, sizeof(result), "%s%s", left, right);
    } else if (strcmp(expr->op, "+") == 0 ||
               strcmp(expr->op, "-") == 0 ||
               strcmp(expr->op, "*") == 0 ||
               strcmp(expr->op, "/") == 0) {
        double a = atof(left);
        double b = atof(right);
        double answer = 0;

        if (strcmp(expr->op, "+") == 0)
            answer = a + b;
        else if (strcmp(expr->op, "-") == 0)
            answer = a - b;
        else if (strcmp(expr->op, "*") == 0)
            answer = a * b;
        else if (b != 0)
            answer = a / b;
        else
            error_set("Division by zero", 0, 0);

        snprintf(result, sizeof(result), "%.10g", answer);
    } else if (strcmp(expr->op, "=") == 0) {
        snprintf(result, sizeof(result), "%s",
                 same_name(left, right) ? "TRUE" : "FALSE");
    } else if (strcmp(expr->op, "<>") == 0 ||
               strcmp(expr->op, "!=") == 0) {
        snprintf(result, sizeof(result), "%s",
                 !same_name(left, right) ? "TRUE" : "FALSE");
    } else {
        double a = atof(left);
        double b = atof(right);
        int answer = 0;

        if (strcmp(expr->op, "<") == 0)
            answer = a < b;
        else if (strcmp(expr->op, ">") == 0)
            answer = a > b;
        else if (strcmp(expr->op, "<=") == 0)
            answer = a <= b;
        else if (strcmp(expr->op, ">=") == 0)
            answer = a >= b;
        else if (strcmp(expr->op, "AND") == 0)
            answer = truthy(left) && truthy(right);
        else if (strcmp(expr->op, "OR") == 0)
            answer = truthy(left) || truthy(right);

        snprintf(result, sizeof(result), "%s",
                 answer ? "TRUE" : "FALSE");
    }

    free(left);
    free(right);

    return copy_string(result);
}

static void execute_list(const Ast *node)
{
    for (; node != NULL; node = node->next) {
        switch (node->type) {
            case AST_BLOCK:
                execute_list(node->body);
                break;

            case AST_DECLARE: {
                char *value = node->expr
                    ? evaluate(node->expr)
                    : copy_string("NULL");

                set_variable(node->name, value);
                free(value);
                break;
            }

            case AST_ASSIGN: {
                char *value = evaluate(node->expr);
                set_variable(node->name, value);
                free(value);
                break;
            }

            case AST_PRINT: {
                char *value = evaluate(node->expr);
                printf("%s\n", value);
                free(value);
                break;
            }

            case AST_SQL:
                sql_execute(node->sql);
                break;

            case AST_EXPR_STMT:
                break;

            case AST_IF:
            case AST_WHILE:
                error_set(
                    "IF and LOOP execution are not implemented yet",
                    0, 0
                );
                break;
        }
    }
}

int interpreter_run(const Ast *program)
{
    clear_variables();

    if (program != NULL)
        execute_list(program);

    clear_variables();

    return error_count() == 0;
}
