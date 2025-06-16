#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

ASTNode *create_ast_block(ASTNode **block, unsigned int stmt_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK;

    // shallow copy again again
    if (block != NULL) {
        node->block.statements = malloc(sizeof(ASTNode*) * stmt_count);
        memcpy(node->block.statements, block, sizeof(ASTNode*) * stmt_count);
    }
    else {
        node->block.statements = NULL;
    }

    node->block.stmt_count = stmt_count;

    node->type_info.kind = TYPE_UNKNOWN;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_param_list(char **params, unsigned int count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    // not used
    node->type = 42;

    // shallow copy again again again
    node->param_list.params = malloc(sizeof(char*) * count);
    memcpy(node->param_list.params, params, sizeof(char*) * count);

    node->param_list.count = count;

    node->type_info.kind = TYPE_UNKNOWN;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode* create_ast_variable_list(char **names, ASTNode **values, unsigned int count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    // not used
    node->type = 69;

    // shallow copy again again again
    node->variable_list.names = malloc(sizeof(char*) * count);
    memcpy(node->variable_list.names, names, sizeof(char*) * count);

    if (values != NULL) {
        node->variable_list.values = malloc(sizeof(ASTNode*) * count);
        memcpy(node->variable_list.values, values, sizeof(ASTNode*) * count);
    }
    else {
        node->variable_list.values = NULL;
    }

    node->variable_list.count = count;

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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
        else if (members[i]->type == AST_METHOD_DEF) method_count++;
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
        } else if (members[i]->type == AST_METHOD_DEF) {
            node->type_decl.methods[m_idx++] = members[i];
        }
    }

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

ASTNode *create_ast_variable(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->variable.name = strdup(name);

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->function_def.called = false;

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;



    return node;
}


ASTNode *create_ast_number(double value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->number = value;

    node->type_info.kind = TYPE_UNKNOWN;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;


    return node;
}

ASTNode *create_ast_string(char* ptr) {
    ASTNode *node = malloc(sizeof(ASTNode));

    node->type = AST_STRING;
    node->string = strdup(ptr);

    node->type_info.kind = TYPE_UNKNOWN;
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

    node->type_info.kind = TYPE_UNKNOWN;
    node->type_info.name = NULL;
    node->type_info.cls = NULL;
    node->type_info.is_literal = false;

    return node;
}

void free_ast(ASTNode *node) {
    if (node == NULL) {
        return;
    }

    // Free type_info strings
    free(node->type_info.name);
    free(node->type_info.cls);

    switch (node->type) {
        case AST_NUMBER:
            // No additional dynamic data
            break;
            
        case AST_STRING:
            free(node->string);
            break;
            
        case AST_VARIABLE:
            free(node->variable.name);
            break;
            
        case AST_BINARY_OP:
            free_ast(node->binary_op.left);
            free_ast(node->binary_op.right);
            break;
            
        case AST_FUNCTION_DEF:
        case AST_METHOD_DEF:
            free(node->function_def.name);
            // Free args array (container only, strings owned by arg nodes)
            free(node->function_def.args);
            // Free argument definition nodes
            for (unsigned int i = 0; i < node->function_def.arg_count; i++) {
                free_ast(node->function_def.args_definitions[i]);
            }
            free(node->function_def.args_definitions);
            free_ast(node->function_def.body);
            break;
            
        case AST_FUNCTION_CALL:
            free(node->function_call.name);
            for (unsigned int i = 0; i < node->function_call.arg_count; i++) {
                free_ast(node->function_call.args[i]);
            }
            free(node->function_call.args);
            break;
            
        case AST_VARIABLE_DEF:
            free(node->variable_def.name);
            free_ast(node->variable_def.body);
            break;
            
        case AST_BLOCK:
            for (unsigned int i = 0; i < node->block.stmt_count; i++) {
                free_ast(node->block.statements[i]);
            }
            free(node->block.statements);
            break;
            
        case AST_LET_IN:
            for (unsigned int i = 0; i < node->let_in.var_count; i++) {
                free(node->let_in.var_names[i]);
                free_ast(node->let_in.var_values[i]);
            }
            free(node->let_in.var_names);
            free(node->let_in.var_values);
            free_ast(node->let_in.body);
            break;
            
        case AST_CONDITIONAL:
            free_ast(node->conditional.hypothesis);
            free_ast(node->conditional.thesis);
            free_ast(node->conditional.antithesis);
            break;
            
        case AST_WHILE_LOOP:
            free_ast(node->while_loop.cond);
            free_ast(node->while_loop.body);
            break;
            
        case AST_TYPE_DEF:
            free(node->type_decl.name);
            free(node->type_decl.base_type);
            for (unsigned int i = 0; i < node->type_decl.field_count; i++) {
                free_ast(node->type_decl.fields[i]);
            }
            free(node->type_decl.fields);
            for (unsigned int i = 0; i < node->type_decl.method_count; i++) {
                free_ast(node->type_decl.methods[i]);
            }
            free(node->type_decl.methods);
            break;
            
        case AST_FIELD_DEF:
            free(node->field_def.name);
            free_ast(node->field_def.default_value);
            break;
            
        case AST_CONSTRUCTOR:
            free(node->constructor.cls);
            for (unsigned int i = 0; i < node->constructor.arg_count; i++) {
                free_ast(node->constructor.args[i]);
            }
            free(node->constructor.args);
            break;
            
        case AST_FIELD_ACCESS:
            free(node->field_access.cls);
            free(node->field_access.field);
            break;
            
        case AST_METHOD_CALL:
            free_ast(node->method_call.cls);
            free(node->method_call.method);
            for (unsigned int i = 0; i < node->method_call.arg_count; i++) {
                free_ast(node->method_call.args[i]);
            }
            free(node->method_call.args);
            break;
            
        default:
            // Unknown node type - no special handling
            break;
    }
    
    free(node);
}

static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        fprintf(stderr, "  ");
    }
}

void ast_print_node(const ASTNode *node, int indent) {
    if (node == NULL) {
        print_indent(indent);
        fprintf(stderr, "NULL\n");
        return;
    }

    switch (node->type) {
        case AST_NUMBER:
            print_indent(indent);
            fprintf(stderr, "NUMBER: %f\n", node->number);
            break;
        case AST_STRING:
            print_indent(indent);
            fprintf(stderr, "STRING: \"%s\"\n", node->string ? node->string : "NULL");
            break;
        case AST_VARIABLE:
            print_indent(indent);
            fprintf(stderr, "VARIABLE: %s\n", node->variable.name ? node->variable.name : "NULL");
            break;
        case AST_BINARY_OP: {
            const char *op_str = "?";
            switch (node->binary_op.op) {
                case OP_ADD: op_str = "+"; break;
                case OP_SUB: op_str = "-"; break;
                case OP_MUL: op_str = "*"; break;
                case OP_DIV: op_str = "/"; break;
                case OP_EXP: op_str = "**"; break;
                case OP_MOD: op_str = "%"; break;
            }
            print_indent(indent);
            fprintf(stderr, "BINARY_OP: %s\n", op_str);
            ast_print_node(node->binary_op.left, indent + 1);
            ast_print_node(node->binary_op.right, indent + 1);
            break;
        }
        case AST_FUNCTION_DEF: {
            print_indent(indent);
            fprintf(stderr, "FUNCTION_DEF: %s\n", node->function_def.name ? node->function_def.name : "NULL");
            print_indent(indent);
            fprintf(stderr, "  ARGUMENTS (%u):\n", node->function_def.arg_count);
            for (size_t i = 0; i < node->function_def.arg_count; i++) {
                if (node->function_def.args_definitions && node->function_def.args_definitions[i]) {
                    ast_print_node(node->function_def.args_definitions[i], indent + 2);
                } else {
                    print_indent(indent + 2);
                    fprintf(stderr, "NULL\n");
                }
            }
            print_indent(indent);
            fprintf(stderr, "  BODY:\n");
            ast_print_node(node->function_def.body, indent + 2);
            break;
        }
        case AST_FUNCTION_CALL: {
            print_indent(indent);
            fprintf(stderr, "FUNCTION_CALL: %s (%u args)\n", node->function_call.name ? node->function_call.name : "NULL", node->function_call.arg_count);
            for (size_t i = 0; i < node->function_call.arg_count; i++) {
                if (node->function_call.args && node->function_call.args[i]) {
                    ast_print_node(node->function_call.args[i], indent + 1);
                } else {
                    print_indent(indent + 1);
                    fprintf(stderr, "NULL\n");
                }
            }
            break;
        }
        case AST_VARIABLE_DEF: {
            print_indent(indent);
            fprintf(stderr, "VARIABLE_DEF: %s\n", node->variable_def.name ? node->variable_def.name : "NULL");
            ast_print_node(node->variable_def.body, indent + 1);
            break;
        }
        case AST_BLOCK: {
            print_indent(indent);
            fprintf(stderr, "BLOCK: %u statements\n", node->block.stmt_count);
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                if (node->block.statements && node->block.statements[i]) {
                    ast_print_node(node->block.statements[i], indent + 1);
                } else {
                    print_indent(indent + 1);
                    fprintf(stderr, "NULL\n");
                }
            }
            break;
        }
        case AST_LET_IN: {
            print_indent(indent);
            fprintf(stderr, "LET_IN: %u variables\n", node->let_in.var_count);
            for (size_t i = 0; i < node->let_in.var_count; i++) {
                print_indent(indent + 1);
                fprintf(stderr, "VAR: %s\n", node->let_in.var_names[i] ? node->let_in.var_names[i] : "NULL");
                ast_print_node(node->let_in.var_values[i], indent + 2);
            }
            print_indent(indent);
            fprintf(stderr, "IN:\n");
            ast_print_node(node->let_in.body, indent + 1);
            break;
        }
        case AST_CONDITIONAL: {
            print_indent(indent);
            fprintf(stderr, "CONDITIONAL:\n");
            print_indent(indent);
            fprintf(stderr, "  HYPOTHESIS:\n");
            ast_print_node(node->conditional.hypothesis, indent + 2);
            print_indent(indent);
            fprintf(stderr, "  THESIS:\n");
            ast_print_node(node->conditional.thesis, indent + 2);
            print_indent(indent);
            fprintf(stderr, "  ANTITHESIS:\n");
            ast_print_node(node->conditional.antithesis, indent + 2);
            break;
        }
        case AST_WHILE_LOOP: {
            print_indent(indent);
            fprintf(stderr, "WHILE_LOOP:\n");
            print_indent(indent);
            fprintf(stderr, "  CONDITION:\n");
            ast_print_node(node->while_loop.cond, indent + 2);
            print_indent(indent);
            fprintf(stderr, "  BODY:\n");
            ast_print_node(node->while_loop.body, indent + 2);
            break;
        }
        case AST_TYPE_DEF: {
            print_indent(indent);
            fprintf(stderr, "TYPE_DEF: %s\n", node->type_decl.name ? node->type_decl.name : "NULL");
            if (node->type_decl.base_type) {
                print_indent(indent);
                fprintf(stderr, "  BASE: %s\n", node->type_decl.base_type);
            }
            print_indent(indent);
            fprintf(stderr, "  FIELDS: %u\n", node->type_decl.field_count);
            for (size_t i = 0; i < node->type_decl.field_count; i++) {
                if (node->type_decl.fields && node->type_decl.fields[i]) {
                    ast_print_node(node->type_decl.fields[i], indent + 2);
                } else {
                    print_indent(indent + 2);
                    fprintf(stderr, "NULL\n");
                }
            }
            print_indent(indent);
            fprintf(stderr, "  METHODS: %u\n", node->type_decl.method_count);
            for (size_t i = 0; i < node->type_decl.method_count; i++) {
                if (node->type_decl.methods && node->type_decl.methods[i]) {
                    ast_print_node(node->type_decl.methods[i], indent + 2);
                } else {
                    print_indent(indent + 2);
                    fprintf(stderr, "NULL\n");
                }
            }
            break;
        }
        case AST_FIELD_DEF: {
            print_indent(indent);
            fprintf(stderr, "FIELD_DEF: %s\n", node->field_def.name ? node->field_def.name : "NULL");
            if (node->field_def.default_value) {
                print_indent(indent);
                fprintf(stderr, "  DEFAULT:\n");
                ast_print_node(node->field_def.default_value, indent + 2);
            }
            break;
        }
        case AST_CONSTRUCTOR: {
            print_indent(indent);
            fprintf(stderr, "CONSTRUCTOR: %s (%u args)\n", node->constructor.cls ? node->constructor.cls : "NULL", node->constructor.arg_count);
            for (size_t i = 0; i < node->constructor.arg_count; i++) {
                if (node->constructor.args && node->constructor.args[i]) {
                    ast_print_node(node->constructor.args[i], indent + 1);
                } else {
                    print_indent(indent + 1);
                    fprintf(stderr, "NULL\n");
                }
            }
            break;
        }
        case AST_FIELD_ACCESS: {
            print_indent(indent);
            fprintf(stderr, "FIELD_ACCESS: %s.%s\n", node->field_access.cls ? node->field_access.cls : "NULL", node->field_access.field ? node->field_access.field : "NULL");
            break;
        }
        case AST_METHOD_CALL: {
            print_indent(indent);
            fprintf(stderr, "METHOD_CALL: %s.%s (%u args)\n",
                    node->method_call.cls->variable.name,
                    node->method_call.method ? node->method_call.method : "NULL",
                    node->method_call.arg_count);
            print_indent(indent);
            fprintf(stderr, "  OBJECT:\n");
            ast_print_node(node->method_call.cls, indent + 2);
            for (size_t i = 0; i < node->method_call.arg_count; i++) {
                if (node->method_call.args && node->method_call.args[i]) {
                    ast_print_node(node->method_call.args[i], indent + 1);
                } else {
                    print_indent(indent + 1);
                    fprintf(stderr, "NULL\n");
                }
            }
            break;
        }
        case AST_METHOD_DEF:
            print_indent(indent);
            fprintf(stderr, "METHOD: %s\n", node->function_def.name ? node->function_def.name : "NULL");
            print_indent(indent);
            fprintf(stderr, "  ARGUMENTS (%u):\n", node->function_def.arg_count);
            for (size_t i = 0; i < node->function_def.arg_count; i++) {
                if (node->function_def.args_definitions && node->function_def.args_definitions[i]) {
                    ast_print_node(node->function_def.args_definitions[i], indent + 2);
                } else {
                    print_indent(indent + 2);
                    fprintf(stderr, "NULL\n");
                }
            }
            print_indent(indent);
            fprintf(stderr, "  BODY:\n");
            ast_print_node(node->function_def.body, indent + 2);
            break;
        default:
            print_indent(indent);
            fprintf(stderr, "UNKNOWN_NODE_TYPE: %d\n", node->type);
    }
}
