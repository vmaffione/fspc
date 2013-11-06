/*
 *  fspc lexical analyzer FLEX template
 *
 *  Copyright (C) 2013  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%{ /* -*- C++ -*- */
# include <cstdlib>
# include <cerrno>
# include <climits>
# include <string>
#include <cstdio>
#include <iostream>
#include <assert.h>

/* Include tha parser main class definition. */
# include "driver.hpp"

/* Include the bison-generated parser header, in order to get the token
   types definition that we return. */
# include "parser.hpp"

/* Circular tokens buffer for good error reporting. */
#include "circular_buffer.hpp"
extern class CircularBuffer last_tokens;

using namespace std;


/* Work around an incompatibility in flex (at least versions
   2.5.31 through 2.5.33): it generates code that does
   not conform to C89.  See Debian bug 333231
   <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.  */
# undef yywrap
# define yywrap() 1

/* By default yylex returns int, we use token_type.
   Unfortunately yyterminate by default returns 0, which is
   not of token_type.  */
#define yyterminate() return token::ENDOF

//#define DEBUG
#ifdef DEBUG
#define IFD(x) (x)
#else
#define IFD(x) 
#endif


/* The following is executed before each rule's action. */
#define YY_USER_ACTION \
    do { \
	yylloc->columns(yyleng); \
	last_tokens.insert(yytext, yyleng); \
    } while (0);


/* ==============================================================
   ============================================================== */
%}

/* We don't want to take a standard yywrap() from fl.so, and so we can
   avoid linking the executable with -lfl. */
%option noyywrap nounput batch debug
%option outfile="scanner.cpp"

DIGIT		[0-9]
LowerCaseID	[_a-z][_a-zA-Z0-9]*
UpperCaseID	[A-Z][_a-zA-Z0-9]*

%x COMMENTS
%x INLINECOMMENTS


%%

%{
  /* This code, which appears before the first rule, is copied 
     verbatim at the beginning of yylex(). At each yylex() 
     invocation, therefore, we mark the current last position as the
     start of the next token.  */

    /* Shortcut typedef. */
    typedef yy::FspParser::token token;

    yylloc->step ();
%}

"/*" {
    BEGIN(COMMENTS);
}

<COMMENTS>"*/" {
    BEGIN(INITIAL);
}

<COMMENTS>[\n]+ {
    /* When in comment state, throw away anything but keep
       tracking locations. */
    yylloc->lines(yyleng);
    yylloc->step();

    /* When reporting an error we want to see the last line only. */
    last_tokens.flush();
}

<COMMENTS>. {
    yylloc->step();
}

"//" {
    BEGIN(INLINECOMMENTS);
}

<INLINECOMMENTS>[\n] {
    /* When in comment state, throw away anything but keep
       tracking locations. */
    yylloc->lines(yyleng);
    yylloc->step();

    /* When reporting an error we want to see the last line only. */
    last_tokens.flush();
    BEGIN(INITIAL);
}

<INLINECOMMENTS>. {
    /*last_tokens.location_step(yylloc);*/
    yylloc->step();
}


{DIGIT}+ {
    yylval->int_value = atoi(yytext); //TODO strtol()
    IFD(cout << "INTEGER: " << yylval->int_value << "\n");
    return token::INTEGER;
}

if { IFD(cout << "IF\n"); return token::IF; }
then { IFD(cout << "THEN\n"); return token::THEN; }
else { IFD(cout << "ELSE\n"); return token::ELSE; }
when { IFD(cout << "WHEN\n"); return token::WHEN; }
const { IFD(cout << "CONST\n"); return token::CONST; }
range { IFD(cout << "RANGE\n"); return token::RANGE; }
set { IFD(cout << "SET\n"); return token::SET; }
property { IFD(cout << "PROPERTY\n"); return token::PROPERTY; }
progress { IFD(cout << "PROGRESS\n"); return token::PROGRESS; }
menu { IFD(cout << "MENU\n"); return token::MENU; }
forall {IFD(cout << "FORALL\n"); return token::FORALL; }
END { IFD(cout << "END\n"); return token::END; }
STOP { IFD(cout << "STOP\n"); return token::STOP; }
ERROR { IFD(cout << "ERROR\n"); return token::ERROR; }

{LowerCaseID} {
    yylval->string_ptr = new string(yytext);
    IFD(cout << "LowerCaseID\n"); 
    return token::LowerCaseID;
}

{UpperCaseID} {
    yylval->string_ptr = new string(yytext);
    IFD(cout << "UpperCaseID\n"); 
    return token::UpperCaseID;
}

"->" {
    IFD(cout << "->\n"); 
    return token::ARROW;
}

".." {
    IFD(cout << "..\n"); 
    return token::DOTDOT;
}

"::" {
    IFD(cout << "::\n");
    return token::SHARING;
}

"||" {
    IFD(cout << "||\n"); 
    return token::OR;
}

"&&" {
    IFD(cout << "&&\n"); 
    return token::AND;
}

"==" {
    IFD(cout << "==\n"); 
    return token::EQUAL;
}

"!=" {
    IFD(cout << "!=\n"); 
    return token::NOTEQUAL;
}

"<=" {
    IFD(cout << "<=\n"); 
    return token::LOE;
}

">=" {
    IFD(cout << ">=\n"); 
    return token::GOE;
}

">>" {
    IFD(cout << ">>\n"); 
    return token::RSHIFT;
}

"<<" {
    IFD(cout << "aa\n"); 
    return token::LSHIFT;
}

"|"|"^"|"&"|"<"|">" {
    IFD(cout << yytext[0] << "\n"); 
    return yy::FspParser::token_type(yytext[0]);
}

"+"|"-"|"*"|"/"|"%"|"!" {
    IFD(cout << yytext[0] << "\n"); 
    return yy::FspParser::token_type(yytext[0]);
}

"("|")"|"["|"]"|"{"|"}"|"="|"."|","|":"|";"|"@"|"\\" {
    IFD(cout << yytext[0] << "\n"); 
    return yy::FspParser::token_type(yytext[0]);
}

"$r" {
    return token::EXPECT_RANGE;
}

"$s" {
    return token::EXPECT_SET;
}

[ \t\r]+ {
    /* Eat up whitespaces, and keep tracking positions. */
    /* last_tokens.location_step(yylloc); */
    yylloc->step();
}

[\n]+ {
    /* Update the line counter and step forward. */
    yylloc->lines(yyleng);
    yylloc->step();

    /* When reporting an error we want to see the last line only. */
    last_tokens.flush();
}

. {
    cerr << "Unrecognized character " << yytext << endl;
    exit(1);
}
%%

/* User code: Functions that can be exported. */

void FspDriver::scan_begin(const char * filename)
{
    yy_flex_debug = trace_scanning;
    if (filename == NULL /* || strcmp(file,"-") == 0 */) {
	yyin = stdin;
    } else if (!(yyin = fopen(filename, "r"))) {
	string err = "cannot open " + string(filename) + ": " + strerror(errno);
	perror(err.c_str());
	exit(EXIT_FAILURE);
    }
}

void FspDriver::scan_end ()
{
    fclose(yyin);
}

