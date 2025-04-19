#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <llvm-c/Core.h>

LLVMValueRef codegen(ASTNode *node, LLVMModuleRef module, LLVMBuilderRef builder);

#endif
