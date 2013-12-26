/*
 *  fspc helper functions
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


#ifndef __UTILS__HH
#define __UTILS__HH

#include <sstream>
#include <assert.h>

using namespace std;

#include "symbols_table.hpp"
#include "parser.hpp"


void semantic_error(FspDriver& driver, const stringstream& ss,
                    const yy::location& loc);
void general_error(FspDriver& driver, const stringstream& ss,
                   const yy::location& loc);
void print_error_location_pretty(const yy::location& loc);

template <class T>
T* is(Symbol *svp)
{
    T* ret = dynamic_cast<T*>(svp);

    assert(ret);

    return ret;
}

/* Helper function used to get a ConstS* from a Symbol*. If the
   object pointed is not a constant, a semantic error is issued. */
template <class T>
T* err_if_not(FspDriver& c, Symbol *svp, const yy::location& loc)
{
    T* ret = dynamic_cast<T*>(svp);

    if (ret == NULL) {
	stringstream errstream;
	errstream << T().className() << " expected";
	semantic_error(c, errstream, loc);
    }

    return ret;
}

#endif
