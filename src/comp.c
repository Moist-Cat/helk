#include "ast.h"
#include "parser.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <expression>\n", argv[0]);
        return 1;
    }

    CodegenContext ctx;
    codegen_init(&ctx, stdout);

    ASTNode* ast;
    if (parse(argv[1], &ast) != 0) {
        fprintf(stderr, "Parse error\n");
        return 1;
    }

    // implementation detail
    // wrap in a main function if it's not a function
    //
    int is_top_level = (ast->type != AST_FUNCTION_DEF);
    if(is_top_level == 1) {
        // easy
        ast = create_ast_function_def("main", ast, NULL, 0);
    }
    else {
        // the body is a function call
        codegen(&ctx, ast);
        ast = create_ast_function_def(
            "main", create_ast_function_call(ast->function_def.name, NULL, 0), NULL, 0
         );
    }

    codegen(&ctx, ast);
    codegen_cleanup(&ctx);

    free_ast(ast);
    return 0;
}
