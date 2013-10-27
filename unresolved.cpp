#include "unresolved.hpp"
#include <iostream>


#define FUNNY   44

static string merge_string_vector(const vector<string>& sv)
{
    string ret;

    for (unsigned int i=0; i<sv.size(); i++) {
        ret += sv[i] + ", ";
    }

    return ret;
}

unsigned int UnresolvedNames::add(const string& name, unsigned int idx)
{
    unsigned int i = idx - FUNNY;

    if (i >= size()) {
        return ~0U;
    }

    names[i].push_back(name);

    /* Look for 'name' in the other entries and remove them, because 'name'
       has been merged to the 'idx' entry.  */
    for (unsigned int k=0; k<names.size(); k++) {
        if (k != i) {
            for (unsigned int j=0; j<names[k].size(); j++) {
                if (name == names[k][j]) {
                    names[k].erase(names[k].begin() + j);
                    /* Return the matching 'idx', so that the caller can
                       merge the 'priv' fields. */
                    return k + FUNNY;
                }
            }
        }
    }

    return ~0U;
}

unsigned int UnresolvedNames::add(const string& name)
{
    for (unsigned int i=0; i<names.size(); i++) {
        for (unsigned int j=0; j<names[i].size(); j++) {
            if (names[i][j] == name) {
                return i + FUNNY;
            }
        }
    }

    vector<string> sv(1);

    sv[0] = name;
    names.push_back(sv);

    return names.size() - 1 + FUNNY;
}

unsigned int UnresolvedNames::size() const
{
    return names.size();
}

unsigned int UnresolvedNames::get_idx(unsigned int i) const
{
    if (i >= size() || !names[i].size()) {
        return ~0U;
    }

    return i + FUNNY;
}

string UnresolvedNames::get_name(unsigned int i) const
{
    if (i >= size() || !names[i].size()) {
        return string();
    }

    return merge_string_vector(names[i]);
}

string UnresolvedNames::lookup(unsigned int idx) const
{
    unsigned int i = idx - FUNNY;

    if (i >= names.size() || !names[i].size()) {
        return string();
    }

    return merge_string_vector(names[i]);
}

void UnresolvedNames::clear()
{
    names.clear();
}

/*
unsigned int UnresolvedNames::lookup(const string& name)
{
    for (unsigned int i=0; i<names.size(); i++) {
        if (names[i] == name) {
            return idxs[i];
        }
    }

    return ~0U;
}*/

