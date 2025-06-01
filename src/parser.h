/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_SRC_PARSER_H_INCLUDED
# define YY_YY_SRC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NUMBER = 258,                  /* NUMBER  */
    IDENTIFIER = 259,              /* IDENTIFIER  */
    PLUS = 260,                    /* PLUS  */
    MINUS = 261,                   /* MINUS  */
    MULTIPLY = 262,                /* MULTIPLY  */
    DIVIDE = 263,                  /* DIVIDE  */
    LPAREN = 264,                  /* LPAREN  */
    RPAREN = 265,                  /* RPAREN  */
    LBRACE = 266,                  /* LBRACE  */
    RBRACE = 267,                  /* RBRACE  */
    FUNCTION = 268,                /* FUNCTION  */
    COMMA = 269,                   /* COMMA  */
    ARROW = 270,                   /* ARROW  */
    LET = 271,                     /* LET  */
    EQUALS = 272,                  /* EQUALS  */
    IN = 273,                      /* IN  */
    SEMICOLON = 274,               /* SEMICOLON  */
    IF = 275,                      /* IF  */
    ELSE = 276,                    /* ELSE  */
    WHILE = 277,                   /* WHILE  */
    STRING_LITERAL = 278,          /* STRING_LITERAL  */
    TYPE = 279,                    /* TYPE  */
    NEW = 280,                     /* NEW  */
    INHERITS = 281,                /* INHERITS  */
    DOT = 282                      /* DOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 28 "src/parser.y"

    int num;
    char* str;
    ASTNode* node;
    struct {
        ASTNode **members;
        unsigned int count;
    } type_members;
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

#line 130 "src/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void *scanner);
/* "%code provides" blocks.  */
#line 24 "src/parser.y"

    int parse(char *text, ASTNode **node);

#line 160 "src/parser.h"

#endif /* !YY_YY_SRC_PARSER_H_INCLUDED  */
