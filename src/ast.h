#ifndef AST_H
#define AST_H

typedef enum {
    OP_ADD = 100,
    OP_SUB,
    OP_MUL,
    OP_DIV
} ASTBinaryOp;

typedef enum {
    AST_BLOCK,
    AST_NUMBER,
    AST_BINARY_OP,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_VARIABLE,
    AST_VARIABLE_DEF
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int number;
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            ASTBinaryOp op;
        } binary_op;
        struct {
            char *name;
            char **args;
            unsigned int arg_count;
            struct ASTNode *body;
        } function_def;
        struct {
            char *name;
            unsigned int arg_count;
            // array of nodes
            struct ASTNode **args;
        } function_call;
        struct {
            char *name;

        } variable;
        struct {
            char *name;
            struct ASTNode *body;
        } variable_def;
        struct {
            struct ASTNode **statements;
            unsigned int stmt_count;
        } block;
    };
} ASTNode;

ASTNode *create_ast_block(ASTNode **block, unsigned int stmt_count);
ASTNode *create_ast_number(int value);
ASTNode *create_ast_binary_op(ASTNode *left, ASTNode *right, ASTBinaryOp op);
ASTNode *create_ast_function_def(char *name, ASTNode *body, char **args, unsigned int arg_count);
ASTNode *create_ast_function_call(char *name, ASTNode **args, unsigned int arg_count);
ASTNode *create_ast_variable(char *name);
ASTNode *create_ast_variable_def(char *name, ASTNode *body);
void free_ast(ASTNode *node);

#endif
