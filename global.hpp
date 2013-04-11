#ifndef __GLOBAL__HH
#define __GLOBAL__HH

#include <cstdio>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <set>
#include <vector>

using namespace std;

//#define NDEBUG
#include <assert.h>

/* Stuff from flex that bison needs to know about. */
#include "scanner.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

/* Context, ContextsSet and ContextsSetStack. */
#include "context.hpp"

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
    void fill_process_table(SymbolsTable& pt) {
	for (int i=0; i<groups.size(); i++) {
	    int found = 0;
	    int index = -1;
	    SymbolValue * svp;
	    ProcessValue * pvp;

	    /* For each set, we have to find the unique process (if any) that
	       is already in 'pt', e.g. a process that has been defined as a
	       proper process (with a non-NULL ProcessNode*) and not just as
	       an alias. */
	    for (int j=0; j<groups[i].size(); j++) {
		if (pt.lookup(groups[i][j].name, svp)) {
		    found++;
		    index = j;
		} else if (!groups[i][j].assigned) {
		    /* We make sure that every non-properly-defined process
		       has been assigned somewhere as an alias.*/
		    stringstream errstream;
errstream << "$* Process " << groups[i][j].name
			<< " undefined";
		    semantic_error(errstream);
		}
	    }
	    assert(found <= 1);
	    if (found == 0) {
		cerr << "Warning, aliases cycle found: {";
		for (int j=0; j<groups[i].size(); j++)
		    cerr << groups[i][j].name << ", ";
		cerr << "}\n";
		cerr << "    A new STOP process will be associated to these aliases\n";
		pvp = new ProcessValue;
		pvp->pnp = new ProcessNode;
		svp = pvp;
		pt.insert(groups[i][0].name, svp);
		index = 0;
	    }
	    pvp = err_if_not_process(svp);
	    for (int j=0; j<groups[i].size(); j++)
		if (j != index) {
		    ProcessValue * npvp = new ProcessValue;

		    npvp->pnp = pvp->pnp;
		    if (!pt.insert(groups[i][j].name, npvp)) {
			/* This really can't happen. */
			stringstream errstream;
errstream << "Impossible duplicate (BUG)\n";
			semantic_error(errstream);
		    }
		    cout << "Process " << groups[i][j].name << " defined (" << npvp->pnp << ")\n";
		}
	}
    }

    void clear() {
	groups.clear();
    }

    void print() {
	cout << "Aliases:\n";
	for (int i=0; i<groups.size(); i++)
	    for (int j=0; j<groups[i].size(); j++)
		cout << i << ": " << groups[i][j].name << ", " << groups[i][j].assigned << "\n";
    }
};

struct Global {
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


    Global() : actions("Global actions table"), record_mode_on(0) { }

    ContextsSet& current_contexts() { return css.top(); }

    void init_fakenode() {
	vector<ProcessEdge> cv;
	struct ProcessEdge e;
	struct FrontierElement fe;
	vector<FrontierElement> frontier;

	/* The fakenode must have a child for each contexts. In this way the 
	   actions associated to the rule 'prefix_actions-->action_labels' can
	   correctly expand each context and return a ProcessNode* for each
	   child. */
	fe.pnp = &fakenode;
	for (int c=0; c<current_contexts().size(); c++) {
	    e.dest = NULL;
	    e.rank = c;	/* The child will combine with the c-th context. */
	    e.action = 0;	/* Not significant. */
	    cv.push_back(e);
	    /* Put the child in the frontier. */
	    fe.child = c;
	    frontier.push_back(fe);
	}
	fakenode.children = cv;
	current_contexts().frontier = frontier;
    }

    void print_forest() {
	cout << "Current ProcessNode fakenode forest:\n";
	for (int i=0; i<fakenode.children.size(); i++)
	    if (fakenode.children[i].dest)
		fakenode.children[i].dest->print(&actions);
    }
};

#endif
