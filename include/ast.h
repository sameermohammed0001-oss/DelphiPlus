
#ifndef DELPHIPLUS_AST_H
#define DELPHIPLUS_AST_H

typedef enum {
    AST_BLOCK,
    AST_DECLARE,
    AST_ASSIGN,
    AST_PRINT,
    AST_SQL,
    AST_IF,
    AST_WHILE,
    AST_EXPR_STMT
} AstType;

typedef enum {
    EXPR_NUMBER,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_NULL,
    EXPR_VARIABLE,
    EXPR_BINARY,
    EXPR_UNARY
} ExprType;

typedef struct Expr Expr;
typedef struct Ast Ast;

struct Expr {
    ExprType type;
    char *text;
    char op[8];
    Expr *left;
    Expr *right;
};

struct Ast {
    AstType type;

    char *name;
    char *datatype;
    char *sql;

    Expr *expr;
    Expr *condition;

    Ast *body;
    Ast *else_body;
    Ast *next;
};

Expr *expr_new(ExprType type, const char *text);
Expr *expr_binary(const char *op, Expr *left, Expr *right);

Ast *ast_new(AstType type);
void ast_append(Ast **list, Ast *node);

void ast_free(Ast *node);
void expr_free(Expr *expr);

#endif
