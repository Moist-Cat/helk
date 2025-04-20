#include "semantic.h"
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static ASTNode* create_main_function(ASTNode** statements, unsigned int count) {
    ASTNode* main_block = malloc(sizeof(ASTNode));
    main_block->type = AST_BLOCK;
    main_block->block.statements = statements;
    main_block->block.stmt_count = count;

    ASTNode* main_func = malloc(sizeof(ASTNode));
    main_func->type = AST_FUNCTION_DEF;
    main_func->function_def.name = strdup("main");
    main_func->function_def.body = main_block;
    // explicit
    main_func->function_def.args = NULL;
    main_func->function_def.arg_count = 0;
    
    return main_func;
}

void sa_block(ASTNode *node) {
    ASTNode **func_defs = NULL;
    ASTNode **main_body = NULL;
    unsigned int func_count = 0, main_count = 0;

    // Separate statements into function definitions and others
    for (unsigned int i = 0; i < node->block.stmt_count; i++) {
        ASTNode *stmt = node->block.statements[i];
        if (stmt->type == AST_FUNCTION_DEF) {
            func_defs = realloc(func_defs, (func_count + 1) * sizeof(ASTNode*));
            func_defs[func_count++] = stmt;
        } else {
            main_body = realloc(main_body, (main_count + 1) * sizeof(ASTNode*));
            main_body[main_count++] = stmt;
        }
    }

    // Create new statements array
    // always add a main function
    unsigned int new_count = func_count + 1;
    ASTNode **new_statements = malloc(new_count * sizeof(ASTNode*));

    // Copy function definitions
    if (func_count > 0) {
        memcpy(new_statements, func_defs, func_count * sizeof(ASTNode*));
    }

    // Add main function (always)
    fprintf(stderr, "\n\nnoger %d\n\n", main_count);
    ASTNode* main_func = create_main_function(main_body, main_count);
    new_statements[func_count] = main_func;

    // Replace original block contents
    free(node->block.statements);
    node->block.statements = new_statements;
    node->block.stmt_count = new_count;

    // Free temporary arrays (not the nodes!)
    free(func_defs);
    // we literally just give the pointer to the main
    // function so we can't free it
    //free(main_body);
}

void semantic_analysis(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_BLOCK:
            fprintf(stderr, "INFO - Performing sem_anal into code block\n");
            sa_block(node);
            // Recurse into new structure
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                semantic_analysis(node->block.statements[i]);
            }
            break;
            
        // XXX stack overflow since the main function has blocks
        //case AST_FUNCTION_DEF:
        //    fprintf(stderr, "INFO - Performing sem_anal into %s\n", node->function_def.name);
        //    semantic_analysis(node->function_def.body);
        //    break;
            
        case AST_BINARY_OP:
            semantic_analysis(node->binary_op.left);
            semantic_analysis(node->binary_op.right);
            break;
            
        // Add other cases as needed
        default:
            break;
    }
}
