/*
 *  fspc symbols table implementation
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
fsp::ActionsTable *fsp::ActionsTable::instance = NULL;

fsp::ActionsTable *fsp::ActionsTable::get()
{
    if (instance == NULL) {
        instance = new ActionsTable("Global actions table");
    }

    return instance;
}

fsp::ActionsTable& fsp::ActionsTable::getref()
{
    ActionsTable *a = ActionsTable::get();

    return *a;
}

int fsp::ActionsTable::insert(const string& s)
{
    pair< map<string, int>::iterator, bool > ins_ret;

    ins_ret = table.insert(pair<string, int>(s, serial));
    if (ins_ret.second) {
	reverse.push_back(ins_ret.first->first);
	return serial++;
    }
    return ins_ret.first->second;
}

int fsp::ActionsTable::lookup(const string& s) const
{
    map<string, int>::const_iterator it = table.find(s);

    if (it == table.end())
	return -1;

    return it->second;
}

string fsp::ActionsTable::lookup(unsigned int idx) const
{
    assert(idx < reverse.size());

    return reverse[idx];
}

void fsp::ActionsTable::print() const
{
    map<string, int>::const_iterator it;

    cout << "Action table '" << name << "'\n";
    for (it=table.begin(); it!=table.end(); it++) {
	cout << "(" << it->first << ", " << it->second << ")\n";
    }
}


/*===================== SymbolsTable implementation ====================== */
bool fsp::SymbolsTable::insert(const string& name, Symbol *ptr)
{
    pair< map<string, Symbol*>::iterator, bool > ret;

    ret = table.insert(pair<string, Symbol*>(name, ptr));

    return ret.second;
}

bool fsp::SymbolsTable::lookup(const string& name, Symbol*& value) const
{
    map<string, Symbol*>::const_iterator it = table.find(name);

    if (it == table.end())
	return false;

    value = it->second;
    return true;
}

bool fsp::SymbolsTable::remove(const string& name)
{
    map<string, Symbol*>::iterator it = table.find(name);

    if (it == table.end())
	return false;

    delete it->second;
    table.erase(it);

    return true;
}

void fsp::SymbolsTable::clear()
{
    map<string, Symbol*>::iterator it;

    for (it = table.begin(); it != table.end(); it++) {
	delete it->second;
    }
    table.clear();
}

fsp::SymbolsTable::~SymbolsTable()
{
    clear();
}

void fsp::SymbolsTable::copyfrom(const fsp::SymbolsTable& st)
{
    map<string, Symbol*>::const_iterator it;

    for (it = st.table.begin(); it != st.table.end(); it++) {
        Symbol *svp = NULL;

        if (it->second) {
            // TODO check why this happens
            svp = it->second->clone();
        }

        table.insert(pair<string, Symbol*>(it->first, svp));
    }
}

fsp::SymbolsTable::SymbolsTable(const fsp::SymbolsTable& st)
{
    copyfrom(st);
}

fsp::SymbolsTable& fsp::SymbolsTable::operator=(const fsp::SymbolsTable& st)
{
    clear();
    copyfrom(st);

    return *this;
}

void fsp::SymbolsTable::print() const
{
    map<string, Symbol*>::const_iterator it;

    cout << "Symbols Table\n";
    for (it=table.begin(); it!=table.end(); it++) {
	cout << "(" << it->first << ",";
	it->second->print();
	cout << ")\n";
    }
}

/* ============================= IntS =============================*/
fsp::Symbol *fsp::IntS::clone() const
{
    IntS *cv = new IntS(val);

    return cv;
}

void fsp::IntS::set(SetS& s) const
{
    s.clear();
    s += int2string(val);
}

/* ============================= RangeS =============================*/
fsp::Symbol *fsp::RangeS::clone() const
{
    RangeS *rv = new RangeS;
    rv->low = low;
    rv->high = high;

    return rv;
}

void fsp::RangeS::set(fsp::SetS& s) const
{
    s.clear();

    for (int i=low; i<=high; i++) {
        s += int2string(i);
    }
}

/* ============================= SetS ================================ */
fsp::SetS& fsp::SetS::combine(const fsp::SetS& ss, bool dot)
{
    int n = actions.size();
    int nss = ss.size();
    string pre = dot ? "." : "[";
    string post = dot ? "" : "]";

    actions.resize(n *nss);

    for (int j=1; j<nss; j++)
	for (int i=0; i<n; i++)
	    actions[j*n+i] = actions[i] + pre + ss[j] + post;
    
    for (int i=0; i<n; i++)
	actions[i] += pre + ss[0] + post;

    return *this;
}

fsp::SetS& fsp::SetS::combine(const string& s, bool dot)
{
    string pre = dot ? "." : "[";
    string post = dot ? "" : "]";

    for (unsigned int i=0; i<actions.size(); i++)
	actions[i] += pre + s + post;

    return *this;
}

fsp::SetS& fsp::SetS::dotcat(const string& s)
{
    return combine(s, true);
}

fsp::SetS& fsp::SetS::dotcat(const SetS& ss)
{
    return combine(ss, true);
}

fsp::SetS& fsp::SetS::indexize(int index)
{
    stringstream sstr;
    sstr << index;
    string suffix = "[" + sstr.str() + "]";

    for (unsigned int i=0; i<actions.size(); i++)
	actions[i] = actions[i] + suffix;

    return *this;
}

fsp::SetS& fsp::SetS::indexize(const SetS& ss)
{
    return combine(ss, true);
}

fsp::SetS& fsp::SetS::indexize(const string& s)
{
    return dotcat(s);
}

fsp::SetS& fsp::SetS::indexize(int low, int high)
{
    int n = actions.size();
    int nr = high - low + 1;

    actions.resize(n *nr);

    for (int j=1; j<nr; j++)
	for (int i=0; i<n; i++)
	    actions[j*n+i] = actions[i] + "[" + int2string(low + j)  + "]";
    
    for (int i=0; i<n; i++)
	actions[i] += "[" + int2string(low) + "]";

    return *this;
    
}

fsp::SetS& fsp::SetS::operator +=(const SetS& ss)
{
    for (unsigned int i=0; i<ss.actions.size(); i++)
	actions.push_back(ss[i]);

    return *this;
}

fsp::SetS& fsp::SetS::operator +=(const string& s)
{
    actions.push_back(s);

    return *this;
}

void fsp::SetS::clear()
{
    actions.clear();
    variable = string();
}

void fsp::SetS::toActionSetValue(ActionSetS& asv)
{
    ActionsTable& at = ActionsTable::getref();

    asv.clear();
    for (unsigned int i = 0; i < actions.size(); i++) {
        asv.add(at.insert(actions[i]));
    }
}

fsp::Symbol *fsp::SetS::clone() const
{
    SetS *sv = new SetS(*this);

    return sv;
}

void fsp::SetS::output(const string& name, const char *filename) const
{
    fstream fout(filename, fstream::out | fstream::app);

    fout << name << " ";
    for (unsigned int i=0; i<actions.size(); i++) {
	fout << actions[i] << " ";
    }
    fout << "\n";
}

void fsp::SetS::output(stringstream& ss) const
{
    unsigned int i = 0;

    ss << "{";
    for (; i<actions.size() - 1; i++) {
	ss << actions[i] << ", ";
    }
    if (i<actions.size()) {
        ss << actions[i];
    }
    ss << "}";
}

void fsp::SetS::print() const
{
    cout << " {";
    for (unsigned int i=0; i<actions.size(); i++) {
	cout << actions[i] << ", ";
    }
    cout << "}\n";
}


/* =========================== RelabelingS =========================*/
void fsp::RelabelingS::print() const
{
    for (unsigned int i=0; i<old_labels.size(); i++) {
	new_labels[i].print();
	cout << " / ";
	old_labels[i].print();
	cout << "\n";
    }
}

fsp::Symbol *fsp::RelabelingS::clone() const
{
    RelabelingS *rlv = new RelabelingS;

    for (unsigned int i=0; i<old_labels.size(); i++) {
	rlv->old_labels.push_back(old_labels[i]);
	rlv->new_labels.push_back(new_labels[i]);
    }

    return rlv;
}

void fsp::RelabelingS::add(const fsp::SetS& new_setvp,
                             const fsp::SetS& old_setvp)
{
    old_labels.push_back(old_setvp);
    new_labels.push_back(new_setvp);
}

void fsp::RelabelingS::merge(fsp::RelabelingS& rlv)
{
    for (unsigned int i=0; i<rlv.old_labels.size(); i++) {
        add(rlv.new_labels[i], rlv.old_labels[i]);
    }
}


/* ============================ HidingS ===========================*/
void fsp::HidingS::print() const
{
    cout << "Hiding: ";
    if (interface)
	cout << "@ ";
    else
	cout << "\\ ";
    setv.print();
}

fsp::Symbol *fsp::HidingS::clone() const
{
    HidingS *hv = new HidingS;

    hv->interface = interface;
    hv->setv = setv;

    return hv;
}


/* =========================== PriorityS ========================= */
void fsp::PriorityS::print() const
{
    cout << "PriorityS: ";
    if (low)
	cout << ">> ";
    else
	cout << "<< ";
    setv.print();
}

fsp::Symbol *fsp::PriorityS::clone() const
{
    PriorityS *pv = new PriorityS;

    pv->low = low;
    pv->setv = setv;

    return pv;
}

/* =========================== ActionSetS ========================= */
bool fsp::ActionSetS::add(unsigned int a)
{
    return actions.insert(a).second;
}

bool fsp::ActionSetS::lookup(unsigned int a) const
{
    return actions.count(a) == 1;
}

void fsp::ActionSetS::clear()
{
    actions.clear();
}

void fsp::ActionSetS::toSetValue(fsp::SetS& setv)
{
    const ActionsTable& at = ActionsTable::getref();

    setv.clear();

    for (set<unsigned int>::iterator it = actions.begin();
                    it != actions.end(); it++) {
        setv += at.lookup(*it);
    }
}

void fsp::ActionSetS::print() const
{
    cout << "ActionSet: ";
    cout << "{";
    for (set<unsigned int>::const_iterator it = actions.begin();
                    it != actions.end(); it++) {
        set<unsigned int>::const_iterator jt = it;

        cout << *it;
        if ((++jt) != actions.end()) {
            cout << ",";
        }
    }
    cout << "}";
}

fsp::Symbol *fsp::ActionSetS::clone() const
{
    ActionSetS *av = new ActionSetS;

    av->actions = actions;

    return av;
}


/* ======================== ProgressS ========================= */
void fsp::ProgressS::print() const
{
    cout << "Progress: ";
    if (conditional) {
        cout << "if ";
        condition.print();
        cout << " then ";
    }
    set.print();
}

fsp::Symbol *fsp::ProgressS::clone() const
{
    ProgressS *pv = new ProgressS;

    pv->condition = condition;
    pv->set = set;
    pv->conditional = conditional;

    return pv;
}


/* ======================== ParametricProcess ========================= */
bool fsp::ParametricProcess::insert(const string& name, int default_value)
{
    for (unsigned int i=0; i<names.size(); i++) {
        if (names[i] == name) {
            return false;
        }
    }

    names.push_back(name);
    defaults.push_back(default_value);

    return true;
}

void fsp::ParametricProcess::clear()
{
    names.clear();
    defaults.clear();
}

void fsp::ParametricProcess::set_translator(ParametricTranslator *trans)
{
    assert(trans);
    translator = trans;
}

void fsp::ParametricProcess::print() const
{
    cout << "ParametricProcess: " << this;
    for (unsigned int i=0; i<names.size(); i++) {
        cout << "   " << names[i] << " [" << defaults[i] << "], ";
    }
}

fsp::Symbol *fsp::ParametricProcess::clone() const
{
    ParametricProcess *pp = new ParametricProcess;

    pp->names = names;
    pp->defaults = defaults;
    pp->translator = translator;

    return pp;
}

