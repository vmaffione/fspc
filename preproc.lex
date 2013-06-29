%{
#include <cstdio>
#include <iostream>
#include <fstream>
#include <set>

using namespace std;

#define DEBUG
#ifdef DEBUG
#define IFD(x) (x)
#else
#define IFD(x) 
#endif


struct Preproc {
    fstream out;

    set<string> ranges;
    set<string> sets;
};

struct Preproc p;


/* The following is executed before each rule's action. */
#define YY_USER_ACTION \
    do { \
	; \
    } while (0);


/* ==============================================================
   ============================================================== */
%}



DIGIT		[0-9]
LowerCaseID	[_a-z][_a-zA-Z0-9]*
UpperCaseID	[A-Z][_a-zA-Z0-9]*

/* We don't want to take a standard yywrap() from fl.so, and so we can
   avoid linking the executable with -lfl. */
%option noyywrap

%option outfile="preproc.cpp"
%option prefix="pp"

%%

%{
  /* This code, which appears before the first rule, is copied 
     verbatim at the beginning of yylex(). At each yylex() 
     invocation, therefore, we mark the current last position as the
     start of the next token.  */
%}


range[ \t\r\n]+{UpperCaseID} {
    string s;
    int i = 5;

    IFD(cout << yytext << "\n");
    while (yytext[i] == ' ' || yytext[i] == '\t' || yytext[i] == '\r'
	    || yytext[i] == '\n') {
	i++;
    }
    p.ranges.insert(&yytext[i]);
    s = yytext;
    s.insert(i, "$r");
    p.out << s;
}

set[ \t]+{UpperCaseID} {
    string s;
    int i = 3;

    IFD(cout << yytext << "\n");
    while (yytext[i] == ' ' || yytext[i] == '\t' || yytext[i] == '\r'
	    || yytext[i] == '\n') {
	i++;
    }
    p.sets.insert(&yytext[i]);
    s = yytext;
    s.insert(i, "$s");
    p.out << s;
    p.out << yytext;
}

{UpperCaseID} {
    string s = yytext;

    IFD(cout << "UpperCaseID " << yytext << "\n");
    if (p.ranges.count(s)) {
	s.insert(0, "$r");
    } else if (p.sets.count(s)) {
	s.insert(0, "$s");
    }
    p.out << s;
}

.|\n {
    // TODO don't filter out
    p.out << yytext;
}

%%

int main()
{
    const char * input_name = "input.fsp";
    FILE *fin = fopen(input_name, "r");

    if (!fin) {
	cerr << "Input error: Can't open " << input_name << "\n";
	return -1;
    }
    yyin = fin;
    p.out.open("o.fsp", ios::out);

    yylex();

    p.out.close();
    fclose(fin);

    return 0;
}

