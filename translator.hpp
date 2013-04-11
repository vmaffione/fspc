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

struct FspTranslator {
    /* Main actions table. */
    struct ActionsTable actions;

    /* Const, Range, Set and Parameter names. */
    struct SymbolsTable identifiers;

    /* Names of local processes. */
    struct SymbolsTable local_processes;

    /* Names of global processes. */
    struct SymbolsTable processes;

    struct ContextsSetStack css;

    /* Storage for a list of parameters identifiers to remove from the 
       identifiers table when a process definition has been completed. */
    vector<string *> parameters;

    struct ProcessNode fakenode;

    struct Aliases aliases;

    int record_mode_on;
    struct SymbolsTable process_models;


    FspTranslator() : actions("Global actions table"), record_mode_on(0) { }

    ContextsSet& current_contexts() { return css.top(); }

    void init_fakenode();

    void print_fakenode_forest();
};

#endif
