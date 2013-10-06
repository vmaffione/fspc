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

#include <cassert>
#include <string>
#include "context.hpp"

using namespace std;

/* ============================ Context ================================ */

Context::Context(const Context& ctx)
{
    names = ctx.names;
    values.resize(ctx.values.size());
    for (unsigned int i=0; i<values.size(); i++)
	    values[i] = ctx.values[i]->clone();
    ruled_out = ctx.ruled_out;
}

bool Context::lookup(const string& s, SymbolValue*& ret) {
    for (unsigned int i=0; i<names.size(); i++)
	if (s == names[i]) {
	    ret = values[i];
	    return true;
	}
    return false;
}

bool Context::insert(const string& s, int value) {
    ConstValue * cvp;
    for (unsigned int i=0; i<names.size(); i++)
	if (s == names[i])
	    return false;
    names.push_back(s);
    cvp = new ConstValue;
    cvp->value = value;
    values.push_back(cvp);
    return true;
}

bool Context::insert(const string& s, const string& value) {
    SetValue * setvp;
    for (unsigned int i=0; i<names.size(); i++)
	if (s == names[i])
	    return false;
    names.push_back(s);
    setvp = new SetValue;
    setvp->actions.push_back(value);
    values.push_back(setvp);
    return true;
}

void Context::print() const {
    cout << "ctx = {";
    for (unsigned int i=0; i<names.size(); i++) {
	cout << names[i] << "="; 
	switch (values[i]->type()) {
	    case SymbolValue::Const:
		cout << ((ConstValue *)values[i])->value;
		break;
	    case SymbolValue::Set:
		cout << ((SetValue *)values[i])->actions[0];
		break;
	}
	cout << ", ";
    }
    cout << "}\n";
}

Context::~Context()
{
    for (unsigned int i=0; i<values.size(); i++)
	delete values[i];
}


/* ========================== ContextsSet ============================= */

ContextsSet::ContextsSet(const ContextsSet& cs)
{
    s.resize(cs.s.size());
    for (unsigned int i=0; i<cs.s.size(); i++) {
	s[i] = new Context(*cs.s[i]);
    }
    frontier = cs.frontier;
}

void ContextsSet::append(Context * ctx) { 
    s.push_back(ctx);
}
void ContextsSet::rule_out(int c) {
    s[c]->ruled_out = true;
}
bool ContextsSet::is_ruled_out(int c) const {
    return s[c]->ruled_out;
}

void ContextsSet::print() const {
    cout << "#contexts = " << s.size() << "; ";
    cout << "frnt: {";
    for (unsigned int i=0; i<frontier.size(); i++)
	cout << "(" << frontier[i].pnp << "," << frontier[i].child
	    << "), ";
    cout << "}\n";
    for (unsigned int i=0; i<s.size(); i++)
	s[i]->print();
}

ContextsSet::~ContextsSet() {
    for (unsigned int i=0; i<s.size(); i++) {
	delete s[i];
    }
}


/* ========================== ContextsSetStack =========================== */

void ContextsSetStack::push(ContextsSet * ctxset) {
    stack.push_back(ctxset);
}

void ContextsSetStack::push_clone() {
    stack.push_back(new ContextsSet(*stack.back()));
}

bool ContextsSetStack::update(ContextsSet * ctxset) {
    if (!stack.size())
	return false;

    delete stack.back();
    stack.back() = ctxset;

    return true;
}

bool ContextsSetStack::pop() {
    if (!stack.size())
	return false;

    delete stack.back();
    stack.pop_back();

    return true;
}

ContextsSet& ContextsSetStack::top() { return *(stack.back()); }

ContextsSetStack::~ContextsSetStack() {
    while (pop()) { }
}


/* ====================== New API implementation ======================= */

bool NewContextSet::insert(const string& var, const RangeValue& v)
{
    for (unsigned int i=0; i<range_vars.size(); i++) {
        if (range_vars[i] == var)
            return false;
    }

    range_vars.push_back(var);
    range_vals.push_back(v);
    /* Lower index in the range selected. */
    cur_index_vals.push_back(v.low);

    return true;
}

bool NewContextSet::insert(const string& var, const SetValue& v)
{
    assert(v.actions.size());

    for (unsigned int i=0; i<set_vars.size(); i++) {
        if (set_vars[i] == var)
            return false;
    }

    set_vars.push_back(var);
    set_vals.push_back(v);
    /* First action in the set selected. */
    cur_action_vals.push_back(0);

    return true;
}

unsigned int NewContextSet::num()
{
    unsigned int ret = 1;

    for (unsigned int i=0; i<range_vals.size(); i++) {
        ret *= range_vals[i].high - range_vals[i].low + 1;
    }

    for (unsigned int i=0; i<set_vals.size(); i++) {
        ret *= set_vals[i].actions.size();
    }

    return ret;
}

void NewContextSet::select_first()
{
    for (unsigned int i=0; i<cur_index_vals.size(); i++) {
        cur_index_vals[i] = range_vals[i].low;
    }

    for (unsigned int i=0; i<cur_action_vals.size(); i++) {
        cur_action_vals[i] = 0;
    }
}

void NewContextSet::select_next()
{
    unsigned int i = 0;

    while (i < cur_index_vals.size()) {
        cur_index_vals[i]++;
        if (cur_index_vals[i] > range_vals[i].high) {
            /* Overflow wrapping. */
            cur_index_vals[i] = range_vals[i].low;
            i++;
        } else {
            break;
        }
    }

    if (i == cur_index_vals.size()) {
        /* Enter here only if all the ranges overflowed. */
        i = 0;
        while (i < cur_action_vals.size()) {
            cur_action_vals[i]++;
            if (cur_action_vals[i] == set_vals[i].actions.size()) {
                /* Overflow wrapping. */
                cur_action_vals[i] = 0;
                i++;
            } else {
                break;
            }
        }
    }
}

bool NewContextSet::lookup(const string& name, int& v)
{
    for (unsigned int i=0; i<range_vars.size(); i++) {
        if (name == range_vars[i]) {
            v = cur_index_vals[i];
            return true;
        }
    }

    return false;
}

bool NewContextSet::lookup(const string& name, string& v)
{
    for (unsigned int i=0; i<set_vars.size(); i++) {
        if (name == set_vars[i]) {
            v = set_vals[i].actions[ cur_action_vals[i] ];
            return true;
        }
    }

    return false;
}

