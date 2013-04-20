%{
#include <cstdio>
#include <iostream>
#include <assert.h>

using namespace std;


#include "parser.hpp"  // to get the token types definition that we return


//#define DEBUG
#ifdef DEBUG
#define IFD(x) (x)
#else
#define IFD(x) 
#endif

%}

DIGIT		[0-9]
LowerCaseID	[_a-z][_a-z0-9]*
UpperCaseID	[A-Z][A-Z0-9]*

%x COMMENTS

/* We don't want to take a standard yywrap() from fl.so, and so we can
   avoid linking the executable with -lfl. */
%option noyywrap

%option outfile="scanner.cpp"

/* %option batch */

%%

"/*" {
    BEGIN(COMMENTS);
}

<COMMENTS>"*/" {
    BEGIN(INITIAL);
}

<COMMENTS>[ \n\t\r]|. {
    /* Throw away anything when in comments state */
}

{DIGIT}+ {
    yylval.int_value = atoi(yytext);
    IFD(cout << "INTEGER: " << yylval.int_value << "\n");
    return INTEGER;
}

{DIGIT}+\.{DIGIT}+ {
    // TODO Useless ==> REMOVE!
    yylval.float_value = atof(yytext); return FLOAT;
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

[ \t\n\r]+ {
    /* eat up whitespaces */
}

. {
    cout << "Unrecognized character " << yytext << endl;
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
	cerr << "I can't open " << input_name << " !\n";
	throw int();
    }
    yybs = yy_create_buffer(fin, YY_BUF_SIZE);
    if (yybs == NULL) {
	cerr << "yy_create_buffer() returned NULL\n";
	throw int();
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
	throw int();
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
	cerr << "I can't open " << input_name << " !\n";
	throw int();
    }
    bi.yybs = yy_create_buffer(bi.fin, YY_BUF_SIZE);
    if (bi.yybs == NULL) {
	cerr << "yy_create_buffer() returned NULL\n";
	throw int();
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
	throw int();
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
	cout << "I can't open " << input_name << " !\n";
	return -1;
    }

    yyin = fin;

    cout << "FLEX buffer " << YY_CURRENT_BUFFER << "\n";

    return 0;
}

