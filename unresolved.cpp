#include "unresolved.hpp"
#include <iostream>
#include <cassert>


#define FUNNY   44  /* Magic number used to generate indexes. */

static string merge_string_vector(const vector<string>& sv)
{
    string ret;

    for (unsigned int i=0; i<sv.size(); i++) {
        ret += sv[i] + ", ";
    }

    return ret;
}

/* Insert a new entry into the table, if it does not exist already.
   Return the index associated to the new entry or the existing entry. */
unsigned int UnresolvedNames::insert(const string& name)
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

/* Add the name 'name' to the entry associated to the index 'idx', which
   must exist (otherwise an assertion fails).
   It may be that there is already an entry in the table which contains
   'name'. In such a case we must remove 'name' from this confilcting entry,
   merging the conflicting entry into the entry 'idx'.
   When a conflicting entry exists, its index is returned. Otherwise,
   this method returns ~0U.
 */
unsigned int UnresolvedNames::append(const string& name, unsigned int idx)
{
    unsigned int i = idx - FUNNY;

    assert(i < size());

    names[i].push_back(name);

    /* Look for 'name' in the other entries and remove it, because 'name'
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

/* Given an entry pointer, return the index contained in the entry. */
unsigned int UnresolvedNames::get_idx(unsigned int i) const
{
    if (i >= size() || !names[i].size()) {
        return ~0U;
    }

    return i + FUNNY;
}

/* Clear the table. */
void UnresolvedNames::clear()
{
    names.clear();
}

/* Given an entry pointer, return the names contained in the entry,
   concatenated together. */
string UnresolvedNames::get_name(unsigned int i) const
{
    if (i >= size() || !names[i].size()) {
        return string();
    }

    return merge_string_vector(names[i]);
}

/* Find the names corresponding to a given index, if any. */
string UnresolvedNames::lookup(unsigned int idx) const
{
    unsigned int i = idx - FUNNY;

    if (i >= names.size() || !names[i].size()) {
        return string();
    }

    return merge_string_vector(names[i]);
}

