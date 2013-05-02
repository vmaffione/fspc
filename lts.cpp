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
#include <assert.h>
#include "lts.hpp"
#include "symbols_table.hpp"
#include "utils.hpp"

using namespace std;


//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


#define ati(index) atp->reverse[index]


/* ====================== class Lts implementation ===================== */
int Lts::lookupAlphabet(int action) const
{
    set<int>::iterator it = alphabet.find(action);
    
    return (it == alphabet.end()) ? -1 : 0;
}

void Lts::updateAlphabet(int action)
{
    alphabet.insert(action);
}

void Lts::printAlphabet() const
{
    set<int>::iterator it;

    cout << "Alphabet: {";
    for (it=alphabet.begin(); it != alphabet.end(); it++)    
	cout << ati(*it) << ", ";
    cout << "}\n";
}

Lts::Lts(int type, struct ActionsTable * p) : atp(p)
{
    struct LtsNode node;

    node.type = type;
    nodes.push_back(node);
    ntr = 0;
    terminal_sets_computed = false;
}

struct LtsConvertData {
    Lts * ltsp;
    map<ProcessNode *, int> * mapp;
};

void lts_convert(struct ProcessNode * pnp, void * opaque)
{
    Lts * ltsp = ((LtsConvertData *)opaque)->ltsp;
    map<ProcessNode *, int> * mapp = ((LtsConvertData *)opaque)->mapp;
    int state = (mapp->find(pnp))->second;

    IFD(cout << "Converting " << pnp << " into " << state << "\n");
    
    switch (pnp->type) {
	case ProcessNode::Normal:
	    ltsp->nodes[state].type = LtsNode::Normal;
	    break;
	case ProcessNode::End:
	    ltsp->nodes[state].type = LtsNode::End;
	    break;
	case ProcessNode::Error:
	    ltsp->nodes[state].type = LtsNode::Error;
	    break;
    }

    Edge e;
    pair< map<ProcessNode*, int>::iterator, bool> ret;

    for (int i=0; i<pnp->children.size(); i++) {
	int next = mapp->size();
	ret = mapp->insert(make_pair(pnp->children[i].dest, next));
	if (ret.second) {
	    ltsp->nodes.push_back(LtsNode());
	    IFD(cout << "(" << pnp->children[i].dest << "," << next << ")\n");
	}
	e.dest = ret.first->second;
	e.action = pnp->children[i].action;
	IFD(cout << "Adding " << e.action << ", " << e.dest << "\n");
	ltsp->nodes[state].children.push_back(e);
	ltsp->updateAlphabet(e.action);
	ltsp->ntr++;
    }
}

Lts::Lts(const struct ProcessNode * cpnp, struct ActionsTable * p) : atp(p)
{
    ProcessNode * pnp;
    ProcessVisitObject f;
    LtsConvertData lcd;
    map<ProcessNode *, int> index_map;

    terminal_sets_computed = false;

    /* We use const_cast<> to remove 'const' from pnp: we are sure that the 
       visit function won't modify the graph. */
    pnp = const_cast<ProcessNode *>(cpnp);

    /* The index_map maps each ProcessNode* in the graph to an index in
       Lts::nodes. The map is build and used during the visit, but we must
       initialize it with respect to the first node. */
    index_map.insert(make_pair(pnp, 0));
    nodes.push_back(LtsNode());

    lcd.ltsp = this;
    lcd.mapp = &index_map;
    f.vfp = &lts_convert;
    f.opaque = &lcd;

    ntr = 0;
    pnp->visit(f, true);
}

Lts::Lts(fstream& fin, struct ActionsTable * p) : atp(p)
{
    int n;
    int na;
    int end;
    int error;
    int s1, s2;
    string a;
    Edge e;

    fin >> name >> n >> ntr >> na >> end >> error;
    if (fin.eof())
	return;
    if (n < 1) {
	cout << "Error: Corrupted input: n < 1\n";
	exit(-1);
    }
    terminal_sets_computed = false;

    nodes.resize(n);
    for (int i=0; i<n; i++)
	nodes[i].type = LtsNode::Normal;
    if (error != -1)
	nodes[error].type = LtsNode::Error;
    if (end != -1)
	nodes[end].type = LtsNode::End;

    for (int j=0; j<ntr; j++) {
	fin >> s1 >> a >> s2;

	if (s1 < 0 || s2 < 0) {
	    cout << "Error: Corrupted input: Negative state number\n";
	    exit(-1);
	}
	if (s1 >= n || s2 >=n) {
	    cout << "Error: Corruped input s1 >= n || s2 >= n\n";
	}
	IFD(cout << "Read (" << s1 << ", " << a << ", " << s2 << ")\n");
	e.dest = s2;
	e.action = atp->insert(a);
	nodes[s1].children.push_back(e);
    }

    for (int j=0;j<na;j++) {
	fin >> a;
	updateAlphabet(atp->insert(a));
    }
}

void Lts::print() const {
    atp->print();
    cout << "LTS " << name << "\n";
    for (int i=0; i<nodes.size(); i++) {
	cout << "State " << i << ":\n";
	for (int j=0; j<nodes[i].children.size(); j++)
	    cout << "    " << ati(nodes[i].children[j].action)
		    << " --> " << nodes[i].children[j].dest << "\n";
    }
    printAlphabet();
    cout << numStates() << " states, " << ntr << " transitions\n";
}

/* BFS on the LTS for useless states removal. */
void Lts::reduce(const vector<LtsNode>& unconnected)
{
    int np = unconnected.size();
    vector<int> frontier(np);
    vector<int> map(np);
    int pop, push;
    int state;
    int n = 0;

    /* We make sure that 'nodes' si empty and 'ntr' is zero. */
    nodes.clear();
    ntr = 0;
    terminal_sets_computed = false;

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
	for (int j=0; j<unconnected[state].children.size(); j++) {
	    int child = unconnected[state].children[j].dest;

	    if (map[child] == -1) {
		map[child] = n++;
		frontier[push++] = child;
	    }
	    e.dest = map[child];
	    e.action = unconnected[state].children[j].action;
	    nodes[map[state]].children.push_back(e);
	    ntr++;
	}
    }

    for (int i=0; i<np; i++)
	if (map[i] != -1)
	    nodes[map[i]].type = unconnected[i].type;

    nodes.resize(n);
}


void Lts::compose(const Lts& p, const Lts& q)
{    
    int n;
    int nq;
    int np;
    Edge e;
    vector<LtsNode> product;

    assert(p.atp == atp && q.atp == atp);

    /* First of all we reset *this, like Lts(ActionsTable *) would do. */
    nodes.clear();
    ntr = 0;
    terminal_sets_computed = false;
    alphabet.clear();

    /* We build the cartesian product of P states and Q states. For 
       convenience, we map the composite state (p,q) with the "linear" state
       p*nq+q (exactly how a matrix is mapped onto a vector. */
    np = p.numStates();
    nq = q.numStates();
    n = np * nq;
    product.resize(n);

    /* Scan the P graph and combine P actions with Q states. */
    for (int ip=0; ip<p.nodes.size(); ip++)
	for (int jp=0; jp<p.nodes[ip].children.size(); jp++) {
	    const Edge& ep = p.nodes[ip].children[jp];
	    /* We analyze an edge of P: (i, ep.action, ep.dest). */
	    e.action = ep.action;

	    if (q.lookupAlphabet(ep.action) == -1) {
		/* If ep.action is not included in the alphabet of Q, this
		   action can be executed by P independently on the state
		   of Q. */
		for (int iq=0; iq<nq; iq++) {
		    e.dest = ep.dest * nq + iq;
		    product[ip*nq+iq].children.push_back(e);
		}
	    } else {
		/* If ep.action is included in the alphabet of Q, this
		   action can be executed by P only together with Q. */
		for (int iq=0; iq<q.nodes.size(); iq++)
		    for (int jq=0; jq<q.nodes[iq].children.size(); jq++) {
			const Edge& eq = q.nodes[iq].children[jq];

			if (eq.action == ep.action) {
			    e.dest = ep.dest * nq + eq.dest;
			    product[ip*nq+iq].children.push_back(e);
			}
		    }
		
	    }
	}

    /* Scan the Q graph and combine Q actions with P states */
    for (int iq=0; iq<q.nodes.size(); iq++)
	for (int jq=0; jq<q.nodes[iq].children.size(); jq++) {
	    const Edge& eq = q.nodes[iq].children[jq];
	    /* We analyze an edge of Q: (i, eq.action, eq.dest). */
	    e.action = eq.action;

	    if (p.lookupAlphabet(eq.action) == -1) {
		/* If ep.action is not included in the alphabet of P, this
		   action can be executed by Q independently on the state
		   of P. */
		for (int ip=0; ip<np; ip++) {
		    e.dest = ip * nq + eq.dest;
		    product[ip*nq+iq].children.push_back(e);
		}
	    } /* else case has already been covered by the previous scan. */
	}

    /* A composed state is an END state when both the components are
       END states. */
    for (int ip=0; ip<p.nodes.size(); ip++)
	for (int iq=0; iq<q.nodes.size(); iq++)
	    if ((p.nodes[ip].type == LtsNode::Error) ||
		    (q.nodes[iq].type == LtsNode::Error))
		product[ip*nq+iq].type = LtsNode::Error;
	    else if ((p.nodes[ip].type == LtsNode::End) &&
		    (q.nodes[iq].type == LtsNode::End))
		product[ip*nq+iq].type = LtsNode::End;

    for (set<int>::iterator it=p.alphabet.begin();
				it!=p.alphabet.end(); it++)
	updateAlphabet(*it);
    for (set<int>::iterator it=q.alphabet.begin();
				it!=q.alphabet.end(); it++)
	updateAlphabet(*it);

    reduce(product);
}

Lts::Lts(const Lts& p, const Lts& q)
{
    assert(p.atp && q.atp == p.atp);
    atp = p.atp;
    compose(p, q);
}

Lts& Lts::compose(const Lts& q)
{
    Lts copy(*this);

    compose(copy, q);

    return *this;
}

int Lts::deadlockAnalysis() const
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

    /* BFS looking for states with no outgoing transitions. We use a BFS
       instead of a DFS because the former is simpler to implement and
       because it finds the sortest path to each deadlock state. */

    /* Initialize a queue that only contains the 0 node. */
    pop = 0;
    push = 1;
    frontier[0] = 0;
    seen[0] = true;
    for (int i=1; i<n; i++)
	seen[i] = false;

    /* Keep visiting until the queue is empty. */
    while (pop != push) {
	int i;

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
	if (i == 0 && nodes[state].type != LtsNode::End) { 
	    int t;
	    int j;
	    string ed;

	    if (nodes[state].type == LtsNode::Normal)
		ed = "Deadlock";
	    else
		ed = "Property violation";
	    cout << ed << " found for process " << name << ": state "
			<< state << "\n";
	    /* Starting from 'state', we follow the backpointers to build the
	       trace to deadlock (in reverse order). */
	    t = pop;
	    j = 0;
	    while (t) {
		action_trace[j++] = actions[t];
		t = back[t];
	    }
	    cout << "	Trace to " << ed << ": ";
	    for (j--; j>=0; j--)
		cout << ati(action_trace[j]) << "->";
	    cout << "\n\n";
	    nd++;
	}
	pop++; /* Pops 'state' from the queue */
    }

    return nd;
}

int Lts::terminalSets()
{
    int n = nodes.size();
    int na = atp->reverse.size();
    int nts = 0;

    if (terminal_sets_computed)
	return terminal_sets.size();
    terminal_sets_computed = true;

    /* Data structures for the iterative DFS implementation */
    vector<int> state_stack(n);  /* Emulated recursion stack */
    vector<int> action_stack(n); /* Actions stack parallel to the previous */
    vector<int> back(n);	 /* Backpointers parallel stack */
    vector<bool> entered(n);	 /* Marks states started to be visited */
    vector<int> next_child(n);	 /* Records the next child to examine */
    vector<int> action_trace(n);
    int top;

    /* Tarjan algorithm data structures */
    vector<int> tarjan_index(n);
    vector<int> tarjan_stack(n);
    vector<int> tarjan_lowlink(n);
    vector<bool> tarjan_in_stack(n); /* Set if state 'i' is on the stack */
    int tarjan_counter;
    int tarjan_top;

    /* Structures useful to represent a strongly connected components. */
    vector<int> tarjan_component_states(n);
    vector<bool> tarjan_state_in_component(n);
    vector<int> tarjan_component_actions(na);
    vector<bool> tarjan_action_in_component(na);

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

	    for (int i=0; i<nodes[state].children.size(); i++) {
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
			for (int i=0; i<nodes[s].children.size(); i++) {
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
			IFD(cout << "    " << ati(action_trace[j]) << "\n");
			ts.trace.push_back(action_trace[j]);
		    }
		    IFD(cout << "Actions in the terminal set: {");
		    for (j=0; j<nca; j++) {
			IFD(cout << ati(tarjan_component_actions[j]) << ", ");
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

SymbolValue * Lts::clone() const
{
    Lts * lv = new Lts(*this);

    return lv;
}

void Lts::visit(const struct LtsVisitObject& lvo) const
{
    int state = 0;
    int n = nodes.size();
    vector<int> frontier(n);
    vector<bool> seen(n);
    int pop, push;

    pop = 0;
    push = 1;
    frontier[0] = 0;
    seen[0] = true;
    for (int i=1; i<n; i++)
	seen[i] = false;

    while (pop != push) {
	state = frontier[pop++];
	/* Invoke the visit function */
	lvo.vfp(state, nodes[state], lvo.opaque);
	for (int i=0; i<nodes[state].children.size(); i++) {
	    int child = nodes[state].children[i].dest;
	    if (!seen[child]) {
		seen[child] = true;
		frontier[push++] = child;
	    }
	}
    }
}

/* Convert an Lts to a ProcessNode*. */
ProcessNode * Lts::toProcessNode(ProcessNodeAllocator& pna) const
{
    vector<ProcessNode *> pnodes(nodes.size());

    for (int i=0; i<nodes.size(); i++)
	pnodes[i] = pna.allocate(nodes[i].type);

    for (int i=0; i<nodes.size(); i++)
	for (int j=0; j<nodes[i].children.size(); j++) {
	    ProcessEdge e;

	    e.dest = pnodes[nodes[i].children[j].dest];
	    e.action = nodes[i].children[j].action;
	    e.rank = 0; //XXX ???
	    pnodes[i]->children.push_back(e);
	}

    return pnodes[0];
}

Lts& Lts::labeling(const SetValue& labels)
{
    if (!labels.actions.size())
	return *this;

    if (labels.actions.size() == 1)
	this->labeling(labels.actions[0]);
    else {
	Lts copy(*this);

	this->labeling(labels.actions[0]);
	for (int i=1; i<labels.actions.size(); i++) {
	    Lts right(copy);

	    right.labeling(labels.actions[i]);
	    this->compose(right);
	}
    }

    return *this;
}

Lts& Lts::labeling(const string& label)
{
    set<int> new_alphabet;
    map<int, int> mapping;

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
    for (int i=0; i<nodes.size(); i++)
	for (int j=0; j<nodes[i].children.size(); j++)
	    nodes[i].children[j].action = mapping[nodes[i].children[j].action];

    return *this;
}

Lts& Lts::sharing(const SetValue& labels)
{
    set<int> new_alphabet;
    map<int, vector<int> > mapping;

    terminal_sets_computed = false;

    /* Update the actions table, compute a one-to-many [old --> new] mapping 
       and update the alphabet. */
    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end(); it++) {
	int old_index;
	int new_index;
	vector<int> new_indexes;

	old_index = *it;
	for (int i=0; i<labels.actions.size(); i++) {
	    new_index = atp->insert(labels.actions[i] + "." + 
						atp->reverse[old_index]);
	    new_alphabet.insert(new_index);
	    new_indexes.push_back(new_index);
	}
	mapping.insert(make_pair(old_index, new_indexes));
    }
    alphabet = new_alphabet;

    /* Replace the children array of each node. */
    for (int i=0; i<nodes.size(); i++) {
	vector<Edge> new_children;

	for (int j=0; j<nodes[i].children.size(); j++) {
	    Edge e = nodes[i].children[j];
	    vector<int> new_indexes = mapping[e.action];

	    for (int k=0; k<new_indexes.size(); k++) {
		e.action = new_indexes[k];
		new_children.push_back(e);
	    }
	}
	nodes[i].children = new_children;
    }

    return *this;
}

Lts& Lts::relabeling(const SetValue& newlabels, const string& oldlabel)
{
    map<int, vector<int> > mapping;
    set<int> new_alphabet = alphabet;

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
	    for (int i=0; i<newlabels.actions.size(); i++) {
		string new_action = action;

		new_action.replace(0, oldlabel.size(), newlabels.actions[i]);
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
    for (int i=0; i<nodes.size(); i++) {
	int original_size = nodes[i].children.size();
	/* We need 'original_size' since we are going to push_back() in 
	   'nodes[i].children', e.g. the same vector we are cycling on. */

	for (int j=0; j<original_size; j++) {
	    Edge e = nodes[i].children[j];
    
	    if (mapping.count(e.action)) {
		vector<int> new_indexes = mapping[e.action];

		e.action = new_indexes[0];
		nodes[i].children[j] = e;
		for (int k=1; k<new_indexes.size(); k++) {
		    e.action = new_indexes[k];
		    nodes[i].children.push_back(e);
		}
	    }
	}
    }

    return *this;
}

Lts& Lts::relabeling(const SetValue& newlabels, const SetValue& oldlabels)
{
    for (int i=0; i<oldlabels.actions.size(); i++)
	this->relabeling(newlabels, oldlabels.actions[i]);

    return *this;
}

Lts& Lts::hiding(const SetValue& s, bool interface)
{
    set<int> new_alphabet;
    int action;

    terminal_sets_computed = false;

    /* Update the alphabet. */
    if (interface) {
	for (int i=0; i<s.actions.size(); i++) {
	    /* The action s.actions[i] can select multiple alphabet
	       elements. */
	    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end();
								it++) {
		string action = ati(*it);
		pair<string::const_iterator, string::iterator> mm;
		/* Prefix match: check if 's.actions[i]' is a prefix of
		   'action'. */
		mm = mismatch(s.actions[i].begin(), s.actions[i].end(),
							    action.begin());
		if (mm.first == s.actions[i].end())
		    new_alphabet.insert(*it);
	    }
	}
    } else {
	new_alphabet = alphabet;
	for (int i=0; i<s.actions.size(); i++) {
	    /* The action s.actions[i] can hide multiple alphabet elements. */
	    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end();
								it++) {
		string action = ati(*it);
		pair<string::const_iterator, string::iterator> mm;
		/* Prefix match: check if 's.actions[i]' is a prefix of
		   'action'. */
		mm = mismatch(s.actions[i].begin(), s.actions[i].end(),
							    action.begin());
		if (mm.first == s.actions[i].end())
		    new_alphabet.erase(*it);
	    }
	}
    }
    alphabet = new_alphabet;

    /* Update the edges actions. */
    for (int i=0; i<nodes.size(); i++)
	for (int j=0; j<nodes[i].children.size(); j++)
	    if (!alphabet.count(nodes[i].children[j].action))
		/* We are sure that atp->lookup("tau") == 0. */
		nodes[i].children[j].action = 0;

    return *this;
}

Lts& Lts::priority(const SetValue& s, bool low)
{
    int action;
    int low_int = (low) ? 1 : 0;
    set<int> priority_actions;
    vector<LtsNode> new_nodes(nodes.size());

    terminal_sets_computed = false;

    for (int i=0; i<s.actions.size(); i++) {
	    /* The action s.actions[i] can select multiple alphabet
	       elements. */
	    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end();
								it++) {
		string action = ati(*it);
		pair<string::const_iterator, string::iterator> mm;
		/* Prefix match: check if 's.actions[i]' is a prefix of
		   'action'. */
		mm = mismatch(s.actions[i].begin(), s.actions[i].end(),
							    action.begin());
		if (mm.first == s.actions[i].end())
		    priority_actions.insert(*it);
	    }
    }

    for (int i=0; i<nodes.size(); i++) {
	vector<Edge> new_children;
	bool found = false;

	for (int j=0; j<nodes[i].children.size(); j++) 
	    if (priority_actions.count(nodes[i].children[j].action)
							    ^ low_int) {
		new_children.push_back(nodes[i].children[j]);
		found = true;
	    }
	if (found) {
	    new_nodes[i].children = new_children;
	    new_nodes[i].type = nodes[i].type;
	} else
	    new_nodes[i] = nodes[i];
    }

    reduce(new_nodes);

    return *this;
}

Lts& Lts::property()
{
    Edge e;

    terminal_sets_computed = false;

    /* Look for the ERROR state. If there is no ERROR state, create one. */
    e.dest = -1;
    for (int i=0; i<nodes.size(); i++)
	if (nodes[i].type == LtsNode::Error) {
	    e.dest = i;
	    break;
	}
    if (e.dest == -1) {
	nodes.push_back(LtsNode());
	nodes.back().type = LtsNode::Error;
	e.dest = nodes.size() - 1;
    }

    /* For each node different from the ERROR node, consider all the actions
       in the alphabet that don't label an edge outgoing from the node.
       For such actions, create an outgoing edge to the ERROR state. */
    for (int i=0; i<nodes.size(); i++)
	if (i != e.dest) {
	    set<int> to_error = alphabet;
	    for (int j=0; j<nodes[i].children.size(); j++)
		to_error.erase(nodes[i].children[j].action);
	    for (set<int>::iterator it=to_error.begin();
					it!=to_error.end(); it++) {
		e.action = *it;
		nodes[i].children.push_back(e);
	    }
	}

    return *this;
}

int Lts::progress(const string& progress_name, const SetValue& s)
{
    terminalSets();

    for (int i=0; i<terminal_sets.size(); i++) {
	TerminalSet& ts = terminal_sets[i];
	bool violation = true;
	/* There is a progress violation for the set 's' if the terminal set
	   'ts' does not contain any action in 's'. */
	for (int j=0; j<s.actions.size(); j++)
	    if(ts.actions.count(atp->lookup(s.actions[j]))) {
		violation = false;
		break;
	    }

	if (violation) {
	    cout << "Progress violation detected for process " << name
		<< " and progress property " << progress_name << ":\n";
	    cout << "	Trace to violation: ";
	    for (int j=0; j<ts.trace.size(); j++)
		cout << ati(ts.trace[j]) << "-> ";
	    cout << "\n";
	    cout << "	Actions in terminal set: {";
	    for (set<int>::iterator it=ts.actions.begin();
		    it!=ts.actions.end(); it++)
		cout << ati(*it) << ", ";
	    cout << "}\n\n";
	    
	}
    }

    return 0;
}


struct OutputData {
    fstream * fsptr;
    ActionsTable * atp;
};

void graphvizVisitFunction(int state, const struct LtsNode& node,
				void * opaque)
{
    OutputData * gvdp = static_cast<OutputData *>(opaque);
    ActionsTable * atp = gvdp->atp;

    for (int i=0; i<node.children.size(); i++)
	(*(gvdp->fsptr)) << state << " -> " << node.children[i].dest
	    << " [label = \"" << ati(node.children[i].action) << "\"];\n";
}

void Lts::graphvizOutput(const char * filename) const
{
    LtsVisitObject lvo;
    OutputData gvd;
    fstream fout;

    fout.open(filename, fstream::out);
    fout << "digraph G {\n";
    fout << "rankdir = LR;\n";
    //fout << "ratio = 1.0;\n";
    for (int i=0; i<nodes.size(); i++) {
	switch (nodes[i].type) {
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
	}
    }

    lvo.vfp = &graphvizVisitFunction;
    gvd.fsptr = &fout;
    gvd.atp = atp;
    lvo.opaque = &gvd;
    visit(lvo);

    fout << "}\n";
    fout.close();
}

void outputVisitFunction(int state, const struct LtsNode& node,
				void * opaque)
{
    OutputData * gvdp = static_cast<OutputData *>(opaque);
    ActionsTable * atp = gvdp->atp;

    for (int i=0; i<node.children.size(); i++)
	(*(gvdp->fsptr)) << state << " " << ati(node.children[i].action)
	    << " " << node.children[i].dest << "\n";
}

void Lts::output(const char * filename) const
{
    LtsVisitObject lvo;
    OutputData gvd;
    fstream fout;
    int end = -1;
    int error = -1;

    fout.open(filename, fstream::app | fstream::out);
    fout << name << " " << nodes.size() << " " << ntr << " " 
		<< alphabet.size() << " ";
    for (int i=0; i<nodes.size(); i++) {
	switch (nodes[i].type) {
	    case LtsNode::End:
		end = i;
		break;
	    case LtsNode::Error:
		error = i;
		break;
	}
    }
    fout << end << " " << error << "\n";

    lvo.vfp = &outputVisitFunction;
    gvd.fsptr = &fout;
    gvd.atp = atp;
    lvo.opaque = &gvd;
    visit(lvo);

    for (set<int>::iterator it=alphabet.begin(); it!=alphabet.end(); it++)
	fout << ati(*it) << " ";
    fout << "\n";

    fout.close();
}


Lts * err_if_not_lts(SymbolValue * svp, const struct YYLTYPE& loc)
{
    if (svp->type() != SymbolValue::Lts) {
	stringstream errstream;
	errstream << "Lts expected";
	semantic_error(errstream, loc);
    }

    return static_cast<Lts *>(svp);
}

/* =========================  LtsComposition ============================= */
void LtsComposition::print() const
{
    cout << "LtsComposition {\n";
    for (int i=0; i<lts.size(); i++)
	if (lts[i])
	    lts[i]->print();
    cout << "}\n";
}

SymbolValue * LtsComposition::clone() const
{
    LtsComposition * lc = new LtsComposition;

    lc->lts.resize(lts.size());
    for (int i=0; i<lts.size(); i++)
	if (lts[i])
	    lc->lts[i] = static_cast<class Lts *>(lts[i]->clone());
	else
	    lc->lts[i] = NULL;

    return lc;
}

LtsComposition * err_if_not_ltscomposition(SymbolValue * svp,
						const struct YYLTYPE& loc)
{
    if (svp->type() != SymbolValue::LtsComposition) {
	stringstream errstream;
	errstream << "LtsComposition expected";
	semantic_error(errstream, loc);
    }

    return static_cast<LtsComposition *>(svp);
}
