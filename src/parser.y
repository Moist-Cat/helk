%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parser.h"
#include "lexer.h"

ASTNode *root;
extern int yylex();
void yyerror(void *scanner, const char *s) { printf("ERROR: %s\n", s); }
%}

%define api.pure
%debug
%lex-param {void *scanner}
%parse-param {void *scanner}

%code provides {
    int parse(char *text, ASTNode **node);
}

%union {
    double num;           // For NUMBER tokens
    char* str;
    ASTNode* node;     // For AST nodes (expressions)
}

%token <num> NUMBER
%token <str> IDENTIFIER
%token PLUS MINUS MULTIPLY DIVIDE
%token LPAREN RPAREN
%token FUNCTION ARROW
%token LET EQUALS

%type <node> expression
%type <node> program
%type <node> function_def
%type <node> variable_def
%type <str> identifier

%%

program:
    function_def  { root = $1; }
    | variable_def {root = $1; }
    | expression { root = $1; }
    ;

function_def:
    FUNCTION identifier LPAREN RPAREN ARROW expression
        { $$ = create_ast_function_def($2, $6); free($2); }
    ;

variable_def:
    LET identifier EQUALS expression { $$ = create_ast_variable_def($2, $4); }

identifier:
    IDENTIFIER { $$ = strdup($1); }
    ;

expression:
    NUMBER              { $$ = create_ast_number($1); }
    | expression PLUS expression   { $$ = create_ast_binary_op($1, $3, OP_ADD); }
    | expression MINUS expression  { $$ = create_ast_binary_op($1, $3, OP_SUB); }
    | expression MULTIPLY expression { $$ = create_ast_binary_op($1, $3, OP_MUL); }
    | expression DIVIDE expression { $$ = create_ast_binary_op($1, $3, OP_DIV); }
    | identifier LPAREN RPAREN    { $$ = create_ast_function_call($1); free($1); }
    | identifier {$$ = create_ast_variable($1); }
    | LPAREN expression RPAREN     { $$ = $2; }
    ;

%%

int parse(char *text, ASTNode **node)
{
    yydebug = 1;
    
    // Parse using Bison.
    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE buffer = yy_scan_string(text, scanner);
    int rc = yyparse(scanner);
    yy_delete_buffer(buffer, scanner);
    yylex_destroy(scanner);
    
    // If parse was successful, return root node.
    if(rc == 0) {
        *node = root;
        return 0;
    }
    // Otherwise return error.
    else {
        return -1;
    }
}
