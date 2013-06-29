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

#include "utils.hpp"
#include "circular_buffer.hpp"



CircularBuffer last_tokens;


void print_error_location(const yy::location& loc, int col)
{
    cout << "@ line " << loc.begin.line << ", col " << loc.begin.column
	    << ":\n";
    if (col == -1)
	last_tokens.print();
    else
	last_tokens.print(col);
}

void syntax_error(const char * s, const yy::location& loc)
{
    print_error_location(loc, -1);
    cout << s << endl;
    exit(-1);
}

void semantic_error(const stringstream& ss, const yy::location& loc)
{
    print_error_location(loc, loc.begin.column);
    cout << "Semantic error: " << ss.str() << "\n";
    exit(-1);
}

ConstValue* err_if_not_const(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Const) {
	stringstream errstream;
	errstream << "Const expected";
	semantic_error(errstream, loc);
    }

    return static_cast<ConstValue *>(svp);
}

RangeValue* err_if_not_range(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Range) {
	stringstream errstream;
	errstream << "Range expected";
	semantic_error(errstream, loc);
    }

    return static_cast<RangeValue *>(svp);
}

SetValue* err_if_not_set(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Set) {
	stringstream errstream;
	errstream << "Set expected";
	semantic_error(errstream, loc);
    }

    return static_cast<SetValue *>(svp);
}

ProcessValue* err_if_not_process(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Process) {
	stringstream errstream;
	errstream << "Process expected";
	semantic_error(errstream, loc);
    }

    return static_cast<ProcessValue *>(svp);
}

ProcnodePairValue* err_if_not_procnodepair(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::ProcnodePair) {
	stringstream errstream;
	errstream << "ProcnodePair expected";
	semantic_error(errstream, loc);
    }

    return static_cast<ProcnodePairValue *>(svp);
}

ArgumentsValue* err_if_not_arguments(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Arguments) {
	stringstream errstream;
	errstream << "Arguments expected";
	semantic_error(errstream, loc);
    }

    return static_cast<ArgumentsValue *>(svp);
}

RelabelingValue* err_if_not_relabeling(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Relabeling) {
	stringstream errstream;
	errstream << "Relabeling expected";
	semantic_error(errstream, loc);
    }

    return static_cast<RelabelingValue *>(svp);
}

HidingValue* err_if_not_hiding(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Hiding) {
	stringstream errstream;
	errstream << "Hiding expected";
	semantic_error(errstream, loc);
    }

    return static_cast<HidingValue *>(svp);
}

PriorityValue* err_if_not_priority(SymbolValue * svp, const yy::location& loc)
{
    if (svp->type() != SymbolValue::Priority) {
	stringstream errstream;
	errstream << "Priority expected";
	semantic_error(errstream, loc);
    }

    return static_cast<PriorityValue *>(svp);
}

ProcessNode * err_if_not_procnode(ProcessBase * pbp, const yy::location& loc)
{
    if (pbp->unresolved() || pbp->connected()) {
	stringstream errstream;
	errstream << "ProcessNode expected";
	semantic_error(errstream, loc);
    }
    return static_cast<ProcessNode *>(pbp);
}


void err_if_not_const_svpvec(SvpVec * vp, const yy::location& loc)
{
    for (unsigned int c=0; c<vp->v.size(); c++)
	err_if_not_const(vp->v[c], loc);
}
