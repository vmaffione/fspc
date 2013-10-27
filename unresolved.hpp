#ifndef __UNRESOLVED__HH__
#define __UNRESOLVED__HH__

#include <vector>
#include <string>

using namespace std;


/* This class contains a table representing unresolved names (in our case FSP
   names). Each entry is used to store the relationship between a numeric
   index (idx) and one or more names.
   As an example:
    =================================
    | idx  | names                  |
    =================================
    |  36  | P[3]                   |
    |  59  | P[8], Q[0]             |
    |  8   | Proc, Q[24], P[2][9]   |
    =================================

   Each index has two purposes:
        @ it is an alias for multiple names.
        @ it is easily stored in the Lts priv field.

   The indexes are automatically generated by the class itself. The user
   only provides the names.
 */
class UnresolvedNames {
        vector< vector<string> > names;

    public:
        unsigned int insert(const string& name);
        unsigned int append(const string& name, unsigned int idx);
        unsigned int size() const { return names.size(); }
        unsigned int get_idx(unsigned int i) const;
        string get_name(unsigned int i) const;
        string lookup(unsigned int idx) const;
        void clear();
};

#endif  /* __UNRESOLVED__HH__ */
