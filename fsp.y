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
%token END STOP ERROR
%token PROPERTY PROGRESS MENU
%token OR AND EQUAL NOTEQUAL LOE GOE LSHIFT RSHIFT

%token <int_value> INTEGER
%token <float_value> FLOAT
%token <string_pointer> UpperCaseID
%token <string_pointer> LowerCaseID

%glr-parser

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
    | property_def
    | progress_def
    | menu_def
    | process_def
    ;


/* Action labels TODO: fix */
action_labels:
    LowerCaseID { T(4); }
    | set { T(5); }
    | action_labels '.' LowerCaseID { T(5); }
    | action_labels '.' set { T(6); }
    | action_labels '[' action_range ']'
    | action_labels '[' expression ']' { T(7); }
    ;

set:
    set_id { T(15); }
    | set_expr
    ;

set_expr:
    '{' set_elements '}' { T(16); }
    ;

action_range:
    range_or_set_id
    | range_expr
    | set_expr
    | variable ':' range_or_set_id
    | variable ':' range_expr
    | variable ':' set_expr
    ;

range_expr:
    expression DOTDOT expression
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


/* Processes */
process_def:
    process_id param_OPT '=' process_body alphabet_extension_OPT '.' {T(21);}
    ;

process_body:
    local_process {T(22);}
    | local_process ',' local_process_defs {T(23);}
    ;

local_process_defs:
    local_process_def {T(24);}
    | local_process_defs ',' local_process_def {T(25);}
    ;

local_process_def:
    process_id index_ranges_OPT '=' local_process {T(26);}
    ;

alphabet_extension_OPT:
    | '+' set {T(27);}
    ;

local_process:
    base_local_process {T(28);}
    | sequential_composition
    | IF expression THEN local_process {T(29);}
    | IF expression THEN local_process ELSE local_process {T(30);}
    | '(' choice ')' {T(31);}
    ;

base_local_process:
    END {T(32);}
    | STOP {T(33);}
    | ERROR {T(34);}
    | process_id indices_OPT {T(35);}
    ;

choice:
    action_prefix {T(36);}
    | choice '|' action_prefix {T(37);}
    ;

action_prefix:
    guard_OPT prefix_actions ARROW local_process {T(38);}
    ;

prefix_actions:
    action_labels {T(39);}
    | prefix_actions ARROW action_labels {T(40);}
    ;

guard_OPT:
    | WHEN expression  {T(41);}
    ;

indices_OPT:
    | indices {T(42);}
    ;

indices:
    '[' expression ']' {T(43);}
    | indices '[' expression ']' {T(44);}
    ;

index_ranges_OPT:
    | index_ranges
    ;

index_ranges:
    '[' expression ']'
    | index_ranges '[' expression ']'
    | '[' action_range ']'
    | index_ranges '[' action_range ']'
    ;

sequential_composition:
    seq_process_list ';' base_local_process
    ;

seq_process_list:
    process_ref
    | seq_process_list ';' process_ref
    ;

process_ref:
    process_id argument_OPT
    ;

argument_OPT:
    |'(' argument_list ')'
    ;

argument_list:
    expression
    | argument_list ',' expression
    ;


/* Composite processes */
ranges_OPT:
    | ranges;

ranges:
    '[' action_range ']'
    | ranges '[' action_range ']'
    ;


/* Parameters */
param_OPT:
    | param
    ;

param:
    '(' parameter_list ')'
    ;

parameter_list:
    parameter
    | parameter_list ',' parameter
    ;

parameter:
    parameter_id '=' expression
    ;


/* Property, Progress and Menu */
// TODO second form of progress
property_def:
    PROPERTY process_def
    ;

progress_def:
    PROGRESS progress_id ranges_OPT '=' set
    ;

menu_def:
    MENU menu_id '=' set
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
/* TODO: complete base_expr */
base_expr:
    INTEGER
    | variable
    | constant_id
    | '(' expression ')'
    ;

/* Some useful alias for LowerCaseID and UpperCaseID. */
variable: LowerCaseID;
constant_id: UpperCaseID;
range_id: UpperCaseID;
set_id: UpperCaseID;
range_or_set_id: set_id;
parameter_id: UpperCaseID;
process_id: UpperCaseID;
progress_id: UpperCaseID;
menu_id: UpperCaseID;

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

