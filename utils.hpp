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


void semantic_error(FspDriver& driver, const stringstream& ss, const yy::location& loc);
void print_error_location_pretty(const yy::location& loc);

/* Helper function used to get a ConstValue* from a SymbolValue*. If the
   object pointed is not a constant, a semantic error is issued. */
ConstValue* err_if_not_const(FspDriver& driver, SymbolValue * svp, const yy::location& loc);
RangeValue* err_if_not_range(FspDriver& driver, SymbolValue * svp, const yy::location& loc);
SetValue* err_if_not_set(FspDriver& driver, SymbolValue * svp, const yy::location& loc);
RelabelingValue* err_if_not_relabeling(FspDriver& driver, SymbolValue * svp, const yy::location& loc);
HidingValue* err_if_not_hiding(FspDriver& driver, SymbolValue * svp, const yy::location& loc);
PriorityValue* err_if_not_priority(FspDriver& driver, SymbolValue * svp, const yy::location& loc);


template <class T>
T* is(SymbolValue *svp)
{
    T* ret = dynamic_cast<T*>(svp);

    assert(ret);

    return ret;
}

#endif
