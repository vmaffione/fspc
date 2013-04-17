#include <cstring>
#include <set>
#include <sstream>
#include <assert.h>
#include "symbols_table.hpp"



//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


string int2string(int x)
{
    stringstream sstr;
    sstr << x;
    return sstr.str();
}

/* ========================= ActionsTable ================================ */
int ActionsTable::insert(const string& s)
{
    pair< map<string, int>::iterator, bool > ins_ret;

    ins_ret = table.insert(pair<string, int>(s, serial));
    if (ins_ret.second) {
	reverse.push_back(ins_ret.first->first);
	return serial++;
    }
    return ins_ret.first->second;
}

int ActionsTable::lookup(const string& s) const
{
    map<string, int>::const_iterator it = table.find(s);

    if (it == table.end())
	return -1;

    return it->second;
}

void ActionsTable::print() const
{
    map<string, int>::const_iterator it;

    cout << "Action table '" << name << "'\n";
    for (it=table.begin(); it!=table.end(); it++) {
	cout << "(" << it->first << ", " << it->second << ")\n";
    }
}


/*===================== SymbolsTable implementation ====================== */
bool SymbolsTable::insert(const string& name, SymbolValue * ptr)
{
    pair< map<string, SymbolValue*>::iterator, bool > ret;

    ret = table.insert(pair<string, SymbolValue*>(name, ptr));

    return ret.second;
}

bool SymbolsTable::lookup(const string& name, SymbolValue*& value) const
{
    map<string, SymbolValue*>::const_iterator it = table.find(name);

    if (it == table.end())
	return false;

    value = it->second;
    return true;
}

bool SymbolsTable::remove(const string& name)
{
    return table.erase(name);
}

void SymbolsTable::clear()
{
    map<string, SymbolValue*>::iterator it;

    for (it=table.begin(); it!=table.end(); it++) {
	delete it->second;
    }
    table.clear();
}

void SymbolsTable::print() const
{
    map<string, SymbolValue*>::const_iterator it;

    cout << "Symbols Table\n";
    for (it=table.begin(); it!=table.end(); it++) {
	cout << "(" << it->first << ",";
	it->second->print();
	cout << ")\n";
    }
}

/* ================================ SvpVec ==============================*/
SvpVec::SvpVec(): shared(false) {
}

SymbolValue * SvpVec::detach(int i) {
    SymbolValue* ret;
    if (i < v.size()) {
	ret = v[i];
	v[i] = NULL;
    } else {
	cerr << "Internal error: tried to detach out of bounds\n";
	return NULL;
    }
    return ret;
}

void SvpVec::print() {
    cout << "{";
    for (int i=0; i<v.size(); i++)
	if (v[i]) {
	    v[i]->print();
	    cout << ", ";
	}
    cout << "}\n";
}

SvpVec::~SvpVec() {
    if (shared) {
	for (int i=0; i<v.size(); i++) {
	    if (v[i]) {
		delete v[i];
		break;
	    }
	}
    } else {
	for (int i=0; i<v.size(); i++)
	    if (v[i])
		delete v[i];
    }
}

/* ============================= ConstValue =============================*/
SymbolValue * ConstValue::clone() const
{
    ConstValue * cv = new ConstValue;
    cv->value = value;

    return cv;
}

/*============================= RangeValue =============================*/
SymbolValue * RangeValue::clone() const
{
    RangeValue * rv = new RangeValue;
    rv->low = low;
    rv->high = high;

    return rv;
}

/*============================= SetValue ================================ */
SetValue& SetValue::dotcat(const string& s)
{
    for (int i=0; i<actions.size(); i++)
	actions[i] += "." + s;

    return *this;
}

SetValue& SetValue::dotcat(const SetValue& ss)
{
    int n = actions.size();
    int nss = ss.actions.size();

    actions.resize(n * nss);

    for (int j=1; j<nss; j++)
	for (int i=0; i<n; i++)
	    actions[j*n+i] = actions[i] + "." + ss.actions[j];
    
    for (int i=0; i<n; i++)
	actions[i] += "." + ss.actions[0];

    return *this;
}

SetValue& SetValue::indexize(int index)
{
    stringstream sstr;
    sstr << index;
    string suffix = "[" + sstr.str() + "]";

    for (int i=0; i<actions.size(); i++)
	actions[i] += suffix;

    return *this;
}

SetValue& SetValue::indexize(const SetValue& ss)
{
    return dotcat(ss);
}

SetValue& SetValue::indexize(int low, int high)
{
    int n = actions.size();
    int nr = high - low + 1;

    actions.resize(n * nr);

    for (int j=1; j<nr; j++)
	for (int i=0; i<n; i++)
	    actions[j*n+i] = actions[i] + "[" + int2string(low + j)  + "]";
    
    for (int i=0; i<n; i++)
	actions[i] += "[" + int2string(low) + "]";

    return *this;
    
}

SetValue& SetValue::operator +=(const SetValue& ss)
{
    for (int i=0; i<ss.actions.size(); i++)
	actions.push_back(ss.actions[i]);

    return *this;
}

SetValue& SetValue::operator +=(const string& s)
{
    actions.push_back(s);

    return *this;
}

SymbolValue * SetValue::clone() const
{
    SetValue * sv = new SetValue(*this);

    return sv;
}

void SetValue::print() const
{
    cout << "r=" << rank << " {";
    for (int i=0; i<actions.size(); i++) {
	cout << actions[i] << ", ";
    }
    cout << "}\n";
}

/* ========================= ProcnodePairValue =========================*/
SymbolValue * ProcnodePairValue::clone() const
{
    return new ProcnodePairValue(*this);
}


/* =========================== ArgumentsValue =============================*/
void ArgumentsValue::print() const
{
    cout << "(";
    for (int i=0; i<args.size(); i++)
	cout << args[i] << ", ";
    cout << ")\n";
}

SymbolValue * ArgumentsValue::clone() const
{
    ArgumentsValue * av = new ArgumentsValue(*this);

    return av;
}


/* ============================= Pvec ============================ */
void Pvec::print(struct ActionsTable * atp)
{
    cout << "Pvec:\n";
    for (int i=0; i<v.size(); i++)
	v[i]->print(atp);
}


/*======================= ProcessNode & Process Value =====================*/

string processNodeTypeString(int type)
{
    switch (type) {
	case ProcessNode::Normal:
	    return "normal";
	case ProcessNode::End:
	    return "END";
	case ProcessNode::Error:
	    return "ERROR";
    }
}

void printVisitFunction(ProcessNode * pnp, void * opaque)
{
    ActionsTable * atp = (ActionsTable *)opaque;

    cout << pnp << "(" << processNodeTypeString(pnp->type) << "):\n";
    for (int i=0; i<pnp->children.size(); i++) {
	ProcessEdge e = pnp->children[i];
	cout << atp->reverse[e.action] << " -> " << e.dest << "\n";
    }
}

void ProcessNode::print(ActionsTable * atp)
{
    struct ProcessVisitObject f;

    f.vfp = &printVisitFunction;
    f.opaque = atp;
    visit(f);
}

void ProcessNode::visit(ProcessVisitObject f)
{
    set<ProcessNode*> visited;
    vector<ProcessNode*> frontier(1);
    int pop, push;
    ProcessNode * current;

    pop = 0;
    push = 1;
    frontier[0] = this;
    visited.insert(this);

    while (pop != push) {
	current = frontier[pop++];
	f.vfp(current, f.opaque);  /* Invoke the specific visit function. */
	for (int i=0; i<current->children.size(); i++) {
	    ProcessEdge e = current->children[i];
	    if (e.dest && visited.count(e.dest) == 0) {
		visited.insert(e.dest);
		frontier.push_back(e.dest);
		push++;
	    }
	}
    }
}

ProcessNode * ProcessNode::clone() const
{
    int nc;
    int pop, push;
    //XXX visited: Now it does not handle loops
    vector<const ProcessNode *> frontier(1);
    vector<ProcessNode*> cloned_frontier(1);
    const ProcessNode * current;
    ProcessNode * cloned;

    pop = 0;
    push = 1;
    frontier[0] = this;
    cloned_frontier[0] = new ProcessNode;

    while (pop != push) {
	current = frontier[pop];
	cloned = cloned_frontier[pop];
	pop++;

	cloned->type = current->type;
	nc = current->children.size();
	cloned->children.resize(nc);
	for (int i=0; i<nc; i++) {
	    cloned->children[i].action = current->children[i].action;
	    cloned->children[i].unresolved_reference =
		current->children[i].unresolved_reference;
	    if (current->children[i].dest) {
		frontier.push_back(current->children[i].dest);
		cloned->children[i].dest = new ProcessNode;
		cloned_frontier.push_back(cloned->children[i].dest);
		push++;
	    } else
		cloned->children[i].dest = NULL;
	}
    }

    return cloned_frontier[0];
}


void freeVisitFunction(struct ProcessNode * pnp, void * opaque)
{
    vector<ProcessNode *> * nodes_ptr = (vector<ProcessNode *> *)opaque;
    nodes_ptr->push_back(pnp);
}

void freeProcessNodeGraph(struct ProcessNode * pnp)
{
    struct ProcessVisitObject f;
    vector<ProcessNode *> nodes;

    /* Collect all the nodes reachable from 'pnp'. */
    f.vfp = &freeVisitFunction;
    f.opaque = &nodes;
    pnp->visit(f);

    /* Free them. */
    for (int i=0; i<nodes.size(); i++)
	delete nodes[i];
}


SymbolValue * ProcessValue::clone() const
{
    ProcessValue * pvp = new ProcessValue;

    pvp->pnp = this->pnp->clone();
    pvp->setvp = static_cast<SetValue *>(this->setvp->clone());

    return pvp;
}

/* =========================== ProcessNodeAllocator ===================== */
ProcessNode * ProcessNodeAllocator::allocate(int type)
{
    ProcessNode * pnp = new ProcessNode(type);

    assert(type == ProcessNode::Normal || type == ProcessNode::End ||
						type == ProcessNode::Error);
    nodes.push_back(pnp);

    return pnp;
}

ConnectedProcess * ProcessNodeAllocator::allocate_connected()
{
    ConnectedProcess * cpp = new ConnectedProcess;

    nodes.push_back(cpp);

    return cpp;
}

UnresolvedProcess * ProcessNodeAllocator::allocate_unresolved(const string&
								    name)
{
    UnresolvedProcess * upp = new UnresolvedProcess(name);

    nodes.push_back(upp);

    return upp;
}

void ProcessNodeAllocator::clear()
{
    for (int i=0; i<nodes.size(); i++)
	delete nodes[i];
    nodes.clear();
}

