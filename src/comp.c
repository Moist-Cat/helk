#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "codegen.h"
#include "semantic.h"

Token* lexer(const char* input, int length, unsigned int* _num_tokens) {
    LexerState* state = malloc(sizeof(LexerState));
    Token* tokens = malloc(sizeof(Token)*0);
    unsigned int num_tokens = 0;

    lexer_init(state, input, length, true);

    Token token;

    while (state->current < state->end) {
        token = lexer_next_token(state);
        fprintf(stderr, "INFO - Ate token (%d, %s) [%d, %d] \n", token.type, token.value, token.line, token.column);
        if (token.type == TOKEN_ERROR) {
            fprintf(
                stderr,
                "ERROR - Invalid token %s (line=%d, column=%d)\n",
                token.value,
                token.line,
                token.column
            );
            free(state);
            free(tokens);
            *_num_tokens = 0;
            return NULL;
        }
        // modify external array
        tokens = realloc(tokens, sizeof(Token) * (num_tokens + 1));
        tokens[num_tokens] = token;
        num_tokens += 1;

        // after to add EOF
        if (token.type == TOKEN_EOF) {
            free(state);
            *_num_tokens = num_tokens;
            return tokens;
        }
    }
    free(state);

    *_num_tokens = num_tokens;

    return tokens;

}

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


    unsigned int num_tokens = 0;
    Token* tokens = lexer(data, strlen(data), &num_tokens);
    if (tokens == NULL) {
        fprintf(stderr, "ERROR - Detected error(s) during tokenization\n");
        exit(1);
    }

    for (unsigned int i = 0; i < num_tokens; i++) {
        fprintf(stderr, "%s ", tokens[i].value);
    }
    fprintf(stderr, "\n");

    int errors = 0;
    ASTNode* ast = parse(tokens, &errors);
    ast_print_node(ast, 0);
    if (errors > 0) {
        fprintf(stderr, "ERROR - Found %d errors during parsing\n", errors);
        exit(1);
    }

    CodegenContext ctx;
    codegen_init(&ctx, stdout);

    bool res = semantic_analysis(ast);

    if (res) {
        codegen(&ctx, ast);
        codegen_cleanup(&ctx);
    }
    else {
        fprintf(stderr, "FATAL - Semantic Analysis failed! Can not generate correct code\n");
    }

    return 0;
}
