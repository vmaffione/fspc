#ifndef __STRINGS__TABLE__H__
#define __STRINGS__TABLE__H__

#include <iostream>
#include <vector>
#include <string>
#include <map>


using namespace std;



string int2string(int x);

struct ActionsTable {
    string name;
    map<string, int> table;
    vector<string> reverse;
    int serial;

    ActionsTable(const string& nm) : serial(0), name(nm) { }
    int insert(const string& s);
    int lookup(const string& s) const;
    void print() const;
};


struct SymbolValue {
    virtual void print() const { };
    virtual int type() const = 0;
    virtual SymbolValue * clone() const = 0;
    virtual int setVariable(const string& s) { return -1; }

    static const int Const = 0;
    static const int Range = 1;
    static const int Set = 2;
    static const int Lts = 3;
    static const int Process = 4;
    static const int ParametricProcess = 5;
    static const int ProcnodePair = 6;
    static const int Arguments = 7;
    static const int LabelingSharing = 8;
    static const int Relabeling = 9;
};

/* Class that supports a list of SymbolValue*. */
struct SvpVec {
    vector<SymbolValue *> v;
    bool shared;

    SvpVec();
    SymbolValue * detach(int i);
    void detach_all();
    void print();
    ~SvpVec();
};

struct ConstValue: public SymbolValue {
    int value;

    void print() const { cout << value; }
    int type() const { return SymbolValue::Const; }
    SymbolValue * clone() const;
};

struct RangeValue: public SymbolValue {
    int low;
    int high;
    string variable;

    void print() const { cout << "[" << low << ", " << high << "]"; }
    int type() const { return SymbolValue::Range; }
    SymbolValue * clone() const;
    virtual int setVariable(const string& s) { variable = s; return 0; }
};

struct SetValue: public SymbolValue {
    vector<string> actions;
    string variable;
    int rank;
    
    SetValue() : rank(0) { }
    void print() const;
    int type() const { return SymbolValue::Set; }
    SymbolValue * clone() const;
    virtual int setVariable(const string& s) { variable = s; return 0; }

    SetValue& dotcat(const string&);
    SetValue& dotcat(const SetValue&);
    SetValue& indexize(int index);
    SetValue& indexize(const SetValue&);
    SetValue& indexize(int low, int high);
    SetValue& operator +=(const SetValue&);
    SetValue& operator +=(const string&);
};

struct ArgumentsValue: public SymbolValue {
    vector<int> args;

    void print() const;
    int type() const { return SymbolValue::Arguments; }
    SymbolValue * clone() const;
};

struct LabelingSharingValue: public SymbolValue {
    SetValue * labeling;
    SetValue * sharing;

    LabelingSharingValue() : labeling(NULL), sharing(NULL) { }
    void print() const;
    int type() const { return SymbolValue::LabelingSharing; }
    SymbolValue * clone() const;
    ~LabelingSharingValue() { delete labeling; delete sharing; }
};

struct RelabelingValue: public SymbolValue {
    vector<SetValue *> old_labels;
    vector<SetValue *> new_labels;

    void add(SetValue *, SetValue *);
    void merge(RelabelingValue& rlv);
    void detach_all();
    int size() const { return old_labels.size(); }
    void print() const;
    int type() const { return SymbolValue::Relabeling; }
    SymbolValue * clone() const;
    ~RelabelingValue();
};

struct ProcessBase {
    virtual bool unresolved() const = 0;
    virtual bool connected() const { return false; }
    virtual void print(ActionsTable * atp) = 0; 
};

struct Pvec {
    vector<ProcessBase *> v;

    void print(struct ActionsTable * atp);
};

struct ProcessNode;

struct ProcessEdge {
    ProcessNode * dest;
    int action;
    int rank;
    string unresolved_reference;
};

typedef void (*ProcessVisitFunction)(struct ProcessNode *,
		    void *);

struct ProcessVisitObject {
    ProcessVisitFunction vfp;
    void * opaque;
};

struct ProcessNode: public ProcessBase {
    vector<ProcessEdge> children;
    int type;

    ProcessNode() : type(ProcessNode::Normal) { }
    ProcessNode(int t) : type(t) { }
    void print(ActionsTable * atp);
    ProcessNode * clone() const;
    bool unresolved() const { return false; }
    void visit(struct ProcessVisitObject);

    static const int Normal = 0;
    static const int End = 1;
    static const int Error = 2;
};

void freeProcessNodeGraph(struct ProcessNode *);

struct UnresolvedProcess: public ProcessBase {
    string reference;

    UnresolvedProcess(const string& s) : reference(s) { }
    bool unresolved() const { return true; }
    void print(ActionsTable * atp) { cout << "Unres " << reference << "\n"; }
};

struct ConnectedProcess: public ProcessBase {
    bool unresolved() const { return false; }
    bool connected() const {return true; }
    void print(ActionsTable * atp) { cout << "Connected\n"; }
};

class ProcessNodeAllocator {
    vector<ProcessBase *> nodes;

  public:
    ProcessNode * allocate(int type);
    ConnectedProcess * allocate_connected();
    UnresolvedProcess * allocate_unresolved(const string& name);
    void clear();
};

struct ProcessValue: public SymbolValue {
    /* A pointer to the first ProcessNode. */
    struct ProcessNode * pnp;

    ProcessValue() : pnp(NULL) { }
    void print(ActionsTable * atp) const { if(pnp) pnp->print(atp); }
    int type() const { return SymbolValue::Process; }
    SymbolValue * clone() const;
};

struct ProcnodePairValue: public SymbolValue {
    struct ProcessNode * first;
    struct ProcessNode * second;

    ProcnodePairValue() : first(NULL), second(NULL) { }
    ProcnodePairValue(ProcessNode * h, ProcessNode * t) : first(h), second(t) { }
    void print() const { cout << first << "," << second; }
    int type() const { return SymbolValue::ProcnodePair; }
    SymbolValue * clone() const;
};

struct SymbolsTable {
    map<string, SymbolValue*> table;

    bool insert(const string& name, SymbolValue *);
    bool lookup(const string& name, SymbolValue*&) const;
    bool remove(const string& name);
    void clear();
    void print() const;
};

#endif
