#include <cstring>
#include "strings_table.hpp"



//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif

/*======================== StringsTable implementation ====================*/
int StringsTable::lookup(const char * s) const
{
    for (int i=0; i<table.size(); i++)
	if (strcmp(s, table[i]) == 0)
	    return i;
    return -1;
}

int StringsTable::insert(const char * s)
{
    int i = lookup(s);
    char * sc = strdup(s);

    if (i == -1) {
	table.push_back(sc); // This makes a copy
	IFD(cout << "Added '" << sc << "' to the actions table ("
		<< table.size()-1 << ")\n");
	return table.size() - 1;
    }
    return i;
}

void StringsTable::print() const
{
    cout << "Actions table:\n";
    for (int i=0; i<table.size(); i++)
	cout << "  " << table[i] << "\n";
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
void ProcessNode::print() const
{
    cout << this << ":\n";
    for (int i=0; i<children.size(); i++)
	cout << children[i].action << " -> " << children[i].dest << "\n";
    for (int i=0; i<children.size(); i++)
	if (children[i].dest)
	    children[i].dest->print();
}

ProcessNode * ProcessNode::clone() const
{
    int nc;
    int pop, push;
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
		frontier[push] = current->children[i].dest;
		cloned->children[i].dest = cloned_frontier[push] =
		    new ProcessNode;
		push++;
	    } else
		cloned->children[i].dest = NULL;
	}
    }

    return cloned_frontier[0];
}

SymbolValue * ProcessValue::clone() const
{
    ProcessValue * pvp = new ProcessValue;

    pvp->pnp = this->pnp->clone();

    return pvp;
}

