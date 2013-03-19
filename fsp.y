%{
#include <cstdio>
#include <iostream>
using namespace std;

// stuff from flex that bison needs to know about:
//extern "C" int yylex();
extern int yylex();
//extern "C" int yyparse();
//extern "C" FILE *yyin;
extern FILE * yyin;

void T(int x)
{
    cout << "<<B>> " << x << "\n";
}

void yyerror(const char *s);
%}

// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":
%union {
    int int_value;
    float float_value;
    char *string_pointer;
}

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:


%token IF THEN ELSE
%token WHEN
%token CONST RANGE SET
%token ARROW
%token DOTDOT
%token END STOP

%token OR AND EQUAL NOTEQUAL LOE GOE LSHIFT RSHIFT

%token <int_value> INTEGER
%token <float_value> FLOAT
%token <string_pointer> UpperCaseID
%token <string_pointer> LowerCaseID


%%

/* Start symbol: an arbitrary long list of fsp_definitions */
fsp_description:
    fsp_definition
    | fsp_description fsp_definition
    ;

/* All the possible type of fsp_definitions */
fsp_definition:
    constant_def
    | range_def
    | set_def
    | action_labels
    ;


/* Action labels TODO: fix */
action_labels:
    LowerCaseID { T(4); }
    | set { T(5); }
    | action_labels '.' LowerCaseID { T(5); }
    | action_labels '.' set { T(6); }
    | action_labels '[' action_range ']' { T(7); }
    ;

action_range:
    range { T(9); }
    | set { T(10); }
    | variable ':' range { T(11); }
    | variable ':' set { T(12); }
    ;

range:
    range_id { T(13); }
    | expression DOTDOT expression { T(14); }
    ;

set:
    set_id { T(15); }
    | '{' set_elements '}' { T(16); }
    ;


/* Const, Range, Set */
constant_def:
    CONST constant_id '=' simple_expression {
	cout << "<<B>> Const\n";
    };

range_def:
    RANGE range_id '=' simple_expression DOTDOT simple_expression {
	cout << "<<B>> Range\n";
    };

set_def:
    SET set_id '=' '{' set_elements '}' {
	cout << "<<B>> Set\n";
    };

set_elements:
    action_labels
    | set_elements ',' action_labels
    ;


/* An expression or a simple_expression: standard operators and priorities. */
expression:
    or_expr {
	cout << "<<B>> Expression recognized!\n";
    }
    ;

simple_expression:
    additive_expr {
	cout << "<<B>> Simple expression recognized!\n";
    }
    ;

or_expr:
    and_expr
    | or_expr OR and_expr
    ;

and_expr:
    bitor_expr
    | and_expr AND bitor_expr
    ;

bitor_expr:
    bitxor_expr
    | bitor_expr '|' bitxor_expr
    ;

bitxor_expr:
    bitand_expr
    | bitxor_expr '^' bitand_expr
    ;

bitand_expr:
    equality_expr
    | bitand_expr '&' equality_expr
    ;

equality_expr:
    relational_expr
    | equality_expr EQUAL relational_expr
    | equality_expr NOTEQUAL relational_expr
    ;

relational_expr:
    shift_expr
    | relational_expr '<' shift_expr
    | relational_expr '>' shift_expr
    | relational_expr LOE shift_expr
    | relational_expr GOE shift_expr
    ;

shift_expr:
    additive_expr
    | shift_expr LSHIFT additive_expr
    | shift_expr RSHIFT additive_expr
    ;

additive_expr:
    multiplicative_expr
    | additive_expr '+' multiplicative_expr
    | additive_expr '-' multiplicative_expr
    ;

multiplicative_expr:
    unary_expr
    | multiplicative_expr '*' unary_expr
    | multiplicative_expr '/' unary_expr
    | multiplicative_expr '%' unary_expr
    ;

unary_expr:
    base_expr
    | '+' base_expr
    | '-' base_expr
    | '!' base_expr
    ;

base_expr:
    INTEGER
    | variable
    | constant_id
    ;

/* Some useful alias for LowerCaseID and UpperCaseID. */
variable: LowerCaseID;
constant_id: UpperCaseID;
range_id: UpperCaseID;
set_id: UpperCaseID;

%%

int main() {
    // open a file handle to a particular file:
    FILE *myfile = fopen("input", "r");
    // make sure it is valid:
    if (!myfile) {
	cout << "I can't open 'input'!" << endl;
	return -1;
    }
    // set flex to read from it instead of defaulting to STDIN:
    yyin = myfile;
    
    // parse through the input until there is no more:
    do {
	yyparse();
    } while (!feof(yyin));
    
    return 0;
}

void yyerror(const char *s) {
    cout << "EEK, parse error!  Message: " << s << endl;
    // might as well halt now:
    exit(-1);
}


/*
action_labels:
    action_label
    | set
    | action_labels '.' action_label
    | action_labels '.' set
    | action_labels '[' action_range ']'
    | action_labels '[' expression ']'
    ;

action_range:
    range
    | set
    | variable: range
    | variable: set
    ;

range:
    range_id
    | expression DOTDOT expression
    ;

set:
    set_id
    | '{' set_elements '}'
    ;
*/
