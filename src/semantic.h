#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>
#include "ast.h"

bool semantic_analysis(ASTNode *node);
ASTNode* transform_ast(ASTNode* node);

#endif
