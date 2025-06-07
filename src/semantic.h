#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdlib.h>
#include <stdbool.h>
#include "ast.h"

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

typedef struct SymbolEntry {
    char* name;
    ASTNode* node;
    struct SymbolEntry* next;
} SymbolEntry;

typedef struct SymbolTable {
    SymbolEntry** entries;
    size_t size;
    struct SymbolTable* parent;
} SymbolTable;

void _semantic_analysis(ASTNode *node, ConstraintSystem* cs, SymbolTable* scope);
bool semantic_analysis(ASTNode *node);
void symbol_table_add(SymbolTable* st, const char* name, ASTNode* node);
void process_node(ASTNode* node, ConstraintSystem* cs, SymbolTable* current_scope);
ASTNode* transform_ast(ASTNode* node, SymbolTable* scope);

#endif
