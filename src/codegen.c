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
    sprintf(temp, "l%d", ctx->temp_counter++);
    return temp;
}

static void add_symbol(CodegenContext* ctx, const char* name, const char* temp) {
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
        .temp = strdup(temp)
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
        int written = sprintf(ptr, "double %s%s", temps[i], (i < arg_count-1) ? ", " : "");
        ptr += written;
        free(temps[i]);
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
        total_len += strlen(temps[i]) + 6; // XXX "double , " per argument
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
    char* cond_label = new_label(ctx);
    char* body_label = new_label(ctx);
    char* end_label = new_label(ctx);

    // Initial unconditional branch to condition
    emit(ctx, "  br label %%%s\n", cond_label);

    // Condition block
    emit(ctx, "%s:\n", cond_label);
    char* cond_temp = gen_expr(ctx, node->while_loop.cond);
    emit(ctx, "  %%while_cond = fcmp one double %s, 0.000000e+00\n", cond_temp);
    emit(ctx, "  br i1 %%while_cond, label %%%s, label %%%s\n",
        body_label, end_label);
    free(cond_temp);

    // Body block
    emit(ctx, "%s:\n", body_label);
    char* body_temp = gen_expr(ctx, node->while_loop.body);
    free(body_temp);
    emit(ctx, "  br label %%%s\n", cond_label);  // Loop back

    // End block
    emit(ctx, "%s:\n", end_label);

    // Return dummy value (0.0)
    char* result = new_temp(ctx);
    emit(ctx, "  %s = fadd double 0.000000e+00, 0.000000e+00\n", result);
    return result;
}


static char* gen_conditional(CodegenContext* ctx, ASTNode* node) {
    char* hyp_temp = gen_expr(ctx, node->conditional.hypothesis);
    char* thesis_label = new_label(ctx);
    char* anti_label = new_label(ctx);
    char* merge_label = new_label(ctx);

    // Compare condition to 0 (false)
    emit(ctx, "  %%cond%d = fcmp one double %s, 0.000000e+00\n", ctx->temp_counter++, hyp_temp);
    emit(ctx, "  br i1 %%cond%d, label %%%s, label %%%s\n\n",
        ctx->temp_counter-1, thesis_label, anti_label);

    // Thesis block
    emit(ctx, "%s:\n", thesis_label);
    char* thesis_temp = gen_expr(ctx, node->conditional.thesis);
    emit(ctx, "  br label %%%s\n\n", merge_label);

    // Antithesis block
    emit(ctx, "%s:\n", anti_label);
    char* anti_temp = gen_expr(ctx, node->conditional.antithesis);
    emit(ctx, "  br label %%%s\n\n", merge_label);

    // Merge point
    emit(ctx, "%s:\n", merge_label);
    char* result_temp = new_temp(ctx);
    emit(ctx, "  %s = phi double [ %s, %%%s ], [ %s, %%%s ]\n",
        result_temp, thesis_temp, thesis_label, anti_temp, anti_label);

    free(hyp_temp);
    free(thesis_temp);
    free(anti_temp);

    return result_temp;
}

// notice how we mimic how the parser parses stuff here in codegen
static char* gen_expr(CodegenContext* ctx, ASTNode* node) {
    /* Generate an expression.
     * Everything here returns something (a temp variable)
     *
     */
    char* temp = new_temp(ctx);
    
    switch (node->type) {
        case AST_NUMBER:
            emit(ctx, "  %s = fadd double 0.000000e+00, %f\n", temp, node->number);
            return temp;
            
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
            
            emit(ctx, "  %s = %s double %s, %s\n", temp, op, left, right);
            free(left);
            free(right);
            return temp;
        }            
        case AST_VARIABLE: {
            const char* var_temp = find_symbol(ctx, node->variable.name);
            if (!var_temp) {
                fprintf(stderr, "WARNING: Undefined variable '%s'\n", node->variable.name);

                emit(ctx, "  %s = fadd double %s%s, 0.000000e+00  ; Load variable\n", temp, "%", node->variable.name);
                return temp;
            }
            emit(ctx, "  %s = fadd double %s, 0.000000e+00 ; Load variable\n", temp, var_temp);
            return temp;
        }
        case AST_VARIABLE_DEF: {
            // notice that we perform NO operation here
            // since storing the value is handled by default
            // (we have to store whatever is inside in a temp anyway)
            // So we simply rename the variable.
            char* value_temp = gen_expr(ctx, node->variable_def.body);
            add_symbol(ctx, node->variable_def.name, value_temp);
            emit(ctx, "  ; Variable assignment: %s = %s\n", 
                node->variable_def.name, value_temp);
            //free(value_temp);
            free(temp); // not needed
            return value_temp;
        }

        case AST_FUNCTION_CALL: {
            char* temp = new_temp(ctx);

            size_t arg_count = node->function_call.arg_count;

            char* call_args = get_call_args(ctx, node->function_call.args, arg_count);

            if (arg_count > 0) {
                emit(ctx, "  %s = call double @%s(%s)\n", temp, node->function_call.name, call_args);
            }
            else {
                emit(ctx, "  %s = call double @%s()\n", temp, node->function_call.name);
            }
            
            free(call_args);

            return temp;
        }
        case AST_CONDITIONAL: {
            free(temp);
            return gen_conditional(ctx, node);
        }
        case AST_WHILE_LOOP: {
            free(temp);
            return gen_while_loop(ctx, node);
        }
        case AST_BLOCK: {
            // Return zero by default
            return codegen_expr_block(ctx, node);
            //emit(ctx, "  %s = fadd double %f, 0.000000e+00\n", temp, ret_value);
            //free(ret_value);
            //return temp;
        }
        default:
            fprintf(stderr, "Error: Failed to parse %d because it is not an expression! \n", node->type);
            free(temp);
            return NULL;
    }
}

void codegen_stmt(CodegenContext* ctx, ASTNode* node) {
    /* purely functional lang */
    switch (node->type) {
        case AST_FUNCTION_DEF: {
            // should ONLY contain functions after sem_anal

            unsigned int arg_count = node->function_def.arg_count;

            char* def_args = get_def_args(node->function_def.args, arg_count);

            if (arg_count > 0) {
                emit(ctx, "\ndefine double @%s(%s) {\n", node->function_def.name, def_args);
            }
            else {
                emit(ctx, "\ndefine double @%s() {\n", node->function_def.name);
            }
            emit(ctx, "entry:\n");
            
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
            break;
        }
        default: {
            char* temp = gen_expr(ctx, node);
            free(temp);
            break;
        }
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

void codegen(CodegenContext* ctx, ASTNode* node) {
    // only statement blocks for now
    fprintf(stderr, "INFO - Generating LLVM IR code\n");
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
    ctx->symbols = NULL;
    ctx->symbols_size = 0;
    emit(ctx, "; ModuleID = 'memelang'\n");
    emit(ctx, "declare double @print(double)\n");
    emit(ctx, "\n");
}

void codegen_cleanup(CodegenContext* ctx) {
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        free(ctx->symbols[i].name);
        free(ctx->symbols[i].temp);
    }
    free(ctx->symbols);
}
