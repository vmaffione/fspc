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

/* Class that supports a list of SymbolValue*. */
struct SvpVec {
    vector<SymbolValue *> v;
    bool shared;

    SvpVec();
    SymbolValue * detach(unsigned int i);
    void detach_all();
    void print();
    ~SvpVec();
};

struct SetValue: public SymbolValue {
    vector<string> actions;
    string variable;
    int rank;
    
    SetValue() : rank(0) { }
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

struct ArgumentsValue: public SymbolValue {
    vector<int> args;

    void print() const;
    int type() const { return SymbolValue::Arguments; }
    SymbolValue * clone() const;
};

struct NewRelabelingValue: public SymbolValue {
    vector<SetValue> old_labels;
    vector<SetValue> new_labels;

    void add(const SetValue& n, const SetValue& o);
    void merge(NewRelabelingValue& rlv);
    unsigned int size() const { return old_labels.size(); }
    void print() const;
    int type() const { return SymbolValue::Relabeling; }
    SymbolValue * clone() const;
};

struct RelabelingValue: public SymbolValue {
    vector<SetValue *> old_labels;
    vector<SetValue *> new_labels;

    void add(SetValue *, SetValue *);
    void merge(RelabelingValue& rlv);
    void detach_all();
    unsigned int size() const { return old_labels.size(); }
    void print() const;
    int type() const { return SymbolValue::Relabeling; }
    SymbolValue * clone() const;
    ~RelabelingValue();
};

struct NewHidingValue: public SymbolValue {
    SetValue setv;
    bool interface;

    NewHidingValue() : interface(false) { }
    void print() const;
    int type() const { return SymbolValue::Hiding; }
    SymbolValue * clone() const;
};

struct HidingValue: public SymbolValue {
    SetValue * setvp;
    bool interface;

    HidingValue() : setvp(NULL), interface(false) { }
    void print() const;
    int type() const { return SymbolValue::Hiding; }
    SymbolValue * clone() const;
    ~HidingValue();
};

struct NewPriorityValue: public SymbolValue {
    SetValue setv;
    bool low;

    NewPriorityValue() : low(false) { }
    void print() const;
    int type() const { return SymbolValue::Priority; }
    SymbolValue * clone() const;
};

struct PriorityValue: public SymbolValue {
    SetValue * setvp;
    bool low;

    PriorityValue() : setvp(NULL), low(false) { }
    void print() const;
    int type() const { return SymbolValue::Priority; }
    SymbolValue * clone() const;
    ~PriorityValue();
};

class ParametricTranslator {
    public:
        virtual ~ParametricTranslator() {}
};

struct NewParametricProcess : public SymbolValue {
    vector<string> names;
    vector<int> defaults;
    ParametricTranslator * translator;

    NewParametricProcess() : translator(NULL) { }
    bool insert(const string& name, int default_value);
    void set_translator(ParametricTranslator *trans);
    void clear();
    void print() const;
    int type() const { return SymbolValue::ParametricProcess; }
    SymbolValue *clone() const;
};

struct ProcessBase {
    virtual bool unresolved() const = 0;
    virtual bool connected() const { return false; }
    virtual void print(ActionsTable * atp) = 0; 
    virtual ~ProcessBase() { }
};

struct Pvec {
    vector<ProcessBase *> v;

    void print(struct ActionsTable * atp);
};

struct ProcessNode;

struct ProcessEdge {
    ProcessNode * dest;
    int action;
    int rank;
    string unresolved_reference;
};

typedef void (*ProcessVisitFunction)(struct ProcessNode *,
		    void *);

struct ProcessVisitObject {
    ProcessVisitFunction vfp;
    void * opaque;
};

struct ProcessNode: public ProcessBase {
    vector<ProcessEdge> children;
    int type;

    ProcessNode() : type(ProcessNode::Normal) { }
    ProcessNode(int t) : type(t) { }
    void print(ActionsTable * atp);
    ProcessNode * clone() const;
    bool unresolved() const { return false; }
    void visit(struct ProcessVisitObject, bool before);

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
};

void freeProcessNodeGraph(struct ProcessNode *);

struct UnresolvedProcess: public ProcessBase {
    string reference;

    UnresolvedProcess(const string& s) : reference(s) { }
    bool unresolved() const { return true; }
    void print(ActionsTable * atp) { cout << "Unres " << reference << "\n"; }
};

struct ConnectedProcess: public ProcessBase {
    bool unresolved() const { return false; }
    bool connected() const {return true; }
    void print(ActionsTable * atp) { cout << "Connected\n"; }
};

class ProcessNodeAllocator {
    vector<ProcessBase *> nodes;

  public:
    ProcessNode * allocate(int type);
    ConnectedProcess * allocate_connected();
    UnresolvedProcess * allocate_unresolved(const string& name);
    void clear();
};

struct ProcessValue: public SymbolValue {
    /* A pointer to the first ProcessNode. */
    struct ProcessNode * pnp;

    ProcessValue() : pnp(NULL) { }
    void print(ActionsTable * atp) const { if(pnp) pnp->print(atp); }
    int type() const { return SymbolValue::Process; }
    SymbolValue * clone() const;
};

// TODO obsolete??
struct ProcnodePairValue: public SymbolValue {
    struct ProcessNode * first;
    struct ProcessNode * second;

    ProcnodePairValue() : first(NULL), second(NULL) { }
    ProcnodePairValue(ProcessNode * h, ProcessNode * t) : first(h), second(t) { }
    void print() const { cout << first << ", " << second; }
    int type() const { return SymbolValue::ProcnodePair; }
    SymbolValue * clone() const;
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
