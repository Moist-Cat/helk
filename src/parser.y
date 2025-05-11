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
    struct {
        ASTNode **statements;
        unsigned int count;
    } block;
    struct {
        char **names;
        ASTNode **values;
        unsigned int count;
    } var_defs;
    struct {
        char *name;
        ASTNode *value;
    } var_def;
    /*
     * if (hypothesis) {BLOCK} else
     */
    struct {
        ASTNode *hyphotesis; // expression that (must) evaluate to a bool
        ASTNode *thesis; //  block
        ASTNode *antithesis; // block
    } conditional;
}

%token <num> NUMBER
%token <str> IDENTIFIER
%token PLUS MINUS MULTIPLY DIVIDE
%token LPAREN RPAREN LBRACE RBRACE
%token FUNCTION COMMA ARROW
%token LET EQUALS IN
%token SEMICOLON
%token IF ELSE

%type <node> expression
%type <node> program
%type <node> function_def
%type <node> variable_def
%type <node> statement
%type <str> identifier
%type <call_args> call_args
%type <decl_args> decl_args
%type <block> statement_block
%type <var_defs> variable_def_list
%type <var_def> variable_def_item

%%

program: statement_block { root = create_ast_block($1.statements, $1.count); free($1.statements);}
       ;

statement_block: /* nothing */ { $$.count = 0; $$.statements = NULL; }
               /* Base case n=1 as well because we have to malloc */
               | statement SEMICOLON  {
                    $$.count = 1;
                    $$.statements = malloc(sizeof(ASTNode*));
                    $$.statements[0] = $1;
                }
               | statement_block statement SEMICOLON {
                    $1.count++;
                    $1.statements = realloc($1.statements, sizeof(ASTNode*) * $1.count);
                    $1.statements[$1.count-1] = $2;
                    $$ = $1;
                }
               ;

statement: variable_def { $$ = $1; }
         | expression { $$ = $1; }
         | function_def { $$ = $1; }
         ;

variable_def_item:
    identifier EQUALS expression {
        $$.name = $1;
        $$.value = $3;
    }
    ;

variable_def_list:
    variable_def_item {
        $$.count = 1;
        $$.names = malloc(sizeof(char*));
        $$.names[0] = $1.name;
        $$.values = malloc(sizeof(ASTNode*));
        $$.values[0] = $1.value;
    }
    | variable_def_list COMMA variable_def_item {
        $$.count = $1.count + 1;
        $$.names = realloc($1.names, sizeof(char*) * $$.count);
        $$.values = realloc($1.values, sizeof(ASTNode*) * $$.count);
        $$.names[$$.count-1] = $3.name;
        $$.values[$$.count-1] = $3.value;
    }
    ;

function_def:
    FUNCTION identifier LPAREN decl_args RPAREN ARROW expression { $$ = create_ast_function_def($2, $7, $4.args, $4.count); free($2); free($4.args); }
            ;

call_args: /* nothing! */ { $$.count = 0; $$.args = NULL; }
         | expression /* last arg*/ { $$.count = 1; $$.args = malloc(sizeof(ASTNode*)); $$.args[0] = $1; }
         | call_args COMMA expression { $1.count++; $1.args =  realloc($1.args, sizeof(ASTNode*) * $1.count); $1.args[$1.count-1] = $3; $$ = $1; }

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
          |
    LET variable_def_list IN expression {
        $$ = create_ast_let_in($2.names, $2.values, $2.count, $4);
        free($2.names);  // Free array containers only
        free($2.values);
    }     
          |
    IF LPAREN expression RPAREN LBRACE statement_block RBRACE ELSE LBRACE statement_block RBRACE {
        $$ = create_ast_conditional(
            $3,
            create_ast_block($6.statements, $6.count),
            create_ast_block($10.statements, $10.count)
        );
        free($6.statements);
        free($10.statements);
    }
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
