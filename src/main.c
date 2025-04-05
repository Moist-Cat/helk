#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/PassBuilder.h>

#include "ast.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char **argv) {
    // Parse input and generate AST
    printf("1");
    ASTNode *ast = yyparse();

    // Perform semantic analysis
    printf("2");
    semantic_analysis(ast);

    // Initialize LLVM
    printf("3");
    LLVMInitializeNativeTarget();
    LLVMModuleRef module = LLVMModuleCreateWithName("my_module");
    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMExecutionEngineRef engine;

    // Generate LLVM IR
    printf("4");
    generate_code(ast, module, builder);

    // Print the generated IR
    printf("5");
    LLVMDumpModule(module);

    // Clean up
    printf("6");
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    free_ast(ast);

    printf("7");
    return 0;
}
