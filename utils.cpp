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

string location_context(const string& filename, const fsp::location& loc)
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

static void print_error_location(const fsp::location& loc, int col)
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

void fsp::print_error_location_pretty(const fsp::location& loc)
{
    print_error_location(loc, loc.begin.column);
}

static void common_error(FspDriver& driver, const stringstream& ss,
                         const fsp::location& loc, const char *errtype)
{
    print_error_location_pretty(loc);
    cout << errtype << " error: " << ss.str() << "\n";
    driver.clear();
    exit(-1);
}

void fsp::semantic_error(FspDriver& driver, const stringstream& ss,
                    const fsp::location& loc)
{
    common_error(driver, ss, loc, "Semantic");
}

void fsp::general_error(FspDriver& driver, const stringstream& ss,
                   const fsp::location& loc)
{
    common_error(driver, ss, loc, "General");
}

