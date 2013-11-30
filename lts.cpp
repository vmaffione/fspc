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


#include <map>
#include <fstream>
#include <algorithm>
#include <list>
#include <queue>
#include <cstdlib>
#include <assert.h>

/* Inline utilities. */
#include "utils.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

/* Symbol tables and symbol types. */
#include "symbols_table.hpp"

/* The fspc shell. */
#include "shell.hpp"

/* Some helpers (intersection routines). */
#include "helpers.hpp"

using namespace std;


//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


static inline bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

/* This routine transforms an action label so that every occurrence of
   ".N" (where "N" is a multi-digit number) is turned into "[N]".
   This is currently used in the "basic" command implementation,
   to make sure that the output basic process definition can be
   successfully compiled by fspc. */
static inline void do_square_int(const string& in, string& out)
{
    unsigned int j = 0;
    unsigned int lim = in.size();
    bool number = false;

    /* A state machine. */
    while (j < lim) {
        if (number) {
            if (!is_digit(in[j])) {
                /* The number is finished. */
                number = false;
                out.push_back(']');
            }
        }
        if (in[j] == '.' && j+1 < lim && is_digit(in[j+1])) {
            /* Found a number after a '.'. */
            number = true;
            out.push_back('[');
            j++;  /* Skip the '.' */
        }
        /* Output the input. */
        out.push_back(in[j]);
        j++;
    }
    if (number) {
        /* The input ends with a digit. */
        out.push_back(']');
    }
}

/* Access an action label by its index. If 'square_ints' is true, the
   stored action label is processed using do_square_int() routine (see
   above). */
string ati(struct ActionsTable * atp, unsigned int index, bool square_ints)
{
    if (index >= atp->reverse.size())
        return "UNKN";

    if (square_ints) {
        string squared;  /* Prepare an empty string. */

        do_square_int(atp->reverse[index], squared);

        return squared;
    }

    return atp->reverse[index];
}

#define CHECKATP(RET) \
    if (atp == NULL) { \
        cout << "[" << __LINE__ << "] Null atp\n"; \
        return RET; \
    }


/* ====================== class Lts implementation ===================== */
yy::Lts::ComposeAlgorithm yy::Lts::compose_algorithm =
                                    &yy::Lts::compose_operational;

int yy::Lts::lookupAlphabet(int action) const
{
    set<int>::iterator it = alphabet.find(action);
    
    return (it == alphabet.end()) ? -1 : 0;
}

void yy::Lts::updateAlphabet(int action)
{
    alphabet.insert(action);
}

void yy::Lts::mergeAlphabetInto(set<int>& actions) const
{
    for (set<int>::iterator it = alphabet.begin();
			it != alphabet.end(); it++) {
	actions.insert(*it);
    }
}

void yy::Lts::mergeAlphabetFrom(const set<int>& actions)
{
    for (set<int>::iterator it = actions.begin();
			it != actions.end(); it++) {
	alphabet.insert(*it);
    }
}

void yy::Lts::printAlphabet(stringstream& ss) const
{
    set<int>::iterator it;

    CHECKATP();

    ss << "Alphabet: {";
    for (it=alphabet.begin(); it != alphabet.end(); it++)    
	ss << ati(atp, *it, false) << ", ";
    ss << "}\n";
}

yy::Lts::Lts(int type, struct ActionsTable * p) : atp(p)
{
    struct LtsNode node;

    nodes.push_back(node);
    set_type(0, type);
    set_priv(0, LtsNode::NoPriv);
    terminal_sets_computed = false;
    end = err = ~0U;
    refcount = 0;
}

void yy::Lts::print() const {
    stringstream ss;

    CHECKATP();

    atp->print();
    cout << "LTS " << name << "\n";
    for (unsigned int i=0; i<nodes.size(); i++) {
	cout << "State " << i << "(priv=" << get_priv(i) << ", type=" <<
            get_type(i) << "):\n";
	for (unsigned int j=0; j<nodes[i].children.size(); j++)
	    cout << "    " << ati(atp, nodes[i].children[j].action, false)
		    << " --> " << nodes[i].children[j].dest << "\n";
    }
    printAlphabet(ss); cout << ss.str();
    cout << numStates() << " states, " << numTransitions() << " transitions\n";
}

void yy::Lts::clear()
{
    nodes.clear();
    infos.clear();
    alphabet.clear();
    terminal_sets.clear();
    terminal_sets_computed = false;
    end = err = ~0U;
}

/* Clean up some space that will not be needed in the future. */
void yy::Lts::cleanup()
{
    bool search = (err == ~0U || end == ~0U);

    for (unsigned int i = 0; search && i < nodes.size(); i++) {
        if (get_type(i) == LtsNode::End) {
            end = i;
            search = (err == ~0U);
        } else if (get_type(i) == LtsNode::Error) {
            err = i;
            search = (end == ~0U);
        }
    }
    infos.clear();
}

int yy::Lts::numTransitions() const
{
    int n = 0;

    for (unsigned int i=0; i<nodes.size(); i++) {
        for (unsigned int j=0; j<nodes[i].children.size(); j++) {
            n++;
        }
    }

    return n;
}

void yy::Lts::copy_node_in(int state, const Lts& lts, int i)
{
    nodes[state] = lts.nodes[i];
    set_priv(state, lts.get_priv(i));
    set_type(state, lts.get_type(i));
}

void yy::Lts::copy_node_out(Lts& lts, int i, int state)
{
    lts.nodes[i] = nodes[state];
    lts.set_priv(i, get_priv(state));
    lts.set_type(i, get_type(state));
}

void yy::Lts::copy_nodes_in(const Lts& lts)
{
    nodes = lts.nodes;
    infos = lts.infos;
}

/* BFS on the LTS for useless states removal. */
void yy::Lts::reduce(const yy::Lts& unconnected)
{
    int np = unconnected.nodes.size();
    vector<int> frontier(np);
    vector<int> map(np);
    int pop, push;
    int state;
    int n = 0;

    /* We make sure that 'nodes' is empty. */
    nodes.clear();
    terminal_sets_computed = false;

    if (!np) {
        return;
    }

    /* Overestimation */
    nodes.resize(np);

    /* At the beginning the frontier contains only the initial state */
    frontier[0] = 0;
    pop = 0; push = 1;
    map[0] = n++;
    for (int i=1; i<np; i++)
	map[i] = -1;

    while (pop != push) {
	Edge e;

	state = frontier[pop++];
	for (unsigned int j=0; j<unconnected.nodes[state].children.size(); j++) {
	    int child = unconnected.nodes[state].children[j].dest;

	    if (map[child] == -1) {
		map[child] = n++;
		frontier[push++] = child;
	    }
	    e.dest = map[child];
	    e.action = unconnected.nodes[state].children[j].action;
	    nodes[map[state]].children.push_back(e);
	}
    }

    for (int i=0; i<np; i++)
	if (map[i] != -1) {
            set_type(map[i], unconnected.get_type(i));
            set_priv(map[i], unconnected.get_priv(i));
        }

    nodes.resize(n);
}


void yy::Lts::compose_declarative(const yy::Lts& p, const yy::Lts& q)
{    
    unsigned int nq;
    unsigned int np;
    Edge e;
    Lts product;

    assert(p.atp == atp && q.atp == atp);

    /* First of all we reset *this, like Lts(ActionsTable *) would do. */
    this->clear();

    /* We build the cartesian product of P states and Q states. For 
       convenience, we map the composite state (p,q) with the "linear" state
       p*nq+q (exactly how a matrix is mapped onto a vector. */
    np = p.numStates();
    nq = q.numStates();
    product.nodes.resize(np * nq);

    /* Scan the P graph and combine P actions with Q states. */
    for (unsigned int ip=0; ip<p.nodes.size(); ip++) {
        for (unsigned int jp=0; jp<p.nodes[ip].children.size(); jp++) {
            const Edge& ep = p.nodes[ip].children[jp];

            /* We analyze an edge of P: (i, ep.action, ep.dest). */
            e.action = ep.action;

            if (q.lookupAlphabet(ep.action) == -1) {
                /* If ep.action is not included in the alphabet of Q, this
                   action can be executed by P independently on the state
                   of Q. */
                for (unsigned int iq=0; iq<nq; iq++) {
                    e.dest = ep.dest * nq + iq;
                    product.nodes[ip*nq+iq].children.push_back(e);
                }
            } else {
                /* If ep.action is included in the alphabet of Q, this
                   action can be executed by P only together with Q. */
                for (unsigned int iq=0; iq<q.nodes.size(); iq++)
                    for (unsigned int jq=0; jq<q.nodes[iq].children.size(); jq++) {
                        const Edge& eq = q.nodes[iq].children[jq];

                        if (eq.action == ep.action) {
                            e.dest = ep.dest * nq + eq.dest;
                            product.nodes[ip*nq+iq].children.push_back(e);
                        }
                    }

            }
        }
    }

    /* Scan the Q graph and combine Q actions with P states */
    for (unsigned int iq=0; iq<q.nodes.size(); iq++) {
        for (unsigned int jq=0; jq<q.nodes[iq].children.size(); jq++) {
            const Edge& eq = q.nodes[iq].children[jq];

            /* We analyze an edge of Q: (i, eq.action, eq.dest). */
            e.action = eq.action;

            if (p.lookupAlphabet(eq.action) == -1) {
                /* If ep.action is not included in the alphabet of P, this
                   action can be executed by Q independently on the state
                   of P. */
                for (unsigned int ip=0; ip<np; ip++) {
                    e.dest = ip * nq + eq.dest;
                    product.nodes[ip*nq+iq].children.push_back(e);
                }
            } /* else case has already been covered by the previous scan. */
        }
    }

    /* A composed state is an END state when both the components are
       END states. */
    for (unsigned int ip=0; ip<p.nodes.size(); ip++) {
        for (unsigned int iq=0; iq<q.nodes.size(); iq++)
            if ((p.get_type(ip) == LtsNode::Error) ||
                    (q.get_type(iq) == LtsNode::Error)) {
                product.set_type(ip*nq+iq, LtsNode::Error);
            } else if ((p.get_type(ip) == LtsNode::End) &&
                    (q.get_type(iq) == LtsNode::End)) {
                product.set_type(ip*nq+iq, LtsNode::End);
            }
    }

    mergeAlphabetFrom(p.alphabet);
    mergeAlphabetFrom(q.alphabet);

    reduce(product);
}

/* Helper function used by Lts::compose_operational(). */
void yy::Lts::update_composition(unsigned int idx,
                               unsigned int dst_ip, const yy::Lts& p,
                               unsigned int dst_iq, const yy::Lts& q,
                               unsigned int nq, Edge& e,
                               map<unsigned int, unsigned int>& direct,
                               vector<unsigned int>& inverse)
{
    pair< map<unsigned int, unsigned int>::iterator, bool> insr;
    pair<unsigned int, unsigned int> couple;

    couple.first = dst_ip*nq + dst_iq;
    couple.second = nodes.size();
    insr = direct.insert(couple);
    if (insr.second) {
        unsigned int type = LtsNode::Normal;

        if (p.get_type(dst_ip) == LtsNode::Error ||
                q.get_type(dst_iq) == LtsNode::Error) {
            type = LtsNode::Error;
        } else if (p.get_type(dst_ip) == LtsNode::End &&
                q.get_type(dst_iq) == LtsNode::End) {
            type = LtsNode::End;
        }
        nodes.push_back(LtsNode());
        set_type(nodes.size() - 1, type);
        inverse.push_back(couple.first);
    }

    e.dest = insr.first->second;
    nodes[idx].children.push_back(e);
}

void yy::Lts::compose_operational(const yy::Lts& p, const yy::Lts& q)
{
    unsigned int nq = q.numStates();
    /* Maps '(ip, iq)' --> 'idx', where 'idx' is an index in the vector
       'nodes' */
    map<unsigned int, unsigned int> direct;
    /* Maps 'idx' --> 'ip*nq + iq'. */
    vector<unsigned int> inverse;
    unsigned idx = 0;
    unsigned ip, iq;
    Edge e;

    assert(p.atp == atp && q.atp == atp);

    /* First of all we reset *this, like Lts(ActionsTable *) would do. */
    nodes.clear();
    terminal_sets_computed = false;
    alphabet.clear();
    end = err = ~0U;

    nodes.push_back(LtsNode());
    set_type(nodes.size() - 1, LtsNode::Normal);
    direct[0] = 0;
    inverse.push_back(0);

    while (idx < nodes.size()) {
        ip = inverse[idx] / nq;
        iq = inverse[idx] % nq;

        for (unsigned int jp = 0; jp < p.nodes[ip].children.size(); jp++) {
            const Edge& ep = p.nodes[ip].children[jp];

            e.action = ep.action;
            if (q.lookupAlphabet(ep.action) == -1) {
                update_composition(idx, ep.dest, p,
                                   iq, q, nq, e, direct,
                                   inverse);
            } else {
                for (unsigned int jq = 0; jq < q.nodes[iq].children.size();
                                                                jq++) {
                    const Edge& eq = q.nodes[iq].children[jq];

                    if (eq.action == ep.action) {
                        update_composition(idx, ep.dest, p,
                                           eq.dest, q,
                                           nq, e, direct, inverse);
                    }
                }
            }
        }

        for (unsigned int jq = 0; jq < q.nodes[iq].children.size(); jq++) {
            const Edge& eq = q.nodes[iq].children[jq];

            e.action = eq.action;
            if (p.lookupAlphabet(eq.action) == -1) {
                update_composition(idx, ip, p, eq.dest,
                                   q, nq, e, direct,
                                   inverse);
            }
        }

        idx++;
    }

    mergeAlphabetFrom(p.alphabet);
    mergeAlphabetFrom(q.alphabet);
}

void yy::Lts::compose(const yy::Lts& p, const yy::Lts& q)
{
    (this->*compose_algorithm)(p, q);
}

yy::Lts::Lts(const yy::Lts& p, const yy::Lts& q)
{
    assert(p.atp && q.atp == p.atp);
    atp = p.atp;
    compose(p, q);
    refcount = 0;
}

yy::Lts& yy::Lts::compose(const yy::Lts& q)
{
    yy::Lts copy(*this);

    compose(copy, q);

    return *this;
}

int yy::Lts::deadlockAnalysis(stringstream& ss) const
{
    int nd = 0;
    int state = 0;
    int n = nodes.size();
    vector<int> frontier(n);  /* The frontier is managed as a queue */
    vector<int> actions(n);   /* The action used to reach frontier[i] */
    vector<int> back(n);      /* Backpointer of frontier[i] */
    vector<bool> seen(n);     /* seen[i] is set if state i has been enqueued */
    vector<int> action_trace(n);
    int pop, push;  /* Queue indexes */

    CHECKATP(0);

    if (!n) {
        return 0;
    }

    /* BFS looking for states with no outgoing transitions. We use a BFS
       instead of a DFS because the former is simpler to implement and
       because it finds the shortest path to each deadlock state. */

    /* Initialize a queue that only contains the 0 node. */
    pop = 0;
    push = 1;
    frontier[0] = 0;
    seen[0] = true;
    for (int i=1; i<n; i++)
	seen[i] = false;

    /* Keep visiting until the queue is empty. */
    while (pop != push) {
	unsigned int i;

	/* Pop a state and examine all its child. */
	state = frontier[pop];
	for (i=0; i<nodes[state].children.size(); i++) {
	    int child = nodes[state].children[i].dest;
	    if (!seen[child]) {
		seen[child] = true;
		back[push] = state;
		actions[push] = nodes[state].children[i].action;
		frontier[push++] = child;
	    }
	}

	/* No outgoing transitions ==> Deadlock state */
	if (i == 0 && get_type(state) != LtsNode::End) { 
	    int t;
	    int j;
	    string ed;

	    if (get_type(state) == LtsNode::Normal)
		ed = "Deadlock";
	    else
		ed = "Property violation";
	    ss << ed << " found for process " << name << ": state "
			<< state << "\n";
	    /* Starting from 'state', we follow the backpointers to build the
	       trace to deadlock (in reverse order). */
	    t = pop;
	    j = 0;
	    while (t) {
		action_trace[j++] = actions[t];
		t = back[t];
	    }
	    ss << "	Trace to " << ed << ": ";
	    for (j--; j>=0; j--)
		ss << ati(atp, action_trace[j], false) << "->";
	    ss << "\n\n";
	    nd++;
	}
	pop++; /* Pops 'state' from the queue */
    }

    return nd;
}

int yy::Lts::terminalSets()
{
    int n = nodes.size();
    int na = atp->reverse.size();
    int nts = 0;

    if (terminal_sets_computed)
	return terminal_sets.size();
    terminal_sets_computed = true;

    /* Data structures for the iterative DFS implementation */
    vector<unsigned int> state_stack(n);  /* Emulated recursion stack */
    vector<unsigned int> action_stack(n); /* Actions stack parallel to the previous */
    vector<unsigned int> back(n);	 /* Backpointers parallel stack */
    vector<bool> entered(n);	 /* Marks states started to be visited */
    vector<unsigned int> next_child(n);	 /* Records the next child to examine */
    vector<unsigned int> action_trace(n);
    int top;

    /* Tarjan algorithm data structures */
    vector<unsigned int> tarjan_index(n);
    vector<unsigned int> tarjan_stack(n);
    vector<unsigned int> tarjan_lowlink(n);
    vector<bool> tarjan_in_stack(n); /* Set if state 'i' is on the stack */
    int tarjan_counter;
    int tarjan_top;

    /* Structures useful to represent a strongly connected components. */
    vector<int> tarjan_component_states(n);
    vector<bool> tarjan_state_in_component(n);
    vector<int> tarjan_component_actions(na);
    vector<bool> tarjan_action_in_component(na);

    CHECKATP(0);

    /* Initialize DFS structures: at the beginning only the 0 node is on
       the state stack */
    top = 0;
    state_stack[0] = 0;
    for (int i=0; i<n; i++) {
	entered[i] = false;
	next_child[i] = 0;
    }

    /* Initialize Tarjan structures */
    tarjan_counter = 0;
    tarjan_top = -1;
    for (int i=0; i<n; i++) {
	tarjan_index[i] = -1;  /* becomes useless because of entered*/
	tarjan_in_stack[i] = tarjan_state_in_component[i] = false;
    }
    for (int i=0; i<na; i++)
	tarjan_action_in_component[i] = false;

    /* The main loop takes (but doesn't pop) the top element and start
       (restart) to examine its children, pushing one of them the state 
       stack. */
    while (top != -1) {
	int state;
	int child;

	state = state_stack[top];

	if (!entered[state]) {
	    /* If this is the first time we visit 'state', we must do what
	       we do at the beginning of a recursive implementation of 
	       tarjan(state) (see Wikipedia), before we start to examine
	       the children. */
	    tarjan_index[state] = tarjan_lowlink[state] = tarjan_counter++;
	    tarjan_stack[++tarjan_top] = state;
	    tarjan_in_stack[state] = true;
	    entered[state] = true;
	    IFD(cout << state << ".start\n");
	}

	if (next_child[state] == nodes[state].children.size()) {
	    /* If there are not more children to examine, we
	       unroll the recursion stack popping out the node 'state'.
	       Here we have to finalize the visit, applying the core of the 
	       Tarjan algorithm. */

	    IFD(cout << state << ".end\n");

	    for (unsigned int i=0; i<nodes[state].children.size(); i++) {
		int child = nodes[state].children[i].dest; 
		if (tarjan_index[child] > tarjan_index[state]) {
		    /* If this condition holds, child must be a descendent
		       of 'state' in the DFS tree. In this case the Tarjan
		       algorithm tells we must consider the lowlink of the
		       child. */
		    if (tarjan_lowlink[child] < tarjan_lowlink[state])
			tarjan_lowlink[state] = tarjan_lowlink[child];
		} else if (tarjan_in_stack[child]) {
		    /* If this condition holds, the child is not a descendent
		       of 'state' in the DFS tree. In this case the Tarjan
		       algorithm tells we must consider the index of the child
		       only if the child is currently in the Tarjan stack. */
		    if (tarjan_index[child] < tarjan_lowlink[state])
			tarjan_lowlink[state] = tarjan_index[child];
		}
	    }

	    /* Check if this node is the "root" of a strongly connected
	       component. */
	    if (tarjan_index[state] == tarjan_lowlink[state]) {
		int s;
		int nc = 0;  /* Number of states in the component */
		int nca = 0; /* Number of actions in the component */
		bool terminal;
		Edge e;

		/* Extract the strongly connected component from the Tarjan
		   stack. */
		IFD(cout << "Component: ");
		do {
		    s = tarjan_stack[tarjan_top--];
		    tarjan_component_states[nc++] = s;
		    tarjan_state_in_component[s] = true;
		    tarjan_in_stack[s] = false;
		    IFD(cout << s << " ");
		} while (s != state);
		IFD(cout << "\n");


		if (nc == n) {
		    /* We are not interested in trivial terminal sets. A
		       terminal set is trivial when it is the whole LTS. */
		    terminal = false;
		} else {
		    /* Check if the component is a terminal one, e.g. with
		       no transitions that go out of the component states. */
		    terminal = true;
		    for (int j=0; j<nc; j++) {
			s = tarjan_component_states[j];
			for (unsigned int i=0; i<nodes[s].children.size(); i++) {
			    e = nodes[s].children[i];
			    if (!tarjan_state_in_component[e.dest]) {
				/* We have a transition that exit the
				   component */
				terminal = false;
				goto clear_flags;
			    } else if (!tarjan_action_in_component[e.action]) {
				/* We add the action to the set of actions in
				   the component (if not already added). */
				tarjan_action_in_component[e.action] = true;
				tarjan_component_actions[nca++] = e.action;
			    }
			}
		    }
		}

		if (terminal) {
		    int t;
		    int j;
		    TerminalSet& ts = (
			terminal_sets.push_back(TerminalSet()),
						    terminal_sets.back());
		    /* If the component is terminal, we output the trace of
		       actions to get to the component. */
#ifdef DEBUG
		    cout << "Terminal set of states: ";
		    for (j=0; j<nc; j++)
			cout << tarjan_component_states[j] << " ";
		    cout << "\n";
#endif

		    /* We examine the action_stack (without popping anything) 
		       and build the action_trace in reverse order. */
		    j = 0;
		    t = top;
		    while (t) {
			action_trace[j++] = action_stack[t];
			t = back[t];
		    }
		    IFD(cout << "Trace to the terminal set:\n");
		    for (j--; j>=0; j--) {
			IFD(cout << "    " << ati(atp, action_trace[j], false) << "\n");
			ts.trace.push_back(action_trace[j]);
		    }
		    IFD(cout << "Actions in the terminal set: {");
		    for (j=0; j<nca; j++) {
			IFD(cout << ati(atp, tarjan_component_actions[j], false) << ", ");
			ts.actions.insert(tarjan_component_actions[j]);
		    }
		    IFD(cout << "}\n");
		    nts++;
		}
clear_flags:
		/* Clear two flags arrays: 'tarjan_state_in_component' and 
		   'tarjan_action_in_component'. */
		for (int j=0; j<nc; j++)
		    tarjan_state_in_component[tarjan_component_states[j]] =
								    false;
		for (int j=0; j<nca; j++)
		    tarjan_action_in_component[tarjan_component_actions[j]] =
								    false;
	    }

	    top--;  /* We pop the state, since we've completed the visit. */
	} else {
	    /* If there are still children to examine, let's continue our
	       DFS visit. */
	    child = nodes[state].children[next_child[state]].dest;
	    if (!entered[child]) {
		state_stack[++top] = child;
		action_stack[top] =
			    nodes[state].children[next_child[state]].action;
		back[top] = top - 1;
		IFD(cout << child << ".push\n");
	    }
	    next_child[state]++;
	}

    }

    return nts;
}

bool yy::Lts::isDeterministic() const
{
    for (unsigned int i=0; i<nodes.size(); i++) {
	map<int, int> links;
	pair<map<int, int>::iterator, bool> ret;

	/* For each node, we have to check that the mapping
	   action --> destination_node is injective (one-to-one).*/
	for (unsigned int j=0; j<nodes[i].children.size(); j++) {
	    ret = links.insert(pair<int, int>(nodes[i].children[j].action,
				    nodes[i].children[j].dest));
	    if (!ret.second)
		return false;
	}
    }

    return true;
}

Symbol * yy::Lts::clone() const
{
    yy::Lts * lv = new yy::Lts(*this);

    return lv;
}

void yy::Lts::visit(const struct LtsVisitObject& lvo) const
{
    int state = 0;
    int n = nodes.size();
    vector<int> frontier(n);
    vector<bool> seen(n);
    int pop, push;

    if (!n) {
        return;
    }

    pop = 0;
    push = 1;
    frontier[0] = 0;
    seen[0] = true;
    for (int i=1; i<n; i++)
	seen[i] = false;

    while (pop != push) {
	state = frontier[pop++];
	/* Invoke the visit function */
	lvo.vfp(state, *this, nodes[state], lvo.opaque);
	for (unsigned int i=0; i<nodes[state].children.size(); i++) {
	    int child = nodes[state].children[i].dest;
	    if (!seen[child]) {
		seen[child] = true;
		frontier[push++] = child;
	    }
	}
    }
}

yy::Lts& yy::Lts::labeling(const SetS& labels)
{
    if (!labels.size())
	return *this;

    if (labels.size() == 1)
	this->labeling(labels[0]);
    else {
	yy::Lts copy(*this);

	this->labeling(labels[0]);
	for (unsigned int i=1; i<labels.size(); i++) {
	    yy::Lts right(copy);

	    right.labeling(labels[i]);
	    this->compose(right);
	}
    }

    return *this;
}

yy::Lts& yy::Lts::labeling(const string& label)
{
    set<int> new_alphabet;
    map<int, int> mapping;

    CHECKATP(*this);

    terminal_sets_computed = false;

    /* Update the actions table, compute a one-to-one [old --> new] mapping
       and update the alphabet. */
    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end(); it++) {
	int old_index;
	int new_index;

	old_index = *it;
	new_index = atp->insert(label + "." + atp->reverse[old_index]);
	new_alphabet.insert(new_index);
	mapping.insert(make_pair(old_index, new_index));
    }
    alphabet = new_alphabet;

    /* Update the edges actions. */
    for (unsigned int i=0; i<nodes.size(); i++)
	for (unsigned int j=0; j<nodes[i].children.size(); j++)
	    nodes[i].children[j].action = mapping[nodes[i].children[j].action];

    return *this;
}

yy::Lts& yy::Lts::sharing(const SetS& labels)
{
    set<int> new_alphabet;
    map<int, vector<int> > mapping;

    CHECKATP(*this);

    terminal_sets_computed = false;

    /* Update the actions table, compute a one-to-many [old --> new] mapping 
       and update the alphabet. */
    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end(); it++) {
	int old_index;
	int new_index;
	vector<int> new_indexes;

	old_index = *it;
	for (unsigned int i=0; i<labels.size(); i++) {
	    new_index = atp->insert(labels[i] + "." +
		                    atp->reverse[old_index]);
	    new_alphabet.insert(new_index);
	    new_indexes.push_back(new_index);
	}
	mapping.insert(make_pair(old_index, new_indexes));
    }
    alphabet = new_alphabet;

    /* Replace the children array of each node. */
    for (unsigned int i=0; i<nodes.size(); i++) {
	vector<Edge> new_children;

	for (unsigned int j=0; j<nodes[i].children.size(); j++) {
	    Edge e = nodes[i].children[j];
	    vector<int> new_indexes = mapping[e.action];

	    for (unsigned int k=0; k<new_indexes.size(); k++) {
		e.action = new_indexes[k];
		new_children.push_back(e);
	    }
	}
	nodes[i].children = new_children;
    }

    return *this;
}

yy::Lts& yy::Lts::relabeling(const SetS& newlabels, const string& oldlabel)
{
    map<int, vector<int> > mapping;
    set<int> new_alphabet = alphabet;

    CHECKATP(*this);

    terminal_sets_computed = false;

    /* Update the actions table, compute a one to many [old --> new]
       mapping and update the alphabet. */
    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end(); it++) {
	int old_index;
	int new_index;
	vector<int> new_indexes;
	string action;
	pair<string::const_iterator, string::iterator> mm;

	old_index = *it;
	action = atp->reverse[old_index];
	/* Prefix match: check if 'oldlabel' is a prefix of 'action'. */
	mm = mismatch(oldlabel.begin(), oldlabel.end(), action.begin());
	if (mm.first == oldlabel.end()) {
	    for (unsigned int i=0; i<newlabels.size(); i++) {
		string new_action = action;

		new_action.replace(0, oldlabel.size(), newlabels[i]);
		new_index = atp->insert(new_action);
		new_alphabet.insert(new_index);
		new_indexes.push_back(new_index);
	    }
	    new_alphabet.erase(old_index);
	    mapping.insert(make_pair(old_index, new_indexes));
	}
    }
    alphabet = new_alphabet;

    /* Replace the children that are to be replaced. */
    for (unsigned int i=0; i<nodes.size(); i++) {
	unsigned int original_size = nodes[i].children.size();
	/* We need 'original_size' since we are going to push_back() in 
	   'nodes[i].children', e.g. the same vector we are cycling on. */

	for (unsigned int j=0; j<original_size; j++) {
	    Edge e = nodes[i].children[j];
    
	    if (mapping.count(e.action)) {
		vector<int> new_indexes = mapping[e.action];

		e.action = new_indexes[0];
		nodes[i].children[j] = e;
		for (unsigned int k=1; k<new_indexes.size(); k++) {
		    e.action = new_indexes[k];
		    nodes[i].children.push_back(e);
		}
	    }
	}
    }

    return *this;
}

yy::Lts& yy::Lts::relabeling(const SetS& newlabels, const SetS& oldlabels)
{
    for (unsigned int i=0; i<oldlabels.size(); i++)
	this->relabeling(newlabels, oldlabels[i]);

    return *this;
}

yy::Lts& yy::Lts::hiding(const SetS& s, bool interface)
{
    set<int> new_alphabet;

    CHECKATP(*this);

    terminal_sets_computed = false;

    /* Update the alphabet. */
    if (interface) {
	for (unsigned int i=0; i<s.size(); i++) {
	    /* The action s[i] can select multiple alphabet
	       elements. */
	    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end();
								it++) {
		string action = ati(atp, *it, false);
		pair<string::const_iterator, string::iterator> mm;
                string cand = s[i];

		/* Prefix match: check if 's[i]' is a prefix of
		   'action'. */
		mm = mismatch(cand.begin(), cand.end(), action.begin());
		if (mm.first == cand.end())
		    new_alphabet.insert(*it);
	    }
	}
    } else {
	new_alphabet = alphabet;
	for (unsigned int i=0; i<s.size(); i++) {
	    /* The action s[i] can hide multiple alphabet elements. */
	    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end();
								it++) {
		string action = ati(atp, *it, false);
		pair<string::const_iterator, string::iterator> mm;
                string cand = s[i];

		/* Prefix match: check if 's[i]' is a prefix of
		   'action'. */
		mm = mismatch(cand.begin(), cand.end(), action.begin());
		if (mm.first == cand.end())
		    new_alphabet.erase(*it);
	    }
	}
    }
    alphabet = new_alphabet;

    /* Update the edges actions. */
    for (unsigned int i=0; i<nodes.size(); i++)
	for (unsigned int j=0; j<nodes[i].children.size(); j++)
	    if (!alphabet.count(nodes[i].children[j].action))
		/* We are sure that atp->lookup("tau") == 0. */
		nodes[i].children[j].action = 0;

    return *this;
}

yy::Lts& yy::Lts::priority(const SetS& s, bool low)
{
    int low_int = (low) ? 1 : 0;
    set<int> priority_actions;
    Lts new_lts;

    CHECKATP(*this);

    new_lts.nodes.resize(nodes.size());

    terminal_sets_computed = false;

    for (unsigned int i=0; i<s.size(); i++) {
	    /* The action s[i] can select multiple alphabet
	       elements. */
	    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end();
								it++) {
		string action = ati(atp, *it, false);
		pair<string::const_iterator, string::iterator> mm;
                string cand = s[i];

		/* Prefix match: check if 's[i]' is a prefix of
		   'action'. */
		mm = mismatch(cand.begin(), cand.end(), action.begin());
		if (mm.first == cand.end())
		    priority_actions.insert(*it);
	    }
    }

    for (unsigned int i=0; i<nodes.size(); i++) {
	vector<Edge> new_children;
	bool found = false;

	for (unsigned int j=0; j<nodes[i].children.size(); j++) 
	    if (priority_actions.count(nodes[i].children[j].action)
							    ^ low_int) {
		new_children.push_back(nodes[i].children[j]);
		found = true;
	    }
	if (found) {
	    new_lts.nodes[i].children = new_children;
            new_lts.set_type(i, get_type(i));
            new_lts.set_priv(i, get_priv(i));
	} else {
            copy_node_out(new_lts, i, i);
        }
    }

    reduce(new_lts);

    return *this;
}

yy::Lts& yy::Lts::property()
{
    Edge e;

    /* First of all we have to make sure that the process is deterministic.
       If not, don't do anything. */
    if (!isDeterministic())
	return *this;

    terminal_sets_computed = false;

    /* Look for the ERROR state. If there is no ERROR state, create one. */
    e.dest = ~0U;
    for (unsigned int i=0; i<nodes.size(); i++) {
	switch (get_type(i)) {
	    case LtsNode::Error:
		e.dest = i;
		break;
	    case LtsNode::End:
		/* Convert an END node into a normal node. */
		set_type(i, LtsNode::Normal);
		break;
	}
    }
    if (e.dest == ~0U) {
	nodes.push_back(LtsNode());
	set_type(nodes.size() - 1, LtsNode::Error);
	e.dest = nodes.size() - 1;
    }

    /* For each node different from the ERROR node, consider all the actions
       in the alphabet that don't label an edge outgoing from the node.
       For such actions, create an outgoing edge to the ERROR state. */
    for (unsigned int i=0; i<nodes.size(); i++)
	if (i != e.dest) {
	    set<int> to_error = alphabet;
	    for (unsigned int j=0; j<nodes[i].children.size(); j++)
		to_error.erase(nodes[i].children[j].action);
	    for (set<int>::iterator it=to_error.begin();
					it!=to_error.end(); it++) {
		e.action = *it;
		nodes[i].children.push_back(e);
	    }
	}

    return *this;
}

int yy::Lts::progress(const string& progress_name, const ProgressS& pr,
					    stringstream& ss)
{
    CHECKATP(0);

    terminalSets();

    for (unsigned int i=0; i<terminal_sets.size(); i++) {
	TerminalSet& ts = terminal_sets[i];
	bool violation;

        if (pr.conditional) {
            /* There is a conditional progress violation for the progress 'pr'
               if the intersection between 'pr.condition' and the terminal set
              'ts' is not empty and the intersection between 'pr.set' and 'ts'
              is empty. */
            violation = intersection_exists(pr.condition.actions, ts.actions) &&
                        !intersection_exists(pr.set.actions, ts.actions);
        } else {
            /* There is an uncoditional progress violation for the progress 'pr'
               if the intersection between 'pr.set' and the terminal set 'ts'
               is empty. */
            violation = !intersection_exists(pr.set.actions, ts.actions);
        }

	if (violation) {
	    ss << "Progress violation detected for process " << name
		<< " and progress property " << progress_name << ":\n";
	    ss << "	Trace to violation: ";
	    for (unsigned int j=0; j<ts.trace.size(); j++)
		ss << ati(atp, ts.trace[j], false) << "-> ";
	    ss << "\n";
	    ss << "	Actions in terminal set: {";
	    for (set<unsigned int>::iterator it=ts.actions.begin();
		    it!=ts.actions.end(); it++)
		ss << ati(atp, *it, false) << ", ";
	    ss << "}\n\n";
	    
	}
    }

    return 0;
}

struct OutputData {
    fstream * fsptr;
    ActionsTable * atp;
};

void yy::Lts::graphvizOutput(const char * filename) const
{
    fstream fout;

    CHECKATP();

    fout.open(filename, fstream::out);
    fout << "digraph G {\n";
    fout << "rankdir = LR;\n";
    //fout << "ratio = 1.0;\n";
    for (unsigned int i=0; i<nodes.size(); i++) {
	switch (get_type(i)) {
	    case LtsNode::Normal:
		fout << i
		    << " [shape=circle,style=filled, fillcolor=pink];\n";
		break;
	    case LtsNode::End:
		fout << i
		    << " [shape=circle,style=filled, fillcolor=blue];\n";
		break;
	    case LtsNode::Error:
		fout << i
		    << " [shape=circle,style=filled, fillcolor=red];\n";
		break;
            case LtsNode::Unresolved:
		fout << i
		    << " [shape=circle,style=filled, fillcolor=green];\n";
	}
    }

    for (unsigned int i=0; i<nodes.size(); i++) {
        for (unsigned int j=0; j<nodes[i].children.size(); j++) {
            const Edge& e = nodes[i].children[j];

	    fout << i << " -> " << e.dest
	            << " [label = \"" << ati(atp, e.action, false) << "\"];\n";
        }
    }

    fout << "}\n";
    fout.close();
}

void yy::Lts::print_trace(const vector<int>& trace, stringstream& ss) const
{
    int size = trace.size();

    CHECKATP();

    if (!size)
	return;

    ss << "    Current trace:\n";
    ss << "        ";
    for (int i=0; i<size-1; i++) {
	ss << ati(atp, trace[i], false) << " -> ";
    }
    ss << ati(atp, trace[size-1], false) << "\n";
}

void yy::Lts::simulate(Shell& sh, const ActionSetS *menu) const
{
    stringstream ss;
    int state = 0;
    vector<int> trace;

    CHECKATP();

    for (;;) {
	unsigned int i;
	string choice;
	set<unsigned int> elegible_actions_set;
	vector<unsigned int> elegible_actions;
        vector<unsigned int> system_actions;
	char * dummy;
	unsigned long idx;
	vector<int> dest;
        unsigned int a;

	/* Build the elegible actions as a set in order to remove 
	   duplicates. */
	for (i=0; i<nodes[state].children.size(); i++) {
	    elegible_actions_set.insert(nodes[state].children[i].action);
	}

	/* Build two vector<int>'s from the set<int>. */
	for (set<unsigned int>::iterator it = elegible_actions_set.begin();
		it != elegible_actions_set.end(); it++) {
            if (!menu || menu->lookup(*it)) {
                /* Actions that can be chosen by the user. */
	        elegible_actions.push_back(*it);
            } else {
                /* Actions that must be chosen by the system (because they
                   are excluded by an user-specified menu. */
                system_actions.push_back(*it);
            }
	}

	print_trace(trace, ss);

	if (elegible_actions.size() + system_actions.size() == 0) {
            /* No actions are possible: Quit the simulation. */
	    ss << "    Simulation done.\n";
	    sh.putsstream(ss, true); ss.clear();
	    break;
	}

choose:
	ss << "    Elegible actions: \n";
	for (i=0; i<elegible_actions.size(); i++) {
	    ss << "	    (" << i+1 << ") "
		    << ati(atp, elegible_actions[i], false) << "\n";
	}
        if (system_actions.size()) {
            /* There are actions excluded from the menu) */
	    ss << "	    (" << i+1 << ") " << "system choice\n";
            i++;
        }
	ss << "    Your choice ('q' to quit): ";
	sh.putsstream(ss, false); ss.clear();
	sh.readline(choice);
	if (choice.size() && choice[0] == 'q')
	    return;

	idx = strtoul(choice.c_str(), &dummy, 10);
	if ((idx < 1) || (idx > i) || (*dummy != '\0')) {
	    ss << "        Invalid choice\n\n";
	    goto choose;
	}
	idx--; /* Back to 0-based indexes. */

        if (idx >= elegible_actions.size()) {
            /* System chosen action. */
            a = system_actions[0]; // TODO random choice
        } else {
            /* User chosen action. */
            a = elegible_actions[idx];
        }

        for (i=0; i<nodes[state].children.size(); i++) {
            if (nodes[state].children[i].action == a)
                dest.push_back(nodes[state].children[i].dest);
        }
        trace.push_back(a);

        /* TODO make a random choice here. */
        assert(dest.size() > 0);
        state = dest[0];

        ss << "\n";
        sh.putsstream(ss, true); ss.clear();
    }
}

static void basicVisitFunction(int state, const yy::Lts& lts, const struct LtsNode& node,
				void * opaque)
{
    OutputData * bvd = static_cast<OutputData *>(opaque);
    ActionsTable * atp = bvd->atp;
    fstream * fsptr = bvd->fsptr;
    int size = node.children.size();

    *fsptr << ",\nS" << state << " = ";
    if (!size) {
	assert(lts.get_type(state) != LtsNode::Normal);
	switch (lts.get_type(state)) {
	    case LtsNode::Error:
		*fsptr << "ERROR";
		break;
	    case LtsNode::End:
		*fsptr << "END";
		break;
	    default:
		assert(0);
	}
    } else {
	*fsptr << "(";

	for (int i=0; i<size-1; i++) {
	    *fsptr << ati(atp, node.children[i].action, true)
		<< " -> S" << node.children[i].dest << "\n  | ";
	}
	*fsptr << ati(atp, node.children[size-1].action, true)
	    << " -> S" << node.children[size-1].dest << ")";
    }
}

void yy::Lts::basic(const string& outfile, stringstream& ss) const
{
    fstream fout(outfile.c_str(), fstream::out);
    LtsVisitObject lvo;
    OutputData bvd;

    CHECKATP();

    fout << name << " = S0";

    lvo.vfp = basicVisitFunction;
    bvd.atp = atp;
    bvd.fsptr = &fout;
    lvo.opaque = &bvd;
    visit(lvo);

    fout << ".\n";

    fout.close();
}

/* Minimize the number of states of the LTS. For now the
   algorithm only aggregates states that are reacheable
   through a finite tau-steps. */
void yy::Lts::minimize(stringstream& ss)
{
    set<unsigned int> frontier;
    vector<unsigned int> current;
    vector<bool> seen(nodes.size());
    unsigned int st;
    unsigned int cur_idx;

    if (!nodes.size()) {
        return;
    }

    /* Three data structures here:
        - 'current', a set which contains the current tau-reachable
          nodes that have been seen (not necessarily visited) during
           the visit
        - 'frontier', a set which contains the nodes seen during the
           visit that are not part of the 'current' set
        - 'seen', a bitmap that marks the nodes seen during the visit

       Note that if 'seen[x]' is true, then 'x' must be contained in
       either 'current' or 'frontier', but not both.
    */

    for (unsigned int i = 0; i < seen.size(); i++) {
        seen[i] = false;
    }

    /* At initialization time 'frontier' is empty, while 'current'
       contains the zero node. 'cur_idx' is an index into 'current'.*/
    cur_idx = 0;
    current.push_back(0);
    seen[0] = true;

    for (;;) {
        /* In the generic iteration, we first try to extract a node from
           'current' which has not been visited yet (although it may have
           been seen many times). */
        if (cur_idx < current.size()) {
            st = current[cur_idx++];
        } else {
           /* If all the nodes in 'current' have been visited, it means the
              'current' is a tau-reachable set that can be replaced by a single
              state in a minimized LTS.
              We then try to extract a node from 'frontier'.
            */
            ss << "New set found: {";
            for (unsigned int i = 0; i < current.size(); i++) {
                ss << current[i] << ",";
            }
            ss << "}\n";

            if (frontier.empty()) {
                /* Frontier is empty, we've done with the minimizing visit. */
                break;
            }

            /* Reset 'current' so that contains the node just extracted from
               'frontier', which is then removed from 'frontier'. This
               situation is analogue to what happens at initialization time,
               where 'current' contains a starting node that will aggregate
               all the tau-reachable node to itself. */
            current.clear();
            st = *(frontier.begin());
            frontier.erase(frontier.begin());
            current.push_back(st);
            cur_idx = 1;
        }

        const LtsNode& n = nodes[st];

        /* Visit a node. */
        for (unsigned int i = 0; i < n.children.size(); i++) {
            const Edge& e = n.children[i];

            if (e.action == 0) {
                /* This is a tau-transition. */
                if (seen[e.dest] && frontier.count(e.dest)) {
                    /* This happens if we "mistakenly" added the destination
                       to 'frontier', because of a previous non-tau-transition
                       towards the same destination. Make up for the mistake,
                       removing the destination from 'frontier', and pretending
                       that the destination has never been seen before. It will
                       be added to 'current' below. */
                    frontier.erase(e.dest);
                    seen[e.dest] = false;
                }
                if (!seen[e.dest]) {
                    /* Add the destination to 'current' if it has not been
                       seen previously. */
                    current.push_back(e.dest);
                }
            } else if (!seen[e.dest]) {
                /* This transition has a regular action. Add the destination to
                   frontier if it has not been seen previously. */
                frontier.insert(e.dest);
            }
            /* Whatever happened in this inner loop iteration, mark the
               destination as seen. */
            seen[e.dest] = true;
        }
    }
}

void LtsNode::offset(int offset)
{
    for (unsigned int j=0; j<children.size(); j++) {
        children[j].dest += offset;
    }
}

/* This function is used to concatenate 'lts' to *this, without
   creating any transitions between the two. This function returns the
   offset that can be used to compute the 'Edge.dest' field of connection
   going from this->nodes[k] to a state in 'lts' after the concatenation.
*/
unsigned int yy::Lts::append(const yy::Lts& lts, unsigned int first)
{
    unsigned int offset = nodes.size() - first;

    assert(first < nodes.size());

    /* Append the new nodes in this->nodes, offsetting the destinations. */
    for (unsigned int i=first; i<lts.nodes.size(); i++) {
        nodes.push_back(LtsNode());
        copy_node_in(nodes.size() - 1, lts, i);
        nodes.back().offset(offset);
    }

    mergeAlphabetFrom(lts.alphabet);

    return offset;
}

/* This function extends *this appending 'lts' to the start node
   (nodes[0]). The two Lts object are connected by an edge labeled
   with 'label'.
*/
yy::Lts& yy::Lts::zerocat(const yy::Lts& lts, const string& label)
{
    unsigned int offset = append(lts, 0);
    Edge e;

    CHECKATP(*this);

    /* Make the connection. */
    e.dest = offset;
    e.action = atp->insert(label);
    alphabet.insert(e.action);
    nodes[0].children.push_back(e);

    return *this;
}

/* Remove incomplete nodes (and related transitions) from *this, compacting
   the this->nodes vector. */
void yy::Lts::removeType(unsigned int type, unsigned int zero_idx,
                         bool call_reduce)
{
    vector<unsigned int> remap(nodes.size());
    unsigned int cnt = 0;

    /* Create the mapping from the original state names (indexes) to the names after
       compacting. */
    if (zero_idx != ~0U) {
        assert(zero_idx < nodes.size());
        remap[zero_idx] = cnt++;
    }
    for (unsigned int i=0; i<nodes.size(); i++) {
        if (get_type(i) == type) {
            remap[i] = ~0U;  /* Undefined remapping. */
        } else if (i != zero_idx) {
            remap[i] = cnt++;
        }
    }

    Lts new_lts;

    new_lts.nodes.resize(cnt);

    /* Regenerate this->nodes using the mapping. */
    for (unsigned int i=0; i<nodes.size(); i++) {
        const LtsNode& n = nodes[i];
        unsigned k = remap[i];

        if (k != ~0U) {
            /* Rule out incomplete nodes. */
            new_lts.set_type(k, get_type(i));
            new_lts.set_priv(k, get_priv(i));
            for (unsigned int j=0; j<n.children.size(); j++) {
                Edge e = n.children[j];

                if (remap[e.dest] != ~0U) {
                    /* Rule out transitions towards incomplete nodes. */
                    e.dest = remap[e.dest];
                    new_lts.nodes[k].children.push_back(e);
                }
            }
        }
    }

    if (call_reduce) {
        reduce(new_lts);
    } else {
        copy_nodes_in(new_lts);
    }
}

/* Append the LTS objects contained into 'ltsv' to *this, replacing each
   transition 'x --> I(k)' (where 'x' is a node in *this, and 'I(k)' an
   incomplete node in *this whose priv field is 'k') with the transition
   'x -> ltsv[k][0]', where 'lts[k][0]' is the zero node of 'ltsv[k]'.
   The incomplete nodes in *this are then removed from *this.
*/
yy::Lts& yy::Lts::incompcat(const vector<yy::Lts>& ltsv)
{
    unsigned int num_nodes = nodes.size();
    vector<unsigned int> offsets(ltsv.size());
    unsigned int priv;

    /* Prepare the 'offset' array, parallel to 'ltsv'. The entry 'offset[k]'
       contains the state index of *this corresponding to 'ltsv[k][0]'
       (after ltsv[k] has been appended to *this, obviously).*/
    for (unsigned int i=0; i<ltsv.size(); i++) {
        /* '~0U' means that 'ltsv[k]' has not been appended yet to *this. */
        offsets[i] = ~0U;
    }

    for (unsigned int i=0; i<num_nodes; i++) {
        unsigned int sz = nodes[i].children.size();

        for (unsigned int j=0; j<sz; j++) {
            Edge e = nodes[i].children[j];

            if (get_type(e.dest) == LtsNode::Incomplete) {
                priv = get_priv(e.dest);
                assert(priv < offsets.size());
                if (offsets[priv] == ~0U) {
                    offsets[priv] = append(ltsv[priv], 0);
                }
                /* Replace incomplete node destinations with the zero
                   node of 'ltsv[priv]'. */
                e.dest = offsets[priv];
                nodes[i].children.push_back(e);
            }
        }
    }

    /* Compact *this in order to remove incomplete nodes and related
       transitions. */
    removeType(LtsNode::Incomplete, ~0U, false);

    return *this;
}

/* Append 'lts' to *this, and then create a connection from *this[0] to
   lts[0].
*/
yy::Lts& yy::Lts::zeromerge(const yy::Lts& lts)
{
    unsigned int offset = append(lts, 1);

    /* Make the connections. */
    for (unsigned int j=0; j<lts.nodes[0].children.size(); j++) {
        Edge e = lts.nodes[0].children[j];

        e.dest += offset;
        nodes[0].children.push_back(e);
    }

    return *this;
}

/* Append 'lts' to *this and connect the End node of this to 'lts[0]'. */
bool yy::Lts::endcat(const yy::Lts& lts)
{
    unsigned int offset;
    unsigned int x;

    /* Find the End node (well, the first that we run into, but this
       method should be invoked after the mergeEndNodes method). */
    for (x=0; x<nodes.size(); x++) {
        if (get_type(x) == LtsNode::End) {
            break;
        }
    }

    if (x == nodes.size()) {
        return false;
    }

    /* Append everithing but the 'lts[0]'. */
    offset = append(lts, 1);

    /* Replace the End node with 'lts[0]'. */
    copy_node_in(x, lts, 0);
    /* Offset the transitions of 'lts[0]'. */
    nodes[x].offset(offset);

    return true;
}

/* Merge all the End nodes into a single End node, modifying the
   involved transitions accordingly. */
yy::Lts& yy::Lts::mergeEndNodes()
{
    unsigned int x;

    /* Select the first End node that we run into. */
    for (x=0; x<nodes.size(); x++) {
        if (get_type(x) == LtsNode::End) {
            break;
        }
    }

    if (x < nodes.size()) {
        bool zombies = false;

        /* Make all the transitions towards an End node point to
           the selected End node x. */
        for (unsigned int i=0; i<nodes.size(); i++) {
            for (unsigned int j=0; j<nodes[i].children.size(); j++) {
                Edge& e = nodes[i].children[j];

                if (get_type(e.dest) == LtsNode::End) {
                    e.dest = x;
                }
            }
        }
        /* Turn the type of the non-selected End nodes into LtsNode::Zombie,
           so that the can be removed using the removeType method. */
        for (unsigned int i=0; i<nodes.size(); i++) {
            if (get_type(i) == LtsNode::End && i != x) {
                set_type(i, LtsNode::Zombie);
                zombies = true;
            }
        }
        if (zombies) {
            removeType(LtsNode::Zombie, ~0U, false);
        }
    }

    return *this;
}

static void extend_infos(vector<LtsNodeInfo>& infos, unsigned int request,
                 unsigned int hint)
{
    unsigned int size = infos.size();
    unsigned int max = LtsNode::NoPriv;
    unsigned int type = LtsNode::Normal;

    if (request >= size) {
        for (unsigned int i = size; i < hint; i++) {
            infos.push_back(LtsNodeInfo());
            infos.back().priv = max;
            infos.back().type = type;
        }
    }
}

void yy::Lts::set_type(unsigned int state, unsigned int type)
{
    assert(state < nodes.size());

    extend_infos(infos, state, nodes.size());

    infos[state].type = type;
    if (type == LtsNode::End) {
        end = state;
    } else if (type == LtsNode::Error) {
        err = state;
    }
}

unsigned int yy::Lts::get_type(unsigned int state) const
{
    assert(state < nodes.size());

    if (state >= infos.size()) {
        if (state == end) {
            return LtsNode::End;
        }
        if (state == err) {
            return LtsNode::Error;
        }
        return LtsNode::Normal;
    }

    return infos[state].type;
}

/* Set the 'priv' field of *this[state] to 'val'. */
void yy::Lts::set_priv(unsigned int state, unsigned int val)
{
    assert(state < nodes.size());

    extend_infos(infos, state, nodes.size());

    infos[state].priv = val;
}

/* Get the 'priv' field of *this[state]. */
unsigned int yy::Lts::get_priv(unsigned int state) const
{
    assert(state < nodes.size());

    if (state >= infos.size()) {
        return LtsNode::NoPriv;
    }

    return infos[state].priv;
}

/* Scan the graph looking for transitions towards unresolved nodes. Say
   'X -> U(p)' is one of those transitions, where U(p) is an unresolved
   node whose priv field is 'p'. This transition is replaced by the
   transition 'X -> R(p)', where R(p) is a non-unresolved node with priv 'p'.
   If R(p) does not exist in the graph, this method returns immediately 'p',
   reporting to the caller that it has not been possible to resolve the
   name 'p'.
   Afterwards, all the unresolved nodes are removed from the graph.
   TODO add same caching/precomputation to make this function more efficient.
*/
unsigned int yy::Lts::resolve()
{
    unsigned int priv;
    unsigned int zero_idx = ~0U;

    for (unsigned int i=0; i<nodes.size(); i++) {
        unsigned int sz = nodes[i].children.size();

        if (get_type(i) != LtsNode::Unresolved
                    && get_priv(i) == get_priv(0)) {
            /* Look for a non-unresolved node with the same priv as the zero
               node, which may be an unresolved nodes (this happens with definitions
               in the form 'P = Q, [...], Q = ... .') */
            zero_idx = i;
        }
        for (unsigned int j=0; j<sz; j++) {
            Edge& e = nodes[i].children[j];

            if (get_type(e.dest) == LtsNode::Unresolved) {
                bool found = false;

                priv = get_priv(e.dest);
                assert(priv != LtsNode::NoPriv);
                /* Look for a non-unresolved node with a matching 'priv'. */
                for (unsigned int k=0; k<nodes.size(); k++) {
                    if (get_type(k) != LtsNode::Unresolved
                                    && get_priv(k) == priv) {
                        /* Replace the unresolved destination with the
                           state 'k'. */
                        e.dest = k;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return priv;
                }
            }
        }
    }

    /* Compact the Lts in order to remove unresolved nodes and related
       transitions. Using the 'zero_idx' argument, we specify what node
       we want to be remapped to the zero node. With complex local
       process definitions you get an aribitrary graph, and so
       it is necessary to specify this. */
    removeType(LtsNode::Unresolved, zero_idx, true);

    /* Reset the priv fields. */
    for (unsigned int i=0; i<nodes.size(); i++) {
        set_priv(i, LtsNode::NoPriv);
    }

    return LtsNode::NoPriv;
}

/* Scan the graph looking for nodes whose 'priv' is contained in the 'privs'
   set. When an element of 'privs' is matched, is removed from the set, so
   that when the method returns, the 'privs' set contains the unmatched
   elements.
   <<<UNUSED>>>
*/
void yy::Lts::check_privs(set<unsigned int>& privs)
{
    for (unsigned int i=0; i<nodes.size(); i++) {
        unsigned int priv = get_priv(i);

        if (privs.count(priv)) {
            privs.erase(priv);
        }
    }
}

/* Scan the 'priv' fields, replacing each occurence of 'old_priv' with
   'new_priv'.
*/
void yy::Lts::replace_priv(unsigned int new_priv, unsigned int old_priv)
{
    for (unsigned int i=0; i<nodes.size(); i++) {
        if (get_priv(i) == old_priv) {
            set_priv(i, new_priv);
        }
    }
}


/* =========================== LtsPtr ============================== */
//#define DBG_REFCNT

#ifdef DBG_REFCNT
#define DREF(p) do \
    cout << __func__ << ":"<< p << ":" << p->refcount << "\n"; \
    while (0);
#else  /* !DBG_REFCNT */
#define DREF(p)
#endif /* !DBG_REFCNT */

yy::LtsPtr::LtsPtr() : ptr(NULL)
{
}

yy::LtsPtr::LtsPtr(yy::Lts *lts)
{
    ptr = lts;

    if (ptr) {
        ptr->refcount++;
        DREF(lts);
    }
}

yy::LtsPtr::LtsPtr(const LtsPtr& p)
{
    ptr = p.ptr;

    if (ptr) {
        ptr->refcount++;
        DREF(ptr);
    }
}

yy::LtsPtr& yy::LtsPtr::operator=(yy::LtsPtr& p)
{
    if (ptr) {
        ptr->refcount--;
        DREF(ptr);
        assert(ptr->refcount >= 0);
        if (ptr->refcount == 0) {
            delete ptr;
        }
    }

    ptr = p.ptr;

    if (ptr) {
        ptr->refcount++;
        DREF(ptr);
    }

    return *this;
}

yy::LtsPtr& yy::LtsPtr::operator=(yy::Lts *lts)
{
    if (ptr) {
        ptr->refcount--;
        DREF(ptr);
        assert(ptr->refcount >= 0);
        if (ptr->refcount == 0) {
            delete ptr;
        }
    }

    ptr = lts;

    if (ptr) {
        ptr->refcount++;
        DREF(ptr);
    }

    return *this;
}

yy::LtsPtr::operator Lts*()
{
    return ptr;
}

yy::Lts* yy::LtsPtr::delegate()
{
    if (ptr) {
        ptr->refcount++;
        DREF(ptr);
    }

    return ptr;
}

yy::LtsPtr::~LtsPtr()
{
    if (ptr) {
        ptr->refcount--;
        DREF(ptr);
        assert(ptr->refcount >= 0);
        if (ptr->refcount == 0) {
            delete ptr;
        }
    }

    ptr = NULL;
}

void yy::LtsPtr::clear()
{
    if (ptr) {
        ptr->refcount--;
        DREF(ptr);
        assert(ptr->refcount >= 0);
        if (ptr->refcount == 0) {
            delete ptr;
        }
    }

    ptr = NULL;
}

