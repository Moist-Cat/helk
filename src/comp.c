#include "ast.h"
#include "parser.h"
#include "codegen.h"
#include "semantic.h"
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

    semantic_analysis(ast);

    codegen(&ctx, ast);
    codegen_cleanup(&ctx);

    //free_ast(ast);
    return 0;
}
