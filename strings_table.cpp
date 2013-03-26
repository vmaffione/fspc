#include <cstring>
#include <set>
#include "strings_table.hpp"



//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


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


/*============================= ConstValue =============================*/
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

/*============================= SetValue =============================*/
SymbolValue * SetValue::clone() const
{
    SetValue * sv = new SetValue(*this);

    return sv;
}

SetValue::SetValue(const SetValue& sv)
{
    ssp = new StringsSet(*(sv.ssp));
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

    return pvp;
}

