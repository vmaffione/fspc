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


%{
#include <cstdio>
#include <iostream>
#include <assert.h>

using namespace std;

/* Include the bison-generated parser header, in order to get the token
   types definition that we return, and the CircularBuffer. */
#include "parser.hpp" 


//#define DEBUG
#ifdef DEBUG
#define IFD(x) (x)
#else
#define IFD(x) 
#endif



/* The following is executed before each rule's action. */
#define YY_USER_ACTION \
    do { \
	yylloc.last_column += yyleng; \
	last_tokens.insert(yytext, yyleng); \
    } while (0);


/* Initialize LOC. */
# define LOCATION_RESET(Loc)                  \
  (Loc).first_column = (Loc).first_line = 1;  \
  (Loc).last_column =  (Loc).last_line = 1;

/* Advance of NUM lines. */
# define LOCATION_LINES(Loc, Num)             \
  (Loc).last_column = 1;                      \
  (Loc).last_line += Num;

/* Restart: move the first cursor to the last position. */
# define LOCATION_STEP(Loc)                   \
  (Loc).first_column = (Loc).last_column;     \
  (Loc).first_line = (Loc).last_line;
/* ==============================================================
   ============================================================== */
%}



DIGIT		[0-9]
LowerCaseID	[_a-z][_a-zA-Z0-9]*
UpperCaseID	[A-Z][_a-zA-Z0-9]*

%x COMMENTS

/* We don't want to take a standard yywrap() from fl.so, and so we can
   avoid linking the executable with -lfl. */
%option noyywrap

%option outfile="scanner.cpp"

/* %option batch */

%%

%{
  /* This code, which appears before the first rule, is copied 
     verbatim at the beginning of yylex(). At each yylex() 
     invocation, therefore, we mark the current last position as the
     start of the next token.  */
    LOCATION_STEP(yylloc);
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
    LOCATION_LINES(yylloc, yyleng); LOCATION_STEP(yylloc);

    /* When reporting an error we want to see the last line only. */
    last_tokens.flush();
}

<COMMENTS>. {
    LOCATION_STEP(yylloc);
}

{DIGIT}+ {
    yylval.int_value = atoi(yytext);
    IFD(cout << "INTEGER: " << yylval.int_value << "\n");
    return INTEGER;
}

if { IFD(cout << "IF\n"); return IF; }
then { IFD(cout << "THEN\n"); return THEN; }
else { IFD(cout << "ELSE\n"); return ELSE; }
when { IFD(cout << "WHEN\n"); return WHEN; }
const { IFD(cout << "CONST\n"); return CONST; }
range { IFD(cout << "RANGE\n"); return RANGE; }
set { IFD(cout << "SET\n"); return SET; }
property { IFD(cout << "PROPERTY\n"); return PROPERTY; }
progress { IFD(cout << "PROGRESS\n"); return PROGRESS; }
menu { IFD(cout << "MENU\n"); return MENU; }
forall {IFD(cout << "FORALL\n"); return FORALL; }
END { IFD(cout << "END\n"); return END; }
STOP { IFD(cout << "STOP\n"); return STOP; }
ERROR { IFD(cout << "ERROR\n"); return ERROR; }

{LowerCaseID} {
    yylval.string_ptr = new string(yytext);
    IFD(cout << "LowerCaseID\n"); 
    return LowerCaseID;
}

{UpperCaseID} {
    yylval.string_ptr = new string(yytext);
    IFD(cout << "UpperCaseID\n"); 
    return UpperCaseID;
}

"->" {
    IFD(cout << "->\n"); 
    return ARROW;
}

".." {
    IFD(cout << "..\n"); 
    return DOTDOT;
}

"::" {
    IFD(cout << "::\n");
    return SHARING;
}

"||" {
    IFD(cout << "||\n"); 
    return OR;
}

"&&" {
    IFD(cout << "&&\n"); 
    return AND;
}

"==" {
    IFD(cout << "==\n"); 
    return EQUAL;
}

"!=" {
    IFD(cout << "!=\n"); 
    return NOTEQUAL;
}

"<=" {
    IFD(cout << "<=\n"); 
    return LOE;
}

">=" {
    IFD(cout << ">=\n"); 
    return GOE;
}

">>" {
    IFD(cout << ">>\n"); 
    return RSHIFT;
}

"<<" {
    IFD(cout << "aa\n"); 
    return LSHIFT;
}

"|"|"^"|"&"|"<"|">" {
    IFD(cout << yytext[0] << "\n"); 
    return yytext[0];
}

"+"|"-"|"*"|"/"|"%"|"!" {
    IFD(cout << yytext[0] << "\n"); 
    return yytext[0];
}

"("|")"|"["|"]"|"{"|"}"|"="|"."|","|":"|";"|"@"|"\\" {
    IFD(cout << yytext[0] << "\n"); 
    return yytext[0];
}

[ \t\r]+ {
    /* Eat up whitespaces, and keep tracking positions. */
    LOCATION_STEP(yylloc);
}

[\n]+ {
    /* Update the line counter and step forward. */
    LOCATION_LINES(yylloc, yyleng); LOCATION_STEP(yylloc);

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

void InputBuffersStack::pop()
{
    BufferInfo& bi = buffers.back();

    if (bi.type == BufferInfo::File) {
	fclose(bi.fin);
    }

    yy_delete_buffer(bi.yybs);
    assert(buffers.size());
    buffers.pop_back();
    yy_switch_to_buffer(buffers.back().yybs);
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

