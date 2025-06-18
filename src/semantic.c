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

static char* new_constructor(char* cls) {
    char* temp = malloc(16 + 16 + 16);
    sprintf(temp, "%s_constructor", cls);
    return temp;
}

static char* new_instance_type(char* cls) {
    char* temp = malloc(16 + 16 + 16);
    sprintf(temp, "%%struct.%s*", cls);
    return temp;
}

static char* new_method(char* cls, ASTNode* node) {
    char* temp = malloc(16 + 16 + 16);
    // NOTE add arg types
    sprintf(temp, "%s_%s", cls, node->method_call.method);
    return temp;
}

void coerce(ASTNode* node) {
    for (unsigned int j = 0; j < node->type_decl.method_count; j++) {
        node->type_decl.methods[j]->function_def.args_definitions[0]->type_info.name = new_instance_type(node->type_decl.name);
        node->type_decl.methods[j]->function_def.args_definitions[0]->type_info.cls = strdup(node->type_decl.name);
        node->type_decl.methods[j]->function_def.args_definitions[0]->type_info.kind = hash(node->type_decl.name);
    }
}

// type inference
bool solve_constraints(ConstraintSystem* cs) {
    fprintf(stderr, "INFO - Solving constraints...\n");
    bool changed;
    bool res = true;
    do {
        changed = false;
        for(size_t i=0; i<cs->count; i++) {
            TypeConstraint* c = &cs->constraints[i];
            TypeInfo* t = c->expected;
            if (c->node == NULL) {
                fprintf(stderr, "ERROR -Invalid constraint (c->node is null))! %p %zu\n", c->node, t->kind);
                exit(1);
                continue;
            }
            //fprintf(stderr, "DEBUG - %d %p %p\n", c->node->type, t, c->node, c->node->type_info);
            if (t == NULL) {
                fprintf(stderr, "ERROR - Null constraint detected\n");
                if (c->node->type == AST_VARIABLE) {
                    // error constraint
                    fprintf(stderr, "%s",c->node->variable.name);
                    res = false;
                }
                continue;
            }
            size_t expected = t->kind;
            size_t actual = c->node->type_info.kind;

            fprintf(stderr, "DEBUG - Expected: %zu ; Actual: %zu ; Node: %d\n\n", expected, actual, c->node->type);

            if (c->node->type == AST_FUNCTION_CALL) {
                //fprintf(stderr, "For function %s\n", c->node->function_call.name);
            }
            else if (c->node->type == AST_VARIABLE_DEF) {
                //fprintf(stderr, "For variable %s\n", c->node->variable_def.name);
            }

            if (c->node->type_info.cls != NULL) {
                //fprintf(stderr, "With type %s\n", c->node->type_info.name);
            }
            if (t->cls != NULL) {
                //fprintf(stderr, "Expected type %s\n", t->name);
            }

            // Propagate concrete -> unknown
            if(expected != TYPE_UNKNOWN &&
               actual == TYPE_UNKNOWN) {
                fprintf(stderr, "INFO - Solved type for node type=%d; to %zu\n", c->node->type, c->node->type_info.kind);

                c->node->type_info.kind = ((TypeInfo*) (c->expected))->kind;
                c->node->type_info.name = ((TypeInfo*) (c->expected))->name;
                c->node->type_info.cls = ((TypeInfo*) (c->expected))->cls;

                changed = true;
            }

            // Check for conflicts
            if(expected != TYPE_UNKNOWN &&
               actual != TYPE_UNKNOWN &&
               expected != actual) {
               // XXX
               fprintf(stderr, "ERROR - Literal type mismatch (current_type=%d); [%d, %d]\n", c->node->type, c->node->line, c->node->column);
               res = false;
            }
        }
    } while(changed);

    return res;
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
        minargs[0] = "[min_param_1]";
        minargs[1] = "[min_param_2]";
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

ASTNode* lookup_method(char* name, ASTNode* cls, SymbolTable* scope) {
    for (unsigned int i = 0; i < cls->type_decl.method_count; i++) {
        if (strcmp(cls->type_decl.methods[i]->function_def.name, name) == 0) {
            return cls->type_decl.methods[i];
        }
    }
    // didn't find it
    if (cls->type_decl.base_type) {
        fprintf(
            stderr,
            "INFO - Found child class '%s' of '%s'\n",
            cls->type_decl.name,
            cls->type_decl.base_type
        );
        ASTNode* parent = symbol_table_lookup(scope, cls->type_decl.base_type);

        ASTNode* temp = lookup_method(name, parent, scope);

        if (temp) {
            return temp;
        }
    }
    // didn't find it in parent class
    fprintf(stderr, "ERROR - No method called %s in %s\n", name, cls->type_decl.name);
    exit(1);
    return NULL;
}

int lookup_index(ASTNode* node, ASTNode* cls, SymbolTable* scope, ASTNode** correct_field) {
    int index = 0;
    if (cls->type_decl.base_type) {
        fprintf(
            stderr,
            "INFO - Found child class '%s' of '%s'\n",
            cls->type_decl.name,
            cls->type_decl.base_type
        );
        ASTNode* parent = symbol_table_lookup(scope, cls->type_decl.base_type);

        int temp = lookup_index(node, parent, scope, correct_field);

        if (temp > 0) {
            return temp;
        }
        // positive
        // this is the number of fields
        index = -temp;
    }

    for (int i = 0; i < (int) cls->type_decl.field_count; i++) {
        fprintf(
            stderr,
            "DEBUG - Comparing %s and %s during field access\n",
            cls->type_decl.fields[i]->field_def.name,
            node->field_access.field
         );
        if (strcmp(cls->type_decl.fields[i]->field_def.name, node->field_access.field) == 0) {
            fprintf(
                stderr,
                "INFO - Found position for %s -> %d\n",
                cls->type_decl.fields[i]->field_def.name,
                i + index
             );
            node->field_access.pos = i + index;
            *correct_field = cls->type_decl.fields[i];
        }
    }
    return -(cls->type_decl.field_count + index);
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

    // mark as called
    function_def->function_def.called = 1;

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
        _semantic_analysis(call->function_call.args[i], cs, func_scope);

        // Add constraint: arg_type == param_type
        if (!function_def->function_def.args_definitions[i]) {
            fprintf(
                stderr,
                "ERROR - No definition for %s in %s",
                function_def->function_def.args[i],
                function_def->function_def.name
            );

            add_constraint(cs, create_ast_variable("Definition not found\n"), NULL);
            exit(1);
        }
        fprintf(stderr, "%p\n", function_def->function_def.args_definitions[i]);
        add_constraint(
            cs,
            function_def->function_def.args_definitions[i],
            &call->function_call.args[i]->type_info
        );

        // Add parameter to symbol table
        fprintf(
            stderr,
            "INFO - Adding parameter '%s' to symbol table for function %s (type %zu)\n",
            function_def->function_def.args[i],
            function_def->function_def.name,
            call->function_call.args[i]->type_info.kind
        );
        symbol_table_add(
            func_scope,
            function_def->function_def.args[i],
            function_def->function_def.args_definitions[i]
        );


    }
    // 4. Process return type constraint
    add_constraint(cs, call, &function_def->type_info);
    solve_constraints(cs);
    if (function_def->type_info.kind == TYPE_UNKNOWN) {
        _semantic_analysis(function_def, cs, func_scope);
    }
}

void process_method_call(
    ASTNode* call,
    ConstraintSystem* cs,
    SymbolTable* current_scope
) {
    // do not modify external constraints
    /*
    ConstraintSystem* cs = malloc(sizeof(ConstraintSystem));
    cs->count = _cs->count;
    cs->capacity = _cs->capacity;

    cs->constraints = malloc(_cs->capacity * sizeof(TypeConstraint));
    cs->constraints = memcpy(cs->constraints, _cs->constraints, _cs->capacity * sizeof(TypeConstraint));
    */

    ASTNode* ref = call->method_call.cls;
    if (!ref || !ref->type_info.cls) {
        fprintf(
            stderr,
            "WARNING - Could not access the class via the instance in %d.%s\n",
            ref->type,
            call->method_call.method
        );
        return;
    }
    fprintf(stderr, "%p",ref->type_info.cls);
    ASTNode* cls = symbol_table_lookup(current_scope, ref->type_info.cls);

    if (cls->type_info.cls == NULL) {
        // no-op
        fprintf(
            stderr,
            "ERROR - Class %s not found for method %s\n",
            cls->type_decl.name,
            call->method_call.method
        );

        return;
    }
    fprintf(
        stderr,
        "INFO - Accessing method '%s' of %s during analysis\n",
        call->method_call.method,
        cls->type_info.cls
    );

    ASTNode* function_def = lookup_method(call->method_call.method, cls, current_scope);

    function_def->function_def.called = 1;

    if(!function_def) {
        fprintf(stderr,  "ERROR - Undefined method '%s'\n", call->method_call.method);
        add_constraint(cs, create_ast_variable("Function def not found\n"), NULL);
        exit(1);
        return;
    }

    if(function_def->type != AST_METHOD_DEF) {
        fprintf(stderr, "ERORR - '%s' is not a method\n", call->method_call.method);
        add_constraint(cs, create_ast_variable("Wrong method type\n"), NULL);
        exit(1);
        return;
    }

    // 2. Create new scope for parameters
    fprintf(stderr, "Creating new scope for method %s\n", call->method_call.method);
    SymbolTable* func_scope = create_symbol_table(current_scope);
    //SymbolTable* func_scope = current_scope;

    // 3. Process arguments and add to scope
    if(call->method_call.arg_count != (function_def->function_def.arg_count)) {
        fprintf(
            stderr,
            "ERROR - Argument count mismatch for '%s' (%d vs %d)\n",
            call->method_call.method,
            call->method_call.arg_count,
            function_def->function_def.arg_count
        );
        add_constraint(cs, create_ast_variable("Argument mismatch\n"), NULL);
        exit(1);
        return;
    }

    for(size_t i=0; i<call->method_call.arg_count; i++) {
        // Process argument expression
        fprintf(stderr, "INFO - Method of type=%d (i=%zu)\n", call->method_call.args[i]->type, i);
        _semantic_analysis(call->method_call.args[i], cs, func_scope);

        // Add constraint: arg_type == param_type
        // Avoid adding constraints for self
        // self is passed implicitly so we will get an
        // error otherwise
        if (!(i == 0 && (function_def->function_def.args_definitions[i]->type_info.kind != TYPE_UNKNOWN))) {
            add_constraint(
                cs,
                function_def->function_def.args_definitions[i],
                &call->method_call.args[i]->type_info
            );
        }

        // Add parameter to symbol table
        fprintf(
            stderr,
            "INFO - Adding parameter '%s' to symbol table for method %s (type %zu)\n",
            function_def->function_def.args[i],
            function_def->function_def.name,
            call->method_call.args[i]->type_info.kind
        );
        symbol_table_add(func_scope,
                        function_def->function_def.args[i],
                        function_def->function_def.args_definitions[i]);
    }

    // 4. Process return type constraint
    add_constraint(cs, call, &function_def->type_info);
    solve_constraints(cs);

    if (function_def->type_info.kind == TYPE_UNKNOWN) {
        _semantic_analysis(function_def, cs, func_scope);
    }
    fprintf(
        stderr,
        "-> %zu %s %zu %zu\n",
        function_def->type_info.kind,
        function_def->function_def.name,
        function_def->function_def.args_definitions[0]->type_info.kind,
        call->method_call.args[0]->type_info.kind
    );
}


void process_let_in(
    ASTNode* node,
    ConstraintSystem* cs,
    SymbolTable* current_scope
) {
    fprintf(stderr, "Creating new scope for let-in\n");
    SymbolTable* let_scope = create_symbol_table(current_scope);

    for(size_t i=0; i<node->let_in.var_count; i++) {
        fprintf(
            stderr,
            "Adding parameter '%s' to symbol table for let-in (type %zu)\n",
            node->let_in.var_names[i],
            node->let_in.var_values[i]->type_info.kind
        );
        symbol_table_add(
            let_scope,
            node->let_in.var_names[i],
            node->let_in.var_values[i]
        );
        _semantic_analysis(node->let_in.var_values[i], cs, let_scope);
    }

    _semantic_analysis(node->let_in.body, cs, let_scope);
    add_constraint(cs, node, &node->let_in.body->type_info);
}

void process_node(ASTNode* node, ConstraintSystem* cs, SymbolTable* current_scope) {

    if ((node->type_info.kind > 100) && (node->type_info.name == NULL)) {
        fprintf(stderr, "--------- WARNING - %d %s %zu\n", node->type, node->variable.name, node->type_info.kind);
        //node->type_info.kind = 0;
        //exit(1);
    }
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
            TypeInfo *lit = malloc(sizeof(TypeInfo));
            lit->kind = TYPE_DOUBLE;
            lit->is_literal = true;

            // Hypothesis must be numeric (treated as float)
            add_constraint(cs, node->conditional.hypothesis, lit);

            // Branches must match
            add_constraint(cs, node->conditional.thesis,
                &node->conditional.antithesis->type_info);
            add_constraint(cs, node->conditional.antithesis,
                &node->conditional.thesis->type_info);

            // type depends on body
            add_constraint(cs, node,
                &node->conditional.thesis->type_info);

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
            process_function_call(
                node,
                cs, 
                current_scope
            );
            break;
        }
        case AST_FUNCTION_DEF: {
            fprintf(stderr, "INFO - Found function (name=%s) during constraint collection\n", node->function_def.name);
            symbol_table_add(current_scope, node->function_def.name, node);

            SymbolTable* func_scope = create_symbol_table(current_scope);

            for (size_t i = 0; i < node->function_def.arg_count; i++) {
                symbol_table_add(
                    func_scope,
                    node->function_def.args[i],
                    node->function_def.args_definitions[i]
                );
            }

            if (node->function_def.body != NULL) {
                add_constraint(cs, node,
                    &node->function_def.body->type_info);
                _semantic_analysis(node->function_def.body, cs, func_scope);
            }

            break;
        }
        case AST_VARIABLE: {
            fprintf(stderr, "INFO - Found variable (name=%s) during constraint collection\n", node->variable.name);
            ASTNode* variable_def = symbol_table_lookup(current_scope, node->variable.name);

            if (!variable_def) {
                fprintf(stderr,  "Undefined variable '%s' [%d, %d]\n", node->variable.name, node->line, node->column);
                add_constraint(cs, create_ast_variable("Undefined variable\n"), NULL);
                exit(1);
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
        case AST_LET_IN: {
            process_let_in(
                node,
                cs, 
                current_scope
            );
            break;
        }
        case AST_WHILE_LOOP: {
            TypeInfo *lit = malloc(sizeof(TypeInfo));
            lit->kind = TYPE_DOUBLE;
            lit->is_literal = true;

            // default to float
            add_constraint(cs, node, &node->while_loop.body->type_info);
            add_constraint(cs, node->while_loop.cond, lit);
            break;
        }

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
        case AST_CONSTRUCTOR: {
            TypeInfo *lit = malloc(sizeof(TypeInfo));

            lit->name = new_instance_type(node->constructor.cls);
            lit->cls = strdup(node->constructor.cls);
            lit->kind = hash(node->constructor.cls);
            lit->is_literal = true;

            add_constraint(cs, node, lit);

            ASTNode* cls_def = symbol_table_lookup(current_scope, node->constructor.cls);

            if (!cls_def) {
                fprintf(stderr,  "Undefined class constructor '%s' [%d, %d]\n", node->constructor.cls, node->line, node->column);
                add_constraint(cs, create_ast_variable("Undefined class constructor\n"), NULL);
                exit(1);
                break;
            }
            int index = node->constructor.arg_count - 1;
            while (index > 0) {
                for (int i = cls_def->type_decl.field_count - 1; i >= 0; i--) {
                    if (i < 0) {
                        break;
                    }
                    fprintf(stderr, "%d\n", i);
                    add_constraint(
                        cs,
                        node->constructor.args[index],
                        &cls_def->type_decl.fields[i]->type_info
                    );
                    index -= 1;
                }
                fprintf(stderr, "%d %d\n", cls_def->type_decl.field_count, index);
                fprintf(stderr, "%s\n", cls_def->type_decl.base_type);
                if (index > 0 && (cls_def->type_decl.base_type == NULL)) {
                    fprintf(
                        stderr,
                        "%d extra fields in constructor [%d, %d]\n",
                        index,
                        node->line,
                        node->column
                    );
                    add_constraint(cs, create_ast_variable("Too many fields for constructor\n"), NULL);
                    exit(1);
                }
                if (cls_def->type_decl.base_type != NULL) {
                    cls_def = symbol_table_lookup(current_scope, cls_def->type_decl.base_type);
                }
            }

            break;
        }
        case AST_METHOD_CALL: {
            process_method_call(
                node,
                cs,
                current_scope
            );
            break;
        }

        case AST_TYPE_DEF: {
            symbol_table_add(current_scope, node->type_decl.name, node);
            node->type_info.cls = node->type_decl.name;
            break;
        }
        case AST_METHOD_DEF: {
            SymbolTable* func_scope = create_symbol_table(current_scope);
            for (size_t i = 0; i < node->function_def.arg_count; i++) {
                symbol_table_add(
                    func_scope,
                    node->function_def.args[i],
                    node->function_def.args_definitions[i]
                );
            }

            if (node->function_def.body != NULL) {
                add_constraint(cs, node,
                    &node->function_def.body->type_info);
                _semantic_analysis(node->function_def.body, cs, func_scope);
            }
            break;
        }

        case AST_FIELD_ACCESS: {
            ASTNode* ref = symbol_table_lookup(current_scope, node->field_access.cls);
            if (!ref || !ref->type_info.cls) {
                fprintf(
                    stderr,
                    "-------------------- WARNING - Could not access the class via the instance in %s.%s\n",
                    node->field_access.cls,
                    node->field_access.field
                );
                break;
            }
            ASTNode* cls = symbol_table_lookup(current_scope, ref->type_info.cls);
            fprintf(
                stderr,
                "INFO - Accessing classs instance '%s' field '%s'\n",
                cls->type_info.cls,
                node->field_access.field
            );
            ASTNode* correct_field = NULL;
            int res = lookup_index(node, cls, current_scope, &correct_field);

            if (correct_field == NULL) {
                fprintf(
                    stderr,
                    "ERROR - Field not found (%s, %s) [%d, %d]\n",
                    node->field_access.cls,
                    node->field_access.field,
                    node->line,
                    node->column
                );
                // add error constraint
                add_constraint(cs, create_ast_variable("Field not found\n"), NULL);
                break;
            }

            add_constraint(cs, node, &correct_field->type_info);
            break;
        }
        case AST_FIELD_REASSIGN: {
            ASTNode* val = symbol_table_lookup(current_scope, node->field_reassign.value);
            add_constraint(cs, node, &val->type_info);
        }
        case AST_FIELD_DEF: {
            add_constraint(cs, node, &node->field_def.default_value->type_info);
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
                if (val.stmts == NULL) {
                    fprintf(
                        stderr,
                        "WARNING - NULL detected inside let-in (%s)",
                        node->let_in.var_names[i]
                    );
                }
                else {
                    append_stmts(&res, val.stmts, val.stmt_count);
                }

                // Create variable definition
                ASTNode* def = create_ast_variable_def(
                    node->let_in.var_names[i],
                    val.expr
                );
                if (val.expr->type_info.kind == 0) {
                    fprintf(
                        stderr,
                        "WARNING - Type=%zu (UNKOWN) for node type %d in let-in\n",
                        node->let_in.var_values[i]->type_info.kind,
                        node->let_in.var_values[i]->type
                    );
                }
                def->type_info.kind = val.expr->type_info.kind;
                def->type_info.name = val.expr->type_info.name;
                def->type_info.cls = val.expr->type_info.cls;

                res.stmts = realloc(res.stmts, (res.stmt_count + 1) * sizeof(ASTNode*));
                res.stmts[res.stmt_count++] = def;
            }

            // Process body
            FlattenResult body = flatten(node->let_in.body);
            
            if (body.stmts == NULL) {
                fprintf(
                    stderr,
                    "WARNING - NULL detected inside let-in body"
                );
            }
            else {
                append_stmts(&res, body.stmts, body.stmt_count);
            }

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
static ASTNode* transform_constructor(ASTNode* node) {
    // Transform constructor arguments
    char* cname = new_constructor(node->constructor.cls);
    ASTNode* new_node = create_ast_function_call(
        cname,
        node->constructor.args,
        node->constructor.arg_count
    );
    // XXX delet
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
    // self
    fprintf(stderr, "INFO - Transforming method call\n");
    // XXX seems to rely on undefined behaviour
    // the lifetime of the string varies
    fprintf(
        stderr,
        "INFO - Self type %zu %s\n",
        node->method_call.cls->type_info.kind,
        node->method_call.cls->type_info.cls
    );
    // no need to rely on the symbol table at all since
    // the type was already inferred

    char* mname = new_method(node->method_call.cls->type_info.cls, node);
    ASTNode* new_node = create_ast_function_call(
        mname,
        node->method_call.args,
        node->method_call.arg_count
    );

    new_node->type_info.kind = node->type_info.kind;
    new_node->type_info.name = node->type_info.name;
    new_node->type_info.cls = node->type_info.cls;

    return new_node;
}


void transform_base(ASTNode* node, ctx, SymbolTable* scope) {
    if (node->type_decl.base_type == NULL) {
        return;
    }
    // Add base function to methods if there's a parent
    for (size_t i = 0; i < node->type_decl.method_count; i++) {
        // Create base function only for methods that exist in parent
        char* method_name = node->type_decl.methods[i]->function_def.name;
        char parent_method_name[256];
        ASTNode* base_call = lookup_method(method_name, node, scope);

        // Create base function call: parent_method(self, ...)
        // Create base function: base() => parent_method(...)
        ASTNode* base_func = create_ast_function_def(
            "base",
            NULL,  // No parameters
            0,
            base_call
        );
        
        // Add base function to method body
        //ASTNode* new_body = create_block();
        //new_body->block.stmt_count = 1;
        //new_body->block.statements = malloc(sizeof(ASTNode*));
        //new_body->block.statements[0] = base_func;
        
        // Append original body
        //append_to_block(new_body, node->type_decl.methods[i]->function.body);
        
        // Replace method body
        //node->type_decl.methods[i]->function.body = new_body;
    }
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
        case AST_METHOD_DEF:
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
        case AST_TYPE_DEF: {
            fprintf(stderr, "INFO - Found type def %s\n", node->type_decl.name);
            for (size_t i = 0; i < node->type_decl.method_count; i++) {
                node->type_decl.methods[i] = transform_ast(node->type_decl.methods[i], scope);
            }
            for (size_t i = 0; i < node->type_decl.field_count; i++) {
                node->type_decl.fields[i] = transform_ast(node->type_decl.fields[i], scope);
            }

            //coerce(node);

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
            ASTNode* constructor = create_ast_function_def(
                cname,
                NULL,
                cargs,
                node->type_decl.field_count
            );

            constructor->type_info.name = "CLASS_DEF";
            constructor->type_info.kind = hash(cname);
            constructor->type_info.cls = "CLASS_DEF";

            symbol_table_add(scope, cname, constructor);
            break;
        }
        case AST_METHOD_CALL: {
            node = transform_method_call(node, scope);
            break;
        }
        case AST_FIELD_ACCESS: {
            break;
        }
        default: {
            break;
        }
    }

    // Then apply transformation to current node
    return node;
}

static ASTNode* create_main_function(ASTNode** statements, unsigned int count) {
    ASTNode* main_block = malloc(sizeof(ASTNode));
    main_block->type = AST_BLOCK;

    statements = realloc(statements, (count) * sizeof(ASTNode*));
    main_block->block.statements = statements;
    main_block->block.stmt_count = count;

    ASTNode* main_func = malloc(sizeof(ASTNode));
    main_func->type = AST_FUNCTION_DEF;
    main_func->function_def.name = strdup("main");
    main_func->function_def.body = main_block;
    // explicit
    main_func->function_def.args = NULL;
    main_func->function_def.arg_count = 0;
    main_func->function_def.called = 1;
    
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
    fprintf(
        stderr,
        "INFO - Parent methods: %d; Child methods=%d\n",
        parent->type_decl.method_count,
        node->type_decl.method_count
    );
    fprintf(
        stderr,
        "INFO - Parent fields: %d; Child fields=%d\n",
        parent->type_decl.field_count,
        node->type_decl.field_count
    );

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
                    node->type_decl.methods[j]->function_def.name,
                    parent->type_decl.methods[i]->function_def.name
                ) == 0
            ) {
                fprintf(
                    stderr,
                    "%s appears in both parent and child\n",
                    node->type_decl.methods[j]->function_def.name
                );
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

        // propagate called
        node->type_decl.methods[counter + node->type_decl.method_count]->function_def.called = parent->type_decl.methods[i]->function_def.called;

        // typeshit
        node->type_decl.methods[counter + node->type_decl.method_count]->type_info.kind = parent->type_decl.methods[i]->type_info.kind;
        node->type_decl.methods[counter + node->type_decl.method_count]->type_info.name = parent->type_decl.methods[i]->type_info.name;
        node->type_decl.methods[counter + node->type_decl.method_count]->type_info.cls = parent->type_decl.methods[i]->type_info.cls;

        // propagate types
        for (unsigned int pk = 1; pk < node->type_decl.methods[counter + node->type_decl.method_count]->function_def.arg_count; pk++) {
            node->type_decl.methods[counter + node->type_decl.method_count]->function_def.args_definitions[pk] = parent->type_decl.methods[i]->function_def.args_definitions[pk];
        }
        // self
        node->type_decl.methods[counter + node->type_decl.method_count]->function_def.args_definitions[0]->type_info.name = new_instance_type(node->type_decl.name);
        node->type_decl.methods[counter + node->type_decl.method_count]->function_def.args_definitions[0]->type_info.cls = strdup(node->type_decl.name);
        node->type_decl.methods[counter + node->type_decl.method_count]->function_def.args_definitions[0]->type_info.kind = hash(node->type_decl.name);
        counter += 1;
    }

    fprintf(
        stderr,
        "INFO - Parent fields: %d; Child fields=%d\n",
        parent->type_decl.field_count,
        node->type_decl.field_count
    );

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
            fprintf(stderr, "INFO - Performing sem_anal into function def %s\n", node->function_def.name);
            break;
        }
        case AST_LET_IN: {
            //for (size_t i = 0; i < node->let_in.var_count; i++) {
            //    _semantic_analysis(node->let_in.var_values[i], cs, scope);
            //}
            //_semantic_analysis(node->let_in.body, cs, scope);
            break;
        }
        case AST_FUNCTION_CALL: {
            fprintf(stderr, "INFO - Performing sem_anal into function call %s\n", node->function_call.name);
            // XXX args too
            //for (size_t i = 0; i < node->block.stmt_count; i++) {
            //    _semantic_analysis(node->function_call.args[i], cs, scope);
            //}
            break;
        }
        case AST_VARIABLE_DEF: {
            fprintf(stderr, "INFO - Performing sem_anal into variable def %s\n", node->variable_def.name);
            _semantic_analysis(node->variable_def.body, cs, scope);
            break;
        }
        case AST_VARIABLE: {
            fprintf(stderr, "INFO - Found terminal variable %s\n", node->variable.name);
            process_node(node, cs, scope);
            break;
        }
        case AST_NUMBER: {
            fprintf(stderr, "INFO - Found terminal number %f\n", node->number);
            process_node(node, cs, scope);
            break;
        }
        case AST_STRING: {
            fprintf(stderr, "INFO - Found terminal string %s\n", node->string);
            process_node(node, cs, scope);
            break;
        }
        case AST_BINARY_OP: {
            fprintf(stderr, "INFO - Found binary op\n");
            _semantic_analysis(node->binary_op.left, cs, scope);
            _semantic_analysis(node->binary_op.right, cs, scope);
            break;
        }
        case AST_CONDITIONAL: {
            fprintf(stderr, "INFO - Found conditional\n");
            _semantic_analysis(node->conditional.hypothesis, cs, scope);
            _semantic_analysis(node->conditional.thesis, cs, scope);
            _semantic_analysis(node->conditional.antithesis, cs, scope);
            break;
        }
        case AST_CONSTRUCTOR: {
            fprintf(stderr, "INFO - Found constructor for %s\n", node->constructor.cls);
            for (size_t i = 0; i < node->constructor.arg_count; i++) {
                _semantic_analysis(node->constructor.args[i], cs, scope);
            }
            break;
        }
        case AST_TYPE_DEF: {
            fprintf(stderr, "INFO - Found type def %s \n", node->type_decl.name);
            for (size_t i = 0; i < node->type_decl.field_count; i++) {
                _semantic_analysis(node->type_decl.fields[i], cs, scope);
                _semantic_analysis(node->type_decl.fields[i]->field_def.default_value, cs, scope);
            }
            for (size_t i = 0; i < node->type_decl.method_count; i++) {
                _semantic_analysis(node->type_decl.methods[i], cs, scope);
                fprintf(stderr, "%d\n", node->type_decl.methods[i]->type);
            }
            // no new global symbols inside methods nor class definitions
            // so we let the processor handle it
            break;
        }
        case AST_FIELD_DEF: {
            fprintf(
                stderr,
                "INFO - Found field def %s\n",
                node->field_def.name
            );
            _semantic_analysis(node->field_def.default_value, cs, scope);
            break;
        }
        case AST_FIELD_REASSIGN: {
            fprintf(
                stderr,
                "INFO - Found field reassign %s.%s\n",
                node->field_reassign.field_access->field_access.cls,
                node->field_reassign.field_access->field_access.field
            );
            _semantic_analysis(node->method_call.cls, cs, scope);
        }
        case AST_FIELD_ACCESS: {
            // XXX
            fprintf(
                stderr,
                "INFO - Found field access %s.%s\n",
                node->field_access.cls,
                node->field_access.field
            );
            break;
        }

        case AST_METHOD_CALL: {
            fprintf(
                stderr,
                "INFO - Found method call %d.%s\n",
                node->method_call.cls->type,
                node->method_call.method
            );
            _semantic_analysis(node->method_call.cls, cs, scope);
            break;
        }

        case AST_WHILE_LOOP: {
            _semantic_analysis(node->while_loop.cond, cs, scope);
            _semantic_analysis(node->while_loop.body, cs, scope);
            break;
        }

        default:
            break;
    }
}

bool semantic_analysis(ASTNode *node) {
    switch (node->type) {
        // the only case
        case AST_BLOCK: {
            bool res;
            SymbolTable* scope = create_symbol_table(NULL);
            ConstraintSystem cs = {NULL, 0, 0};

            sa_block(node); // reorganize code in functions (transform the parent)

            _semantic_analysis(node, &cs, scope); // symbol table
            // dump old CS to reduce runtime
            free(cs.constraints);

            cs.constraints = NULL;
            cs.count = 0;
            cs.capacity = 0;

            _semantic_analysis(node, &cs, scope); // type checking (simple)
            
            solve_constraints(&cs);
            _semantic_analysis(node, &cs, scope); // type checking (symbols)

            res = solve_constraints(&cs);

            // second round to get custom types
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
