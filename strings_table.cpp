#include <cstring>
#include "strings_table.h"



//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif

int StringsTable::lookup(const char * s) const
{
    for (int i=0; i<table.size(); i++)
	if (strcmp(s, table[i]) == 0)
	    return i;
    return -1;
}

int StringsTable::insert(const char * s)
{
    int i = lookup(s);
    char * sc = strdup(s);

    if (i == -1) {
	table.push_back(sc); // This makes a copy
	IFD(cout << "Added '" << sc << "' to the actions table ("
		<< table.size()-1 << ")\n");
	return table.size() - 1;
    }
    return i;
}

void StringsTable::print() const
{
    cout << "Actions table:\n";
    for (int i=0; i<table.size(); i++)
	cout << "  " << table[i] << "\n";
}
