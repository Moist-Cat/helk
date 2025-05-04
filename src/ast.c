#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ASTNode *create_ast_block(ASTNode **block, unsigned int stmt_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;

    // shallow copy again again
    node->block.statements = malloc(sizeof(ASTNode) * stmt_count);
    memcpy(node->block.statements, block, sizeof(ASTNode) * stmt_count);

    node->block.stmt_count = stmt_count;

    return node;
}

ASTNode* create_ast_let_in(char **names, ASTNode **values, unsigned int count, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_LET_IN;

    // shallow copy again again again
    node->let_in.var_names = malloc(sizeof(char*) * count);
    memcpy(node->let_in.var_names, names, sizeof(char*) * count);

    node->let_in.var_values = malloc(sizeof(ASTNode) * count);
    memcpy(node->let_in.var_values, values, sizeof(ASTNode) * count);

    node->let_in.var_count = count;
    node->let_in.body = body;
    return node;
}


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

    // shallow copy again
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


ASTNode *create_ast_number(double value) {
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
                free(node->function_def.args);
                break;
            case AST_FUNCTION_CALL:
                free(node->function_call.name);
                free(node->function_call.args);
                break;
            case AST_VARIABLE_DEF:
                free(node->variable_def.name);
                free_ast(node->variable_def.body);
            case AST_VARIABLE:
                // XXX memory leak, we can't free the variable
                // 
                //free(node->variable.name);
                break;
            case AST_BLOCK:
                for (size_t i = 0; i < node->block.stmt_count; i++) {
                    free_ast(node->block.statements[i]);
                }
                free(node->block.statements);
                break;
            default:
                break;
        }
        free(node);
    }
}
