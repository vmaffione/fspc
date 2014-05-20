/*
 *  fspc Labelled Transition System (LTS) implementation
 *
 *  Copyright (C) 2013-2014  Vincenzo Maffione
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

#include "symbols_table.hpp"
#include "location.hh"

#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <string>
#include <set>
#include <assert.h>
#include <stdint.h>

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

/* A complete LTS edge (includes source node).
   Only used internally. */
class CEdge {
    public:
        uint32_t src;
        uint32_t action;
        uint32_t dest;

        bool operator==(const CEdge& e) const
        {
            return src == e.src && action == e.action && dest == e.dest;
        }
        bool operator<(const CEdge& e) const
        {
            return (src < e.src) ||
                    (src == e.src && action < e.action) ||
                    (src == e.src && action == e.action && dest < e.dest);
        }
};

/* An LTS node, containing a list of edges. */
struct LtsNode {
    vector<Edge> children;

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
    static const int Incomplete = 3;
    static const int Unresolved = 4;
    static const int Zombie = 5;

    static const unsigned int NoPriv = ~0U;

    void offset(int offset);
};

/* Private information associated to each LTS node. This information
   is not stored into the LtsNode class, because it is only necessary
   when parsing an FSP "process definition". In particular, we don't
   need this info when parsing an FSP "composite process definition",
   where the memory demand can be extremely high. In this way we can
   use the info when needed and free the memory when it is not
   needed anymore. */
struct LtsNodeInfo {
    unsigned int priv;
    unsigned int type;
};


class Serializer;
class Deserializer;
class Shell;
class FspDriver;


namespace fsp {

class Lts;

typedef void (*LtsVisitFunction)(int, const Lts&, const struct LtsNode&, void*);

struct LtsVisitObject {
    LtsVisitFunction vfp;
    void *opaque;
};


/* An LTS. */
class Lts: public Symbol {
    vector<LtsNode> nodes;
    vector<LtsNodeInfo> infos;
    unsigned int end;
    unsigned int err;

    set<unsigned int> alphabet;

    vector<TerminalSet> terminal_sets;
    bool terminal_sets_computed;

    void copy_node_in(int state, const Lts& lts, int i);
    void copy_node_out(Lts& lts, int i, int state);
    void copy_nodes_in(const Lts& lts);

    void update_composition(unsigned int idx, unsigned int dst_ip,
                            const fsp::Lts& p, unsigned int dst_iq,
                            const fsp::Lts& q, unsigned int nq, Edge& e,
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
    void initial_partitions(list< set<unsigned int> >& partitions,
                            unsigned int *partitions_map,
                            set<unsigned int>& tau_dead_set);
    void reduce_to_partitions(stringstream &ss,
                              const list< set<unsigned int> >& partitions,
                              const unsigned int *partitions_map,
                              const set<unsigned int>& tau_dead_set);
    void reachable_partitions_set(unsigned int state, unsigned int action,
                                  const unsigned int *partitions_map,
                                  const set<unsigned int>& tau_dead_set,
                                  set<unsigned int>& result) const;

    void __traces(stringstream &ss, set<CEdge>& marked,
                  vector<unsigned int>& trace, unsigned int state);

    friend class ::Serializer;
    friend class ::Deserializer;

  public:
    string name;

    Lts() { err = end = ~0U; }
    Lts(int); /* One state Lts: Stop, End or Error */
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
    void graphvizOutput(const char *filename, bool compress) const;
    void simulate(Shell& sh, const ActionSetS *asv) const;
    void basic(const string& outfile, stringstream& ss) const;
    void minimize(stringstream& ss);
    void traces(stringstream& ss);

    void updateAlphabet(unsigned int action);
    int lookupAlphabet(unsigned int action) const;
    void mergeAlphabetInto(set<unsigned int>& actions) const;
    void mergeAlphabetFrom(const set<unsigned int>& actions);
    int alphabetSize() const { return alphabet.size(); }
    void printAlphabet(stringstream& ss, bool compress) const;
    set<unsigned int> getAlphabet() const { return alphabet; }
    unsigned int append(const Lts&, unsigned int first);
    Lts& zerocat(const Lts& lts, const string& label);
    Lts& incompcat(const vector< SmartPtr<Lts> >& ltsv);
    Lts& zeromerge(const Lts& lts);
    unsigned int resolve();
    Lts& mergeEndNodes();
    bool endcat(const Lts& lts);

    void set_priv(unsigned int state, unsigned int val);
    unsigned int get_priv(unsigned int state) const;
    void set_type(unsigned int state, unsigned int type);
    unsigned int get_type(unsigned int state) const;
    void check_privs(set<unsigned int>& privs);
    void replace_priv(unsigned int new_priv, unsigned int old_priv);
    vector<Edge> get_children(unsigned int state) const;
    bool in_tau_deadlock(unsigned int state) const;
    void reachable_actions_set(unsigned int state,
                               const set<unsigned int>& tau_dead_set,
                               set<unsigned int>& result) const;
    void collapse_tau_chains(stringstream& ss);

    void clear();
    void cleanup();

    /* Methods to implement because of the base class. */
    void print() const;
    const char *className() const { return "Lts"; }
    Symbol *clone() const;
};

fsp::Lts *err_if_not_lts(FspDriver& driver, Symbol *svp, const fsp::location& loc);

struct LtsPtrS : public Symbol {
    SmartPtr<Lts> val;

    const char *className() const { return "LtsPtr"; }
};

struct LtsVecS : public Symbol {
    vector< SmartPtr<Lts> > val;

    const char *className() const { return "LtsVec"; }
};

void compress_action_labels(const set<unsigned int>& actions,
                            set<string>& result, bool compress);

} /* namespace fsp */

#endif
