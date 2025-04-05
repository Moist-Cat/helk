#ifndef AST_H
#define AST_H

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} ASTBinaryOp;

typedef struct ASTNode {
    enum {
        AST_NUMBER,
        AST_BINARY_OP
    } type;
    union {
        int number;
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            ASTBinaryOp op;
        } binary_op;
    };
} ASTNode;

ASTNode *create_ast_number(int value);
ASTNode *create_ast_binary_op(ASTNode *left, ASTNode *right, ASTBinaryOp op);
void free_ast(ASTNode *node);

#endif
