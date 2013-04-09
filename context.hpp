#ifndef __CONTEXT__H__
#define __CONTEXT__H__

#include <iostream>
#include "symbols_table.hpp"

using namespace std;


class Context {
  public:
    vector<string> names;
    vector<SymbolValue *> values;
    bool ruled_out;

    Context() { ruled_out = 0; }
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
    void append(Context * ctx);
    void rule_out(int c);
    bool is_ruled_out(int c) const;
    void print() const;
    ~ContextsSet();
};


/* Stack of contexts sets. */
struct ContextsSetStack {
    vector<ContextsSet *> stack;

    ContextsSetStack() { }
    void push(ContextsSet * ctxset) {
	stack.push_back(ctxset);
    }
    void push_clone() {
	stack.push_back(new ContextsSet(*stack.back()));
    }
    void update(ContextsSet * ctxset) {
	delete stack.back();
	stack.back() = ctxset;
    }
    void pop() {
	delete stack.back();
	stack.pop_back();
    }
    ContextsSet& top() { return *(stack.back()); }
};

#endif
