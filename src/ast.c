#include "ast.h"
#include <stdlib.h>
#include <string.h>

ASTNode *create_ast_variable_def(char *name, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE_DEF;
    node->variable_def.name = strdup(name);
    node->variable_def.body = body;
    return node;
}

ASTNode *create_ast_variable(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->variable.name = strdup(name);
    return node;
}

ASTNode *create_ast_function_def(char *name, ASTNode *body, char **args, unsigned int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_DEF;
    node->function_def.name = strdup(name);
    node->function_def.body = body;

    // XXX shallow copy
    node->function_def.args = malloc(sizeof(ASTNode) * arg_count);
    memcpy(node->function_def.args, args, sizeof(ASTNode) * arg_count);
    node->function_def.arg_count = arg_count;
    return node;
}

ASTNode *create_ast_function_call(char *name, ASTNode **args, unsigned int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_CALL;
    node->function_call.name = strdup(name);

    // shallow copy
    node->function_call.args = malloc(sizeof(ASTNode) * arg_count);
    memcpy(node->function_call.args, args, sizeof(ASTNode) * arg_count);
    node->function_call.arg_count = arg_count;

    return node;
}


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
        switch (node->type) {
            case AST_BINARY_OP:
                free_ast(node->binary_op.left);
                free_ast(node->binary_op.right);
                break;
            case AST_FUNCTION_DEF:
                free(node->function_def.name);
                free_ast(node->function_def.body);
                break;
            case AST_FUNCTION_CALL:
                free(node->function_call.name);
                break;
            default:
                break;
        }
        free(node);
    }
}
