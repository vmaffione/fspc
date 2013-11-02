/*
 *  fspc symbols table implementation
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


#include <set>
#include <sstream>
#include <fstream>
#include <assert.h>

/* Symbols tables and symbol types. */
#include "symbols_table.hpp"

#include "helpers.hpp"



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
    map<string, SymbolValue*>::iterator it = table.find(name);

    if (it == table.end())
	return false;

    delete it->second;
    table.erase(it);

    return true;
}

void SymbolsTable::clear()
{
    map<string, SymbolValue*>::iterator it;

    for (it = table.begin(); it != table.end(); it++) {
	delete it->second;
    }
    table.clear();
}

SymbolsTable::~SymbolsTable()
{
    clear();
}

void SymbolsTable::copyfrom(const SymbolsTable& st)
{
    map<string, SymbolValue*>::const_iterator it;

    for (it = st.table.begin(); it != st.table.end(); it++) {
        SymbolValue *svp = NULL;

        if (it->second) {
            // TODO check why this happens
            svp = it->second->clone();
        }

        table.insert(pair<string, SymbolValue*>(it->first, svp));
    }
}

SymbolsTable::SymbolsTable(const SymbolsTable& st)
{
    copyfrom(st);
}

SymbolsTable& SymbolsTable::operator=(const SymbolsTable& st)
{
    clear();
    copyfrom(st);

    return *this;
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

SymbolValue * SvpVec::detach(unsigned int i) {
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

void SvpVec::detach_all() {
    for (unsigned int i=0; i<v.size(); i++)
	v[i] = NULL;
}

void SvpVec::print() {
    cout << "{";
    for (unsigned int i=0; i<v.size(); i++)
	if (v[i]) {
	    v[i]->print();
	    cout << ", ";
	}
    cout << "}\n";
}

SvpVec::~SvpVec() {
    if (shared) {
	for (unsigned int i=0; i<v.size(); i++) {
	    if (v[i]) {
		delete v[i];
		break;
	    }
	}
    } else {
	for (unsigned int i=0; i<v.size(); i++)
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

void ConstValue::set(SetValue& s) const
{
    s.actions.clear();
    s += int2string(value);
}

/*============================= RangeValue =============================*/
SymbolValue * RangeValue::clone() const
{
    RangeValue * rv = new RangeValue;
    rv->low = low;
    rv->high = high;

    return rv;
}

void RangeValue::set(SetValue& s) const
{
    s.actions.clear();

    for (int i=low; i<=high; i++) {
        s += int2string(i);
    }
}

/*============================= SetValue ================================ */
SetValue& SetValue::dotcat(const string& s)
{
    for (unsigned int i=0; i<actions.size(); i++)
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

    for (unsigned int i=0; i<actions.size(); i++)
	actions[i] = actions[i] + suffix;

    return *this;
}

SetValue& SetValue::indexize(const SetValue& ss)
{
    return dotcat(ss);
}

SetValue& SetValue::indexize(const string& s)
{
    return dotcat(s);
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
    for (unsigned int i=0; i<ss.actions.size(); i++)
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

void SetValue::output(const string& name, const char * filename) const
{
    fstream fout(filename, fstream::out | fstream::app);

    fout << name << " ";
    for (unsigned int i=0; i<actions.size(); i++) {
	fout << actions[i] << " ";
    }
    fout << "\n";
}

void SetValue::print() const
{
    cout << "r=" << rank << " {";
    for (unsigned int i=0; i<actions.size(); i++) {
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
    for (unsigned int i=0; i<args.size(); i++)
	cout << args[i] << ", ";
    cout << ")\n";
}

SymbolValue * ArgumentsValue::clone() const
{
    ArgumentsValue * av = new ArgumentsValue(*this);

    return av;
}


/* =========================== NewRelabelingValue =========================*/
void NewRelabelingValue::print() const
{
    for (unsigned int i=0; i<old_labels.size(); i++) {
	new_labels[i].print();
	cout << " / ";
	old_labels[i].print();
	cout << "\n";
    }
}

SymbolValue * NewRelabelingValue::clone() const
{
    NewRelabelingValue * rlv = new NewRelabelingValue;

    for (unsigned int i=0; i<old_labels.size(); i++) {
	rlv->old_labels.push_back(old_labels[i]);
	rlv->new_labels.push_back(new_labels[i]);
    }

    return rlv;
}

void NewRelabelingValue::add(const SetValue& new_setvp,
                             const SetValue& old_setvp)
{
    old_labels.push_back(old_setvp);
    new_labels.push_back(new_setvp);
}

void NewRelabelingValue::merge(NewRelabelingValue& rlv)
{
    for (unsigned int i=0; i<rlv.old_labels.size(); i++) {
        add(rlv.new_labels[i], rlv.old_labels[i]);
    }
}


/* ============================ RelabelingValue ===========================*/
void RelabelingValue::print() const
{
    for (unsigned int i=0; i<old_labels.size(); i++) {
	new_labels[i]->print();
	cout << " / ";
	old_labels[i]->print();
	cout << "\n";
    }
}

SymbolValue * RelabelingValue::clone() const
{
    RelabelingValue * rlv = new RelabelingValue;

    for (unsigned int i=0; i<old_labels.size(); i++) {
	rlv->old_labels.push_back(
			    static_cast<SetValue *>(old_labels[i]->clone()));
	rlv->new_labels.push_back(
			    static_cast<SetValue *>(new_labels[i]->clone()));
    }

    return rlv;
}

void RelabelingValue::add(SetValue * new_setvp, SetValue * old_setvp)
{
    assert(new_setvp && old_setvp);
    old_labels.push_back(old_setvp);
    new_labels.push_back(new_setvp);
}

void RelabelingValue::merge(RelabelingValue& rlv)
{
    for (unsigned int i=0; i<rlv.old_labels.size(); i++)
	if (rlv.old_labels[i] && rlv.new_labels[i]) {
	    add(rlv.new_labels[i], rlv.old_labels[i]);
	}
    rlv.detach_all();
}

void RelabelingValue::detach_all()
{
    for (unsigned int i=0; i<old_labels.size(); i++)
	old_labels[i] = new_labels[i] = NULL;
}

RelabelingValue::~RelabelingValue()
{
    for (unsigned int i=0; i<old_labels.size(); i++) {
	if (old_labels[i])
	    delete old_labels[i];
	if (new_labels[i])
	    delete new_labels[i];
    }
}


/* ============================ NewHidingValue ===========================*/
void NewHidingValue::print() const
{
    cout << "Hiding: ";
    if (interface)
	cout << "@ ";
    else
	cout << "\\ ";
    setv.print();
}

SymbolValue * NewHidingValue::clone() const
{
    NewHidingValue * hv = new NewHidingValue;

    hv->interface = interface;
    hv->setv = setv;

    return hv;
}

/* ============================= HidingValue =============================*/
void HidingValue::print() const
{
    cout << "Hiding: ";
    if (interface)
	cout << "@ ";
    else
	cout << "\\ ";
    if (setvp)
	setvp->print();
}

SymbolValue * HidingValue::clone() const
{
    HidingValue * hv = new HidingValue;

    hv->interface = interface;
    if (setvp)
	hv->setvp = static_cast<SetValue *>(setvp->clone());

    return hv;
}

HidingValue::~HidingValue()
{
    if (setvp)
	delete setvp;
}


/* ============================ PriorityValue =========================== */
void PriorityValue::print() const
{
    cout << "Priority: ";
    if (low)
	cout << ">> ";
    else
	cout << "<< ";
    if (setvp)
	setvp->print();
}

SymbolValue * PriorityValue::clone() const
{
    PriorityValue * pv = new PriorityValue;

    pv->low = low;
    if (setvp)
	pv->setvp = static_cast<SetValue *>(setvp->clone());

    return pv;
}

PriorityValue::~PriorityValue()
{
    if (setvp)
	delete setvp;
}


/* ======================== NewParametricProcess ========================= */
bool NewParametricProcess::insert(const string& name, int default_value)
{
    names.push_back(name);
    defaults.push_back(default_value);

    return true;
}

void NewParametricProcess::clear()
{
    names.clear();
    defaults.clear();
}

void NewParametricProcess::set_translator(ParametricTranslator *trans)
{
    assert(trans);
    translator = trans;
}

void NewParametricProcess::print() const
{
    cout << "ParametricProcess: ";
    for (unsigned int i=0; i<names.size(); i++) {
        cout << "   " << names[i] << " [" << defaults[i] << "]\n";
    }
}

SymbolValue * NewParametricProcess::clone() const
{
    NewParametricProcess * pp = new NewParametricProcess;

    pp->names = names;
    pp->defaults = defaults;
    pp->translator = translator;

    return pp;
}


/* ============================= Pvec =================================== */
void Pvec::print(struct ActionsTable * atp)
{
    cout << "Pvec:\n";
    for (unsigned int i=0; i<v.size(); i++)
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
	default:
	    return "";
    }
}

void printVisitFunction(ProcessNode * pnp, void * opaque)
{
    ActionsTable * atp = (ActionsTable *)opaque;

    cout << pnp << "(" << processNodeTypeString(pnp->type) << "):\n";
    for (unsigned int i=0; i<pnp->children.size(); i++) {
	ProcessEdge e = pnp->children[i];
	cout << atp->reverse[e.action] << " -> " << e.dest << "\n";
    }
}

void ProcessNode::print(ActionsTable * atp)
{
    struct ProcessVisitObject f;

    f.vfp = &printVisitFunction;
    f.opaque = atp;
    visit(f, true);
}

void ProcessNode::visit(ProcessVisitObject f, bool before)
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
	if (before)
	    /* Invoke the specific visit function before. */
	    f.vfp(current, f.opaque);  
	for (unsigned int i=0; i<current->children.size(); i++) {
	    ProcessEdge e = current->children[i];
	    if (e.dest && visited.count(e.dest) == 0) {
		visited.insert(e.dest);
		frontier.push_back(e.dest);
		push++;
	    }
	}
	if (!before)
	    /* Invoke the specific visit function after. */
	    f.vfp(current, f.opaque);  
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
    pnp->visit(f, true);

    /* Free them. */
    for (unsigned int i=0; i<nodes.size(); i++)
	delete nodes[i];
}


SymbolValue * ProcessValue::clone() const
{
    ProcessValue * pvp = new ProcessValue;

    pvp->pnp = this->pnp->clone();

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
    for (unsigned int i=0; i<nodes.size(); i++)
	delete nodes[i];
    nodes.clear();
}

