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
#include <string>
#include <fstream>

using namespace std;

/* Token buffer support. */
#include "circular_buffer.hpp"

/* Main class driver. */
#include "driver.hpp"


/* Global variable shared between all the compilation units. */
CircularBuffer last_tokens;

string location_context(const string& filename, const yy::location& loc)
{
    string ret;
    fstream fin;
    string s;
    int len;
    unsigned int from;

    fin.open(filename.c_str(), ios::in);

    if (fin.fail()) {
        return ret;
    }

    /* Discard 'loc.begin.line-1' lines. */
    for (unsigned int i=0; i<loc.begin.line; i++) {
        getline(fin, s);
        if (fin.eof()) {
            return ret;
        }
    }

    from = loc.begin.column - 1;
    if (from >= s.size()) {
        from = s.size() - 1;
    }

    len = loc.end.column - from + 1;
    if (len < 0) {
        len = 0;
    } else if (len > static_cast<int>(s.size())) {
        len = s.size();
    }

    ret = s.substr(from, len);

    return ret;
}

static void print_error_location(const yy::location& loc, int col)
{
    assert(loc.begin.filename);

    string filename = *loc.begin.filename;
    string context = location_context(filename, loc);

    if (loc.begin.line == loc.end.line) {
        cout << "@ " << filename << ", line " << loc.begin.line << ", cols " << loc.begin.column
                << "-" << loc.end.column << ":\n";
    } else {
        cout << "@ " << filename << ", lines " << loc.begin.line << "-" << loc.end.line << ":\n";
    }

    if (col == -1)
	last_tokens.print(context);
    else
	last_tokens.print(context, col);
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

ConstS* err_if_not_const(FspDriver& driver, Symbol * svp, const yy::location& loc)
{
    if (svp->type() != Symbol::Const) {
	stringstream errstream;
	errstream << "Const expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<ConstS *>(svp);
}

RangeS* err_if_not_range(FspDriver& driver, Symbol * svp, const yy::location& loc)
{
    if (svp->type() != Symbol::Range) {
	stringstream errstream;
	errstream << "Range expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<RangeS *>(svp);
}

SetS* err_if_not_set(FspDriver& driver, Symbol * svp, const yy::location& loc)
{
    if (svp->type() != Symbol::Set) {
	stringstream errstream;
	errstream << "Set expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<SetS *>(svp);
}

RelabelingS* err_if_not_relabeling(FspDriver& driver, Symbol * svp, const yy::location& loc)
{
    if (svp->type() != Symbol::Relabeling) {
	stringstream errstream;
	errstream << "Relabeling expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<RelabelingS *>(svp);
}

HidingS* err_if_not_hiding(FspDriver& driver, Symbol * svp, const yy::location& loc)
{
    if (svp->type() != Symbol::Hiding) {
	stringstream errstream;
	errstream << "Hiding expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<HidingS *>(svp);
}

PriorityS* err_if_not_priority(FspDriver& driver, Symbol * svp, const yy::location& loc)
{
    if (svp->type() != Symbol::Priority) {
	stringstream errstream;
	errstream << "PriorityS expected";
	semantic_error(driver, errstream, loc);
    }

    return static_cast<PriorityS *>(svp);
}

