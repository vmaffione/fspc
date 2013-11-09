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


struct SymbolValue {
    virtual void print() const { };
    virtual int type() const = 0;
    virtual SymbolValue * clone() const = 0;
    virtual int setVariable(const string& s) { return -1; }

    static const int Const = 0;
    static const int Range = 1;
    static const int Set = 2;
    static const int Lts = 3;
    static const int Process = 4;
    static const int ParametricProcess = 5;
    static const int ProcnodePair = 6;
    static const int Arguments = 7;
    static const int Relabeling = 9;
    static const int Hiding = 10;
    static const int Priority = 11;
    static const int LtsComposition = 12;

    virtual ~SymbolValue() { }
};

struct SetValue: public SymbolValue {
    vector<string> actions;
    string variable;
    
    SetValue() { }
    void print() const;
    int type() const { return SymbolValue::Set; }
    SymbolValue * clone() const;
    virtual int setVariable(const string& s) { variable = s; return 0; }
    bool hasVariable() const { return variable.size(); }

    SetValue& combine(const string&, bool);
    SetValue& combine(const SetValue&, bool);

    SetValue& dotcat(const string&);
    SetValue& dotcat(const SetValue&);
    SetValue& indexize(int index);
    SetValue& indexize(const SetValue&);
    SetValue& indexize(const string&);
    SetValue& indexize(int low, int high);
    SetValue& operator +=(const SetValue&);
    SetValue& operator +=(const string&);

    void output(const string& name, const char * filename) const;
    void output(stringstream& ss) const;
};

struct ConstValue: public SymbolValue {
    int value;

    void print() const { cout << value; }
    int type() const { return SymbolValue::Const; }
    void set(SetValue&) const;
    SymbolValue * clone() const;
};

struct RangeValue: public SymbolValue {
    int low;
    int high;
    string variable;

    void print() const { cout << "[" << low << ", " << high << "]"; }
    int type() const { return SymbolValue::Range; }
    void set(SetValue&) const;
    SymbolValue * clone() const;
    virtual int setVariable(const string& s) { variable = s; return 0; }
};

struct RelabelingValue: public SymbolValue {
    vector<SetValue> old_labels;
    vector<SetValue> new_labels;

    void add(const SetValue& n, const SetValue& o);
    void merge(RelabelingValue& rlv);
    unsigned int size() const { return old_labels.size(); }
    void print() const;
    int type() const { return SymbolValue::Relabeling; }
    SymbolValue * clone() const;
};

struct HidingValue: public SymbolValue {
    SetValue setv;
    bool interface;

    HidingValue() : interface(false) { }
    void print() const;
    int type() const { return SymbolValue::Hiding; }
    SymbolValue * clone() const;
};

struct PriorityValue: public SymbolValue {
    SetValue setv;
    bool low;

    PriorityValue() : low(false) { }
    void print() const;
    int type() const { return SymbolValue::Priority; }
    SymbolValue * clone() const;
};

class ParametricTranslator {
    public:
        virtual ~ParametricTranslator() {}
};

struct ParametricProcess : public SymbolValue {
    vector<string> names;
    vector<int> defaults;
    ParametricTranslator * translator;

    ParametricProcess() : translator(NULL) { }
    bool insert(const string& name, int default_value);
    void set_translator(ParametricTranslator *trans);
    void clear();
    void print() const;
    int type() const { return SymbolValue::ParametricProcess; }
    SymbolValue *clone() const;
};

struct SymbolsTable {
    map<string, SymbolValue*> table;

    void copyfrom(const SymbolsTable& st);

    SymbolsTable() { }
    SymbolsTable(const SymbolsTable& st);
    SymbolsTable& operator=(const SymbolsTable& st);
    bool insert(const string& name, SymbolValue *);
    bool lookup(const string& name, SymbolValue*&) const;
    bool remove(const string& name);
    int size() const { return table.size(); }
    void clear();
    void print() const;
    ~SymbolsTable();
};

#endif
