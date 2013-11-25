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
#include <stdint.h>

#include "symbols_table.hpp"
#include "location.hh"

using namespace std;


struct TerminalSet {
    vector<int> trace;
    set<unsigned int> actions;
};

/* An LTS edge. */
struct Edge {
    uint32_t dest;
    uint32_t action;
};

#define INFO_TYPE_BITS  3U

struct LtsNode {
    vector<Edge> children;
    unsigned int info; /* Stores type. */

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
    static const int Incomplete = 3;
    static const int Unresolved = 4;
    static const int Zombie = 5;

    static const unsigned int TypeMask = (1 << INFO_TYPE_BITS) - 1;
    static const unsigned int NoPriv = ~0U;

    void offset(int offset);
};

class Serializer;
class Deserializer;
class Shell;
class FspDriver;


namespace yy {

class Lts;

typedef void (*LtsVisitFunction)(int, const Lts&, const struct LtsNode&, void*);

struct LtsVisitObject {
    LtsVisitFunction vfp;
    void *opaque;
};

class Lts: public Symbol {
    vector<LtsNode> nodes;
    vector<unsigned int> privs;
    ActionsTable * atp;

    set<int> alphabet;

    vector<TerminalSet> terminal_sets;
    bool terminal_sets_computed;

    void copy_node_in(int state, const Lts& lts, int i);
    void copy_node_out(Lts& lts, int i, int state);
    void copy_nodes_in(const Lts& lts);

    void update_composition(unsigned int idx, unsigned int dst_ip,
                            const yy::Lts& p, unsigned int dst_iq,
                            const yy::Lts& q, unsigned int nq, Edge& e,
                            map<unsigned int, unsigned int>& direct,
                            vector<unsigned int>& inverse);
    void compose_declarative(const Lts& p, const Lts& q);
    void compose_operational(const Lts& p, const Lts& q);

    /* Pointer to a composition algorithm (member function). */
    typedef void (Lts::*ComposeAlgorithm)(const Lts&, const Lts&);
    /* Pointer to the currently used composition algorithm. */
    static ComposeAlgorithm compose_algorithm;

    void compose(const Lts& p, const Lts& q);
    void reduce(const Lts& unconnected);
    void print_trace(const vector<int>& trace, stringstream& ss) const;
    void removeType(unsigned int type, unsigned int zero_idx,
                    bool call_reduce);

    friend class ::Serializer;
    friend class ::Deserializer;

  public:
    string name;

    Lts() { atp = NULL; } /* Invalid instance, used by tree. */
    Lts(int, struct ActionsTable *); /* One state Lts: Stop, End or Error */
    Lts(const Lts& p, const Lts& q); /* Parallel composition */
    int numStates() const { return nodes.size(); }
    int numTransitions() const;
    int deadlockAnalysis(stringstream& ss) const;
    int terminalSets();
    bool isDeterministic() const;
    Lts& compose(const Lts& q);
    Lts& labeling(const SetS& labels);
    Lts& labeling(const string& label);
    Lts& sharing(const SetS& labels);
    Lts& relabeling(const SetS& newlabels, const SetS& oldlabels);
    Lts& relabeling(const SetS& newlabels, const string& oldlabel);
    Lts& hiding(const SetS& s, bool interface);
    Lts& priority(const SetS& s, bool low);
    Lts& property();
    int progress(const string& progress_name, const ProgressS& pr,
		    stringstream& ss);
    void visit(const struct LtsVisitObject&) const;
    void graphvizOutput(const char * filename) const;
    void simulate(Shell& sh, const ActionSetS *asv) const;
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
    unsigned int resolve();
    Lts& mergeEndNodes();
    bool endcat(const Lts& lts);

    void set_priv(unsigned int state, unsigned int val);
    unsigned int get_priv(unsigned int state) const;
    void set_type(unsigned int state, unsigned int type) {
        nodes[state].info &= ~LtsNode::TypeMask;
        nodes[state].info |= type & LtsNode::TypeMask;
    }
    unsigned int get_type(unsigned int state) const {
        return nodes[state].info & LtsNode::TypeMask;
    }
    void check_privs(set<unsigned int>& privs);
    void replace_priv(unsigned int new_priv, unsigned int old_priv);

    void clear();
    void cleanup();

    /* Methods to implement because of the base class. */
    void print() const;
    const char *className() const { return "Lts"; }
    Symbol * clone() const;
};

yy::Lts * err_if_not_lts(FspDriver& driver, Symbol * svp, const yy::location& loc);

} /* namespace yy */

#endif
