#ifndef __GLOBAL__HH
#define __GLOBAL__HH

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <set>
#include <vector>

using namespace std;

//#define NDEBUG
#include <assert.h>

#include "context.hpp"
#include "symbols_table.hpp"
#include "utils.hpp"
#include "callbacks.hpp"


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


struct FspCompiler;

struct FspTranslator {
    FspCompiler * gdp;

    /* Names of local processes. */
    struct SymbolsTable local_processes;

    /* A stack of contexts set for translating a process_def. */
    struct ContextsSetStack css;

    struct ProcessNode fakenode;

    struct Aliases aliases;


    FspTranslator(struct FspCompiler * p) : gdp(p) {
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
};


struct FspCompiler {
    /* Main actions table. */
    struct ActionsTable actions;

    /* Const, Range, Set and Parameter names. */
    struct SymbolsTable identifiers;

    /* Names of global processes. */
    struct SymbolsTable processes;
    
    int record_mode_on;
    ParametricProcess parametric;
    struct SymbolsTable process_models;

    /* Storage for a list of parameters identifiers to remove from the 
       identifiers table when a process definition has been completed. */
    vector<string *> parameters; //XXX obsoleted by 'parametric'??

    /* The main translator. */
    FspTranslator tr;

    FspCompiler() : actions("Global actions table"), record_mode_on(0), 
							    tr(this) { }

};



#endif
