#ifndef __STRINGS__TABLE__H__
#define __STRINGS__TABLE__H__

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "strings_set.hpp"
//#include "lts.hpp"

using namespace std;


struct StringsTable {
    vector<char *> table;

    int insert(const char *);
    int lookup(const char *) const;
    void print() const;
};


struct SymbolValue {
    virtual void print() const = 0;
    virtual int type() const = 0;
    virtual SymbolValue * clone() const = 0;

    static const int Const = 0;
    static const int Range = 1;
    static const int Set = 2;
    static const int Lts = 3;
    static const int Process = 4;
};

struct ConstValue: public SymbolValue {
    int value;

    void print() const { cout << value; }
    int type() const { return SymbolValue::Const; }
    SymbolValue * clone() const;
};

struct RangeValue: public SymbolValue {
    int low;
    int high;

    void print() const { cout << "[" << low << ", " << high << "]"; }
    int type() const { return SymbolValue::Range; }
    SymbolValue * clone() const;
};

struct SetValue: public SymbolValue {
    StringsSet * ssp;
    
    SetValue() : ssp(NULL) {}
    SetValue(const SetValue&);
    void print() const { ssp->print(); }
    int type() const { return SymbolValue::Set; }
    SymbolValue * clone() const;
};


struct ProcessBase {
    virtual bool unresolved() const = 0;
};

struct ProcessNode;

struct ProcessEdge {
    ProcessNode * dest;
    string action;
};

struct ProcessNode: public ProcessBase {
    vector<ProcessEdge> children;
    int type;
    vector<ProcessNode*> frontier_shortcut;

    ProcessNode() : type(ProcessNode::Normal) { }
    ProcessNode(int t) : type(t) { }
    void print() const;
    ProcessNode * clone() const;
    bool unresolved() const { return false; }

    //TODO write the destructor!!

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
};

struct UnresolvedProcess: public ProcessBase {
    string reference;

    UnresolvedProcess(const string& s) : reference(s) { }
    bool unresolved() const { return true; }
};

struct ProcessValue: public SymbolValue {
    struct ProcessNode * pnp;

    ProcessValue() : pnp(NULL) {}
    void print() const { pnp->print(); }
    int type() const { return SymbolValue::Process; }
    SymbolValue * clone() const;
};



struct SymbolsTable {
    map<string, SymbolValue*> table;

    bool insert(const string& name, SymbolValue *);
    bool lookup(const string& name, SymbolValue*&) const;
    void print() const;
};

#endif
