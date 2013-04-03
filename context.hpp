#ifndef __CONTEXT__H__
#define __CONTEXT__H__

#include <iostream>
#include "strings_table.hpp"

using namespace std;


class Context {
    vector<string> names;
    vector<SymbolValue *> values;

  public:
    Context() { }
    Context(const Context&);
    bool lookup(const string& s, SymbolValue*& ret);
    bool insert(const string& s, int value);
    bool insert(const string& s, const string& value);
    void print() const;
    ~Context();
};

struct FrontierElement {
    ProcessNode * pnp;
    int child;
};

struct ContextsSet {
    vector<Context *> s;
    vector<FrontierElement> frontier;

    ContextsSet() { }
    ContextsSet(const ContextsSet&);
    int size() const { return s.size(); }
    Context*& operator[](int i) { return s[i]; }
    void append(Context * ctx) { s.push_back(ctx); }
    void filter(const vector<bool>&);
    void print() const;
    ~ContextsSet();
};

#endif
