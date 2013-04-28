#ifndef __UTILS__HH
#define __UTILS__HH

#include <sstream>
#include <assert.h>

using namespace std;

#include "symbols_table.hpp"
#include "parser.hpp"

void semantic_error(const stringstream& ss, const struct YYLTYPE& loc);

/* Helper function used to get a ConstValue* from a SymbolValue*. If the
   object pointed is not a constant, a semantic error is issued. */
ConstValue* err_if_not_const(SymbolValue * svp, const struct YYLTYPE& loc);
RangeValue* err_if_not_range(SymbolValue * svp, const struct YYLTYPE& loc);
SetValue* err_if_not_set(SymbolValue * svp, const struct YYLTYPE& loc);
ProcessValue* err_if_not_process(SymbolValue * svp, const struct YYLTYPE& loc);
ProcnodePairValue* err_if_not_procnodepair(SymbolValue * svp, const struct YYLTYPE& loc);
ArgumentsValue* err_if_not_arguments(SymbolValue * svp, const struct YYLTYPE& loc);
RelabelingValue* err_if_not_relabeling(SymbolValue * svp, const struct YYLTYPE& loc);
HidingValue* err_if_not_hiding(SymbolValue * svp, const struct YYLTYPE& loc);
PriorityValue* err_if_not_priority(SymbolValue * svp, const struct YYLTYPE& loc);
ProcessNode * err_if_not_procnode(ProcessBase * pbp, const struct YYLTYPE& loc);

void err_if_not_const_svpvec(SvpVec * vp, const struct YYLTYPE& loc);



inline ConstValue* is_const(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Const);

    return static_cast<ConstValue *>(svp);
}

inline RangeValue* is_range(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Range);

    return static_cast<RangeValue *>(svp);
}

inline SetValue* is_set(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Set);

    return static_cast<SetValue *>(svp);
}

inline ProcessValue* is_process(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Process);

    return static_cast<ProcessValue *>(svp);
}

inline ProcnodePairValue* is_procnodepair(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::ProcnodePair);

    return static_cast<ProcnodePairValue *>(svp);
}

inline ArgumentsValue* is_arguments(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Arguments);

    return static_cast<ArgumentsValue *>(svp);
}

inline RelabelingValue* is_relabeling(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Relabeling);

    return static_cast<RelabelingValue *>(svp);
}

inline HidingValue* is_hiding(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Hiding);

    return static_cast<HidingValue *>(svp);
}

inline PriorityValue* is_priority(SymbolValue * svp)
{
    assert(svp->type() == SymbolValue::Priority);

    return static_cast<PriorityValue *>(svp);
}

inline ProcessNode * is_procnode(ProcessBase * pbp)
{
    assert(!(pbp->unresolved() || pbp->connected()));

    return static_cast<ProcessNode *>(pbp);
}

#endif
