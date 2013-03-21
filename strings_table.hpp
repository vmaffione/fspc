#ifndef __STRINGS__TABLE__H__
#define __STRINGS__TABLE__H__

#include <iostream>
#include <vector>
#include <string>

using namespace std;


struct StringsTable {
    vector<char *> table;

    int insert(const char *);
    int lookup(const char *) const;
    void print() const;
};

#endif
