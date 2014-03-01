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

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <set>
#include <assert.h>
#include <stdint.h>

#include "symbols_table.hpp"
#include "fsp_location.hh"

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

#if 0
int generate_ID()
{
    static int s_nID = 0;
    return s_nID++;
}

struct KanellakisSmolkaState{
    int state;
    Lts* owner;
    KanellakisSmolkaState(int s, Lts* l): state(s), owner(l){}
    bool operator==(const KanellakisSmolkaState& other){
        return state == other.state && owner == other.owner;
    }
};

typedef set<KanellakisSmolkaState> Block;
typedef int BlockId;

struct UniqueBlock{
  Block block;
  BlockId blockId;
  UniqueBlock(Block b, BlockId i): block(b), blockId(i){}
};

typedef map<KanellakisSmolkaState, UniqueBlock> Partition;

set<BlockId> reached_blocks(KanellakisSmolkaState s, Partition& p){
    vector<Edge> s_children = s.owner->get_children(s.state);
    set<int> reached_blocks;
    vector<Edge>::iterator i = s_children.begin()
    for(; i != s_children.end(); i++){
        if(i->action == label){
            KanellakisSmolkaState reached(i->dest, s.owner);
            reached_blocks.insert(p[reached].blockId);
        }
    }
    return reached_blocks;
}

Partition Kanellakis_Smolka_split(UniqueBlock& b, int label, Partition& p)
{
    assert(!b.block.empty());
    KanellakisSmolkaState s = *(b.block.begin());
    set<BlockId> s_reached_blocks(s, p);
    Block b1, b2;
    for(Block::iterator i = b.begin(); i != b.end; i++){
        KanellakisSmolkaState t = *i;
        set<BlockId> t_reached_blocks(t, p);
        if(s_reached_blocks == t_reached_blocks) b1.insert(t);
        else b2.insert(t);
    }
    UniqueBlock ub1(b1, generate_ID());
    if(!b2.empty()){
        UniqueBlock ubi2(b2, generate_ID());
        //DA FINIRE, FORSE
    }
}
#endif


/* An LTS. */
class Lts: public Symbol {
    vector<LtsNode> nodes;
    vector<LtsNodeInfo> infos;
    unsigned int end;
    unsigned int err;

    set<int> alphabet;

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
    void graphvizOutput(const char *filename) const;
    void simulate(Shell& sh, const ActionSetS *asv) const;
    void basic(const string& outfile, stringstream& ss) const;
    void minimize(stringstream& ss);
    void traces(stringstream& ss);

    void updateAlphabet(int action);
    int lookupAlphabet(int action) const;
    void mergeAlphabetInto(set<int>& actions) const;
    void mergeAlphabetFrom(const set<int>& actions);
    int alphabetSize() const { return alphabet.size(); }
    void printAlphabet(stringstream& ss) const;
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

} /* namespace fsp */

#endif
