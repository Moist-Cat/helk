%{
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "parser.h"
#include <string.h>

char* process_string(const char* input) {
    // Remove quotes and process escape sequences
    size_t len = strlen(input) - 2;
    char* out = malloc(len + 1);
    char* p = out;
    
    for(const char* s = input+1; *s && *(s+1); s++) {
        if(*s == '\\') {
            switch(*(++s)) {
                case 'n': *p++ = '\n'; break;
                case 't': *p++ = '\t'; break;
                case '"': *p++ = '"'; break;
                case '\\': *p++ = '\\'; break;
                default: 
                    fprintf(stderr, "Unknown escape: \\%c\n", *s);
                    *p++ = *s;
            }
        } else {
            *p++ = *s;
        }
    }
    *p = '\0';
    return out;
}
%}

%option noyywrap
%option bison-bridge
%option bison-locations
%option reentrant

%%
[ \t\n]                 ; // Skip whitespace
\"([^"\\]|\\.)*\"   { 
    yylval->str = process_string(yytext); 
    return STRING_LITERAL; 
}
[0-9]+                 { yylval->num = atoi(yytext); return NUMBER; }
"new"                  { return NEW; }
"."                    { return DOT; }
"type"                 { return TYPE; }
"inherits"             { return INHERITS; }
"if"                   { return IF; }
"else"                 { return ELSE; }
"in"                   { return IN; }
"let"                  { return LET; }
"while"                { return WHILE; }
"function"             { return FUNCTION; }
";"                    { return SEMICOLON; }
","                    { return COMMA; }
"="                    { return EQUALS; }
"=>"                   { return ARROW; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->str = strdup(yytext); return IDENTIFIER; }
"^"                    { return EXP; }
"+"                    { return PLUS; }
"-"                    { return MINUS; }
"*"                    { return MULTIPLY; }
"/"                    { return DIVIDE; }
"("                    { return LPAREN; }
")"                    { return RPAREN; }
"{"                    { return LBRACE; }
"}"                    { return RBRACE; }
.                      { printf("Unknown character: %s\n", yytext); }

%%
