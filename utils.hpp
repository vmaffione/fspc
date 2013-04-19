#ifndef __UTILS__HH
#define __UTILS__HH

#include <sstream>

using namespace std;

#include "symbols_table.hpp"

void semantic_error(const stringstream& ss);

/* Helper function used to get a ConstValue* from a SymbolValue*. If the
   object pointed is not a constant, a semantic error is issued. */
ConstValue* err_if_not_const(SymbolValue * svp);
RangeValue* err_if_not_range(SymbolValue * svp);
SetValue* err_if_not_set(SymbolValue * svp);
ProcessValue* err_if_not_process(SymbolValue * svp);
ProcnodePairValue* err_if_not_procnodepair(SymbolValue * svp);
ArgumentsValue* err_if_not_arguments(SymbolValue * svp);
LabelingSharingValue* err_if_not_labelingsharing(SymbolValue * svp);
RelabelingValue* err_if_not_relabeling(SymbolValue * svp);
ProcessNode * err_if_not_procnode(ProcessBase * pbp);

#endif
