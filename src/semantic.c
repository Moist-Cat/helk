#include "semantic.h"
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

size_t hash(const char* s) {
    size_t h = 5381;
    while(*s) h = ((h << 5) + h) + *s++;
    return h;
}

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


            fprintf(stderr, "Expected: %d ; Actual: %d ; Node: %d\n\n", expected, actual, c->node->type);

            if (c->node->type == AST_FUNCTION_CALL) {
                fprintf(stderr, "For function %s\n", c->node->function_call.name);
            }
            else if (c->node->type == AST_VARIABLE_DEF) {
                fprintf(stderr, "For variable %s\n", c->node->variable_def.name);
            }

            // Propagate concrete -> unknown
            if(expected != TYPE_UNKNOWN &&
               actual == TYPE_UNKNOWN) {
                c->node->type_info.kind = ((TypeInfo*) (c->expected))->kind;
                c->node->type_info.name = ((TypeInfo*) (c->expected))->name;
                c->node->type_info.cls = ((TypeInfo*) (c->expected))->cls;

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


SymbolTable* create_symbol_table(SymbolTable* parent) {
    SymbolTable* st = malloc(sizeof(SymbolTable));
    st->entries = calloc(16, sizeof(SymbolEntry*));
    st->size = 16;
    st->parent = parent;

    if (parent == NULL) {
        // init builtins
        char **pargs = malloc(sizeof(char*)*1);
        pargs[0] = "[print_param_1]";
        ASTNode* print = create_ast_function_def("print", NULL, pargs, 1);
        print->type_info.kind = TYPE_DOUBLE;
        symbol_table_add(st, "print", print);

        char **psargs = malloc(sizeof(char*)*1);
        psargs[0] = "[prints_param_1]";
        ASTNode* prints = create_ast_function_def("prints", NULL, psargs, 1);
        prints->type_info.kind = TYPE_DOUBLE;
        symbol_table_add(st, "prints", prints);

        char **maxargs = malloc(sizeof(char*)*2);
        maxargs[0] = "[max_param_1]";
        maxargs[1] = "[max_param_2]";
        ASTNode* maxima = create_ast_function_def("max", NULL, maxargs, 2);
        maxima->type_info.kind = TYPE_DOUBLE;
        symbol_table_add(st, "max", maxima);

        char **minargs = malloc(sizeof(char*)*2);
        psargs[0] = "[min_param_1]";
        psargs[1] = "[min_param_2]";
        ASTNode* minima = create_ast_function_def("min", NULL, minargs, 2);
        minima->type_info.kind = TYPE_DOUBLE;
        symbol_table_add(st, "min", minima);

        char **pow_args = malloc(sizeof(char*)*2);
        pow_args[0] = "[pow_param_1]";
        pow_args[1] = "[pow_param_2]";
        ASTNode* power = create_ast_function_def("pow", NULL, pow_args, 2);
        power->type_info.kind = TYPE_DOUBLE;
        symbol_table_add(st, "pow", power);
    }

    return st;
}

void symbol_table_add(SymbolTable* st, const char* name, ASTNode* node) {
    size_t idx = hash(name) % st->size;
    SymbolEntry* entry = malloc(sizeof(SymbolEntry));
    entry->name = strdup(name);
    entry->node = node;
    entry->next = st->entries[idx];
    st->entries[idx] = entry;
}

ASTNode* symbol_table_lookup(SymbolTable* st, const char* name) {
    for(SymbolTable* curr = st; curr; curr = curr->parent) {
        size_t idx = hash(name) % curr->size;
        for(SymbolEntry* e = curr->entries[idx]; e; e = e->next) {
            if(strcmp(e->name, name) == 0) {
                fprintf(stderr, "INFO - Found symbol %s\n", name);
                return e->node;
            }
        }
    }
    return NULL;
}

void process_function_call(
    ASTNode* call,
    ConstraintSystem* cs,
    SymbolTable* current_scope
) {
    // 1. Lookup function definition
    fprintf(stderr, "INFO - Looking for symbol %s\n", call->function_call.name);
    ASTNode* function_def = symbol_table_lookup(current_scope, call->function_call.name);

    if(!function_def) {
        fprintf(stderr,  "Undefined function '%s'\n", call->function_call.name);
        return;
    }

    if(function_def->type != AST_FUNCTION_DEF) {
        fprintf(stderr, "'%s' is not a function\n", call->function_call.name);
        return;
    }

    // 2. Create new scope for parameters
    fprintf(stderr, "Creating new scope for function %s\n", call->function_call.name);
    SymbolTable* func_scope = create_symbol_table(current_scope);

    // 3. Process arguments and add to scope
    if(call->function_call.arg_count != function_def->function_def.arg_count) {
        fprintf(stderr, "Argument count mismatch for '%s'\n",
                    call->function_call.name);
        return;
    }

    for(size_t i=0; i<call->function_call.arg_count; i++) {
        // Process argument expression
        process_node(call->function_call.args[i], cs, func_scope);

        // Get expected parameter type
        //TypeInfo expected = function_def->function_def.args[i]->type_info;

        // Add constraint: arg_type == param_type
        //add_constraint(cs, call->function_call.args[i],
        //              expected, call->function_call.args[i]->type_info);

        // Add parameter to symbol table
        fprintf(stderr, "Adding parameter %s to symbol table\n", function_def->function_def.args[i]);
        symbol_table_add(func_scope,
                        function_def->function_def.args[i],
                        call->function_call.args[i]);
    }

    // 4. Process return type constraint
    add_constraint(cs, call, &function_def->type_info);
}

void process_node(ASTNode* node, ConstraintSystem* cs, SymbolTable* current_scope) {
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
            if (node->type_info.is_literal) {
                break;
            }
            process_function_call(
                node,
                cs, 
                current_scope
            );

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
        case AST_VARIABLE: {
            fprintf(stderr, "INFO - Found variable (name=%s) during constraint collection\n", node->variable.name);
            ASTNode* variable_def = symbol_table_lookup(current_scope, node->variable.name);

            if (!variable_def) {
                fprintf(stderr,  "Undefined variable '%s'\n", node->variable.name);
                break;
            }

            add_constraint(cs, node,
                &variable_def->type_info);
            break;
        }
        case AST_VARIABLE_DEF: {
            symbol_table_add(current_scope, node->variable_def.name, node->variable_def.body);

            add_constraint(cs, node,
                &node->variable_def.body->type_info);
            break;
        }
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

        default:
            res.expr = node;
            break;
    }

    return res;
}

// Transform object creation
static char* new_constructor(char* cls) {
    char* temp = malloc(16 + 16);
    sprintf(temp, "%s_constructor", cls);
    return temp;
}

static char* new_instance_type(char* cls) {
    char* temp = malloc(16 + 16);
    sprintf(temp, "%%struct.%s*", cls);
    return temp;
}

static char* new_method(char* cls, ASTNode* node) {
    char* temp = malloc(16 + 16);
    // NOTE add arg types
    sprintf(temp, "%s_%s", cls, node->method_call.method);
    return temp;
}

static ASTNode* transform_constructor(ASTNode* node) {
    // Transform constructor arguments
    char* cname = new_constructor(node->constructor.cls);
    ASTNode* new_node = create_ast_function_call(
        cname,
        node->constructor.args,
        node->constructor.arg_count
    );
    new_node->type_info.cls = strdup(node->constructor.cls);
    new_node->type_info.name = new_instance_type(node->constructor.cls);
    new_node->type_info.kind = hash(cname);
    new_node->type_info.is_literal = true;

    // dealloc
    free(node->constructor.cls);

    return new_node;
}

// Transform method calls
static ASTNode* transform_method_call(ASTNode* node, SymbolTable* scope) {
    ASTNode* ref = symbol_table_lookup(scope, node->method_call.cls);
    if (!ref || !ref->type_info.cls) {
        fprintf(
            stderr,
            "WARNING - Could not access the class via the instance in %s.%s\n",
            node->method_call.cls,
            node->method_call.method
        );
        return node;
    }
    ASTNode* cls = symbol_table_lookup(scope, ref->type_info.cls);
    fprintf(
        stderr,
        "INFO - Accessing metod '%s' of %s\n",
        node->method_call.method,
        cls->type_info.cls
    );

    char* mname = new_method(cls->type_info.cls, node);
    ASTNode* new_node = create_ast_function_call(
        mname,
        node->method_call.args,
        node->method_call.arg_count
    );
    // XXX resolve type 
    new_node->type_info.kind = 1;
    new_node->type_info.is_literal = false;

    return new_node;
}

ASTNode* transform_ast(ASTNode* node, SymbolTable* scope) {
    fprintf(stderr, "Node type=%d\n", node->type);
    if (!node) return NULL;

    // First transform children recursively
    switch (node->type) {
        case AST_BLOCK: {
            fprintf(stderr, "Transforming AST_BLOCK\n");
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                node->block.statements[i] = transform_ast(node->block.statements[i], scope);
            }                      
            break;
        }
        case AST_FUNCTION_DEF: {
            fprintf(stderr, "Transforming AST_FUNCTION_DEF\n");
            node->function_def.body = transform_ast(node->function_def.body, scope);
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

            node = transform_ast(new_block, scope);
            break;
        }
        case AST_CONDITIONAL: {
            node->conditional.hypothesis = transform_ast(node->conditional.hypothesis, scope);
            node->conditional.thesis = transform_ast(node->conditional.thesis, scope);
            node->conditional.antithesis = transform_ast(node->conditional.antithesis, scope);

            break;
        }
        case AST_WHILE_LOOP: {
            node->while_loop.cond = transform_ast(node->while_loop.cond, scope);
            node->while_loop.body = transform_ast(node->while_loop.body, scope);
            break;
        }
        case AST_BINARY_OP: {
            node->binary_op.left = transform_ast(node->binary_op.left, scope);
            node->binary_op.right = transform_ast(node->binary_op.right, scope);

            if (node->binary_op.op == OP_EXP) {
                ASTNode** pow_args = malloc(sizeof(ASTNode*)*2);
                pow_args[0] = node->binary_op.left;
                pow_args[1] = node->binary_op.right;
                node = create_ast_function_call("pow", pow_args, 2);
            }
            break;
        }
        case AST_FUNCTION_CALL: {
            for (size_t i = 0; i < node->function_call.arg_count; i++) {
                node->function_call.args[i] = transform_ast(node->function_call.args[i], scope);
            }
            break;
        }
        case AST_VARIABLE_DEF: {
            node->variable_def.body = transform_ast(node->variable_def.body, scope);
            break;
        }
        case AST_CONSTRUCTOR: {
            node = transform_constructor(node);
            break;
        }
        case AST_METHOD_CALL: {
            node = transform_method_call(node, scope);
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

    statements = realloc(statements, (count + 1) * sizeof(ASTNode*));
    main_block->block.statements = statements;
    main_block->block.stmt_count = count + 1;

    statements[count] = create_ast_number(0);

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
        if ((stmt->type == AST_FUNCTION_DEF) || (stmt->type == AST_TYPE_DEF)) {
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

void inherit(ASTNode* node, ASTNode* parent) {
    if (!parent) {
        return;
    }

    // sanity check
    for (unsigned int i = 0; i < node->type_decl.field_count; i++) {
        for (unsigned int j = i; j < parent->type_decl.field_count; j++) {
            if (
                node->type_decl.fields[i]->field_def.name == parent->type_decl.fields[i]->field_def.name
            ) {
                fprintf(
                    stderr,
                    "ERROR - Redeclaration of field '%s' in '%s'!",
                    node->type_decl.fields[i]->field_def.name,
                    node->type_decl.name
                );
                exit(1);
            }
        }
    }
    unsigned int old_count = node->type_decl.field_count;
    node->type_decl.field_count += parent->type_decl.field_count;
    node->type_decl.fields = realloc(
        node->type_decl.fields,
        node->type_decl.field_count * sizeof(ASTNode*)
    );
    for (unsigned int i = 0; i < parent->type_decl.field_count; i++) {
        if (i < old_count) {
            // reallocate own field first
            node->type_decl.fields[i + parent->type_decl.field_count] = node->type_decl.fields[i];
        }
        node->type_decl.fields[i] = parent->type_decl.fields[i];
    }

    node->type_decl.methods = realloc(
        node->type_decl.methods,
        (node->type_decl.method_count + parent->type_decl.method_count) * sizeof(ASTNode*)
    );
    unsigned int counter = 0;
    bool flag = false;
    for (unsigned int i = 0; i < parent->type_decl.method_count; i++) {
        flag = false;
        for (unsigned int j = 0; j < node->type_decl.method_count; j++) {
            // reallocate own field first
            if (
                strcmp(
                    node->type_decl.methods[j]->function_def.name, parent->type_decl.methods[i]->function_def.name
                ) == 0
            ) {
                fprintf(stderr, "%s\n", node->type_decl.methods[j]->function_def.name);
                flag = true;
                break;
            }
        }
        if (flag) {
            continue;
        }
        fprintf(
            stderr,
            "INFO - (counter=%d, method_count=%d, parent_method=%s\n)",
            counter,
            node->type_decl.method_count, parent->type_decl.methods[i]->function_def.name
        );
        // create a new one since we modify the name in codegen to
        // detach the method
        node->type_decl.methods[counter + node->type_decl.method_count] = create_ast_function_def(
            parent->type_decl.methods[i]->function_def.name,
            parent->type_decl.methods[i]->function_def.body,
            parent->type_decl.methods[i]->function_def.args,
            parent->type_decl.methods[i]->function_def.arg_count
        );
        counter += 1;
    }
    node->type_decl.method_count += counter;
}

void _semantic_analysis(ASTNode *node, ConstraintSystem* cs, SymbolTable* scope) {
    /* Anything read-only */
    // at this point, let-ins do not exist
    if (!node) return;

    process_node(node, cs, scope);

    switch (node->type) {
        case AST_BLOCK: {
            fprintf(stderr, "INFO - Performing sem_anal into code block\n");
            // Recurse into new structure
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                _semantic_analysis(node->block.statements[i], cs, scope);
            }
            break;
        }
        case AST_FUNCTION_DEF: {
            fprintf(stderr, "INFO - Performing sem_anal into function %s\n", node->function_def.name);
            _semantic_analysis(node->function_def.body, cs, scope);
            break;
        }
        case AST_FUNCTION_CALL: {
            fprintf(stderr, "INFO - Performing sem_anal into function call %s\n", node->function_call.name);
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                _semantic_analysis(node->function_call.args[i], cs, scope);
            }
            break;
        }
        case AST_VARIABLE_DEF: {
            fprintf(stderr, "INFO - Performing sem_anal into variable %s\n", node->variable_def.name);
            _semantic_analysis(node->variable_def.body, cs, scope);
            break;
        }
        case AST_BINARY_OP: {
            _semantic_analysis(node->binary_op.left, cs, scope);
            _semantic_analysis(node->binary_op.right, cs, scope);
            break;
        }
        case AST_TYPE_DEF: {
            // XXX double
            if (node->type_decl.base_type) {
                fprintf(
                    stderr,
                    "INFO - Found child class '%s' of '%s'\n",
                    node->type_decl.name,
                    node->type_decl.base_type
                );
                ASTNode* parent = symbol_table_lookup(scope, node->type_decl.base_type);

                inherit(node, parent);
            }
            char **cargs = malloc(sizeof(char*)*node->type_decl.field_count);
            for (unsigned int i = 0; i < node->type_decl.field_count; i++) {
                cargs[i] = "[param]";
            }
            char *cname = new_constructor(node->type_decl.name);
            ASTNode* constructor = create_ast_function_def(cname, NULL, cargs, node->type_decl.field_count);
            constructor->type_info.kind = hash(cname);
            symbol_table_add(scope, cname, constructor);
            // add class as well
            symbol_table_add(scope, node->type_decl.name, node);
            node->type_info.cls = node->type_decl.name;
            break;
        }
        case AST_FIELD_ACCESS: {
            ASTNode* ref = symbol_table_lookup(scope, node->field_access.cls);
            if (!ref->type_info.cls) {
                fprintf(
                    stderr,
                    "WARNING - Could not access the class via the instance in %s.%s",
                    node->field_access.cls,
                    node->field_access.field
                );
                break;
            }
            ASTNode* cls = symbol_table_lookup(scope, ref->type_info.cls);
            fprintf(
                stderr,
                "INFO - Accessing classs instance '%s' field '%s'\n",
                cls->type_info.cls,
                node->field_access.field
            );
            for (unsigned int i = 0; i < cls->type_decl.field_count; i++) {
                if (strcmp(cls->type_decl.fields[i]->field_def.name, node->field_access.field) == 0) {
                    node->field_access.pos = i;
                }
            }
            break;
        }
    }
}

bool semantic_analysis(ASTNode *node) {
    switch (node->type) {
        // the only case
        case AST_BLOCK: {
            sa_block(node); // reorganize code in functions (transform the parent)
            SymbolTable* scope = create_symbol_table(NULL);
            node = transform_ast(node, scope); // embrace FLATness (transform the children)
            ConstraintSystem cs = {NULL, 0, 0};
            _semantic_analysis(node, &cs, scope); // type checking (read-only)
            
            bool res = solve_constraints(&cs);
            // DEBUG to see final types clearly
            solve_constraints(&cs);

            node = transform_ast(node, scope); // embrace FLATness (transform the children)
            return res;
        }
        default: {
            fprintf(stderr, "FATAL - Could not recognize root node (%d, it's not a block)\n", node->type);
            return false;
        }
    }
    return true;
}
