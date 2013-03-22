#ifndef __STRINGS__TABLE__H__
#define __STRINGS__TABLE__H__

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "strings_set.hpp"

using namespace std;


struct StringsTable {
    vector<char *> table;

    int insert(const char *);
    int lookup(const char *) const;
    void print() const;
};


struct SymbolValue {
    /* Is the symbol value is temporary (a literal) or is it taken from
       the symbol table? */
    bool temp;

    virtual void print() const = 0;
    virtual int type() const = 0;

    static const int Const = 0;
    static const int Range = 1;
    static const int Set = 2;
};

struct ConstValue: public SymbolValue {
    int value;

    void print() const { cout << value; }
    int type() const { return SymbolValue::Const; }
};

struct RangeValue: public SymbolValue {
    int low;
    int high;

    void print() const { cout << "[" << low << ", " << high << "]"; }
    int type() const { return SymbolValue::Range; }
};

struct SetValue: public SymbolValue {
    StringsSet * ssp;
    
    SetValue() : ssp(NULL) {}
    void print() const { ssp->print(); }
    int type() const { return SymbolValue::Set; }
};


struct SymbolsTable {
    map<string, SymbolValue*> table;

    bool insert(const string& name, SymbolValue *);
    bool lookup(const string& name, SymbolValue*&) const;
    void print() const;
};

#endif
