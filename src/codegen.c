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

static char* get_call_args(CodegenContext* ctx, ASTNode** args, size_t arg_count) {
    if (arg_count == 0) return strdup("");

    char** temps = malloc(arg_count * sizeof(char*));
    size_t total_len = 0;

    // Generate code for all arguments first
    for (size_t i = 0; i < arg_count; i++) {
        temps[i] = gen_expr(ctx, args[i]);
        total_len += strlen(temps[i]) + 6; // XXX "i32 , " per argument
    }

    // Build arguments string
    char* result = malloc(total_len + 1);
    char* ptr = result;

    for (size_t i = 0; i < arg_count; i++) {
        int written = sprintf(ptr, "i32 %s%s", temps[i], (i < arg_count-1) ? ", " : "");
        ptr += written;
        free(temps[i]);
    }

    free(temps);
    return result;
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
            emit(ctx, "  %s = add i32 0, %d\n", temp, node->number);
            return temp;
            
        case AST_BINARY_OP: {
            char* left = gen_expr(ctx, node->binary_op.left);
            char* right = gen_expr(ctx, node->binary_op.right);
            const char* op = NULL;
            
            switch (node->binary_op.op) {
                case OP_ADD: op = "add"; break;
                case OP_SUB: op = "sub"; break;
                case OP_MUL: op = "mul"; break;
                case OP_DIV: op = "sdiv"; break;
            }
            
            emit(ctx, "  %s = %s i32 %s, %s\n", temp, op, left, right);
            free(left);
            free(right);
            return temp;
        }            
        case AST_VARIABLE: {
            const char* var_temp = find_symbol(ctx, node->variable.name);
            if (!var_temp) {
                fprintf(stderr, "Error: Undefined variable '%s'\n", node->variable.name);
                free(temp);
                return NULL;
            }
            emit(ctx, "  %s = add i32 %s, 0  ; Load variable\n", temp, var_temp);
            return temp;
        }

        case AST_FUNCTION_CALL: {
            char* temp = new_temp(ctx);

            size_t arg_count = node->function_call.arg_count;

            char* call_args = get_call_args(ctx, node->function_call.args, arg_count);

            if (arg_count > 0) {
                emit(ctx, "  %s = call i32 @%s(%s)\n", temp, node->function_call.name, call_args);
            }
            else {
                emit(ctx, "  %s = call i32 @%s()\n", temp, node->function_call.name);
            }
            
            free(call_args);

            return temp;
        }
        case AST_BLOCK: {
            // no idea what's the return value of a block of expressions
            // XXX this obviously explodes if you declare a function inside a function
            // don't do that
            codegen_block(ctx, node);
            
            // Return zero by default
            char* temp = new_temp(ctx);
            emit(ctx, "  %s = xor i32 0, 0\n", temp, temp, temp);
            return temp;
        }
        default:
            fprintf(stderr, "Error: Failed to parse %d because it is not an expression! \n", node->type);
            free(temp);
            return NULL;
    }
}

void codegen_stmt(CodegenContext* ctx, ASTNode* node) {
    switch (node->type) {
        case AST_FUNCTION_DEF: {
            // should ONLY contain functions after sem_anal
            emit(ctx, "\ndefine i32 @%s() {\n", node->function_def.name);
            emit(ctx, "entry:\n");
            
            char* result = gen_expr(ctx, node->function_def.body);
            if (result) {
                emit(ctx, "  ret i32 %s\n", result);
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
        case AST_VARIABLE_DEF: {
            // notice that we perform NO operation here
            char* value_temp = gen_expr(ctx, node->variable_def.body);
            add_symbol(ctx, node->variable_def.name, value_temp);
            emit(ctx, "  ; Variable assignment: %s = %s\n", 
                node->variable_def.name, value_temp);
            free(value_temp);
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
    emit(ctx, "declare i32 @print(i32)\n");
    emit(ctx, "\n");
}

void codegen_cleanup(CodegenContext* ctx) {
    for (size_t i = 0; i < ctx->symbols_size; i++) {
        free(ctx->symbols[i].name);
        free(ctx->symbols[i].temp);
    }
    free(ctx->symbols);
}
