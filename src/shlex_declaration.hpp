#include "sh_parser.hpp"

/* The YY_DECL declaration is needed by:
    (1) sh.lex (sh_scanner.cpp), because Flex must define the
        prototype for the yylex() function.
    (2) sh.ypp (sh_parser.cpp), because Bison must know the
        lexer function prototype.
*/

/* Tell Flex the lexer's prototype... */
#define YY_DECL \
    sh::ShParser::token_type \
    shlex(sh::ShParser::semantic_type* shlval)
/* ... and declare it for the parser's sake. */
YY_DECL;

