#include "smart_pointers.hpp"
#include <cassert>

using namespace std;


/* =========================== PtrCheckTable ========================= */

fsp::PtrCheckTable *fsp::PtrCheckTable::instance = NULL;

fsp::PtrCheckTable *fsp::PtrCheckTable::get()
{
    if (!instance) {
        instance = new PtrCheckTable;
    }

    return instance;
}

void fsp::PtrCheckTable::check()
{
    map<void *, unsigned int>::iterator it;

    DBR(cout << "Check for dangling references..\n");
    for (it = t.begin(); it != t.end(); it++) {
        DBR(cout << "[" << it->first << "] = " << it->second << "\n");
        assert(it->second == 0);
    }
    DBR(cout << "References are ok!\n");
}

void fsp::PtrCheckTable::ref(void *ptr)
{
    pair< map<void *, unsigned int>::iterator, bool > ret;

    ret = t.insert(make_pair(ptr, 1));
    if (!ret.second) {
        ret.first->second++;
    }
    DBR(cout << "++[" << ret.first->first << "] = " <<
                ret.first->second << "\n");
}

void fsp::PtrCheckTable::unref(void *ptr)
{
    map<void *, unsigned int>::iterator it;

    it = t.find(ptr);
    assert(it != t.end());
    assert(it->second > 0);
    it->second--;
    DBR(cout << "--[" << it->first << "] = " << it->second << "\n");
}
