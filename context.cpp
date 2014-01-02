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


/* ====================== Context implementation ======================= */

bool ContextSet::insert(const string& var, const fsp::SetS& v)
{
    assert(v.size());

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

unsigned int ContextSet::num()
{
    unsigned int ret = 1;

    for (unsigned int i=0; i<sets.size(); i++) {
        ret *= sets[i].size();
    }

    return ret;
}

void ContextSet::select_first()
{
    for (unsigned int i=0; i<indexes.size(); i++) {
        indexes[i] = 0;
    }
}

bool ContextSet::select_next()
{
    unsigned int i = 0;

    /* Here is not necessary to scan the combination in "reverse mode", like we
       do in computePrefixActions(). */
    while (i < indexes.size()) {
        indexes[i]++;
        if (indexes[i] == sets[i].size()) {
            /* Wraparound: Go to the next variable. */
            indexes[i] = 0;
            i++;
        } else {
            return true;  /* There are more combinations. */
        }
    }

    return false;  /* There are no more combinations. */
}

bool ContextSet::lookup(const string& name, string& v)
{
    /* Not so useful, use ContextSet lookup instead. XXX */
    for (unsigned int i=0; i<vars.size(); i++) {
        if (name == vars[i]) {
            v = sets[i][ indexes[i] ];
            return true;
        }
    }

    return false;
}

/* ========================= Context class implementation =========================== */

bool Context::insert(const string& name, const string& s)
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

bool Context::lookup(const string& name, string& ret)
{
    for (unsigned int i=0; i<vars.size(); i++) {
        if (name == vars[i]) {
            ret = vals[i];
            return true;
        }
    }

    return false;
}

bool Context::remove(const string& name)
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

bool Context::operator!=(const Context& c)
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

void Context::clear()
{
    vars.clear();
    vals.clear();
}

