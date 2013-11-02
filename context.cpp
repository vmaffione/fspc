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

bool NewContextSet::insert(const string& var, const SetValue& v)
{
    assert(v.actions.size());

    for (unsigned int i=0; i<vars.size(); i++) {
        if (vars[i] == var)
            return false;
    }

    vars.push_back(var);
    sets.push_back(v);
    /* First action in the set selected. */
    indexes.push_back(0);

    return true;
}

unsigned int NewContextSet::num()
{
    unsigned int ret = 1;

    for (unsigned int i=0; i<sets.size(); i++) {
        ret *= sets[i].actions.size();
    }

    return ret;
}

void NewContextSet::select_first()
{
    for (unsigned int i=0; i<indexes.size(); i++) {
        indexes[i] = 0;
    }
}

bool NewContextSet::select_next()
{
    unsigned int i = 0;

    /* Here is not necessary to scan the combination in "reverse mode", like we
       do in computePrefixActions(). */
    while (i < indexes.size()) {
        indexes[i]++;
        if (indexes[i] == sets[i].actions.size()) {
            /* Wraparound: Go to the next variable. */
            indexes[i] = 0;
            i++;
        } else {
            return true;  /* There are more combinations. */
        }
    }

    return false;  /* There are no more combinations. */
}

bool NewContextSet::lookup(const string& name, string& v)
{
    /* Not so useful, use ContextSet lookup instead. XXX */
    for (unsigned int i=0; i<vars.size(); i++) {
        if (name == vars[i]) {
            v = sets[i].actions[ indexes[i] ];
            return true;
        }
    }

    return false;
}

/* ========================= NewContext class implementation =========================== */

bool NewContext::insert(const string& name, const string& s)
{
    for (unsigned int i=0; i<vars.size(); i++) {
        if (name == vars[i]) {
            return false;
        }
    }
    vars.push_back(name);
    vals.push_back(s);

    return true;
}

bool NewContext::lookup(const string& name, string& ret)
{
    for (unsigned int i=0; i<vars.size(); i++) {
        if (name == vars[i]) {
            ret = vals[i];
            return true;
        }
    }

    return false;
}

bool NewContext::remove(const string& name)
{
    for (unsigned int i=0; i<vars.size(); i++) {
        if (name == vars[i]) {
            vars.erase(vars.begin() + i);
            vals.erase(vals.begin() + i);
            return true;
        }
    }

    return false;
}

bool NewContext::operator!=(const NewContext& c)
{
    if (vars.size() != c.vars.size()) {
        return true;
    }

    for (unsigned int i=0; i<vars.size(); i++) {
        if (vars[i] != c.vars[i] || vals[i] != c.vals[i]) {
            return true;
        }
    }

    return false;
}

void NewContext::clear()
{
    vars.clear();
    vals.clear();
}

