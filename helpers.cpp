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


#include "helpers.hpp"
#include <cstdlib>
#include <sstream>


string int2string(int x)
{
    stringstream sstr;
    sstr << x;
    return sstr.str();
}

int string2int(const string& s)
{
    int ret = atoi(s.c_str());

    return ret;
}

void lts_name_extension(const vector<int>& values, string& extension)
{
    extension = "";

    if (values.size()) {
	unsigned int i = 0;

	extension = "(";
	for (; i<values.size() - 1; i++) {
	   extension += int2string(values[i]) + ",";
	}
	extension += int2string(values[i]) + ")";
    }
}

bool intersection_exists(const set<unsigned int>& first,
                         const set<unsigned int>& second)
{
    const set<unsigned int>& bigger = (first.size() > second.size())
                                        ? first : second;
    const set<unsigned int>& smaller = (first.size() > second.size())
                                        ? second : first;

    for (set<unsigned int>:: iterator it = smaller.begin();
                            it != smaller.end(); it++) {
        if (bigger.count(*it)) {
            return true;
        }
    }

    return false;
}

