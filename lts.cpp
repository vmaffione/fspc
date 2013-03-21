#include "lts.hpp"
#include "strings_table.hpp"


//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


/* Global actions table that holds all the action names */
static struct StringsTable actionsTable;
#define ati(index) actionsTable.table[(index)]


/******************* class Lts implementation ********************/
int Lts::lookupAlphabet(int action) const
{
    for (int i=0; i<alphabet.size(); i++)
	if (alphabet[i] == action)
	    return i;
    return -1;
}

void Lts::updateAlphabet(int action)
{
    int i = lookupAlphabet(action);
    
    if (i == -1)
	alphabet.push_back(action);
}

Lts::Lts(string nm, const char * filename) : name(nm)
{
    fstream fin;
    int n;
    int s1, s2;
    string a;
    Edge e;

    valid = true;
    /* Discover the number of nodes */
    n = -1;
    fin.open(filename, ios_base::in);
    if (fin.fail()) {
	valid = false;
	return;
    }

    for (;;) {
	fin >> s1 >> a >> s2;
	if (fin.fail())
	    break;

	if (s1 < 0 || s2 < 0) {
	    cout << "Error: negative state number\n";
	    valid = false;
	    return;
	}
	if (s1 > n)
	    n = s1;
	if (s2 > n)
	    n = s2;
	IFD(cout << "Read (" << s1 << ", " << a << ", " << s2 << ")\n");
    }
    n++;
    fin.close();

    ntr = 0;
    nodes.resize(n);
    for (int i=0; i<n; i++)
	nodes[i].end = 0;

    /* Build the graph */
    fin.open(filename, ios_base::in);
    for (;;) {
	fin >> s1 >> a >> s2;
	if (fin.fail())
	    break;
	e.dest = s2;
	e.action = actionsTable.insert(a.c_str());  // XXX: not optimized we could avoid using string
	IFD(cout << "Adding (" << s1 << ", " << ati(e.action) 
		<< ", " << e.dest << ")\n";)
	nodes[s1].children.push_back(e);  /* Insert a new edge */
	ntr++;
	updateAlphabet(e.action);
    }
    fin.close();
}

void Lts::print() const {
    actionsTable.print();
    cout << "LTS " << name << "\n";
    for (int i=0; i<nodes.size(); i++) {
	cout << "State: " << i << "\n";
	for (int j=0; j<nodes[i].children.size(); j++)
	    cout << "    " << ati(nodes[i].children[j].action)
		    << " --> " << nodes[i].children[j].dest << "\n";
    }
    cout << "Alphabet: {";
    for (int i=0; i<alphabet.size(); i++)
	cout << ati(alphabet[i]) << ", ";
    cout << "}\n";
    cout << numStates() << " states, " << ntr << " transitions\n";
}

/* BFS on the LTS for useless states removal. */
void Lts::compositionReduce(const vector<LtsNode>& product)
{
    int np = product.size();
    vector<int> frontier(np);
    vector<int> map(np);
    int pop, push;
    int state;
    int n = 0;

    /* Overestimation */
    nodes.resize(np);

    /* At the beginning the frontier contains only the initial state */
    frontier[0] = 0;
    pop = 0; push = 1;
    map[0] = n++;
    for (int i=1; i<np; i++)
	map[i] = -1;
    ntr = 0;

    while (pop != push) {
	Edge e;

	state = frontier[pop++];
	for (int j=0; j<product[state].children.size(); j++) {
	    int child = product[state].children[j].dest;

	    if (map[child] == -1) {
		map[child] = n++;
		frontier[push++] = child;
	    }
	    e.dest = map[child];
	    e.action = product[state].children[j].action;
	    nodes[map[state]].children.push_back(e);
	    ntr++;
	}
    }

    for (int i=0; i<np; i++)
	if (map[i] != -1)
	    nodes[map[i]].end = product[i].end;

    nodes.resize(n);
}


void Lts::compose(const Lts& p, const Lts& q)
{    
    int n;
    int nq;
    int np;
    Edge e;
    vector<LtsNode> product;

    valid = true;
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
	    updateAlphabet(ep.action);  // TODO controlla che va bene in entrambi i casi!
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
		    updateAlphabet(eq.action);
		}
	    } /* else case has already been covered by the previous scan. */
	}

    /* A composed state is and END state when both the components are
       END states. */
    for (int ip=0; ip<p.nodes.size(); ip++)
	for (int iq=0; iq<q.nodes.size(); iq++)
	    product[ip*nq+iq].end = p.nodes[ip].end && q.nodes[iq].end;

    compositionReduce(product);
}

Lts::Lts(string nm, const Lts& p, const Lts& q) : name(nm)
{
    compose(p, q);
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
	if (i == 0 && !nodes[state].end) { 
	    int t;
	    int j;

	    cout << "Deadlock state: " << state << "\n";
	    /* Starting from 'state', we follow the backpointers to build the
	       trace to deadlock (in reverse order). */
	    t = pop;
	    j = 0;
	    while (t) {
		action_trace[j++] = actions[t];
		t = back[t];
	    }
	    for (j--; j>=0; j--)
		cout << "    " << ati(action_trace[j]) << "\n";
	    nd++;
	}
	pop++; /* Pops 'state' from the queue */
    }

    return nd;
}

int Lts::terminalSets() const
{
    int n = nodes.size();
    int na = actionsTable.table.size();
    int nts = 0;

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


		if (nc == 1 || nc == n) {
		    /* We are not interested in trivial terminal sets. A
		       terminal set is trivial when it is a deadlock state
		       (just one element) or when it is the whole LTS. */
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
		    /* If the component is terminal, we output the trace of
		       action to get to the component. */
		    cout << "Terminal set of states: ";
		    for (j=0; j<nc; j++)
			cout << tarjan_component_states[j] << " ";
		    cout << "\n";

		    /* We examine the action_stack (without popping anything) 
		       and build the action_trace in reverse order. */
		    j = 0;
		    t = top;
		    while (t) {
			action_trace[j++] = action_stack[t];
			t = back[t];
		    }
		    cout << "Trace to the terminal set:\n";
		    for (j--; j>=0; j--)
			cout << "    " << ati(action_trace[j]) << "\n";
		    cout << "Actions in the terminal set: {";
		    for (j=0; j<nca; j++)
			cout << ati(tarjan_component_actions[j]) << ", ";
		    cout << "}\n";
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