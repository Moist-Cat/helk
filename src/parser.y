%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
%}

%union {
    int num;           // For NUMBER tokens
    ASTNode* node;     // For AST nodes (expressions)
}

%token <num> NUMBER   // Associate NUMBER with the "num" field in the union
%token PLUS MINUS MULTIPLY DIVIDE
%token LPAREN RPAREN

%type <node> expression  // Associate expressions with the "node" field
%type <node> program    // Add this line: program non-terminal uses "node" field

%%

program:
    expression          { $$ = $1; }
    ;

expression:
    NUMBER              { $$ = create_ast_number($1); }
    | expression PLUS expression   { $$ = create_ast_binary_op($1, $3, OP_ADD); }
    | expression MINUS expression  { $$ = create_ast_binary_op($1, $3, OP_SUB); }
    | expression MULTIPLY expression { $$ = create_ast_binary_op($1, $3, OP_MUL); }
    | expression DIVIDE expression { $$ = create_ast_binary_op($1, $3, OP_DIV); }
    | LPAREN expression RPAREN     { $$ = $2; }
    ;

%%

int yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
    return 0;
}
