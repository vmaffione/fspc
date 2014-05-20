/*
 *  fspc contexts of variables implementation
 *
 *  Copyright (C) 2013-2014  Vincenzo Maffione
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

#include "symbols_table.hpp"

#include <iostream>

using namespace std;


/* This object mantains a list of action sets, with
   associated variable names. Therefore, it represents a set of "contexts",
   where each context is a different combination of value assignment to
   the variables. Using the select_next() method is possible to iterate
   over the contexts. The lookup methods return the values relative to
   the currently selected context. */
class ContextSet {
    vector<string> vars;
    vector<fsp::SetS> sets;
    vector<unsigned int> indexes;

public:
    bool insert(const string&, const fsp::SetS&);
    unsigned int num();  /* Number of contexts in the set. */
    bool select_next();
    void select_first();
    bool lookup(const string&, string&);
};


class Context {
        vector<string> vars;
        vector<string> vals;

    public:
        bool insert(const string&, const string&);
        bool lookup(const string&, string&);
        bool remove(const string&);
        bool operator!=(const Context& ctx);
        void clear();
};

#endif

