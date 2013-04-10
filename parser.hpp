/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton interface for Bison GLR parsers in C
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
/* Line 2579 of glr.c  */
#line 14 "fsp.ypp"

/* Symbols tables. */
#include "symbols_table.hpp"


/* Line 2579 of glr.c  */
#line 52 "parser.hpp"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IF = 258,
     THEN = 259,
     ELSE = 260,
     WHEN = 261,
     CONST = 262,
     RANGE = 263,
     SET = 264,
     ARROW = 265,
     DOTDOT = 266,
     END = 267,
     STOP = 268,
     ERROR = 269,
     PROPERTY = 270,
     PROGRESS = 271,
     MENU = 272,
     FORALL = 273,
     SHARING = 274,
     OR = 275,
     AND = 276,
     EQUAL = 277,
     NOTEQUAL = 278,
     LOE = 279,
     GOE = 280,
     LSHIFT = 281,
     RSHIFT = 282,
     INTEGER = 283,
     FLOAT = 284,
     UpperCaseID = 285,
     LowerCaseID = 286
   };
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2579 of glr.c  */
#line 25 "fsp.ypp"

    int int_value;
    float float_value;
    std::string *string_ptr;
    class SymbolValue *sv_ptr;
    struct SvpVec * svpvec_ptr;
    struct Pvec * pvec_ptr;


/* Line 2579 of glr.c  */
#line 107 "parser.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
