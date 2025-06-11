#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

ASTNode *create_ast_block(ASTNode **block, unsigned int stmt_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;

    // shallow copy again again
    node->block.statements = malloc(sizeof(ASTNode*) * stmt_count);
    memcpy(node->block.statements, block, sizeof(ASTNode*) * stmt_count);

    node->block.stmt_count = stmt_count;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode* create_ast_let_in(char **names, ASTNode **values, unsigned int count, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_LET_IN;

    // shallow copy again again again
    node->let_in.var_names = malloc(sizeof(char*) * count);
    memcpy(node->let_in.var_names, names, sizeof(char*) * count);

    node->let_in.var_values = malloc(sizeof(ASTNode*) * count);
    memcpy(node->let_in.var_values, values, sizeof(ASTNode*) * count);

    node->let_in.var_count = count;
    node->let_in.body = body;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}


ASTNode* create_ast_while_loop(ASTNode* cond, ASTNode* body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_LOOP;

    node->while_loop.cond = cond;
    node->while_loop.body = body;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode* create_ast_conditional(ASTNode* hypothesis, ASTNode* thesis, ASTNode* antithesis) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_CONDITIONAL;

    // we are not leaking memory
    // and if we are, then it's not that much
    // if it is, then it's not relevant for our use-case
    node->conditional.hypothesis = hypothesis;
    node->conditional.thesis = thesis;
    node->conditional.antithesis = antithesis;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

// Create field definition
ASTNode* create_ast_field_def(char* name, ASTNode* default_value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_FIELD_DEF;
    node->field_def.name = strdup(name);
    node->field_def.default_value = default_value;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

// Create type definition
ASTNode* create_ast_type_def(char* name, char* base_type,
                            ASTNode** members, unsigned int member_count) {
    // Separate fields and methods
    size_t field_count = 0;
    size_t method_count = 0;

    for (size_t i = 0; i < member_count; i++) {
        if (members[i]->type == AST_FIELD_DEF) field_count++;
        else if (members[i]->type == AST_FUNCTION_DEF) method_count++;
    }

    // Allocate type definition
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_TYPE_DEF;
    node->type_decl.name = strdup(name);
    node->type_decl.base_type = base_type ? strdup(base_type) : NULL;
    node->type_decl.fields = malloc(sizeof(ASTNode*) * field_count);
    node->type_decl.field_count = field_count;
    node->type_decl.methods = malloc(sizeof(ASTNode*) * method_count);
    node->type_decl.method_count = method_count;

    // Populate fields and methods
    size_t f_idx = 0, m_idx = 0;
    for (size_t i = 0; i < member_count; i++) {
        if (members[i]->type == AST_FIELD_DEF) {
            node->type_decl.fields[f_idx++] = members[i];
        } else if (members[i]->type == AST_FUNCTION_DEF) {
            node->type_decl.methods[m_idx++] = members[i];
        }
    }

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_constructor(char* cls, ASTNode **args, unsigned int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_CONSTRUCTOR;
    node->constructor.cls = strdup(cls);

    // shallow copy again
    node->constructor.args = malloc(sizeof(ASTNode*) * arg_count);
    memcpy(node->constructor.args, args, sizeof(ASTNode*) * arg_count);
    node->constructor.arg_count = arg_count;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_field_access(char* cls, char* field) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FIELD_ACCESS;
    node->field_access.cls = strdup(cls);
    node->field_access.field = strdup(field);

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_method_call(ASTNode* cls, char* method, ASTNode **args, unsigned int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_METHOD_CALL;

    node->method_call.cls = cls;
    node->method_call.method = strdup(method);

    if (arg_count != 0) {
        node->method_call.args = malloc(sizeof(ASTNode*) * arg_count);
        memcpy(node->method_call.args, args, sizeof(ASTNode*) * arg_count);
    }
    else {
        node->method_call.args = NULL;
    }
    node->method_call.arg_count = arg_count;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_variable_def(char *name, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE_DEF;
    node->variable_def.name = strdup(name);
    node->variable_def.body = body;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_variable(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->variable.name = strdup(name);

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_function_def(char *name, ASTNode *body, char **args, unsigned int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_DEF;
    node->function_def.name = strdup(name);
    node->function_def.body = body;

    // shallow copy again
    node->function_def.args_definitions = malloc(sizeof(ASTNode*) * arg_count);
    node->function_def.args = malloc(sizeof(ASTNode*) * arg_count);
    memcpy(node->function_def.args, args, sizeof(ASTNode*) * arg_count);
    node->function_def.arg_count = arg_count;

    for (unsigned int i = 0; i < arg_count; i++) {
        node->function_def.args_definitions[i] = create_ast_variable_def(args[i], NULL);
    }

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_function_call(char *name, ASTNode **args, unsigned int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_CALL;
    node->function_call.name = strdup(name);

    // shallow copy
    node->function_call.args = malloc(sizeof(ASTNode*) * arg_count);
    memcpy(node->function_call.args, args, sizeof(ASTNode*) * arg_count);
    node->function_call.arg_count = arg_count;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;



    return node;
}


ASTNode *create_ast_number(double value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->number = value;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;


    return node;
}

ASTNode *create_ast_string(char* ptr) {
    ASTNode *node = malloc(sizeof(ASTNode));

    node->type = AST_STRING;
    node->string = strdup(ptr);

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_binary_op(ASTNode *left, ASTNode *right, ASTBinaryOp op) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->binary_op.left = left;
    node->binary_op.right = right;
    node->binary_op.op = op;

    node->type_info.kind = 0;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

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
