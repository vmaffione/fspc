#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;


struct Edge {
    int dest;
    int action;
};

struct LtsNode {
    vector<Edge> children;
    bool end; /* set if the node is an END node */
};

class Lts {
    string name;
    vector<LtsNode> nodes;
    int ntr;
    bool valid;

    vector<int> alphabet;
    void updateAlphabet(int action);
    int lookupAlphabet(int action) const;

    void compositionReduce(const vector<LtsNode>& product);
	
  public:
    Lts(string nm) : name(nm), ntr(0), valid(true) {}
    Lts(string nm, const char * filename);
    Lts(string nm, const Lts& p, const Lts& q); /* Parallel composition */
    bool isValid() const { return valid; };
    void print() const;
    int numStates() const { return nodes.size(); }
    int numTransitions() const { return ntr; }
    int deadlockAnalysis() const;
    int terminalSets() const;

    void compose(const Lts& p, const Lts& q);
};

