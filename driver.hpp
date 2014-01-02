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
#include "lts.hpp"


using namespace std;


/* Tell Flex the lexer's prototype... */
#define YY_DECL                                        \
  fsp::FspParser::token_type                         \
  fsplex(fsp::FspParser::semantic_type* fsplval,      \
         fsp::FspParser::location_type* fsplloc,      \
         FspDriver& driver)
/* ... and declare it for the parser's sake. */
YY_DECL;

namespace fsp {
    class TreeNode;
};

struct NestingContext {
    Context ctx;
    UnresolvedNames unres;
    ParametricProcess parameters;
    vector<string> overridden_names;
    vector<Symbol *> overridden_values;
};

class DependencyGraph {
        map<string, vector<string> > table;
        typedef map< string, vector<string> >::iterator table_iterator;

    public:
        bool add(const string& depends, const string& on);
        void findDependencies(const string& depends, vector<string>& result);
        void print();
};

/* Conducting the whole scanning and parsing of fspcc. */
class FspDriver
{
    public:
        CompilerOptions cop;

	/* Const, Range, Set and Parameter objects. */
	SymbolsTable identifiers;

	/* Global processes. */
	SymbolsTable processes;

	/* Progress properties. */
	SymbolsTable progresses;

        /* Menu sets. */
        SymbolsTable menus;

        /* Dependency graph of non local processes. */
        DependencyGraph deps;

        /* Stores the root node of each non local process definition
           (both simple and composite processes) along with default
           parameter values. */
	SymbolsTable parametric_processes;

        /* Current value of variables (e.g. action/process indexes). */
        Context ctx;

        /* Keep track of process names to be resolved and their aliases. */
        UnresolvedNames unres;

        /* The names and values of the parameters used in a process
           translation. This is used to (1) restore the previous
           translator context and (2) compute the extended name of
           an LTS. */
        ParametricProcess parameters;

        /* Overridden names support. */
        vector<string> overridden_names;
        vector<Symbol *> overridden_values;

        /* Nesting support for parametric process references. */
        vector<NestingContext> nesting_stack;
        bool nesting_save();
        void nesting_restore();

        /* The parsing result. */
        fsp::TreeNode *tree;

        /* Preprocessed file, input to the parser. */
	string remove_file;


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

        void translateDeclarations();
        bool shouldTranslateNow(const string& name);
        void findParametricProcesses();
        void computeDependencyGraph();
        void doProcessesTranslation();
        void translateProcessesDefinitions();

        fsp::SmartPtr<fsp::Lts> getLts(const string& name, bool create);

	/* Error handling. */
	void error(const fsp::location& l, const std::string& m);
	void error(const std::string& m);
};

#endif // ! __DRIVER__HH

