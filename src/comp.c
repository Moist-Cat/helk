#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/PassBuilder.h>
#include <llvm-c/Analysis.h>

#include "ast.h"
#include "parser.h"
#include "codegen.h"



int main() {
    LLVMModuleRef module = LLVMModuleCreateWithName("memelang");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMExecutionEngineRef engine;
    
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    LLVMLinkInMCJIT();

    // Create execution engine.
    char *msg;
    if(LLVMCreateExecutionEngineForModule(&engine, module, &msg) == 1) {
        fprintf(stderr, "%s\n", msg);
        LLVMDisposeMessage(msg);
        return 1;
    }
    
    // Setup optimizations.
    LLVMPassManagerRef pass_manager =  LLVMCreateFunctionPassManagerForModule(module);
    LLVMInitializeFunctionPassManager(pass_manager);

    // Main REPL loop.
    while(1) {
        // Show prompt.
        fprintf(stderr, "ready > ");

        // Read input.
        char *input = NULL;
        size_t len = 0;
        
        if(getline(&input, &len, stdin) == -1) {
            fprintf(stderr, "Error reading from stdin\n");
            break;
        }
        
        // Exit if 'quit' is read.
        if(strcmp(input, "quit\n") == 0) {
            break;
        }
        
        // Parse
        ASTNode *node = NULL;
        int rc = parse(input, &node);
        if(rc != 0) {
            fprintf(stderr, "Parse error\n");
            continue;
        }
        
        // wrap in a function if it's an expression
        bool is_top_level = (node->type != AST_FUNCTION_DEF);
        if(is_top_level) {
            node = create_ast_function_def("", node);
        }

        // Codegen
        LLVMValueRef value = codegen(node, module, builder);
        printf("\n*blobs* doko?\n");
        if(value == NULL) {
            fprintf(stderr, "Unable to codegen for node\n");
            continue;
        }

        // Print detailed information about the value
        fprintf(stderr, "Generated value:\n");
        LLVMDumpValue(value);  // This shows the IR representation
        fprintf(stderr, "Value type: ");
        LLVMDumpType(LLVMTypeOf(value));  // Shows the LLVM type
        fprintf(stderr, "\n");

        // Check if it's a constant
        if (LLVMIsConstant(value)) {
            fprintf(stderr, "Value is a constant\n");
        }

        LLVMDumpModule(module);

        // Run it if it's a top level expression.
        if(is_top_level) {
            void *p = LLVMGetPointerToGlobal(engine, value);
            if (!p) {
                fprintf(stderr, "Failed to get function pointer\n");
                return 0;
            }
            char *error = NULL;
            if (LLVMVerifyFunction(value, LLVMAbortProcessAction)) {
                fprintf(stderr, "Invalid function: %s\n", error);
                LLVMDisposeMessage(error);
                return 0;
            }
            double (*P)() = (double (*)())(intptr_t)p;
            fprintf(stderr, "Evaluated to %f\n", P());
        }
        else if(node->type == AST_FUNCTION_DEF) {
            // If this is a function then optimize it.
            LLVMRunFunctionPassManager(pass_manager, value);
        }
        
        //free_ast(node);
    }
    
    // Dump entire module.
    LLVMDumpModule(module);

	LLVMDisposePassManager(pass_manager);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

    return 0;
}
