%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parser.h"
#include <string.h>
%}

%option noyywrap
%option bison-bridge
%option bison-locations
%option reentrant

%%

[ \t\n]                 ; // Skip whitespace
[0-9]+                  { yylval->num = atoi(yytext); return NUMBER; }
"if"            { return IF; }
"else"          { return ELSE; }
"in"             { return IN; }
"let"             { return LET; }
"function"             { return FUNCTION; }
";"                    { return SEMICOLON; }
","                    { return COMMA; }
"="                    { return EQUALS; }
"=>"                   { return ARROW; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->str = strdup(yytext); return IDENTIFIER; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MULTIPLY; }
"/"                     { return DIVIDE; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
.                       { printf("Unknown character: %s\n", yytext); }

%%
