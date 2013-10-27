#ifndef __UNRESOLVED__HH__
#define __UNRESOLVED__HH__

#include <vector>
#include <string>

using namespace std;


class UnresolvedNames {
        vector< vector<string> > names;

    public:
        unsigned int add(const string& name);
        unsigned int add(const string& name, unsigned int idx);
        //unsigned int lookup(const string& name);
        unsigned int size() const;
        unsigned int get_idx(unsigned int i) const;
        string get_name(unsigned int i) const;
        string lookup(unsigned int idx) const;
        void clear();
};

#endif  /* __UNRESOLVED__HH__ */
