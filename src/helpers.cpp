/*
 *  fspc helper functions
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


#include "helpers.hpp"

#include <cstdlib>
#include <sstream>
#include <unistd.h>


string int2string(int x)
{
    stringstream sstr;
    sstr << x;
    return sstr.str();
}

int string2int(const string& s, int& ret)
{
    char *dummy;
    const char *cstr = s.c_str();

    ret = strtoul(cstr, &dummy, 10);
    if (!s.size() || *dummy != '\0') {
        ret = ~0U;
        return -1;
    }

    return 0;
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

void merge_string_vec(const vector<string>& vec, string& res,
                        const string& separator)
{
    res.clear();

    for (unsigned int i = 0; i < vec.size(); i++) {
        if (i) {
            res += separator;
        }
        res += vec[i];
    }
}

/* Generate a per process unique name, and insert/append a
   prefix/suffix string to it. */
string get_tmp_name(const string& prefix, const string& suffix)
{
    return prefix + "." + int2string(getpid()) + "." + suffix;
}

/* Generate a per-process unique file name which is going to belong
   in the current working directory, using 'name' as a seed. */
string get_tmp_name_cwd(const string& name)
{
    char *cwd = get_current_dir_name();
    string ret = cwd;
    string name_ = name;

    /* Sanitize the seed. */
    for (unsigned int i = 0; i < name.size(); i++) {
        if (name[i] == '\\' || name [i] == '/') {
            name_[i] = '.';
        }
    }

    ret += "/" + get_tmp_name("", name_);

    free(cwd);

    return ret;
}

string set2string(const set<string>& s)
{
    string ret = "{";

    for (set<string>::const_iterator sit = s.begin(); sit != s.end(); sit++) {
        if (sit != s.begin()) {
            ret += ",";
        }
        ret += *sit;
    }
    ret += "}";

    return ret;
}
