#ifndef __LTS__H__
#define __LTS__H__

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "strings_table.hpp"

using namespace std;


struct Edge {
    int dest;
    int action;
};

struct LtsNode {
    vector<Edge> children;
    unsigned int type; /* set if the node is an END node */

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
};

class Lts: public SymbolValue {
    vector<LtsNode> nodes;
    int ntr;
    bool valid;

    vector<int> alphabet;
    void updateAlphabet(int action);
    int lookupAlphabet(int action) const;

    void compositionReduce(const vector<LtsNode>& product);
	
  public:
    Lts() : ntr(0), valid(true) {}
    Lts(int); /* One state Lts: Stop, End or Error */
    Lts(const char * filename);
    Lts(const Lts& p, const Lts& q); /* Parallel composition */
    bool isValid() const { return valid; };
    int numStates() const { return nodes.size(); }
    int numTransitions() const { return ntr; }
    int deadlockAnalysis() const;
    int terminalSets() const;
    void compose(const Lts& p, const Lts& q);

    void print() const;
    int type() const { return SymbolValue::Lts; }
    SymbolValue * clone() const;
};

#endif
