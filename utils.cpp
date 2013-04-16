#include <iostream>
#include <cstdlib>
#include "utils.hpp"

void semantic_error(const stringstream& ss)
{
    cout << "Semantic error: " << ss.str() << "\n";
    exit(-1);
}

ConstValue* err_if_not_const(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Const) {
	stringstream errstream;
errstream << "Const expected";
	semantic_error(errstream);
    }

    return (ConstValue *)svp;
}

RangeValue* err_if_not_range(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Range) {
	stringstream errstream;
errstream << "Range expected";
	semantic_error(errstream);
    }

    return (RangeValue *)svp;
}

SetValue* err_if_not_set(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Set) {
	stringstream errstream;
errstream << "Set expected";
	semantic_error(errstream);
    }

    return (SetValue *)svp;
}

ProcessValue* err_if_not_process(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Process) {
	stringstream errstream;
errstream << "Process expected";
	semantic_error(errstream);
    }

    return (ProcessValue *)svp;
}

ProcnodePairValue* err_if_not_procnodepair(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::ProcnodePair) {
	stringstream errstream;
	errstream << "ProcnodePair expected";
	semantic_error(errstream);
    }

    return (ProcnodePairValue *)svp;
}

ProcessNode * err_if_not_procnode(ProcessBase * pbp)
{
    if (pbp->unresolved() || pbp->connected()) {
	stringstream errstream;
errstream << "ProcessNode expected";
	semantic_error(errstream);
    }
    return (ProcessNode *)pbp;
}
