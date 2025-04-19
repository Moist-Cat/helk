#include "codegen.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    LLVMValueRef value;
} Symbol;

Symbol *functions = NULL;
size_t num_functions = 0;

size_t num_vars = 0;
Symbol *symbol_table = NULL;

LLVMValueRef codegen(ASTNode *node, LLVMModuleRef module, LLVMBuilderRef builder) {
	switch (node->type) {
		case AST_NUMBER:
        	return LLVMConstReal(LLVMDoubleType(), node->number);
    	case AST_BINARY_OP:
		    LLVMValueRef left = codegen(node->binary_op.left, module, builder);
		    LLVMValueRef right = codegen(node->binary_op.right, module, builder);
		    switch (node->binary_op.op) {
		        case OP_ADD: return LLVMBuildAdd(builder, left, right, "addtmp");
		        case OP_SUB: return LLVMBuildSub(builder, left, right, "subtmp");
		        case OP_MUL: return LLVMBuildMul(builder, left, right, "multmp");
		        case OP_DIV: return LLVMBuildUDiv(builder, left, right, "divtmp");
		        default: return NULL;
		     }
        case AST_FUNCTION_DEF: {
            // Create function type (no args, returns int)
            LLVMTypeRef ret_type = LLVMDoubleType();
            LLVMTypeRef func_type = LLVMFunctionType(ret_type, NULL, 0, 0);
            
            // Create function
            LLVMValueRef func = LLVMAddFunction(module, node->function_def.name, func_type);
            
            // Create entry block
			LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "entry");
			LLVMPositionBuilderAtEnd(builder, block);
            
            // Generate body
            LLVMValueRef body_val = codegen(node->function_def.body, module, builder);
            LLVMBuildRet(builder, body_val);

            // Store function reference
            functions = realloc(functions, ++num_functions * sizeof(Symbol));
            functions[num_functions-1] = (Symbol){
                .name = node->function_def.name,
                .value = func
            };
            
            return func;
        }
        case AST_FUNCTION_CALL: {
            // Look up function
            for (size_t i = 0; i < num_functions; i++) {
                if (strcmp(functions[i].name, node->function_call.name) == 0) {
                    return LLVMBuildCall2(builder, 
                        LLVMDoubleType(),
                        functions[i].value, NULL, 0, "calltmp");
                }
            }
            fprintf(stderr, "Unknown function: %s\n", node->function_call.name);
            return NULL;
        }
        case AST_VARIABLE_DEF: {
            LLVMValueRef value = codegen(node->variable_def.body, module, builder);
            if (!value) {
                return NULL;
            }

            symbol_table = realloc(symbol_table, (num_vars + 1)*sizeof(Symbol));
            symbol_table[num_vars] = (Symbol){
                .name = strdup(node->variable_def.name),
                .value = value
            };

            num_vars += 1;

            return value;
        }
        case AST_VARIABLE: {
            for (size_t i = 0; i < num_vars; i++) {
                if (strcmp(symbol_table[i].name, node->variable.name) == 0) {
                    return symbol_table[i].value;
                }
            }
            fprintf(stderr, "Unknown variable: %s\n", node->variable.name);
            return NULL;
        }
        default:
            return NULL;
    }
}


