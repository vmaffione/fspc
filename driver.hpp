#ifndef __DRIVER__HH
#define __DRIVER__HH

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <set>
#include <vector>
#include <string>
#include <cstdio>

//#define NDEBUG
#include <assert.h>

#include "context.hpp"
#include "symbols_table.hpp"
#include "utils.hpp"
#include "parser.hpp"
#include "interface.hpp"
#include "unresolved.hpp"


using namespace std;


/* Tell Flex the lexer's prototype... */
#define YY_DECL                                        \
  yy::FspParser::token_type                         \
  yylex(yy::FspParser::semantic_type* yylval,      \
         yy::FspParser::location_type* yylloc,      \
         FspDriver& driver)
/* ... and declare it for the parser's sake. */
YY_DECL;

namespace yy {
    class TreeNode;
};

struct NestingContext {
    Context ctx;
    UnresolvedNames unres;
    ParametricProcess paramproc;
    vector<string> overridden_names;
    vector<Symbol *> overridden_values;
    bool replay;
};

/* Conducting the whole scanning and parsing of fspcc. */
class FspDriver
{
    public:
	/* Main actions table. */
	struct ActionsTable actions;

	/* Const, Range, Set and Parameter objects. */
	struct SymbolsTable identifiers;

	/* Global processes. */
	struct SymbolsTable processes;

	/* Progress properties. */
	struct SymbolsTable progresses;

        /* Menu sets. */
        struct SymbolsTable menus;

	struct SymbolsTable parametric_processes;

        /* Current value of variables (e.g. action/process indexes). */
        Context ctx;

        /* Keep track of process names to be resolved and their aliases. */
        UnresolvedNames unres;

        /* The names of the parameters used in a process definition. */
        ParametricProcess paramproc;

        /* Overridden names support. */
        vector<string> overridden_names;
        vector<Symbol *> overridden_values;

        /* Are we currently in replay mode? */
        bool replay;

        /* Nesting support for parametric process references. */
        vector<NestingContext> nesting_stack;
        void nesting_save(bool replay);
        void nesting_restore();

        /* The parsing result. */
        yy::TreeNode *tree;
        /* =========================================================== */

	string remove_file;
	string current_file;


	FspDriver();
	virtual ~FspDriver();
	void clear();	/* Destructor like */

	/* Handling the scanner. */
	void scan_begin(const char * filename);
	void scan_end();
	bool trace_scanning;

	/* Run the parser.  Return 0 on success. */
	int parse(const CompilerOptions& co);
	bool trace_parsing;

        void findProcessesDefinitions();

	/* Error handling. */
	void error(const yy::location& l, const std::string& m);
	void error(const std::string& m);
};

#endif // ! __DRIVER__HH

