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


#ifndef __STRINGS__TABLE__H__
#define __STRINGS__TABLE__H__

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>

#include "smart_pointers.hpp"

using namespace std;


struct ActionsTable {
    string name;
    map<string, int> table;
    vector<string> reverse;
    int serial;

    ActionsTable() { }
    ActionsTable(const string& nm) { serial = 0; name = nm; insert("tau"); }
    int insert(const string& s);
    int lookup(const string& s) const;
    void print() const;

    ~ActionsTable() { table.clear(); reverse.clear(); }
};


struct Symbol {
    /* Reference counter used to implement the fsp::SmartPtr<fsp::Lts> smart pointer class. */
    unsigned refcount;
    DBR(unsigned delegated);

    Symbol() { refcount = 0; DBR(delegated = 0); }

    virtual void print() const { };
    virtual const char *className() const = 0;
    virtual Symbol *clone() const { return NULL; }
    virtual int setVariable(const string& s) { return -1; }
    virtual ~Symbol() { }
};

struct ActionSetS;

struct SetS: public Symbol {
    vector<string> actions;
    string variable;
    
    SetS() { }
    void print() const;
    const char *className() const { return "Set"; }
    Symbol *clone() const;
    virtual int setVariable(const string& s) { variable = s; return 0; }
    bool hasVariable() const { return variable.size(); }
    unsigned int size() const { return actions.size(); }
    string operator[](unsigned int i) const { return actions[i]; }
    string& operator[] (unsigned int i) { return actions[i]; }

    SetS& combine(const string&, bool);
    SetS& combine(const SetS&, bool);

    SetS& dotcat(const string&);
    SetS& dotcat(const SetS&);
    SetS& indexize(int index);
    SetS& indexize(const SetS&);
    SetS& indexize(const string&);
    SetS& indexize(int low, int high);
    SetS& operator +=(const SetS&);
    SetS& operator +=(const string&);
    void clear();

    void toActionSetValue(ActionsTable& at, ActionSetS& asv);
    void output(const string& name, const char * filename) const;
    void output(stringstream& ss) const;
};

struct IntS: public Symbol {
    int value;

    void print() const { cout << value; }
    const char *className() const { return "Const"; }
    void set(SetS&) const;
    Symbol * clone() const;
};

struct RangeS: public Symbol {
    int low;
    int high;
    string variable;

    void print() const { cout << "[" << low << ", " << high << "]"; }
    const char *className() const { return "Range"; }
    void set(SetS&) const;
    Symbol * clone() const;
    virtual int setVariable(const string& s) { variable = s; return 0; }
};

struct RelabelingS: public Symbol {
    vector<SetS> old_labels;
    vector<SetS> new_labels;

    void add(const SetS& n, const SetS& o);
    void merge(RelabelingS& rlv);
    unsigned int size() const { return old_labels.size(); }
    void print() const;
    const char *className() const { return "Relabeling"; }
    Symbol * clone() const;
};

struct HidingS: public Symbol {
    SetS setv;
    bool interface;

    HidingS() : interface(false) { }
    void print() const;
    const char *className() const { return "Hiding"; }
    Symbol * clone() const;
};

struct PriorityS: public Symbol {
    SetS setv;
    bool low;

    PriorityS() : low(false) { }
    void print() const;
    const char *className() const { return "Priority"; }
    Symbol * clone() const;
};

struct ActionSetS : public Symbol {
    set<unsigned int> actions;

    bool add(unsigned int a);
    bool lookup(unsigned int a) const;
    void clear();
    void toSetValue(const ActionsTable& at, SetS& sv);
    void print() const;
    const char *className() const { return "ActionSet"; }
    Symbol *clone() const;
};

struct ProgressS : public Symbol {
    ActionSetS condition;
    ActionSetS set;
    bool conditional;

    ProgressS() : conditional(false) { }
    void print() const;
    const char *className() const { return "Progress"; }
    Symbol *clone() const;
};

class ParametricTranslator {
    public:
        virtual ~ParametricTranslator() {}
};

struct ParametricProcess : public Symbol {
    vector<string> names;
    vector<int> defaults;
    ParametricTranslator * translator;

    ParametricProcess() : translator(NULL) { }
    bool insert(const string& name, int default_value);
    void set_translator(ParametricTranslator *trans);
    void clear();
    void print() const;
    const char *className() const { return "Parametric"; }
    Symbol *clone() const;
};

struct StringS : public Symbol {
    std::string val;

    const char *className() const { return "String"; }
};

struct IntVecS : public Symbol {
    std::vector<int> val;

    const char *className() const { return "IntVec"; }
};


struct SymbolsTable {
    map<string, Symbol*> table;

    void copyfrom(const SymbolsTable& st);

    SymbolsTable() { }
    SymbolsTable(const SymbolsTable& st);
    SymbolsTable& operator=(const SymbolsTable& st);
    bool insert(const string& name, Symbol *);
    bool lookup(const string& name, Symbol*&) const;
    bool remove(const string& name);
    unsigned int size() const { return table.size(); }
    void clear();
    void print() const;
    ~SymbolsTable();
};

#endif
