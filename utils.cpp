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

ArgumentsValue* err_if_not_arguments(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Arguments) {
	stringstream errstream;
	errstream << "Arguments expected";
	semantic_error(errstream);
    }

    return (ArgumentsValue *)svp;
}

LabelingSharingValue* err_if_not_labelingsharing(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::LabelingSharing) {
	stringstream errstream;
	errstream << "LabelingSharing expected";
	semantic_error(errstream);
    }

    return (LabelingSharingValue *)svp;
}

RelabelingValue* err_if_not_relabeling(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Relabeling) {
	stringstream errstream;
	errstream << "Relabeling expected";
	semantic_error(errstream);
    }

    return (RelabelingValue *)svp;
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
