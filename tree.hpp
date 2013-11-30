#ifndef __TREE_HPP__
#define __TREE_HPP__

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "symbols_table.hpp"
#include "lts.hpp"
#include "context.hpp"
#include "location.hh"


struct FspDriver;

namespace yy {

/* Parse tree node base class. It derives from ParametricTranslator,
   so that a TreeNode* can be used with the ParametricProcess class. */
class TreeNode : public ParametricTranslator {
    protected:
        vector<TreeNode *> children;
        location loc;

        SetS computeActionLabels(FspDriver& c, SetS base,
                                     const vector<TreeNode*>& elements,
                                     unsigned int idx);
        yy::Lts *computePrefixActions(FspDriver& c,
                                     const vector<TreeNode *>& als,
                                     unsigned int idx,
                                     vector<Context>& ctxcache);
        void post_process_definition(FspDriver& c, Lts *res,
                                           const string& name);
        /* TODO when everything will work, remove this method and
           declare a ProcessRefBase class from which ProcessRefNode an
           ProcessRefSeq derive, and define the translate method of the
           base class with the code contained in process_ref_translate. */
        void process_ref_translate(FspDriver& c, yy::Lts **res);

    public:
        TreeNode() { }
        virtual ~TreeNode();
        void addChild(TreeNode *n, const location& loc);
        void print(ofstream& os);
        virtual void translate(FspDriver& dr);
        virtual void combination(FspDriver& dr, string index, bool first) { }
        virtual string getClassName() const;
        virtual void clear() { }
        void translate_children(FspDriver& dr);
        void clear_children();
};


/* ======================== FIRST DERIVATION LEVEL =========================
   The first derivation level adds the content to a parse tree node. This
   content can be an integer, a string, or other object types. */

class IntTreeNode : public TreeNode {
    public:
        int res;

        IntTreeNode() : TreeNode() { }
};

class FloatTreeNode : public TreeNode {
    public:
        float res;

        FloatTreeNode() : TreeNode() { }
};

class StringTreeNode : public TreeNode {
    public:
        std::string res;

        StringTreeNode() : TreeNode() { }
};

class LtsTreeNode : public TreeNode {
    public:
        yy::Lts *res;

        LtsTreeNode() : TreeNode() { }
        void clear() { /* XXX */ }
};


/* ======================== SECOND DERIVATION LEVEL ========================
   The second level of derivation adds a syntax meaning to a parse tree node:
   this means that the node corresponds to a terminal or non-terminal symbol
   in the FSP grammar. Each node is able to "translate" (towards LTS) the
   subtree rooted in itself. */

class BaseExpressionNode : public IntTreeNode {
    public:
        string getClassName() const { return "BaseExpression"; }
        BaseExpressionNode() : IntTreeNode() { }
        void translate(FspDriver &dr);
};

class IntegerNode : public IntTreeNode {
    public:
        string getClassName() const { return "Integer"; }
        IntegerNode() : IntTreeNode() { }
};

class LowerCaseIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "LowerCaseId"; }
        LowerCaseIdNode() : StringTreeNode() { }
};

class UpperCaseIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "UpperCaseId"; }
        UpperCaseIdNode() : StringTreeNode() { }
};

class VariableIdNode : public LowerCaseIdNode {
    public:
        string getClassName() const { return "VariableId"; }
        VariableIdNode() : LowerCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ConstantIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "ConstantId"; }
        ConstantIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class RangeIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "RangeId"; }
        RangeIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class SetIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "SetId"; }
        SetIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ConstParameterIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "ConstParameterId"; }
        ConstParameterIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ParameterIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "ParameterId"; }
        ParameterIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ProcessIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "ProcessId"; }
        ProcessIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ProgressIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "ProgressId"; }
        ProgressIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class MenuIdNode : public UpperCaseIdNode {
    public:
        string getClassName() const { return "MenuId"; }
        MenuIdNode() : UpperCaseIdNode() { }
        void translate(FspDriver &dr);
};

class ExpressionNode : public IntTreeNode {
    public:
        string getClassName() const { return "Expression"; }
        ExpressionNode() : IntTreeNode() { }
        void translate(FspDriver&);
};

class OperatorNode : public TreeNode {
    public:
        string sign;

        OperatorNode(const string& s) : TreeNode(), sign(s) { }
        string getClassName() const { return sign; }
};

class OpenParenNode : public TreeNode {
    public:
        string getClassName() const { return "("; }
        OpenParenNode() : TreeNode() { }
};

class CloseParenNode : public TreeNode {
    public:
        string getClassName() const { return ")"; }
        CloseParenNode() : TreeNode() { }
};

class ProgressDefNode : public TreeNode {
    public:
        string getClassName() const { return "ProgressDef"; }
        void translate(FspDriver& c);
        void combination(FspDriver& c, string index, bool first);
        ProgressDefNode() : TreeNode() { }
};

class PropertyDefNode : public TreeNode {
    public:
        string getClassName() const { return "PropertyDef"; }
        void translate(FspDriver& c);
        PropertyDefNode() : TreeNode() { }
};

class PropertyNode : public TreeNode {
    public:
        string getClassName() const { return "property"; }
        PropertyNode() : TreeNode() { }
};

class MenuDefNode : public TreeNode {
    public:
        string getClassName() const { return "MenuDef"; }
        void translate(FspDriver& c);
        MenuDefNode() : TreeNode() { }
};

class MenuKwdNode : public TreeNode {
    public:
        string getClassName() const { return "menu"; }
        MenuKwdNode() : TreeNode() { }
};

class HidingInterfNode : public TreeNode {
    public:
        HidingS res;

        string getClassName() const { return "HidingInterf"; }
        HidingInterfNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class HidingNode : public TreeNode {
    public:
        HidingNode() : TreeNode() { }
        string getClassName() const { return "Hiding"; }
};

class InterfNode : public TreeNode {
    public:
        InterfNode() : TreeNode() { }
        string getClassName() const { return "Interf"; }
};

class RelabelDefNode : public TreeNode {
    public:
        RelabelingS res;

        string getClassName() const { return "RelabelDef"; }
        RelabelDefNode() : TreeNode() { }
        void translate(FspDriver& c);
        void combination(FspDriver& dr, string index, bool first);
};

class SlashNode : public TreeNode {
    public:
        SlashNode() : TreeNode() { }
        string getClassName() const { return "Slash"; }
};

class ForallNode : public TreeNode {
    public:
        ForallNode() : TreeNode() { }
        string getClassName() const { return "forall"; }
};

class RelabelDefsNode : public TreeNode {
    public:
        RelabelingS res;

        RelabelDefsNode() : TreeNode() { }
        string getClassName() const { return "RelabelDefs"; }
        void translate(FspDriver& c);
};

class CommaNode : public TreeNode {
    public:
        CommaNode() : TreeNode() { }
        string getClassName() const { return ","; }
};

class OpenCurlyNode : public TreeNode {
    public:
        OpenCurlyNode() : TreeNode() { }
        string getClassName() const { return "{"; }
};

class CloseCurlyNode : public TreeNode {
    public:
        CloseCurlyNode() : TreeNode() { }
        string getClassName() const { return "}"; }
};

class BracesRelabelDefsNode : public TreeNode {
    public:
        RelabelingS res;

        string getClassName() const { return "BracesRelabelDefs"; }
        BracesRelabelDefsNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ParameterNode : public TreeNode {
    public:
        ParameterNode() : TreeNode() { } 
        string getClassName() const { return "Parameter"; }
        void translate(FspDriver& c);
};

class AssignNode : public TreeNode {
    public:
        AssignNode() : TreeNode() { }
        string getClassName() const { return "="; }
};

class ParameterListNode : public TreeNode {
    public:
        ParameterListNode() : TreeNode() { }
        string getClassName() const { return "ParameterList"; }
};

class ParamNode : public TreeNode {
    public:
        ParamNode() : TreeNode() { }
        string getClassName() const { return "Param"; }
};

class ColonNode : public TreeNode {
    public:
        ColonNode() : TreeNode() { }
        string getClassName() const { return ":"; }
};

class LabelingNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "Labeling"; }
        LabelingNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class DoubleColonNode : public TreeNode {
    public:
        DoubleColonNode() : TreeNode() { }
        string getClassName() const { return "::"; }
};

class SharingNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "Sharing"; }
        SharingNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ProcessRefNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ProcessRef"; }
        ProcessRefNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class ParallelCompNode : public TreeNode {
    public:
        vector<Lts*> res;

        string getClassName() const { return "ParallelComp"; }
        ParallelCompNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class CompositeElseNode : public LtsTreeNode {
    public:
        string getClassName() const { return "CompositeElse"; }
        CompositeElseNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class ElseNode : public TreeNode {
    public:
        ElseNode() : TreeNode() { } 
        string getClassName() const { return "else"; }
};

class CompositeBodyNode : public LtsTreeNode {
    public:
        string getClassName() const { return "CompositeBody"; }
        CompositeBodyNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
        void combination(FspDriver& dr, string index, bool first);
};

class IfNode : public TreeNode {
    public:
        IfNode() : TreeNode() { } 
        string getClassName() const { return "if"; }
};

class ThenNode : public TreeNode {
    public:
        ThenNode() : TreeNode() { } 
        string getClassName() const { return "Then"; }
};

class CompositeDefNode : public LtsTreeNode {
    public:
        CompositeDefNode() : LtsTreeNode() { }
        string getClassName() const { return "CompositeDef"; }
        void translate(FspDriver& c);
};

class ArgumentListNode : public TreeNode {
    public:
        vector<int> res;

        string getClassName() const { return "ArgumentList"; }
        ArgumentListNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ArgumentsNode : public TreeNode {
    public:
        vector<int> res;

        string getClassName() const { return "Arguments"; }
        ArgumentsNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ProcessRefSeqNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ProcessRefSeq"; }
        ProcessRefSeqNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class SeqProcessListNode : public LtsTreeNode {
    public:
        string getClassName() const { return "SeqProcessList"; }
        SeqProcessListNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class SemicolonNode : public TreeNode {
    public:
        SemicolonNode() : TreeNode() { }
        string getClassName() const { return ";"; }
};

class SeqCompNode : public LtsTreeNode {
    public:
        string getClassName() const { return "SeqComp"; }
        SeqCompNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class IndexRangesNode : public TreeNode {
    public:
        vector<TreeNode *> res;

        string getClassName() const { return "IndexRanges"; }
        IndexRangesNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class OpenSquareNode : public TreeNode {
    public:
        OpenSquareNode() : TreeNode() { } 
        string getClassName() const { return "["; }
};

class CloseSquareNode : public TreeNode {
    public:
        CloseSquareNode() : TreeNode() { } 
        string getClassName() const { return "]"; }
};

class IndicesNode : public StringTreeNode {
    public:
        string getClassName() const { return "Indices"; }
        IndicesNode() : StringTreeNode() { }
        void translate(FspDriver& c);
};

class GuardNode : public IntTreeNode {
    public:
        string getClassName() const { return "Guard"; }
        GuardNode() : IntTreeNode() { }
};

class WhenNode : public TreeNode {
    public:
        WhenNode() : TreeNode() { } 
        string getClassName() const { return "when"; }
};

class PrefixActionsNode : public TreeNode {
    public:
        vector<TreeNode *> res;

        string getClassName() const { return "PrefixActions"; }
        PrefixActionsNode() : TreeNode() { }
        void translate(FspDriver& dr);
};

class ArrowNode : public TreeNode {
    public:
        ArrowNode() : TreeNode() { } 
        string getClassName() const { return "->"; }
};

class ActionPrefixNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ActionPrefix"; }
        ActionPrefixNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class ChoiceNode : public LtsTreeNode {
    public:
        string getClassName() const { return "Choice"; }
        ChoiceNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class BaseLocalProcessNode : public LtsTreeNode {
    public:
        string getClassName() const { return "BaseLocalProcess"; }
        BaseLocalProcessNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class EndNode : public TreeNode {
    public:
        EndNode() : TreeNode() { }
        string getClassName() const { return "END"; }
};

class StopNode : public TreeNode {
    public:
        StopNode() : TreeNode() { } 
        string getClassName() const { return "STOP"; }
};

class ErrorNode : public TreeNode {
    public:
        ErrorNode() : TreeNode() { } 
        string getClassName() const { return "ERROR"; }
};

class ProcessElseNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ProcessElse"; }
        ProcessElseNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class LocalProcessNode : public LtsTreeNode {
    public:
        string getClassName() const { return "LocalProcess"; }
        LocalProcessNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class AlphaExtNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "AlphaExt"; }
        AlphaExtNode() : TreeNode() { }
        void translate(FspDriver& dr);
};

class LocalProcessDefNode : public LtsTreeNode {
    public:
        string getClassName() const { return "LocalProcessDef"; }
        LocalProcessDefNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
        void combination(FspDriver& dr, string index, bool first);
};

class LocalProcessDefsNode : public LtsTreeNode {
    public:
        string getClassName() const { return "LocalProcessDefs"; }
        LocalProcessDefsNode() : LtsTreeNode() { }
        void translate(FspDriver& c);
};

class ProcessBodyNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ProcessBody"; }
        ProcessBodyNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class ProcessDefNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ProcessDef"; }
        ProcessDefNode() : LtsTreeNode() { }
        void translate(FspDriver& dr);
};

class PeriodNode : public TreeNode {
    public:
        PeriodNode() : TreeNode() { } 
        string getClassName() const { return "."; }
};

class SetElementsNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "SetElements"; }
        SetElementsNode() : TreeNode() { }
        void translate(FspDriver&);
};

class SetDefNode : public TreeNode {
    public:
        SetDefNode() : TreeNode() { } 
        string getClassName() const { return "SetDef"; }
        void translate(FspDriver& c);
};

class SetKwdNode : public TreeNode {
    public:
        SetKwdNode() : TreeNode() { } 
        string getClassName() const { return "SetKwd"; }
};

class RangeDefNode : public TreeNode {
    public:
        RangeDefNode() : TreeNode() { } 
        string getClassName() const { return "RangeDef"; }
        void translate(FspDriver& c);
};

class RangeKwdNode : public TreeNode {
    public:
        RangeKwdNode() : TreeNode() { } 
        string getClassName() const { return "RangeKwd"; }
};

class DotDotNode : public TreeNode {
    public:
        DotDotNode() : TreeNode() { } 
        string getClassName() const { return ".."; }
};

class ConstantDefNode : public TreeNode {
    public:
        ConstantDefNode() : TreeNode() { } 
        string getClassName() const { return "ConstDef"; }
        void translate(FspDriver& c);
};

class ConstKwdNode : public TreeNode {
    public:
        ConstKwdNode() : TreeNode() { } 
        string getClassName() const { return "ConstKwd"; }
};

class RangeExprNode : public TreeNode {
    public:
        RangeS res;

        string getClassName() const { return "RangeExpr"; }
        RangeExprNode() : TreeNode() { }
        void translate(FspDriver&);
};

class ActionRangeNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "ActionRange"; }
        ActionRangeNode() : TreeNode() { }
        void translate(FspDriver&);
};

class RangeNode : public TreeNode {
    public:
        RangeS res;

        string getClassName() const { return "Range"; }
        RangeNode() : TreeNode() { }
        void translate(FspDriver&);
};

class SetExprNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "SetExpr"; }
        SetExprNode() : TreeNode() { }
        void translate(FspDriver&);
};

class SetNode : public TreeNode {
    public:
        SetS res;

        string getClassName() const { return "Set"; }
        SetNode() : TreeNode() { }
        void translate(FspDriver&);
};

class ActionLabelsNode : public TreeNode {
    public:
        vector<TreeNode *> res;

        string getClassName() const { return "ActionLabels"; }
        ActionLabelsNode() : TreeNode() { }
        void translate(FspDriver& dr);
};

class RootNode : public TreeNode {
    public:
        RootNode() : TreeNode() { }
        string getClassName() const { return "Root"; }
        void translate(FspDriver& dr);
};

class PrioritySNode : public TreeNode {
    public:
        PriorityS res;

        string getClassName() const { return "PriorityS"; }
        PrioritySNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class RelabelingNode : public TreeNode {
    public:
        RelabelingS res;

        string getClassName() const { return "Relabeling"; }
        RelabelingNode() : TreeNode() { }
        void translate(FspDriver& c);
};

class ProgressKwdNode : public TreeNode {
    public:
        ProgressKwdNode() : TreeNode() { } 
        string getClassName() const { return "ProgressKwd"; }
};


/* Useful wrappers for downcasting a TreeNode pointer to pointers to derived types. */
template <class T>
T* tree_downcast_safe(TreeNode *n)
{
    T *ret = dynamic_cast<T*>(n);

    return ret;
}

template <class T>
T* tree_downcast(TreeNode *n)
{
    T *ret = tree_downcast_safe<T>(n);

    assert(ret);

    return ret;
}

template <class T>
T* tree_downcast_null(TreeNode *n)
{
    if (!n)
        return NULL;
    return tree_downcast<T>(n);
}

/* Declare "_n" as a "_t"*, and assign to it the downcasted "_x". */
#define DTC(_t, _n, _x) \
    _t *_n = tree_downcast<_t>(_x);

/* Same as the previous one, but using the safe downcast function. */
#define DTCS(_t, _n, _x) \
    _t *_n = tree_downcast_safe<_t>(_x);


#define DO_DOWNCAST(_p, _t)  \
    ( { \
        TreeNode *n = _p; \
        _t *tmp = dynamic_cast<_t*>(n); \
        assert(tmp); \
        tmp; \
    } )

} /* namespace yy */

#endif

