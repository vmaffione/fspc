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


struct FspTranslator;

struct AliasElement {
    /* Process name */
    string name;
    /* True if the process have ever been assigned an alias
       (eg. P[3] = T[12][18]). */
    bool assigned;

    AliasElement(const string& nm, bool l) : name(nm), assigned(l) { }
};

/* The purpose of this data structure is to store a list of sets. Each
   sets must contain all the process names that are aliases with each
   other. */
struct Aliases {
    vector< vector<AliasElement> > groups;
    FspTranslator& tr;

    Aliases(FspTranslator& r) : tr(r) { }

    /* When calling insert(left, right), the translator wants to say: Ehy,
       I've found something like 'left = right', where both left and right
       are process names (e.g. when parsing "P = P[0], P[i:R] = (...).", the
       translator will call insert("P","P[0]") ). */
    void insert(const string& left, const string& right);
    /* Tell the data structure to insert all the aliases in the SymbolsTable
       'pt'. The translator will call this function when it wants to move
       the aliases collected during the translation into a process symbols
       table. */
    void fill_process_table(SymbolsTable& pt);
    void clear();
    void print();
};


#include "callbacks.hpp"

class FspDriver;

struct FspTranslator {
    FspDriver& dr;

    /* Names of local processes. */
    struct SymbolsTable local_processes;

    /* A stack of contexts set for translating a process_def. */
    struct ContextsSetStack css;

    struct ProcessNode fakenode;

    struct Aliases aliases;

    vector<string> overridden_names;
    vector<struct SymbolValue *> overridden_values;

    /* We keep track of alphabet extension when using sequential
       process (see toProcessNode in callback__66. */
    set<int> alphabet_extension;

    yy::location locations[8];


    FspTranslator(struct FspDriver& r) : dr(r), aliases(*this) {
	/* Initialize shared data structures: A stack containing a single
	   ContextsSet. This set contains a single empty Context and an
	   empty frontier. */
	ContextsSet * csp = new ContextsSet;
	csp->append(new Context);
	css.push(csp);
    }

    ContextsSet& current_contexts() { return css.top(); }

    void init_fakenode();

    void print_fakenode_forest();

    ~FspTranslator() {
    }
};


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

	int record_mode_on;
	ParametricProcess * parametric;
	struct SymbolsTable parametric_processes;

	/* The main translator. */
	FspTranslator tr;

        /* ========================== New API ======================== */
        NewContext ctx;

        UnresolvedNames unres;

        /* The names of the parameters used in a process definition. */
        NewParametricProcess paramproc;

        /* The parsing result. */
        yy::TreeNode *tree;

        /* If true the parse tree will be built in a "less" recursive way,
           meaning that left recursive grammar rules will be "unrolled". */
        bool iterated_tree; // TODO force iterted_tree=true and remove

        /* =========================================================== */

	/* The ProcessNode allocator. */
	ProcessNodeAllocator pna;

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

	/* Error handling. */
	void error(const yy::location& l, const std::string& m);
	void error(const std::string& m);
};

#endif // ! __DRIVER__HH

