%{
#include <cstdio>
#include <iostream>

using namespace std;


#include "fsp.tab.h"  // to get the token types definition that we return
%}

DIGIT		[0-9]
LowerCaseID	[_a-z][_a-z0-9]*
UpperCaseID	[A-Z][A-Z0-9]*

%%

{DIGIT}+ {
    yylval.int_value = atoi(yytext); return INTEGER;
}

{DIGIT}+\.{DIGIT}+ {
    // TODO Useless ==> REMOVE!
    yylval.float_value = atof(yytext); return FLOAT;
}

if { return IF; }
then { return THEN; }
else { return ELSE; }
when { return WHEN; }
const { return CONST; }
range { return RANGE; }
END { return END; }
STOP { return STOP; }

{LowerCaseID} {
    yylval.string_pointer = strdup(yytext);
    return LowerCaseID;
}

{UpperCaseID} {
    yylval.string_pointer = strdup(yytext);
    return UpperCaseID;
}

"->" {
    return ARROW;
}

".." {
    return DOTDOT;
}

"||" {
    return OR;
}

"&&" {
    return AND;
}

"==" {
    return EQUAL;
}

"!=" {
    return NOTEQUAL;
}

"<=" {
    return LOE;
}

">=" {
    return GOE;
}

"<<" {
    return RSHIFT;
}

">>" {
    return LSHIFT;
}

"|"|"^"|"&"|"<"|">" {
    return yytext[0];
}

"+"|"-"|"*"|"/"|"%"|"!" {
    return yytext[0];
}

"("|")"|"["|"]"|"{"|"}"|"="|"."|","|":" {
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

