%{
#include <cstdio>
#include <iostream>

using namespace std;


#include "fsp.tab.hpp"  // to get the token types definition that we return


#define DEBUG
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

%%

"/*" {
    BEGIN(COMMENTS);
}

<COMMENTS>"*/" {
    BEGIN(INITIAL);
}

<COMMENTS>. {
    /* Throw away everything when in the COMMENTS state */
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
    yylval.string_pointer = strdup(yytext);
    IFD(cout << "LowerCaseID\n"); 
    return LowerCaseID;
}

{UpperCaseID} {
    yylval.string_pointer = strdup(yytext);
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

"<<" {
    IFD(cout << "<<\n"); 
    return RSHIFT;
}

">>" {
    IFD(cout << ">>\n"); 
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

[ \t\n]+ {
    /* eat up whitespaces */
}

. {
    cout << "Unrecognized character " << yytext << endl;
    exit(1);
}
%%

