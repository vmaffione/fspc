#ifndef __STRINGS__TABLE__H__
#define __STRINGS__TABLE__H__

#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;


struct StringsTable {
    vector<char *> table;

    int insert(const char *);
    int lookup(const char *) const;
    void print() const;
};


struct SymbolValue {
    virtual void print() = 0;
};

struct ConstValue: public SymbolValue {
    int value;

    void print() { cout << value << "\n"; }
};

struct RangeValue: public SymbolValue {
    int low;
    int high;

    void print() { cout << "[" << low << ", " << high << "]\n"; }
};


struct SymbolsTable {
    map<string, in;t> table;

    bool insert(const string& name, const int& value);
    bool lookup(const string& name, int& value) const;
    void print() const;
};

#endif
