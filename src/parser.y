%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parser.h"
#include "lexer.h"

#define YYERROR_VERBOSE 1

ASTNode *root;
extern int yylex();
extern char *yytext;

void yyerror(YYLTYPE *loc, void *scanner, const char *s);
%}

%define api.pure
%define parse.error verbose
%debug
%lex-param {void *scanner}
%parse-param {void *scanner}
%locations

%code provides {
    int parse(char *text, ASTNode **node);
}

%union {
    int num;
    char* str;
    ASTNode* node;
    struct {
        ASTNode **args;
        unsigned int count;
    } call_args;
    struct {
        char **args;
        unsigned int count;
    } decl_args;

}

%token <num> NUMBER
%token <str> IDENTIFIER
%token PLUS MINUS MULTIPLY DIVIDE
%token LPAREN RPAREN
%token FUNCTION COMMA ARROW
%token LET EQUALS
%token SEMICOLON

%type <node> expression
%type <node> program
%type <node> function_def
%type <node> variable_def
%type <str> identifier
%type <call_args> call_args
%type <decl_args> decl_args

%%

program: function_def  { root = $1; }
       | variable_def SEMICOLON {root = $1; }
       | expression SEMICOLON{ root = $1; }
       ;

function_def: FUNCTION identifier LPAREN decl_args RPAREN ARROW expression { $$ = create_ast_function_def($2, $7); free($2); }
            ;

call_args: /* nothing! */ { $$.count = 0; $$.args = NULL; }
         | expression /* last arg*/ {$$.count = 1; $$.args = malloc(sizeof(ASTNode*)); $$.args[0] = $1;}
         | call_args COMMA expression {$1.count++; $1.args =  realloc($1.args, sizeof(ASTNode*) * $1.count); $1.args[$1.count-1] = $3; $$ = $1; }

decl_args: { $$.count = 0; $$.args = NULL; }
         | IDENTIFIER     { $$.count = 1; $$.args = malloc(sizeof(char*)); $$.args[0] = strdup($1); }
         | decl_args COMMA IDENTIFIER  { $1.count++; $1.args = realloc($1.args, sizeof(char*) * $1.count); $1.args[$1.count-1] = strdup($3); $$ = $1; }

    

variable_def: LET identifier EQUALS expression { $$ = create_ast_variable_def($2, $4); free($2);}

identifier: IDENTIFIER { $$ = strdup($1); }
          ;

expression: NUMBER              { $$ = create_ast_number($1); }
          | expression PLUS expression   { $$ = create_ast_binary_op($1, $3, OP_ADD); }
          | expression MINUS expression  { $$ = create_ast_binary_op($1, $3, OP_SUB); }
          | expression MULTIPLY expression { $$ = create_ast_binary_op($1, $3, OP_MUL); }
          | expression DIVIDE expression { $$ = create_ast_binary_op($1, $3, OP_DIV); }
          | identifier LPAREN call_args RPAREN    { $$ = create_ast_function_call($1, $3.args, $3.count); free($1); free($3.args);}
          | identifier {$$ = create_ast_variable($1); }
          | LPAREN expression RPAREN     { $$ = $2; }
          ;
%%


// handle errors
void yyerror(YYLTYPE *loc, void *scanner, const char *s) {
    int line = loc->first_line;
    int column = loc->first_column;
    fprintf(stderr,"ParseError: %s in line %d column %d\n", s, line, column);
    fprintf(stderr,"^\n");
}

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

    //char *error;

    //yyerror(scanner, error);
    
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
