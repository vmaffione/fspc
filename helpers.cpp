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

