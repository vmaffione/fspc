/*
 *  fspc Labelled Transition System (LTS) implementation
 *
 *  Copyright (C) 2013  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __LTS__H__
#define __LTS__H__

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <assert.h>

#include "symbols_table.hpp"
#include "location.hh"

using namespace std;


struct TerminalSet {
    vector<int> trace;
    set<int> actions;
};

/* An LTS edge. */
struct Edge {
    unsigned int dest;
    unsigned int action;
};

struct LtsNode {
    vector<Edge> children;
    unsigned int type; /* set if the node is an END node */
    unsigned int priv;

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
    static const int Incomplete = 3;
    static const int Unresolved = 4;

    void offset(int offset);
};

typedef void (*LtsVisitFunction)(int, const struct LtsNode&, void*);

struct LtsVisitObject {
    LtsVisitFunction vfp;
    void * opaque;
};

class Serializer;
class Deserializer;
class Shell;
class FspDriver;


namespace yy {

class Lts: public SymbolValue {
    vector<LtsNode> nodes;
    ActionsTable * atp;

    set<int> alphabet;

    vector<TerminalSet> terminal_sets;
    bool terminal_sets_computed;

    void compose(const Lts& p, const Lts& q);
    void reduce(const vector<LtsNode>& unconnected);
    void print_trace(const vector<int>& trace, stringstream& ss) const;
    void removeType(unsigned int type);

    friend void lts_convert(struct ProcessNode * pnp, void * opaque);
    friend class ::Serializer;
    friend class ::Deserializer;

  public:
    string name;
    int rank;

    Lts() { atp = NULL; } /* Invalid instance, used by tree. */
    Lts(int, struct ActionsTable *); /* One state Lts: Stop, End or Error */
    Lts(const struct ProcessNode *, struct ActionsTable *);
    Lts(const Lts& p, const Lts& q); /* Parallel composition */
    int numStates() const { return nodes.size(); }
    int numTransitions() const;
    int deadlockAnalysis(stringstream& ss) const;
    int terminalSets();
    bool isDeterministic() const;
    Lts& compose(const Lts& q);
    Lts& labeling(const SetValue& labels);
    Lts& labeling(const string& label);
    Lts& sharing(const SetValue& labels);
    Lts& relabeling(const SetValue& newlabels, const SetValue& oldlabels);
    Lts& relabeling(const SetValue& newlabels, const string& oldlabel);
    Lts& hiding(const SetValue& s, bool interface);
    Lts& priority(const SetValue& s, bool low);
    Lts& property();
    int progress(const string& progress_name, const SetValue& s,
		    stringstream& ss);
    void visit(const struct LtsVisitObject&) const;
    ProcessNode* toProcessNode(ProcessNodeAllocator&) const;
    void graphvizOutput(const char * filename) const;
    void simulate(Shell& sh) const;
    void basic(const string& outfile, stringstream& ss) const;

    void updateAlphabet(int action);
    int lookupAlphabet(int action) const;
    void mergeAlphabetInto(set<int>& actions) const;
    void mergeAlphabetFrom(const set<int>& actions);
    int alphabetSize() const { return alphabet.size(); }
    void printAlphabet(stringstream& ss) const;
    unsigned int append(const Lts&, unsigned int first);
    Lts& zerocat(const Lts& lts, const string& label);
    Lts& incompcat(const vector<Lts>& ltsv);
    Lts& zeromerge(const Lts& lts);
    Lts& resolve();

    void set_priv(unsigned int state, unsigned int val);
    unsigned int get_priv(unsigned int state) const;

    /* Methods to implement because of the base class. */
    void print() const;
    int type() const { return SymbolValue::Lts; }
    SymbolValue * clone() const;
};

yy::Lts * err_if_not_lts(FspDriver& driver, SymbolValue * svp, const yy::location& loc);

/* A list of Lts. */
struct LtsComposition: public SymbolValue {
    vector<class Lts *> lts;
    int rank;

    void print() const;
    int type() const { return SymbolValue::LtsComposition; }
    SymbolValue * clone() const;
};

LtsComposition * err_if_not_ltscomposition(FspDriver& driver, SymbolValue * svp,
					    const yy::location& loc);

} /* namespace yy */


inline yy::Lts * is_lts(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Lts);

    return static_cast<yy::Lts *>(svp);
}


inline yy::LtsComposition * is_ltscomposition(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::LtsComposition);

    return static_cast<yy::LtsComposition *>(svp);
}

#endif
