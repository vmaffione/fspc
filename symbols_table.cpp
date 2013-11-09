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

/* ============================= RangeValue =============================*/
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

/* ============================= SetValue ================================ */
SetValue& SetValue::combine(const SetValue& ss, bool dot)
{
    int n = actions.size();
    int nss = ss.actions.size();
    string pre = dot ? "." : "[";
    string post = dot ? "" : "]";

    actions.resize(n * nss);

    for (int j=1; j<nss; j++)
	for (int i=0; i<n; i++)
	    actions[j*n+i] = actions[i] + pre + ss.actions[j] + post;
    
    for (int i=0; i<n; i++)
	actions[i] += pre + ss.actions[0] + post;

    return *this;
}

SetValue& SetValue::combine(const string& s, bool dot)
{
    string pre = dot ? "." : "[";
    string post = dot ? "" : "]";

    for (unsigned int i=0; i<actions.size(); i++)
	actions[i] += pre + s + post;

    return *this;
}

SetValue& SetValue::dotcat(const string& s)
{
    return combine(s, true);
}

SetValue& SetValue::dotcat(const SetValue& ss)
{
    return combine(ss, true);
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
    return combine(ss, true);
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

void SetValue::output(stringstream& ss) const
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

void SetValue::print() const
{
    cout << "r=" << rank << " {";
    for (unsigned int i=0; i<actions.size(); i++) {
	cout << actions[i] << ", ";
    }
    cout << "}\n";
}


/* =========================== RelabelingValue =========================*/
void RelabelingValue::print() const
{
    for (unsigned int i=0; i<old_labels.size(); i++) {
	new_labels[i].print();
	cout << " / ";
	old_labels[i].print();
	cout << "\n";
    }
}

SymbolValue * RelabelingValue::clone() const
{
    RelabelingValue * rlv = new RelabelingValue;

    for (unsigned int i=0; i<old_labels.size(); i++) {
	rlv->old_labels.push_back(old_labels[i]);
	rlv->new_labels.push_back(new_labels[i]);
    }

    return rlv;
}

void RelabelingValue::add(const SetValue& new_setvp,
                             const SetValue& old_setvp)
{
    old_labels.push_back(old_setvp);
    new_labels.push_back(new_setvp);
}

void RelabelingValue::merge(RelabelingValue& rlv)
{
    for (unsigned int i=0; i<rlv.old_labels.size(); i++) {
        add(rlv.new_labels[i], rlv.old_labels[i]);
    }
}


/* ============================ HidingValue ===========================*/
void HidingValue::print() const
{
    cout << "Hiding: ";
    if (interface)
	cout << "@ ";
    else
	cout << "\\ ";
    setv.print();
}

SymbolValue * HidingValue::clone() const
{
    HidingValue * hv = new HidingValue;

    hv->interface = interface;
    hv->setv = setv;

    return hv;
}


/* =========================== PriorityValue ========================= */
void PriorityValue::print() const
{
    cout << "Priority: ";
    if (low)
	cout << ">> ";
    else
	cout << "<< ";
    setv.print();
}

SymbolValue *PriorityValue::clone() const
{
    PriorityValue *pv = new PriorityValue;

    pv->low = low;
    pv->setv = setv;

    return pv;
}


/* ======================== ParametricProcess ========================= */
bool ParametricProcess::insert(const string& name, int default_value)
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

void ParametricProcess::clear()
{
    names.clear();
    defaults.clear();
}

void ParametricProcess::set_translator(ParametricTranslator *trans)
{
    assert(trans);
    translator = trans;
}

void ParametricProcess::print() const
{
    cout << "ParametricProcess: ";
    for (unsigned int i=0; i<names.size(); i++) {
        cout << "   " << names[i] << " [" << defaults[i] << "], "
                << this << "\n";
    }
}

SymbolValue * ParametricProcess::clone() const
{
    ParametricProcess * pp = new ParametricProcess;

    pp->names = names;
    pp->defaults = defaults;
    pp->translator = translator;

    return pp;
}

