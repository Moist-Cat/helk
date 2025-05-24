#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

typedef struct {
    char* name;
    char* temp;
} Symbol;

typedef struct {
    FILE* output;
    int temp_counter;
    int label_counter;
    int _last_merge; // required for nested ifs
    Symbol* symbols;
    size_t symbols_size;
} CodegenContext;

void codegen_init(CodegenContext* ctx, FILE* output);
void codegen_cleanup(CodegenContext* ctx);

static char* gen_expr(CodegenContext* ctx, ASTNode* node);
static char* codegen_expr_block(CodegenContext* ctx, ASTNode* node);
void codegen_block(CodegenContext* ctx, ASTNode* node);
void codegen(CodegenContext* ctx, ASTNode* node);

#endif
