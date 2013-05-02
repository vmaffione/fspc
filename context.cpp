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


#include "context.hpp"

/* ============================ Context ================================ */

Context::Context(const Context& ctx)
{
    names = ctx.names;
    values.resize(ctx.values.size());
    for (int i=0; i<values.size(); i++)
	    values[i] = ctx.values[i]->clone();
    ruled_out = ctx.ruled_out;
}

bool Context::lookup(const string& s, SymbolValue*& ret) {
    for (int i=0; i<names.size(); i++)
	if (s == names[i]) {
	    ret = values[i];
	    return true;
	}
    return false;
}

bool Context::insert(const string& s, int value) {
    ConstValue * cvp;
    for (int i=0; i<names.size(); i++)
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
    for (int i=0; i<names.size(); i++)
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
    for (int i=0; i<names.size(); i++) {
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
    for (int i=0; i<values.size(); i++)
	delete values[i];
}


/* ========================== ContextsSet ============================= */

ContextsSet::ContextsSet(const ContextsSet& cs)
{
    s.resize(cs.s.size());
    for (int i=0; i<cs.s.size(); i++) {
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
    for (int i=0; i<frontier.size(); i++)
	cout << "(" << frontier[i].pnp << "," << frontier[i].child
	    << "), ";
    cout << "}\n";
    for (int i=0; i<s.size(); i++)
	s[i]->print();
}

ContextsSet::~ContextsSet() {
    for (int i=0; i<s.size(); i++) {
	delete s[i];
    }
}
