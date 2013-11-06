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


#include <iostream>
#include <cstdlib>


/* Token buffer support. */
#include "circular_buffer.hpp"

/* Main class driver. */
#include "driver.hpp"


CircularBuffer last_tokens;


static void print_error_location(const yy::location& loc, int col)
{
    cout << "@ line " << loc.begin.line << ", col " << loc.begin.column
	    << ":\n";
    if (col == -1)
	last_tokens.print();
    else
	last_tokens.print(col);
}

void print_error_location_pretty(const yy::location& loc)
{
    print_error_location(loc, loc.begin.column);
}

void semantic_error(FspDriver& driver, const stringstream& ss, const yy::location& loc)
{
    print_error_location_pretty(loc);
    cout << "Semantic error: " << ss.str() << "\n";
    driver.clear();
    exit(-1);
}

ConstValue* err_if_not_const(FspDriver& driver, SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Const) {
	stringstream errstream;
	errstream << "Const expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<ConstValue *>(svp);
}

RangeValue* err_if_not_range(FspDriver& driver, SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Range) {
	stringstream errstream;
	errstream << "Range expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<RangeValue *>(svp);
}

SetValue* err_if_not_set(FspDriver& driver, SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Set) {
	stringstream errstream;
	errstream << "Set expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<SetValue *>(svp);
}

RelabelingValue* err_if_not_relabeling(FspDriver& driver, SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Relabeling) {
	stringstream errstream;
	errstream << "Relabeling expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<RelabelingValue *>(svp);
}

HidingValue* err_if_not_hiding(FspDriver& driver, SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Hiding) {
	stringstream errstream;
	errstream << "Hiding expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<HidingValue *>(svp);
}

PriorityValue* err_if_not_priority(FspDriver& driver, SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Priority) {
	stringstream errstream;
	errstream << "Priority expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<PriorityValue *>(svp);
}

