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


#include <string>
#include <vector>
#include <set>

using namespace std;


string int2string(int x);
int string2int(const string& s, int& ret);
void lts_name_extension(const vector<int>& values, string& extension);
bool intersection_exists(const set<unsigned int>&,
                         const set<unsigned int>&);
void merge_string_vec(const vector<string>& vec, string& res,
                        const string& separator);
string get_tmp_name(const string& prefix, const string& suffix);
string get_tmp_name_cwd(const string& name);

string set2string(const set<string>& s);

template <typename T>
bool equal(const set<T>& first, const set<T>& second)
{
    if (first.size() != second.size()) {
        return false;
    }

    for (typename set<T>::iterator it = first.begin();
                                it != first.end(); it++) {
        if (!second.count(*it)) {
            return false;
        }
    }

    return true;
}

template <typename T>
void merge(const set<T>& mergeable, set<T>& accumulator)
{
    for (typename set<T>::iterator it = mergeable.begin();
                        it != mergeable.end(); it++) {
            accumulator.insert(*it);
    }
}
