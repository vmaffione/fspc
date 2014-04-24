#include "fsp_parser.hpp"

/* The YY_DECL declaration is needed by:
    (1) fsp.lex (fsp_scanner.cpp), because Flex must define the
        prototype for the yylex() function.
    (2) fsp.ypp (fsp_parser.cpp), because Bison must know the
        lexer function prototype.
*/

/* Tell Flex the lexer's prototype... */
#define YY_DECL                                        \
  fsp::FspParser::token_type                         \
  fsplex(fsp::FspParser::semantic_type* fsplval,      \
         fsp::FspParser::location_type* fsplloc)
/* ... and declare it for the parser's sake. */
YY_DECL;

