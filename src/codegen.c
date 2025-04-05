#include "codegen.h"
#include <stddef.h>

LLVMValueRef generate_code(ASTNode *node, LLVMModuleRef module, LLVMBuilderRef builder) {
    if (node->type == AST_NUMBER) {
        return LLVMConstInt(LLVMInt32Type(), node->number, 0);
    } else if (node->type == AST_BINARY_OP) {
        LLVMValueRef left = generate_code(node->binary_op.left, module, builder);
        LLVMValueRef right = generate_code(node->binary_op.right, module, builder);
        switch (node->binary_op.op) {
            case OP_ADD: return LLVMBuildAdd(builder, left, right, "addtmp");
            case OP_SUB: return LLVMBuildSub(builder, left, right, "subtmp");
            case OP_MUL: return LLVMBuildMul(builder, left, right, "multmp");
            case OP_DIV: return LLVMBuildUDiv(builder, left, right, "divtmp");
        }
    }
    return NULL;
}
