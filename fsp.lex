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
# include "driver.hpp"
/* Include the bison-generated parser header, in order to get the token
   types definition that we return, and the CircularBuffer. */
# include "parser.hpp"

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
#define yyterminate() return token::END

//#define DEBUG
#ifdef DEBUG
#define IFD(x) (x)
#else
#define IFD(x) 
#endif


/* The following is executed before each rule's action. */
#define YY_USER_ACTION \
    do { \
	/*last_tokens.location_extend(yylloc, yyleng);*/ \
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
    typedef yy::fsp_parser::token token;

    //last_tokens.location_step(yylloc);
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
    /*last_tokens.location_lines(yylloc, yyleng);
    last_tokens.location_step(yylloc); */
    yylloc->lines(yyleng);
    yylloc->step();

    /* When reporting an error we want to see the last line only. */
    last_tokens.flush();
}

<COMMENTS>. {
    /* last_tokens.location_step(yylloc); */
    yylloc->step();
}

"//" {
    BEGIN(INLINECOMMENTS);
}

<INLINECOMMENTS>[\n] {
    /* When in comment state, throw away anything but keep
       tracking locations. */
    /*last_tokens.location_lines(yylloc, yyleng);
    last_tokens.location_step(yylloc);*/
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
    return yy::fsp_parser::token_type(yytext[0]);
}

"+"|"-"|"*"|"/"|"%"|"!" {
    IFD(cout << yytext[0] << "\n"); 
    return yy::fsp_parser::token_type(yytext[0]);
}

"("|")"|"["|"]"|"{"|"}"|"="|"."|","|":"|";"|"@"|"\\" {
    IFD(cout << yytext[0] << "\n"); 
    return yy::fsp_parser::token_type(yytext[0]);
}

[ \t\r]+ {
    /* Eat up whitespaces, and keep tracking positions. */
    /* last_tokens.location_step(yylloc); */
    yylloc->step();
}

[\n]+ {
    /* Update the line counter and step forward. */
    /*last_tokens.location_lines(yylloc, yyleng);
    last_tokens.location_step(yylloc);*/
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

#include "scanner.hpp"

/* ======================= ScannerBuffer & co. ========================== */
void ScannerBuffer::select()
{
    yy_switch_to_buffer(yybs);
}

ScannerBuffer::~ScannerBuffer()
{
    yy_delete_buffer(yybs);
}

ScannerFileBuffer::ScannerFileBuffer(const char * input_name)
{
    fin = fopen(input_name, "r");
    if (!fin) {
	cerr << "Input error: Can't open " << input_name << "\n";
	cerr << "see 'fspc -h'\n";
	exit(1);
    }
    yybs = yy_create_buffer(fin, YY_BUF_SIZE);
    if (yybs == NULL) {
	cerr << "yy_create_buffer() returned NULL\n";
	cerr << "Please report this bug to <v.maffione@gmail.com>\n";
	exit(1);
    }
}

ScannerFileBuffer::~ScannerFileBuffer()
{
    fclose(fin);
}

ScannerStringBuffer::ScannerStringBuffer(const char * buf, int sz)
{
    YY_BUFFER_STATE last = YY_CURRENT_BUFFER;

    buffer = buf;
    size = sz;
    yybs = yy_scan_bytes(buf, size);
    if (yybs == NULL) {
	cerr << "yy_scan_bytes() returned NULL\n";
	cerr << "Please report this bug to <v.maffione@gmail.com>\n";
	exit(1);
    }
    /* The yy_scan_bytes() function has a side effect: it calls
       yy_switch_to_buffer() on the new buffer. Since we don't want this side
       effect, we revert to the last buffer (if any) by calling the function
       yy_switch_to_buffer() again. */
    if (last)
	yy_switch_to_buffer(last);
}


void InputBuffersStack::push(const char * input_name)
{
    struct BufferInfo bi;

    bi.fin = fopen(input_name, "r");
    if (!bi.fin) {
	cerr << "Input error: Can't open " << input_name << "\n";
	cerr << "see 'fspc -h'\n";
	exit(1);
    }
    bi.yybs = yy_create_buffer(bi.fin, YY_BUF_SIZE);
    if (bi.yybs == NULL) {
	cerr << "yy_create_buffer() returned NULL\n";
	cerr << "Please report this bug to <v.maffione@gmail.com>\n";
	exit(1);
    }
    bi.type = BufferInfo::File;
    buffers.push_back(bi);
    yy_switch_to_buffer(bi.yybs);
}

void InputBuffersStack::push(const char * buffer, int size)
{
    BufferInfo bi;

    bi.buffer = buffer;
    bi.size = size;
    bi.yybs = yy_scan_bytes(buffer, size);
    if (bi.yybs == NULL) {
	cerr << "yy_scan_bytes() returned NULL\n";
	cerr << "Please report this bug to <v.maffione@gmail.com>\n";
	exit(1);
    }
    bi.type = BufferInfo::String;
    buffers.push_back(bi);
    /* The yy_scan_bytes() function has a side effect: it calls
       yy_switch_to_buffer() on the new buffer. Therefore it's not
       necessary to call it again. */
}

bool InputBuffersStack::pop()
{
    if (!buffers.size())
	return false;

    BufferInfo& bi = buffers.back();

    if (bi.type == BufferInfo::File) {
	fclose(bi.fin);
    }

    yy_delete_buffer(bi.yybs);
    assert(buffers.size());
    buffers.pop_back();
    yy_switch_to_buffer(buffers.back().yybs);

    return true;
}

InputBuffersStack::~InputBuffersStack()
{
    while (pop()) { }
}

/* XXX Deprecated. */
int scanner_setup(const char * input_name)
{
    FILE *fin = fopen(input_name, "r");
    if (!fin) {
	cerr << "Input error: Can't open " << input_name << "\n";
	cerr << "see 'fspc -h'\n";
	return -1;
    }

    yyin = fin;

    cout << "FLEX buffer " << YY_CURRENT_BUFFER << "\n";

    return 0;
}

void fsp_driver::scan_begin(const char * filename)
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

void fsp_driver::scan_end ()
{
    fclose(yyin);
}

