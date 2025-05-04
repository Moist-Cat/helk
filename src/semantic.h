#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

void semantic_analysis(ASTNode *node);
ASTNode* transform_ast(ASTNode* node);

#endif
