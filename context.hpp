/*
 *  fspc contexts of variables implementation
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
    unsigned int size() const { return s.size(); }
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

    void push(ContextsSet * ctxset);
    void push_clone();
    bool update(ContextsSet * ctxset);
    bool pop();
    ContextsSet& top();
    ~ContextsSetStack();
};

/* =========================== New API. ==================== */

/* This object mantains a list of action sets, with
   associated variable names. Therefore, it represents a set of "contexts",
   where each context is a different combination of value assignment to
   the variables. Using the select_next() method is possible to iterate
   over the contexts. The lookup methods return the values relative to
   the currently selected context. */
class NewContextSet {
    vector<string> vars;
    vector<SetValue> sets;
    vector<unsigned int> indexes;

public:
    bool insert(const string&, const SetValue&);
    unsigned int num();  /* Number of contexts in the set. */
    bool select_next();
    void select_first();
    bool lookup(const string&, string&);
};


class NewContext {
        vector<string> vars;
        vector<string> vals;

    public:
        bool insert(const string&, const string&);
        bool lookup(const string&, string&);
        bool remove(const string&);
};

#endif

