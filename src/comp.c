#include "ast.h"
#include "parser.h"
#include "codegen.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

static char* read_file(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        fprintf(stderr, "Error: Could not stat file '%s' (%s)\n",
               filename, strerror(errno));
        return NULL;
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open '%s' (%s)\n",
               filename, strerror(errno));
        return NULL;
    }

    char* buffer = malloc(st.st_size + 1);
    if (!buffer) {
        fclose(f);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    long read = fread(buffer, 1, st.st_size, f);
    if (read != st.st_size) {
        free(buffer);
        fclose(f);
        fprintf(stderr, "Error: Read incomplete\n");
        return NULL;
    }

    buffer[st.st_size] = '\0';
    fclose(f);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 1;
    }

    char* data = read_file(argv[1]);
    if (!data) {
        return 1;
    }

    CodegenContext ctx;
    codegen_init(&ctx, stdout);

    ASTNode* ast;
    if (parse(data, &ast) != 0) {
        free(data);
        return 1;
    }
    free(data);

    bool res = semantic_analysis(ast);

    if (res) {
        codegen(&ctx, ast);
        codegen_cleanup(&ctx);
    }
    else {
        fprintf(stderr, "FATAL - Semantic Analysis failed! Can not generate correct code\n");
    }

    free_ast(ast);
    return 0;
}
