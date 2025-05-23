#include "semantic.h"
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    ASTNode** stmts;
    unsigned int stmt_count;
    ASTNode* expr;
} FlattenResult;

typedef struct {
    TypeInfo* expected;
    ASTNode* node;  // For error reporting
} TypeConstraint;

typedef struct {
    TypeConstraint* constraints;
    size_t count;
    size_t capacity;
} ConstraintSystem;

// type inference
bool solve_constraints(ConstraintSystem* cs) {
    fprintf(stderr, "INFO - Solving constraints...\n");
    bool changed;
    do {
        changed = false;
        for(size_t i=0; i<cs->count; i++) {
            TypeConstraint* c = &cs->constraints[i];
            TypeInfo* t = c->expected;
            int expected = t->kind;
            int actual = c->node->type_info.kind;

            fprintf(stderr, "Expected: %d ; Actual: %d\n\n", expected, actual);

            // Handle literals first
            if(c->node->type_info.is_literal) {
                if(expected != TYPE_UNKNOWN &&
                   expected != actual) {
                    fprintf(stderr, "ERROR - Literal type mismatch\n");
                    return false;
                }
                continue;
            }

            // Propagate concrete -> unknown
            if(expected != TYPE_UNKNOWN &&
               actual == TYPE_UNKNOWN) {
                c->node->type_info.kind = ((TypeInfo*) (c->expected))->kind;

                fprintf(stderr, "INFO - Solved type for node type=%d; to %d\n", c->node->type, c->node->type_info.kind);
                changed = true;
            }

            // Check for conflicts
            if(expected != TYPE_UNKNOWN &&
               actual != TYPE_UNKNOWN &&
               expected != actual) {
                fprintf(stderr, "ERROR - Literal type mismatch\n");
                return false;
            }
        }
    } while(changed);

    return true;
}

void add_constraint(ConstraintSystem* cs, ASTNode* node, 
                   TypeInfo* expected) {
    if (cs->count >= cs->capacity) {
        cs->capacity = cs->capacity ? cs->capacity * 2 : 16;
        cs->constraints = realloc(cs->constraints, 
                                cs->capacity * sizeof(TypeConstraint));
    }
    
    cs->constraints[cs->count++] = (TypeConstraint){
        .expected = expected,
        .node = node
    };
}


void process_node(ASTNode* node, ConstraintSystem* cs) {
    switch(node->type) {
        case AST_BINARY_OP: {
            // Both operands must be numeric

            TypeInfo *lit = malloc(sizeof(TypeInfo));
            lit->kind = TYPE_DOUBLE;
            lit->is_literal = true;

            add_constraint(cs, node->binary_op.left, lit);
            add_constraint(cs, node->binary_op.right, lit);
            // Result is float
            add_constraint(cs, node, lit);
            break;
        }

        case AST_CONDITIONAL: {
            /*
            add_constraint(cs, node,
                node->conditional.thesis->type_info);

            // Hypothesis must be numeric (treated as float)
            add_constraint(cs, node->conditional.hypothesis,
                (TypeInfo){TYPE_DOUBLE, false});
            // Branches must match
            add_constraint(cs, node->conditional.thesis,
                node->conditional.antithesis->type_info);
            add_constraint(cs, node->conditional.antithesis,
                node->conditional.thesis->type_info);
            */
            break;
        }

        case AST_BLOCK: {
            fprintf(stderr, "INFO - Found block (size=%d) during constraint collection\n", node->block.stmt_count);

            // depends on the type of the last statement
            if (node->block.stmt_count > 0) {
                add_constraint(cs, node,
                    &node->block.statements[node->block.stmt_count - 1]->type_info);
            }
            else {
                TypeInfo *lit = malloc(sizeof(TypeInfo));
                lit->kind = TYPE_DOUBLE;
                lit->is_literal = true;

                // default to float
                add_constraint(cs, node, lit);
            }
            break;
        }
        case AST_FUNCTION_CALL: {
            TypeInfo *lit = malloc(sizeof(TypeInfo));
            lit->kind = TYPE_DOUBLE;
            lit->is_literal = true;

            add_constraint(cs, node, lit);
            break;
        }
        case AST_FUNCTION_DEF: {
            fprintf(stderr, "INFO - Found function (name=%s) during constraint collection\n", node->function_def.name);

            add_constraint(cs, node,
                &node->function_def.body->type_info);
            break;
        }
        case AST_VARIABLE: {break;}
        case AST_VARIABLE_DEF: {break;}
        case AST_LET_IN: {break;}
        case AST_WHILE_LOOP: {break;}

        case AST_NUMBER: {
            // Literals are terminal - no constraints
            fprintf(stderr, "INFO - Found terminal %f during constraint collection\n", node->number);

            // NOOB NOTE: If we don't malloc the memory is used by something else eventually
            TypeInfo *lit = malloc(sizeof(TypeInfo));
            lit->kind = TYPE_DOUBLE;
            lit->is_literal = true;

            add_constraint(cs, node, lit);
            break;
        }

        case AST_STRING: {
            // Literals are terminal - no constraints
            fprintf(stderr, "INFO - Found terminal '%s' during constraint collection\n", node->string);

            // NOOB NOTE: If we don't malloc the memory is used by something else eventually
            TypeInfo *lit = malloc(sizeof(TypeInfo));
            lit->kind = TYPE_STRING;
            lit->is_literal = true;

            add_constraint(cs, node, lit);
            break;
        }

        // Handle other node types
    }
}

// FLAT
static void append_stmts(FlattenResult* dest, ASTNode** src, unsigned int count) {
    dest->stmts = realloc(dest->stmts, (dest->stmt_count + count) * sizeof(ASTNode*));
    memcpy(dest->stmts + dest->stmt_count, src, count * sizeof(ASTNode*));
    dest->stmt_count += count;
}

static FlattenResult flatten(ASTNode* node) {
    /*
     * FLATten the AST recursively.
     *
     */
    FlattenResult res = {0};

    switch (node->type) {
        case AST_LET_IN: {
            // Process variables
            for (size_t i = 0; i < node->let_in.var_count; i++) {
                FlattenResult val = flatten(node->let_in.var_values[i]);
                append_stmts(&res, val.stmts, val.stmt_count);

                // Create variable definition
                ASTNode* def = create_ast_variable_def(
                    node->let_in.var_names[i],
                    val.expr
                );
                res.stmts = realloc(res.stmts, (res.stmt_count + 1) * sizeof(ASTNode*));
                res.stmts[res.stmt_count++] = def;
            }

            // Process body
            FlattenResult body = flatten(node->let_in.body);
            append_stmts(&res, body.stmts, body.stmt_count);
            res.expr = body.expr;
            break;
        }

        case AST_BINARY_OP: {
            FlattenResult left = flatten(node->binary_op.left);
            FlattenResult right = flatten(node->binary_op.right);
            append_stmts(&res, left.stmts, left.stmt_count);
            append_stmts(&res, right.stmts, right.stmt_count);
            res.expr = create_ast_binary_op(left.expr, right.expr, node->binary_op.op);
            break;
        }
        case AST_FUNCTION_DEF: {
            flatten(node->function_def.body);
        }

        default:
            res.expr = node;
            break;
    }

    return res;
}

ASTNode* transform_ast(ASTNode* node) {
    fprintf(stderr, "Node type=%d\n", node->type);
    if (!node) return NULL;

    // First transform children recursively
    switch (node->type) {
        case AST_BLOCK: {
            fprintf(stderr, "Transforming AST_BLOCK\n");
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                node->block.statements[i] = transform_ast(node->block.statements[i]);
            }                      
            break;
        }
        case AST_FUNCTION_DEF: {
            fprintf(stderr, "Transforming AST_FUNCTION_DEF\n");
            node->function_def.body = transform_ast(node->function_def.body);
            break;
        }
        case AST_LET_IN: {
            fprintf(stderr, "Transforming AST_LET_IN\n");
            FlattenResult washboard = flatten(node);

            ASTNode* new_block = malloc(sizeof(ASTNode));
            new_block->type = AST_BLOCK;
            new_block->block.statements = washboard.stmts;
            new_block->block.stmt_count = washboard.stmt_count;

            // the result of the evaluation of an expression block is the last expression evaluated
            new_block->block.statements = realloc(new_block->block.statements, (new_block->block.stmt_count + 1) * sizeof(ASTNode*));
            new_block->block.statements[new_block->block.stmt_count++] = washboard.expr;

            // free the *struct* holding the data
            // ... memory leak?
            free(node);

            node = new_block;
            break;
        }

        default:
            break;

        // XXX Handle other node types similarly
    }

    // Then apply transformation to current node
    return node;
}

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

void _semantic_analysis(ASTNode *node, ConstraintSystem* cs) {
    /* Anything read-only */
    // at this point, let-ins do not exist
    if (!node) return;

    process_node(node, cs);

    switch (node->type) {
        case AST_BLOCK: {
            fprintf(stderr, "INFO - Performing sem_anal into code block\n");
            // Recurse into new structure
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                _semantic_analysis(node->block.statements[i], cs);
            }
            break;
        }
        case AST_FUNCTION_DEF: {
            fprintf(stderr, "INFO - Performing sem_anal into %s\n", node->function_def.name);
            _semantic_analysis(node->function_def.body, cs);
            break;
        }
        case AST_BINARY_OP: {
            _semantic_analysis(node->binary_op.left, cs);
            _semantic_analysis(node->binary_op.right, cs);
            break;
        }
        default: {
            break;
        }
    }
}

bool semantic_analysis(ASTNode *node) {
    switch (node->type) {
        // the only case
        case AST_BLOCK: {
            sa_block(node); // reorganize code in functions (transform the parent)
            node = transform_ast(node); // embrace FLATness (transform the children)
            ConstraintSystem cs = {NULL, 0, 0};
            _semantic_analysis(node, &cs); // type checking (read-only)
                                           //
            bool res = solve_constraints(&cs);
            return res;
        }
        default: {
            fprintf(stderr, "FATAL - Could not recognize root node (%d, it's not a block)\n", node->type);
            return false;
        }
    }
    return true;
}
