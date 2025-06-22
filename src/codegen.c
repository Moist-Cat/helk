#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static void emit(CodegenContext* ctx, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(ctx->output, format, args);
    va_end(args);
}

char* joink_type(ASTNode* node) {
    if (node->type_info.kind == TYPE_STRING) {
        return "i8*";
    }
    else if (node->type_info.kind == TYPE_DOUBLE) {
        return "double";
    }
    else if (node->type_info.kind == TYPE_UNKNOWN) {
        fprintf(stderr, "WARNING - Type of node %d unknown during codegen\n", node->type);
        return "(unkown)";
    }
    else {
        //return node->type_info.name;
        return "i8*";
    }
}

static char* new_temp(CodegenContext* ctx) {
    char* temp = malloc(16);
    sprintf(temp, "%%t%d", ctx->temp_counter++);
    return temp;
}

static char* new_label(CodegenContext* ctx) {
    char* temp = malloc(16);
    sprintf(temp, "l%d", ctx->label_counter++);
    return temp;
}

static char* to_str_ptr(const char* name) {
    char* temp = malloc(17);
    sprintf(temp, "@%s", name);

    return temp;
}

static char* new_arg(const char* name) {
    char* temp = malloc(17);
    sprintf(temp, "%%%s", name);

    return temp;
}

char* detach_method(char* cls, char* method) {
    char* temp = malloc(16 + 16);
    sprintf(temp, "%s_%s", cls, method);
    return temp;
}


int get_total_memory(ASTNode** fields, unsigned int field_count) {
    unsigned int total = 0;
    for (unsigned int i = 0; i < field_count; i++) {
        // XXX double
        total += 8;
    }

    return total;
}

static void add_symbol(CodegenContext* ctx, const char* name, const char* temp, ASTNode* node) {
    // Check for existing symbol
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        if (strcmp(ctx->symbols[i].name, name) == 0) {
            fprintf(stderr, "Error: Redeclaration of '%s'\n", name);
            return;
        }
    }
    
    // Add new symbol
    ctx->symbols = realloc(ctx->symbols, (ctx->symbols_size + 1) * sizeof(Symbol));
    ctx->symbols[ctx->symbols_size] = (Symbol){
        .name = strdup(name),
        .temp = strdup(temp), // this changes after a redefinition
        .previous_label_name = strdup(temp),
        .phi = strdup(temp),
        .label = ctx->label_counter - 1, // this too
        .previous_label = ctx->label_counter - 1,
        // we use previous_* to identify and restore inconsitencies
        .node = node
    };
    ctx->symbols_size++;
}

CodegenContext* clone_codegen_context(const CodegenContext* original) {
    if (original == NULL) {
        return NULL;
    }

    // Allocate new context
    CodegenContext* clone = malloc(sizeof(CodegenContext));
    if (clone == NULL) {
        return NULL;
    }

    // Shallow copy simple members
    clone->output = original->output;
    clone->temp_counter = original->temp_counter;
    clone->label_counter = original->label_counter;
    clone->_last_merge = original->_last_merge;
    clone->current_label = original->current_label;
    clone->_last_merge_while = original->_last_merge_while;

    // Deep copy symbols array
    clone->symbols_size = original->symbols_size;
    if (original->symbols_size > 0) {
        clone->symbols = malloc(original->symbols_size * sizeof(Symbol));
        if (clone->symbols == NULL) {
            free(clone);
            return NULL;
        }

        // Copy each symbol, performing deep copies of strings
        for (size_t i = 0; i < original->symbols_size; i++) {
            clone->symbols[i].name = original->symbols[i].name ? strdup(original->symbols[i].name) : NULL;
            clone->symbols[i].temp = original->symbols[i].temp ? strdup(original->symbols[i].temp) : NULL;
            clone->symbols[i].previous_label_name = original->symbols[i].previous_label_name ? strdup(original->symbols[i].previous_label_name) : NULL;
            clone->symbols[i].phi = original->symbols[i].phi ? strdup(original->symbols[i].phi) : NULL;
            clone->symbols[i].label = original->symbols[i].label;
            clone->symbols[i].previous_label = original->symbols[i].previous_label;
            clone->symbols[i].node = original->symbols[i].node; // ASTNode doesn't need deep copy
        }
    } else {
        clone->symbols = NULL;
    }

    return clone;
}

static const char* find_symbol(CodegenContext* ctx, const char* name) {
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        if (strcmp(ctx->symbols[i].name, name) == 0) {
            return ctx->symbols[i].temp;
        }
    }
    return NULL;
}

Symbol* fetch_symbol(CodegenContext* ctx, const char* name) {
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        if (strcmp(ctx->symbols[i].name, name) == 0) {
            return &ctx->symbols[i];
        }
    }
    return NULL;
}

void fix_labels(CodegenContext* ctx) {
    /*
     * Values modified inside loops do not persist
     */
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        if (ctx->symbols[i].label != ctx->symbols[i].previous_label) {
            fprintf(stderr, "INFO - Fixing redefinition of %s after exiting loop", ctx->symbols[i].name);
            ctx->symbols[i].temp = strdup(ctx->symbols[i].previous_label_name);
        }
    }
}

static char* get_constructor_types(CodegenContext* ctx, ASTNode** args, unsigned int arg_count) {
    if (arg_count == 0) return strdup("");

    char** temps = malloc(arg_count * sizeof(char*));
    size_t total_len = 0;

    // Generate code for all arguments first
    for (size_t i = 0; i < arg_count; i++) {
        total_len += strlen(joink_type(args[i])) + 2;
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "%s%s", joink_type(args[i]), (i < arg_count-1) ? ", " : "");
        ptr += written;
    }

    free(temps);
    return result;
}

static char* get_call_args(CodegenContext* ctx, ASTNode** args, unsigned int arg_count) {
    if (arg_count == 0) return strdup("");

    char** temps = malloc(arg_count * sizeof(char*));
    size_t total_len = 0;

    // Generate code for all arguments first
    for (size_t i = 0; i < arg_count; i++) {
        temps[i] = gen_expr(ctx, args[i]);
        char* type = joink_type(args[i]);
        total_len += strlen(temps[i]) + strlen(type) + 3;
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "%s %s%s", joink_type(args[i]), temps[i], (i < arg_count-1) ? ", " : "");
        ptr += written;
    }

    return result;
}

static char* get_def_args(char** args, ASTNode** args_definitions, unsigned int arg_count) {
    if (arg_count == 0) return strdup("");

    char** temps = malloc(arg_count * sizeof(char*));
    size_t total_len = 0;

    // Generate code for all arguments first
    for (size_t i = 0; i < arg_count; i++) {
        temps[i] = args[i];
        char* type = joink_type(args_definitions[i]);
        total_len += strlen(temps[i]) + strlen(type) + 4;
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "%s %s%s%s", joink_type(args_definitions[i]) , "%", temps[i], (i < arg_count-1) ? ", " : "");
        ptr += written;
    }

    free(temps);
    return result;
}

static char* get_constructor_args(ASTNode** args, unsigned int arg_count) {
    if (arg_count == 0) return strdup("");

    char** temps = malloc(arg_count * sizeof(char*));
    size_t total_len = 0;

    // Generate code for all arguments first
    for (size_t i = 0; i < arg_count; i++) {
        temps[i] = args[i]->field_def.name;
        char* type = joink_type(args[i]);
        total_len += strlen(temps[i]) + strlen(type) + 5;
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "%s %s%s%s", joink_type(args[i]), "%", temps[i], (i < arg_count-1) ? ", " : "");
        ptr += written;
    }

    free(temps);
    return result;
}

// Generate vtable structure
void generate_vtable(CodegenContext* ctx, ASTNode* node) {
    // Define vtable structure type
    emit(ctx, "%%struct.%s_vtable = type {", node->type_decl.name);
    for (size_t i = 0; i < node->type_decl.method_count; i++) {
        if (i > 0) emit(ctx, ", ");
        ASTNode* method = node->type_decl.methods[i];
        emit(
            ctx,
            "%s (%s)*",
            joink_type(method),
            get_constructor_types(
                ctx,
                method->function_def.args_definitions,
                method->function_def.arg_count
            )
        );
    }
    emit(ctx, "}\n");

    // Create global vtable instance
    emit(ctx, "@%s_vtable = global %%struct.%s_vtable {", node->type_decl.name, node->type_decl.name);
    for (size_t i = 0; i < node->type_decl.method_count; i++) {
        if (i > 0) emit(ctx, ", ");
        char* mangled_name;
        if (node->type_decl.methods[i]->function_def.args_definitions[0]->type_info.kind != node->type_info.kind) {
            mangled_name = node->type_decl.methods[i]->function_def.name;
        }
        else {
            mangled_name = detach_method(node->type_decl.name,
                node->type_decl.methods[i]->function_def.name);
        }
        ASTNode* method = node->type_decl.methods[i];
        emit(
            ctx,
            "%s (%s)*",
            joink_type(method),
            get_constructor_types(
                ctx,
                method->function_def.args_definitions,
                method->function_def.arg_count
            )
        );
        emit(ctx, " @%s", mangled_name);
    }
    emit(ctx, "}\n");
}

void gen_method_call(CodegenContext* ctx, ASTNode* node) {
    /*
     * We have a (virtual) method call, we want to figure out if we
     * should generate a static call or a virtual call using
     * vtable lookups
     */
    emit(ctx, "\n  ; Start virtual method call\n");
    char* temp = new_temp(ctx);

    // Get object pointer from 'self'
    ASTNode* instance = node->method_call.cls;

    // generated twice
    char* obj_temp = gen_expr(ctx, node->method_call.args[0]);

    // Cast vtable pointer
    char* vtable_ptr_temp = new_temp(ctx);
    char* cls = instance->type_info.cls;
    emit(ctx, "  %s = bitcast i8* %s to %%struct.%s*\n",
         vtable_ptr_temp, obj_temp, cls);

    // Load vtable
    char* vtable_temp = new_temp(ctx);
    emit(ctx, "  %s = getelementptr %%struct.%s, %%struct.%s* %s, i32 0, i32 0\n",
         vtable_temp, cls, cls, vtable_ptr_temp);
    emit(ctx, "  %s_vtable = load %%struct.%s_vtable*, %%struct.%s_vtable** %s\n",
         vtable_temp, cls, cls, vtable_temp);

    // Get method pointer
    int method_index = node->method_call.pos;
    char* method_ptr_temp = new_temp(ctx);
    emit(ctx, "  %s = getelementptr %%struct.%s_vtable, %%struct.%s_vtable* %s_vtable, i32 0, i32 %d\n",
         method_ptr_temp, cls, cls, vtable_temp, method_index);

    // Load function pointer
    char* func_ptr_temp = new_temp(ctx);
    emit(ctx, "  %s = load ", func_ptr_temp);
    emit(
        ctx,
        "%s (%s)*",
        joink_type(node),
        get_constructor_types(
            ctx,
            node->function_def.args_definitions,
            node->function_def.arg_count
        )
    );
    emit(ctx, ", ");
    emit(
        ctx,
        "%s (%s)**",
        joink_type(node),
        get_constructor_types(
            ctx,
            node->function_def.args_definitions,
            node->function_def.arg_count
        )
    );
    emit(ctx, " %s\n", method_ptr_temp);

    node->method_call.method = func_ptr_temp;
    emit(ctx, "  ; End virtual method call\n\n");
    return;
}


static char* gen_while_loop(CodegenContext* ctx, ASTNode* node) {
    char* body_label = new_label(ctx);
    int body_cnt = ctx->label_counter - 1;

    // Initial unconditional branch to condition
    emit(ctx, "  ; While body\n", body_label);
    emit(ctx, "  br label %%%s\n", body_label);


    // Body block
    emit(ctx, "%s:\n", body_label);

    CodegenContext* new_ctx = clone_codegen_context(ctx);
    int wtemp = ctx->_last_merge_while;
    fprintf(stderr, "\n--------START-------\n");
    new_ctx->output = stderr;
    gen_expr(new_ctx, node->while_loop.body);

    fprintf(stderr, "\n--------END---------\n");
    fprintf(stderr, "----> merge_while: %d %d\n", ctx->_last_merge_while, new_ctx->_last_merge_while);
    if (wtemp != new_ctx->_last_merge_while) {
        ctx->_last_merge_while = new_ctx->_last_merge_while;
        exit(1);
    }
    else {
        ctx->_last_merge_while = new_ctx->label_counter;
    }
    fprintf(stderr, "----> merge_while: %d %d\n", ctx->_last_merge_while, new_ctx->_last_merge_while);

    // Shallow copy simple members
    //ctx->output = new_ctx->output;
    //ctx->temp_counter = new_ctx->temp_counter;
    //ctx->label_counter = new_ctx->label_counter;
    //ctx->_last_merge = new_ctx->_last_merge;
    //ctx->current_label = new_ctx->current_label;

    gen_redefs(ctx, node->while_loop.body);
    // set current label
    ctx->current_label = body_cnt;
    gen_expr(ctx, node->while_loop.body);
    //free(body_temp);

    // Condition block
    char* cond_temp = gen_expr(ctx, node->while_loop.cond);
    char* end_label = new_label(ctx);
    int end_cnt = ctx->label_counter - 1;
    emit(ctx, "  %%while_cond%d = fcmp one double %s, 0.000000e+00\n", ctx->temp_counter, cond_temp);
    emit(ctx, "  br i1 %%while_cond%d, label %%%s, label %%%s\n", ctx->temp_counter,
        body_label, end_label);
    //free(cond_temp);

    // End block
    emit(ctx, "  ; End while\n", end_label);
    emit(ctx, "%s:\n", end_label);
    // set current label
    ctx->current_label = end_cnt;

    char* result = new_temp(ctx);
    emit(ctx, "  ; Dummy value\n", result);
    emit(ctx, "  %s = fadd double 0.000000e+00, 0.000000e+00\n", result);



    // not needed
    // necessary
    ctx->_last_merge = end_cnt;

    // XXX verify this
    //fix_labels(ctx);

    return result;
}


static char* gen_conditional(CodegenContext* ctx, ASTNode* node) {
    int _last_merge = ctx->_last_merge;

    char* hyp_temp = gen_expr(ctx, node->conditional.hypothesis);

    char* thesis_label = new_label(ctx);
    int thesis_cnt = ctx->label_counter - 1;

    char* anti_label = new_label(ctx);
    int anti_cnt = ctx->label_counter - 1;

    // Compare condition to 0 (false)
    emit(ctx, "  %%cond%d = fcmp one double %s, 0.000000e+00\n", ctx->temp_counter++, hyp_temp);
    emit(ctx, "  br i1 %%cond%d, label %%%s, label %%%s\n\n",
        ctx->temp_counter-1, thesis_label, anti_label);

    // Thesis block
    emit(ctx, "%s:\n", thesis_label);
    // update current label
    ctx->current_label = thesis_cnt;
    char* thesis_temp = gen_expr(ctx, node->conditional.thesis);
    char* merge_label = new_label(ctx);
    int merge_cnt = ctx->label_counter - 1;
    emit(ctx, "  br label %%%s\n\n", merge_label);

    // the merge point might have changed so we have to check
    if (_last_merge != ctx->_last_merge) {
        thesis_cnt = ctx->_last_merge;
        _last_merge = ctx->_last_merge;
    }

    // Antithesis block
    emit(ctx, "%s:\n", anti_label);
    // update current label
    ctx->current_label = anti_cnt;
    char* anti_temp = gen_expr(ctx, node->conditional.antithesis);
    emit(ctx, "  br label %%%s\n\n", merge_label);

    // Verify if we generated a new merge point inside the antithesis
    if (_last_merge != ctx->_last_merge) {
        anti_cnt = ctx->_last_merge;
        _last_merge = ctx->_last_merge;
    }

    emit(ctx, "  ; Conditional merge point\n");
    emit(ctx, "%s:\n", merge_label);
    // update current label
    ctx->current_label = merge_cnt;
    char* result_temp = new_temp(ctx);
    emit(ctx, "  %s = phi %s [ %s, %%l%d ], [ %s, %%l%d ]\n",
        result_temp, joink_type(node), thesis_temp, thesis_cnt, anti_temp, anti_cnt);

    free(hyp_temp);
    free(thesis_temp);
    free(anti_temp);

    ctx->_last_merge = merge_cnt;
    ctx->_last_merge_while = merge_cnt;

    return result_temp;
}

// notice how we mimic how the parser parses stuff here in codegen
static char* gen_expr(CodegenContext* ctx, ASTNode* node) {
    /* Generate an expression.
     * Everything here returns something (a temp variable)
     *
     */
    char* temp;

    switch (node->type) {
        case AST_NUMBER:
            temp = new_temp(ctx);
            emit(ctx, "  %s = fadd double 0.000000e+00, %f\n", temp, node->number);
            return temp;
        case AST_STRING: {
            const char* var_temp = find_symbol(ctx, node->string);
            if (!var_temp) {
                fprintf(stderr, "ERROR - Undefined string '%s'\n", node->string);
                return NULL;
            }

            return var_temp;
        }
            
        case AST_BINARY_OP: {
            char* left = gen_expr(ctx, node->binary_op.left);
            char* right = gen_expr(ctx, node->binary_op.right);
            const char* op = NULL;
            
            switch (node->binary_op.op) {
                case OP_ADD: op = "fadd"; break;
                case OP_SUB: op = "fsub"; break;
                case OP_MUL: op = "fmul"; break;
                case OP_DIV: op = "fdiv"; break;
                case OP_MOD: op = "frem"; break;
            }
            

            temp = new_temp(ctx);
            emit(ctx, "  %s = %s double %s, %s\n", temp, op, left, right);
 
            //free(left);  // variable (const str)!
            //free(right);
            return temp;
        }            
        case AST_VARIABLE: {
            Symbol* symbol = fetch_symbol(ctx, node->variable.name);
            emit(ctx, "  ; Load variable %s (%s)\n", node->variable.name, symbol->temp);
            if (symbol->temp == NULL) {
                exit(1);
            }
            return symbol->temp;
        }
        case AST_VARIABLE_DEF: {
            Symbol* symbol = fetch_symbol(ctx, node->variable_def.name);
            char* t4;

            if (symbol) {
                if (symbol->label == ctx->label_counter - 1) {
                    fprintf(
                        stderr,
                        "WARNING - Dangerous redefinition detected (%s). The variable now points to a new temp var.\n",
                        symbol->name
                    );

                    symbol->temp = gen_expr(ctx, node->variable_def.body);
                    /// XXX reassign
                    //emit(ctx, "  %s = fadd double %s, 0.000000e+00  ; Load variable\n", temp, symbol->temp);
                    //return symbol->temp;
                    return symbol->name;
                }
                t4 = gen_expr(ctx, node->variable_def.body);

                // no-op to make t3 = t4 since we don't know how many operations we will make
                // XXX double
                emit(ctx, "  %s = fadd double %s, 0.000000e+00  ; Load variable\n", symbol->phi, t4);

                symbol->temp = symbol->phi;
                // do not free anything here
            }
            else {
                t4 = gen_expr(ctx, node->variable_def.body);
                add_symbol(ctx, node->variable_def.name, t4, node);
                emit(
                    ctx, "  ; Variable assignment: %s = %s\n", 
                    node->variable_def.name, t4
                );
            }
            return t4;
        }

        case AST_METHOD_CALL:
        case AST_FUNCTION_CALL: {
            char* temp = new_temp(ctx);


            size_t arg_count;
            char* call_args;
            if (node->type == AST_FUNCTION_CALL) {
                arg_count = node->function_call.arg_count;
                call_args = get_call_args(ctx, node->function_call.args, arg_count);
            }
            else {
                arg_count = node->method_call.arg_count;
                call_args = get_call_args(ctx, node->method_call.args, arg_count);
            }
            char* type = joink_type(node);

            if (node->type == AST_METHOD_CALL) {
                // pass self to method
                // this might modify the method name
                gen_method_call(ctx, node);
            }

            // XXX clone symbol table
            // add args
            char* name;
            if (node->type == AST_FUNCTION_CALL) {
                name = node->function_call.name;
            }
            else {
                name = node->method_call.method;
            }
            emit(ctx, "  %s = call %s %s%s(", temp, type, ((char) name[0] != '%') ? "@" : "", name);
            if (arg_count > 0) {
                emit(ctx, "%s", call_args);
            }
            emit(ctx, ")\n");
            
            free(call_args);

            return temp;
        }
        case AST_CONDITIONAL: {
            return gen_conditional(ctx, node);
        }
        case AST_WHILE_LOOP: {
            return gen_while_loop(ctx, node);
        }
        case AST_FIELD_ACCESS: {
            char* temp = new_temp(ctx);
            Symbol* symbol = fetch_symbol(ctx, node->field_access.cls);

            emit(
                ctx,
                "  %s_ref = bitcast i8* %s to %%struct.%s*\n",
                temp,
                symbol->temp,
                symbol->node->type_info.cls
            );


            emit(
                ctx,
                "  %s_ptr = getelementptr %%struct.%s, %s %s_ref, i32 0, i32 %d\n",
                temp,
                symbol->node->type_info.cls,
                symbol->node->type_info.name,
                temp,
                node->field_access.pos + 1 // vtable
            );
            emit(
                ctx,
                "  %s = load %s, %s* %s_ptr\n",
                temp,
                joink_type(node),
                joink_type(node),
                temp
            );
            fprintf(stderr, "node_type=%zu; field_type=%zu pos=%d\n", symbol->node->type_info.kind, node->type_info.kind, node->field_access.pos);
            return temp;
        }
        case AST_FIELD_REASSIGN: {
            fprintf(stderr, "Reassigning field \n");
            char* temp = gen_expr(ctx, node->field_reassign.field_access);
            emit(
                ctx,
                "  store %s %%%s, %s* %s_ptr\n",
                joink_type(node->field_reassign.field_access),
                node->field_reassign.value,
                joink_type(node->field_reassign.field_access),
                temp
            );
            return temp;
        }

        case AST_BLOCK: {
            CodegenContext* new_ctx = clone_codegen_context(ctx);
            char* temp = codegen_expr_block(new_ctx, node);
            // Shallow copy simple members
            ctx->output = new_ctx->output;
            ctx->temp_counter = new_ctx->temp_counter;
            ctx->label_counter = new_ctx->label_counter;
            ctx->_last_merge = new_ctx->_last_merge;
            ctx->current_label = new_ctx->current_label;
            return temp;
            //return codegen_expr_block(ctx, node);
        }
        default: {
            fprintf(stderr, "Error: Failed to parse %d because it is not an expression! \n", node->type);
            return NULL;
        }
    }
}

void gen_redefs(CodegenContext* ctx, ASTNode* node) {
    /* Generate phi for the redefinition
     *
     */
    
    switch (node->type) {
        case AST_BINARY_OP: {
            gen_redefs(ctx, node->binary_op.left);
            gen_redefs(ctx, node->binary_op.right);
            break;
        }            
        case AST_VARIABLE_DEF: {
            // notice that we perform NO operation here
            // since storing the value is handled by default
            // (we have to store whatever is inside in a temp anyway)
            // So we simply rename the variable.

            Symbol* symbol = fetch_symbol(ctx, node->variable_def.name);
            char* type;

            if (node->type_info.kind == TYPE_STRING) {
                type = "i8*";
            }
            else {
                type = "double";
            }

            if (symbol) {
                fprintf(stderr, "INFO - Redefinition detected: %s\n", node->variable_def.name);
                // https://www.cs.utexas.edu/~pingali/CS380C/2010/papers/ssaCytron.pdf
                //
                // I notice that the value was already defined
                // I have the assignment in hand (let's say is %t1)
                // If the label corresponding to the assigned value and our own are
                // the same, simply discard the previous label
                // If it's not the case, generate two labels one for the phi (since the value of
                // the variable could come either from the previous label or our own)
                // and
                // %t2 = phi(%t1, %t3)
                // %t3 = %t2 + 1
                // save %t3 in the symbol table

                // If we didn't define a new label
                // Then this becomes a no-op
                if (symbol->label == ctx->label_counter - 1) {
                    fprintf(stderr, "WARNING - Dangerous redefinition detected. No operation was made\n");
                    return;
                }
                // different labels
                emit(
                    ctx, "  ; Variable redefinition: %s\n", 
                    node->variable_def.name
                );

                char* t1 = symbol->temp;
                char* t2 = new_temp(ctx);
                char* t3 = new_temp(ctx);

                int lbl = ctx->current_label;
                int end;
                if (ctx->_last_merge_while != 0) {
                    end = ctx->_last_merge_while - 1;
                }
                else {
                    end = ctx->label_counter - 1;
                }
                emit(
                    ctx,
                    "  %s = phi %s [%s, %%l%d], [%s, %%l%d]\n",
                    //t2, type, t1, lbl, t3, ctx->label_counter - 1
                    t2, type, t1, lbl, t3, end
                );

                // probably uses the variable (or another variable, recall the gcd algorithm)
                // Whenever "a" is searched in the symbol table, it will appear as t2
                symbol->temp = t2;

                // we are forced to defer the operation
                symbol->phi = t3;
                // do not free anything here
            }
            break;
        }

        case AST_CONDITIONAL: {
            gen_redefs(ctx, node->conditional.thesis);
            gen_redefs(ctx, node->conditional.antithesis);
            break;
        }
        case AST_WHILE_LOOP: {
            // not my problem
            break;
        }
        case AST_BLOCK: {
            // Return zero by defaul
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                gen_redefs(ctx, node->block.statements[i]);
            }
            break;
        }
        default: {
            return;
        }
    }
}

void codegen_stmt(CodegenContext* ctx, ASTNode* node) {
    /* purely functional lang */

    if ((node->type == AST_FUNCTION_DEF) || (node->type == AST_METHOD_DEF)) {
        int enabled = 0;
        if (!node->function_def.called && enabled) {
            fprintf(stderr, "WARNING - %s function was never called so it won't be generated\n", node->function_def.name);
            return;
        }
        // should ONLY contain functions after sem_anal

        CodegenContext* fun_ctx = clone_codegen_context(ctx);

        char* type = joink_type(node);

        char* entry_label = new_label(fun_ctx);
        int entry_cnt = fun_ctx->label_counter - 1;
        unsigned int arg_count = node->function_def.arg_count;

        for (unsigned int i = 0; i < arg_count; i++) {
            add_symbol(
                fun_ctx,
                node->function_def.args[i],
                new_arg(
                    node->function_def.args[i]
                ),
                node->function_def.args_definitions[i]
            );
        }

        char* def_args = get_def_args(
            node->function_def.args,
            node->function_def.args_definitions,
            arg_count
        );

        // special case for fun main
        if (strcmp(node->function_def.name, "main") == 0) {
            emit(fun_ctx, "\ndefine i32 @main() {\n", type, node->function_def.name, def_args);
        }
        else if (arg_count > 0) {
            emit(fun_ctx, "\ndefine %s @%s(%s) {\n", type, node->function_def.name, def_args);
        }
        else {
            emit(fun_ctx, "\ndefine %s @%s() {\n", type, node->function_def.name);
        }
        emit(fun_ctx, "%s:\n", entry_label);
        fun_ctx->current_label = entry_cnt;

        char* result = gen_expr(fun_ctx, node->function_def.body);


        if (strcmp(node->function_def.name, "main") == 0) {
            emit(fun_ctx, "  ret i32 0\n", type, result);
            free(result);
        }
        else if (result) {
            emit(fun_ctx, "  ret %s %s\n", type, result);
            free(result);
        }
        else {
            // panik
            fprintf(stderr, "ERROR - Function `%s` has no return value!\n", node->function_def.name);
            exit(1);
        }
        
        emit(fun_ctx, "}\n");
    }
   else if (node->type == AST_TYPE_DEF) {
        // ... with types
        char* types = get_constructor_types(ctx, node->type_decl.fields, node->type_decl.field_count);
        int total_memory = get_total_memory(node->type_decl.fields, node->type_decl.field_count);
        char* constructor_args = get_constructor_args(node->type_decl.fields, node->type_decl.field_count);

        generate_vtable(ctx, node);

        // define struct
        if (node->type_decl.field_count == 0) {
            emit(ctx, "%%struct.%s = type { %%struct.%s_vtable* }\n", node->type_decl.name, node->type_decl.name);
        }
        else {
            emit(ctx, "%%struct.%s = type { %%struct.%s_vtable*, %s }\n", node->type_decl.name, node->type_decl.name, types);
        }
        emit(ctx, "\n");


        // define constructor
        emit(
            ctx,
            "define i8* @%s_constructor(%s) {\n",
            node->type_decl.name,
            constructor_args
        );
        emit(ctx, "  %%heap_ptr = call i8* @malloc(i32 %d)\n", total_memory);
        emit(ctx, "  %%obj_ptr = bitcast i8* %%heap_ptr to %%struct.%s*\n", node->type_decl.name);

        // Set vtable pointer
        emit(ctx, "\n");
        emit(ctx, "  %%vtable_ptr = getelementptr %%struct.%s, %%struct.%s* %%obj_ptr, i32 0, i32 0\n",
             node->type_decl.name, node->type_decl.name);
        emit(ctx, "  store %%struct.%s_vtable* @%s_vtable, %%struct.%s_vtable** %%vtable_ptr\n",
             node->type_decl.name, node->type_decl.name, node->type_decl.name);
        emit(ctx, "\n");

        for (size_t i = 0; i < node->type_decl.field_count; i++) {
            size_t field_index = i + 1; // +1 for vtable pointer
            emit(
                ctx,
                "  %%%s_ptr = getelementptr %%struct.%s, %%struct.%s* %%obj_ptr, i32 0, i32 %zu\n",
                node->type_decl.fields[i]->field_def.name,
                node->type_decl.name,
                node->type_decl.name,
                field_index
            );
            emit(
                ctx,
                "  store %s %%%s, %s* %%%s_ptr\n",
                joink_type(node->type_decl.fields[i]),
                node->type_decl.fields[i]->field_def.name,
                joink_type(node->type_decl.fields[i]),
                node->type_decl.fields[i]->field_def.name
            );
        }
        emit(ctx, "  ret i8* %%heap_ptr\n");
        emit(ctx, "}\n", node->type_decl.name);

        for (size_t i = 0; i < node->type_decl.method_count; i++) {
            if (node->type_decl.methods[i]->function_def.args_definitions[0]->type_info.kind == node->type_info.kind) {
                node->type_decl.methods[i]->function_def.name = detach_method(
                    node->type_decl.name,
                    node->type_decl.methods[i]->function_def.name
                );
                codegen_stmt(ctx, node->type_decl.methods[i]);
            }
        }
    }
    else {
        char* temp = gen_expr(ctx, node);
        free(temp);
    }
}

void codegen_block(CodegenContext* ctx, ASTNode* node) {
    // weeeeeeeeeee
    for (size_t i = 0; i < node->block.stmt_count; i++) {
        codegen_stmt(ctx, node->block.statements[i]);
    }
}

static char* codegen_expr_block(CodegenContext* ctx, ASTNode* node) {
    /* We assume that whatever is inside this block is an expression */
    char* temp = NULL;

    for (size_t i = 0; i < node->block.stmt_count; i++) {
        // weeeeeeeeeeeeee memory leeeeeeeaks
        temp = gen_expr(ctx, node->block.statements[i]);
    }
    return temp;
}

void _codegen_declarations(CodegenContext* ctx, ASTNode *node) {
    if (!node) {return;}

    fprintf(stderr, "Collecting declarations for node_type=%d \n", node->type);

    switch (node->type) {
        case AST_BLOCK: {
            for (size_t i = 0; i < node->block.stmt_count; i++) {
                _codegen_declarations(ctx, node->block.statements[i]);
            }
            break;
        }
        case AST_STRING: {
            char* escaped = node->string;
            int length = strlen(escaped) + 1; // null-terminated
            char* temp = new_label(ctx);
            
            emit(ctx, "@.str.%s = private unnamed_addr constant [%d x i8] c\"%s\\00\", align 1\n", temp, length, escaped);

            
            emit(ctx,
                "@%s = alias i8, getelementptr inbounds ([%d x i8], [%d x i8]* @.str.%s, i64 0, i64 0)\n", temp, length, length, temp
           );


            const char* str_ptr = to_str_ptr(temp);

            add_symbol(ctx, escaped, str_ptr, node);

            free(str_ptr);
            break;
        }
        case AST_BINARY_OP: {
            _codegen_declarations(ctx, node->binary_op.left);
            _codegen_declarations(ctx, node->binary_op.right);
            break;
        }          
        case AST_VARIABLE_DEF: {
            _codegen_declarations(ctx, node->variable_def.body);
            break;
        }

        case AST_FUNCTION_CALL: {
            for (size_t i = 0; i < node->function_call.arg_count; i++) {
                fprintf(stderr, "%s %p", node->function_call.name, node->function_call.args[i]);
                _codegen_declarations(ctx, node->function_call.args[i]);
            }
            break;
        }
        case AST_CONDITIONAL: {
            _codegen_declarations(ctx, node->conditional.hypothesis);
            _codegen_declarations(ctx, node->conditional.thesis);
            _codegen_declarations(ctx, node->conditional.antithesis);
            break;
        }
        case AST_WHILE_LOOP: {
            _codegen_declarations(ctx, node->while_loop.cond);
            _codegen_declarations(ctx, node->while_loop.body);
            break;
        }

        case AST_FUNCTION_DEF: {
            _codegen_declarations(ctx, node->function_def.body);
            break;
        }
        case AST_TYPE_DEF: {
            // constructor && 
            for (size_t i = 0; i < node->type_decl.method_count; i++) {
                _codegen_declarations(ctx, node->type_decl.methods[i]);
            }
            for (size_t i = 0; i < node->type_decl.field_count; i++) {
                _codegen_declarations(ctx, node->type_decl.fields[i]);
            }
            break;
        }
        default: {
            break;         
        }

    }
}

void codegen_declarations(CodegenContext* ctx, ASTNode *root) {
    emit(ctx, "; ModuleID = 'memelang'\n");
    emit(ctx, "declare double @max(double, double)\n");
    emit(ctx, "declare double @min(double, double)\n");
    emit(ctx, "declare double @pow(double, double)\n");

    emit(ctx, "declare double @print(double)\n");
    emit(ctx, "declare double @prints(i8* nocapture) nounwind\n");
    emit(ctx, "declare i8* @malloc(i32)\n");
    emit(ctx, "declare void @free(i8*)\n");

    _codegen_declarations(ctx, root);
    emit(ctx, "\n");
}

void codegen(CodegenContext* ctx, ASTNode* node) {
    // only statement blocks for now
    fprintf(stderr, "INFO - Generating LLVM IR code\n");

    codegen_declarations(ctx, node);

    switch (node->type) {
        case AST_BLOCK: {
            codegen_block(ctx, node);
            break;
        }
        default: {
            break;
        }
    }
}

void codegen_init(CodegenContext* ctx, FILE* output) {
    ctx->output = output;
    ctx->temp_counter = 0;
    ctx->label_counter = 0;
    ctx->_last_merge = 0;
    ctx->symbols = NULL;
    ctx->symbols_size = 0;
}

void codegen_cleanup(CodegenContext* ctx) {
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        free(ctx->symbols[i].name);
        free(ctx->symbols[i].temp);
    }
    free(ctx->symbols);
}
