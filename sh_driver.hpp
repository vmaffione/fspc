#ifndef __SH_DRIVER__HH
#define __SH_DRIVER__HH

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <set>
#include <vector>
#include <string>
#include <cstdio>

//#define NDEBUG
#include <assert.h>

#include "sh_parser.hpp"


using namespace std;


/* Tell Flex the lexer's prototype... */
#define YY_DECL                                        \
  sh::ShParser::token_type                         \
  shlex(sh::ShParser::semantic_type* shlval,      \
         ShDriver& driver)
/* ... and declare it for the parser's sake. */
YY_DECL;


class ShDriver
{
    public:
	ShDriver();
	virtual ~ShDriver();

	/* Handling the scanner. */
	void scan_begin(const string&);
	void scan_end();
	bool trace_scanning;

	/* Run the parser.  Return 0 on success. */
	int parse();
	bool trace_parsing;

	/* Error handling. */
	//void error(const sh::location& l, const std::string& m);
	void error(const std::string& m);
};

#endif // ! __SH_DRIVER__HH

