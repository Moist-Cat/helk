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
        if (ctx->symbols[i].label  != ctx->symbols[i].previous_label) {
            fprintf(stderr, "INFO - Fixing redefinition of %s after exiting loop", ctx->symbols[i].name);
            free(ctx->symbols[i].temp);
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
        total_len += 9; // XXX "double , " per argument
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written;
        if (args[i]->type_info.kind == TYPE_STRING) {
            written = sprintf(ptr, "i8*%s", (i < arg_count-1) ? ", " : "");
        }
        else {
            written = sprintf(ptr, "double%s", (i < arg_count-1) ? ", " : "");
        }
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
        total_len += strlen(temps[i]) + 9; // XXX "double , " per argument
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written;
        if (args[i]->type_info.kind == TYPE_STRING) {
            written = sprintf(ptr, "i8* %s%s", temps[i], (i < arg_count-1) ? ", " : "");
        }
        else {
            written = sprintf(ptr, "double %s%s", temps[i], (i < arg_count-1) ? ", " : "");
        }
        ptr += written;
    }

    free(temps);
    return result;
}

static char* get_def_args(char** args, unsigned int arg_count) {
    if (arg_count == 0) return strdup("");

    char** temps = malloc(arg_count * sizeof(char*));
    size_t total_len = 0;

    // Generate code for all arguments first
    for (size_t i = 0; i < arg_count; i++) {
        temps[i] = args[i];
        total_len += strlen(temps[i]) + 9; // XXX "double , " per argument
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "double %s%s%s", "%", temps[i], (i < arg_count-1) ? ", " : "");
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
        total_len += strlen(temps[i]) + 9; // XXX "double , " per argument
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "double %s%s%s", "%", temps[i], (i < arg_count-1) ? ", " : "");
        ptr += written;
    }

    free(temps);
    return result;
}

static char* gen_while_loop(CodegenContext* ctx, ASTNode* node) {
    char* body_label = new_label(ctx);

    // Initial unconditional branch to condition
    emit(ctx, "  br label %%%s\n", body_label);

    // Body block
    emit(ctx, "%s:\n", body_label);
    gen_redefs(ctx, node->while_loop.body);
    char* body_temp = gen_expr(ctx, node->while_loop.body);
    free(body_temp);

    // Condition block
    char* cond_temp = gen_expr(ctx, node->while_loop.cond);
    char* end_label = new_label(ctx);
    emit(ctx, "  %%while_cond = fcmp one double %s, 0.000000e+00\n", cond_temp);
    emit(ctx, "  br i1 %%while_cond, label %%%s, label %%%s\n",
        body_label, end_label);
    free(cond_temp);

    // End block
    emit(ctx, "%s:\n", end_label);

    // Return dummy value (0.0)
    char* result = new_temp(ctx);
    emit(ctx, "  %s = fadd double 0.000000e+00, 0.000000e+00\n", result);

    // XXX verify this
    ///fix_labels(ctx);
    return result;
}


static char* gen_conditional(CodegenContext* ctx, ASTNode* node) {
    int _last_merge = ctx->_last_merge;

    char* thesis_label = new_label(ctx);
    int thesis_cnt = ctx->label_counter - 1;

    char* anti_label = new_label(ctx);
    int anti_cnt = ctx->label_counter - 1;

    char* merge_label = new_label(ctx);
    int merge_cnt = ctx->label_counter - 1;

    char* hyp_temp = gen_expr(ctx, node->conditional.hypothesis);

    // Compare condition to 0 (false)
    emit(ctx, "  %%cond%d = fcmp one double %s, 0.000000e+00\n", ctx->temp_counter++, hyp_temp);
    emit(ctx, "  br i1 %%cond%d, label %%%s, label %%%s\n\n",
        ctx->temp_counter-1, thesis_label, anti_label);

    // Thesis block
    emit(ctx, "%s:\n", thesis_label);
    char* thesis_temp = gen_expr(ctx, node->conditional.thesis);
    emit(ctx, "  br label %%%s\n\n", merge_label);

    // the merge point might have changed so we have to check
    if (_last_merge != ctx->_last_merge) {
        fprintf(stderr, "blob %d %d\n", ctx->label_counter, thesis_cnt);
        thesis_cnt = ctx->_last_merge;
        _last_merge = ctx->_last_merge;
    }

    // Antithesis block
    emit(ctx, "%s:\n", anti_label);
    char* anti_temp = gen_expr(ctx, node->conditional.antithesis);
    emit(ctx, "  br label %%%s\n\n", merge_label);

    // Verify if we generated a new merge point inside the antithesis
    if (_last_merge != ctx->_last_merge) {
        fprintf(stderr, "blob %d %d\n", ctx->label_counter, anti_cnt);
        anti_cnt = ctx->_last_merge;
        _last_merge = ctx->_last_merge;
    }

    // Merge point
    emit(ctx, "%s:\n", merge_label);
    char* result_temp = new_temp(ctx);
    //emit(ctx, "  %s = phi double [ %s, %%%s ], [ %s, %%%s ]\n",
    //    result_temp, thesis_temp, thesis_label, anti_temp, anti_label);
    emit(ctx, "  %s = phi double [ %s, %%l%d ], [ %s, %%l%d ]\n",
        result_temp, thesis_temp, thesis_cnt, anti_temp, anti_cnt);

    free(hyp_temp);
    free(thesis_temp);
    free(anti_temp);

    ctx->_last_merge = merge_cnt;

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
            }
            

            temp = new_temp(ctx);
            emit(ctx, "  %s = %s double %s, %s\n", temp, op, left, right);
 
            //free(left);  // variable (const str)!
            //free(right);
            return temp;
        }            
        case AST_VARIABLE: {
            const char* var_temp = find_symbol(ctx, node->variable.name);
            if (!var_temp) {
                fprintf(stderr, "Could not find symbol %s!\n", node->variable.name);
                return new_arg(node->variable.name);
            }
            emit(ctx, "  ; Load variable %s\n", node->variable.name);
            return var_temp;
        }
        case AST_VARIABLE_DEF: {
            Symbol* symbol = fetch_symbol(ctx, node->variable_def.name);
            char* type;
            char* t4;

            // XXX do not reassign strings!
            if (node->type_info.kind == TYPE_STRING) {
                type = "i8*";
            }
            else {
                type = "double";
            }

            if (symbol) {
                if (symbol->label == ctx->label_counter - 1) {
                    fprintf(
                        stderr,
                        "WARNING - Invalid redefinition detected (%s). No operation was made\n",
                        symbol->name
                    );
                    return symbol->name;
                }
                t4 = gen_expr(ctx, node->variable_def.body);

                // no-op to make t3 = t4 since we don't know how many operations we will make
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

        case AST_FUNCTION_CALL: {
            char* temp = new_temp(ctx);

            size_t arg_count = node->function_call.arg_count;

            char* call_args = get_call_args(ctx, node->function_call.args, arg_count);
            char* type;

            if (node->type_info.kind == TYPE_STRING) {
                type = "i8*";
            }
            else if (node->type_info.kind == TYPE_DOUBLE) {
                type = "double";
            }
            else if (node->type_info.kind == TYPE_UNKNOWN) {
                fprintf(
                    stderr,
                    "WARNING - Type for node type=%d is unknown during codegen",
                    node->type
                );
                type = "double";
            }
            else {
                type = node->type_info.name;
            }

            if (arg_count > 0) {
                emit(ctx, "  %s = call %s @%s(%s)\n", temp, type, node->function_call.name, call_args);
            }
            else {
                emit(ctx, "  %s = call %s @%s()\n", temp, type, node->function_call.name);
            }
            
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
                "  %s_ptr = getelementptr %%struct.%s, %s %s, i32 0, i32 %d\n",
                temp,
                symbol->node->type_info.cls,
                symbol->node->type_info.name,
                symbol->temp,
                node->field_access.pos
            );
            emit(
                ctx,
                "  %s = load double, double* %s_ptr\n",
                temp,
                temp
            );
            return temp;
        }
        case AST_BLOCK: {
            return codegen_expr_block(ctx, node);
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
                    fprintf(stderr, "WARNING - Invalid redefinition detected. No operation was made\n");
                    return;
                }
                // different labels
                emit(
                    ctx, "  ; Variable redefiniton: %s\n", 
                    node->variable_def.name
                );

                char* t1 = symbol->temp;
                char* t2 = new_temp(ctx);
                char* t3 = new_temp(ctx);

                emit(
                    ctx,
                    "  %s = phi %s [%s, %%l%d], [%s, %%l%d]\n",
                    t2, type, t1, symbol->label, t3, ctx->label_counter - 1
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
            // XXX
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

    if (node->type == AST_FUNCTION_DEF) {
        // should ONLY contain functions after sem_anal

        unsigned int arg_count = node->function_def.arg_count;

        char* def_args = get_def_args(node->function_def.args, arg_count);
        char* entry_label = new_label(ctx);

        if (arg_count > 0) {
            emit(ctx, "\ndefine double @%s(%s) {\n", node->function_def.name, def_args);
        }
        else {
            emit(ctx, "\ndefine double @%s() {\n", node->function_def.name);
        }
        emit(ctx, "%s:\n", entry_label);
        

        char* result = gen_expr(ctx, node->function_def.body);
        if (result) {
            emit(ctx, "  ret double %s\n", result);
            free(result);
        }
        else {
            // panik
            fprintf(stderr, "ERROR - Function `%s` has no return value!\n", node->function_def.name);
            exit(1);
        }
        
        emit(ctx, "}\n");
    }
   else if (node->type == AST_TYPE_DEF) {
        // ... with types
        char* types = get_constructor_types(ctx, node->type_decl.fields, node->type_decl.field_count);
        int total_memory = get_total_memory(node->type_decl.fields, node->type_decl.field_count);
        char* constructor_args = get_constructor_args(node->type_decl.fields, node->type_decl.field_count);
        // define struct
        emit(ctx, "%%struct.%s = type { %s }\n", node->type_decl.name, types);


        // define constructor
        emit(
            ctx,
            "define %%struct.%s* @%s_constructor(%s) {\n",
            node->type_decl.name,
            node->type_decl.name,
            constructor_args
        );
        emit(ctx, "  %%heap_ptr = call i8* @malloc(i32 %d)\n", total_memory);
        emit(ctx, "  %%obj_ptr = bitcast i8* %%heap_ptr to %%struct.%s*\n", node->type_decl.name);

        for (size_t i = 0; i < node->type_decl.field_count; i++) {
            emit(
                ctx,
                "  %%%s_ptr = getelementptr %%struct.%s, %%struct.%s* %%obj_ptr, i32 0, i32 %d\n",
                node->type_decl.fields[i]->field_def.name,
                node->type_decl.name,
                node->type_decl.name,
                i
            );
            // XXX double
            emit(
                ctx,
                "  store double %%%s, double* %%%s_ptr\n",
                node->type_decl.fields[i]->field_def.name,
                node->type_decl.fields[i]->field_def.name
            );
        }
        emit(ctx, "  ret %%struct.%s* %%obj_ptr\n", node->type_decl.name);
        emit(ctx, "}\n", node->type_decl.name);

        for (size_t i = 0; i < node->type_decl.method_count; i++) {
            node->type_decl.methods[i]->function_def.name = detach_method(
                node->type_decl.name,
                node->type_decl.methods[i]->function_def.name
            );
            codegen_stmt(ctx, node->type_decl.methods[i]);
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
                _codegen_declarations(ctx, node->function_call.args[i]);
            }
            break;
        }
        case AST_FUNCTION_DEF: {
            _codegen_declarations(ctx, node->function_def.body);
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
