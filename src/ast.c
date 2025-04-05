#include "ast.h"
#include <stdlib.h>

ASTNode *create_ast_number(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->number = value;
    return node;
}

ASTNode *create_ast_binary_op(ASTNode *left, ASTNode *right, ASTBinaryOp op) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->binary_op.left = left;
    node->binary_op.right = right;
    node->binary_op.op = op;
    return node;
}

void free_ast(ASTNode *node) {
    if (node) {
        if (node->type == AST_BINARY_OP) {
            free_ast(node->binary_op.left);
            free_ast(node->binary_op.right);
        }
        free(node);
    }
}
