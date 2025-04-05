#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>

void semantic_analysis(ASTNode *node) {
    // Perform semantic checks here
    // Example: Check for division by zero
    if (node->type == AST_BINARY_OP && node->binary_op.op == OP_DIV) {
        if (node->binary_op.right->type == AST_NUMBER && node->binary_op.right->number == 0) {
            fprintf(stderr, "Semantic Error: Division by zero\n");
            exit(1);
        }
    }
}
