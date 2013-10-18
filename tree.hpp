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

/* Parse tree node base class. */
class TreeNode {
    protected:
        vector<TreeNode *> children;
        location loc;

        SetValue computeActionLabels(FspDriver& c, SetValue base,
                                     const vector<TreeNode*>& elements,
                                     unsigned int idx);
        yy::Lts computePrefixActions(FspDriver& c,
                                     const vector<TreeNode *>& als,
                                     unsigned int idx);

    public:
        virtual ~TreeNode();
        void addChild(TreeNode *n);
        void addChild(unsigned int t);
        void print(ofstream& os);
        virtual void translate(FspDriver& dr);
        virtual string getClassName() const;
        void translate_children(FspDriver& dr);
};


/* ============================= FIRST DERIVATION LEVEL ==============================
   The first derivation level adds the content to a parse tree node. This content can be
   an integer, a string, or other object types. */

class IntTreeNode : public TreeNode {
    public:
        int value;
        int res;

        IntTreeNode(int v) : value(v), res(v) { }
};

class FloatTreeNode : public TreeNode {
    public:
        float value;
        float res;

        FloatTreeNode(float v) : value(v), res(v) { }
};

class StringTreeNode : public TreeNode {
    public:
        std::string *value;
        /* The memory pointed by "value" is freed during the parsing. We therefore use the
           "saved" field to retrieve the string after the parsing (e.g. when printing the
           parse tree. */
        std::string saved;
        std::string res;

        StringTreeNode(std::string *v) : value(v), saved(*v), res(*v) { }
};

/* XXX unused */
class SymbolTreeNode : public TreeNode {
    public:
        class SymbolValue *value;

        SymbolTreeNode(class SymbolValue *v) : value(v) { }
};

class SvpVecTreeNode : public TreeNode {
    public:
        class SvpVec *value;

        SvpVecTreeNode(class SvpVec *v) : value(v) { }
};

class PvecTreeNode : public TreeNode {
    public:
        yy::Lts res;
        class Pvec *value;

        PvecTreeNode(class Pvec *v) : value(v) { }
};

class LtsTreeNode : public TreeNode {
    public:
        yy::Lts res;
        yy::Lts *value;

        LtsTreeNode(yy::Lts *v) : value(v) { }
};


/* ============================= SECOND DERIVATION LEVEL =============================
   The second level of derivation adds a syntax meaning to a parse tree node: this means
   that the node corresponds to a terminal or non-terminal symbol in the FSP grammar.
   Each node is able to "translate" (towards LTS) the subtree rooted in itself. */

class BaseExpressionNode : public SvpVecTreeNode {
    public:
        int res;

        string getClassName() const { return "BaseExpression"; }
        BaseExpressionNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver &dr);
};

/* XXX unused */
class IntegerNode : public IntTreeNode {
    public:
        string getClassName() const { return "Integer"; }
        IntegerNode(int v) : IntTreeNode(v) { }

};

class VariableIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "VariableId"; }
        VariableIdNode(string *v) : StringTreeNode(v) { }
};

class ConstantIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "ConstantId"; }
        ConstantIdNode(string *v) : StringTreeNode(v) { }

};

class RangeIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "RangeId"; }
        RangeIdNode(string *v) : StringTreeNode(v) { }
};

class SetIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "SetId"; }
        SetIdNode(string *v) : StringTreeNode(v) { }
};

class ConstParameterIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "ConstParameterId"; }
        ConstParameterIdNode(string *v) : StringTreeNode(v) { }

};

class ParameterIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "ParameterId"; }
        ParameterIdNode(string *v) : StringTreeNode(v) { }

};

class ProcessIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "ProcessId"; }
        ProcessIdNode(string *v) : StringTreeNode(v) { }
};

class ProgressIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "ProgressId"; }
        ProgressIdNode(string *v) : StringTreeNode(v) { }

};

class VariableNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Variable"; }
        VariableNode(SvpVec *v) : SvpVecTreeNode(v) { }
};

class ExpressionNode : public SvpVecTreeNode {
    public:
        int res;

        string getClassName() const { return "Expression"; }
        ExpressionNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class OperatorNode : public TreeNode {
    public:
        string sign;
        OperatorNode(const string& s) : sign(s) { }
        string getClassName() const { return sign; }
};

class OpenParenNode : public TreeNode {
    public:
        string getClassName() const { return "("; }
};

class CloseParenNode : public TreeNode {
    public:
        string getClassName() const { return ")"; }
};

class ProgressDefNode : public TreeNode {
    public:
        string getClassName() const { return "ProgressDef"; }
};

class PropertyDefNode : public TreeNode {
    public:
        string getClassName() const { return "PropertyDef"; }
};

class PropertyNode : public TreeNode {
    public:
        string getClassName() const { return "property"; }
};

class HidingInterfNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "HidingInterf"; }
        HidingInterfNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class HidingNode : public TreeNode {
    public:
        string getClassName() const { return "Hiding"; }
};

class InterfNode : public TreeNode {
    public:
        string getClassName() const { return "Interf"; }
};

class RelabelDefNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "RelabelDef"; }
        RelabelDefNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class SlashNode : public TreeNode {
    public:
        string getClassName() const { return "Slash"; }
};

class ForallNode : public TreeNode {
    public:
        string getClassName() const { return "forall"; }
};

class RelabelDefsNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "RelabelDefs"; }
        RelabelDefsNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class CommaNode : public TreeNode {
    public:
        string getClassName() const { return ","; }
};

class OpenCurlyNode : public TreeNode {
    public:
        string getClassName() const { return "{"; }
};

class CloseCurlyNode : public TreeNode {
    public:
        string getClassName() const { return "}"; }
};

class BracesRelabelDefsNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "BracesRelabelDefs"; }
        BracesRelabelDefsNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ParameterNode : public TreeNode {
    public:
        string getClassName() const { return "Parameter"; }

};

class AssignNode : public TreeNode {
    public:
        string getClassName() const { return "="; }
};

class ParameterListNode : public TreeNode {
    public:
        string getClassName() const { return "ParameterList"; }
};

class ParamNode : public TreeNode {
    public:
        string getClassName() const { return "Param"; }
};

class ColonNode : public TreeNode {
    public:
        string getClassName() const { return ":"; }
};

class LabelingNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Labeling"; }
        LabelingNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class DoubleColonNode : public TreeNode {
    public:
        string getClassName() const { return "::"; }
};

class SharingNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Sharing"; }
        SharingNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ProcessRefNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "ProcessRef"; }
        ProcessRefNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ParallelCompNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "ParallelComp"; }
        ParallelCompNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class CompositeElseNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "CompositeElse"; }
        CompositeElseNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ElseNode : public TreeNode {
    public:
        string getClassName() const { return "else"; }
};

class CompositeBodyNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "CompositeBody"; }
        CompositeBodyNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class IfNode : public TreeNode {
    public:
        string getClassName() const { return "if"; }
};

class ThenNode : public TreeNode {
    public:
        string getClassName() const { return "Then"; }
};

class CompositeDefNode : public TreeNode {
    public:
        string getClassName() const { return "CompositeDef"; }

};

class ArgumentListNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "ArgumentList"; }
        ArgumentListNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ArgumentsNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Arguments"; }
        ArgumentsNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ProcessRefSeqNode : public PvecTreeNode {
    public:
        string getClassName() const { return "ProcessRefSeq"; }
        ProcessRefSeqNode(Pvec *v) : PvecTreeNode(v) { }

};

class SeqProcessListNode : public PvecTreeNode {
    public:
        string getClassName() const { return "SeqProcessList"; }
        SeqProcessListNode(Pvec *v) : PvecTreeNode(v) { }

};

class SemicolonNode : public TreeNode {
    public:
        string getClassName() const { return ";"; }
};

class SeqCompNode : public PvecTreeNode {
    public:
        string getClassName() const { return "SeqComp"; }
        SeqCompNode(Pvec *v) : PvecTreeNode(v) { }

};

class IndexRangesNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "IndexRanges"; }
        IndexRangesNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class OpenSquareNode : public TreeNode {
    public:
        string getClassName() const { return "["; }
};

class CloseSquareNode : public TreeNode {
    public:
        string getClassName() const { return "]"; }
};

class IndicesNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Indices"; }
        IndicesNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class GuardNode : public SvpVecTreeNode {
    public:
        int res;

        string getClassName() const { return "Guard"; }
        GuardNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class WhenNode : public TreeNode {
    public:
        string getClassName() const { return "when"; }
};

class PrefixActionsNode : public PvecTreeNode {
    public:
        vector<TreeNode *> res;
        string getClassName() const { return "PrefixActions"; }
        PrefixActionsNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class ArrowNode : public TreeNode {
    public:
        string getClassName() const { return "->"; }
};

class ActionPrefixNode : public PvecTreeNode {
    public:
        string getClassName() const { return "ActionPrefix"; }
        ActionPrefixNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class ChoiceNode : public PvecTreeNode {
    public:
        string getClassName() const { return "Choice"; }
        ChoiceNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class BaseLocalProcessNode : public PvecTreeNode {
    public:
        string getClassName() const { return "BaseLocalProcess"; }
        BaseLocalProcessNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class EndNode : public TreeNode {
    public:
        string getClassName() const { return "END"; }
};

class StopNode : public TreeNode {
    public:
        string getClassName() const { return "STOP"; }
};

class ErrorNode : public TreeNode {
    public:
        string getClassName() const { return "ERROR"; }
};

class ProcessElseNode : public PvecTreeNode {
    public:
        string getClassName() const { return "ProcessElse"; }
        ProcessElseNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class LocalProcessNode : public PvecTreeNode {
    public:
        string getClassName() const { return "LocalProcess"; }
        LocalProcessNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class AlphaExtNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "AlphaExt"; }
        AlphaExtNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class LocalProcessDefNode : public PvecTreeNode {
    public:
        string getClassName() const { return "LocalProcessDef"; }
        LocalProcessDefNode(Pvec *v) : PvecTreeNode(v) { }

};

class LocalProcessDefsNode : public PvecTreeNode {
    public:
        string getClassName() const { return "LocalProcessDefs"; }
        LocalProcessDefsNode(Pvec *v) : PvecTreeNode(v) { }

};

class ProcessBodyNode : public PvecTreeNode {
    public:
        string getClassName() const { return "ProcessBody"; }
        ProcessBodyNode(Pvec *v) : PvecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class ProcessDefNode : public LtsTreeNode {
    public:
        string getClassName() const { return "ProcessDef"; }
        ProcessDefNode(Lts *v) : LtsTreeNode(v) { }
        void translate(FspDriver& dr);

};

class PeriodNode : public TreeNode {
    public:
        string getClassName() const { return "."; }
};

class SetElementsNode : public SvpVecTreeNode {
    public:
        SetValue res;

        string getClassName() const { return "SetElements"; }
        SetElementsNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class SetDefNode : public TreeNode {
    public:
        string getClassName() const { return "SetDef"; }
};

class SetKwdNode : public TreeNode {
    public:
        string getClassName() const { return "SetKwd"; }
};

class RangeDefNode : public TreeNode {
    public:
        string getClassName() const { return "RangeDef"; }

};

class RangeKwdNode : public TreeNode {
    public:
        string getClassName() const { return "RangeKwd"; }

};

class DotDotNode : public TreeNode {
    public:
        string getClassName() const { return ".."; }
};

class ConstDefNode : public TreeNode {
    public:
        string getClassName() const { return "ConstDef"; }

};

class ConstKwdNode : public TreeNode {
    public:
        string getClassName() const { return "ConstKwd"; }

};

class RangeExprNode : public SvpVecTreeNode {
    public:
        RangeValue res;

        string getClassName() const { return "RangeExpr"; }
        RangeExprNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class ActionRangeNode : public SvpVecTreeNode {
    public:
        SetValue res;

        string getClassName() const { return "ActionRange"; }
        ActionRangeNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class RangeNode : public SvpVecTreeNode {
    public:
        RangeValue res;

        string getClassName() const { return "Range"; }
        RangeNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class SetExprNode : public SvpVecTreeNode {
    public:
        SetValue res;

        string getClassName() const { return "SetExpr"; }
        SetExprNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class SetNode : public SvpVecTreeNode {
    public:
        SetValue res;

        string getClassName() const { return "Set"; }
        SetNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver&);
};

class ActionLabelsNode : public SvpVecTreeNode {
    public:
        vector<TreeNode *> res;

        string getClassName() const { return "ActionLabels"; }
        ActionLabelsNode(SvpVec *v) : SvpVecTreeNode(v) { }
        void translate(FspDriver& dr);
};

class LowerCaseIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "LowerCaseId"; }
        LowerCaseIdNode(string *v) : StringTreeNode(v) { }

};

class UpperCaseIdNode : public StringTreeNode {
    public:
        string getClassName() const { return "UpperCaseId"; }
        UpperCaseIdNode(string *v) : StringTreeNode(v) { }

};

class RootNode : public TreeNode {
    public:
        string getClassName() const { return "Root"; }
        void translate(FspDriver& dr);
};

class PriorityNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Priority"; }
        PriorityNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class LeftShiftNode : public TreeNode {
    public:
        string getClassName() const { return "<<"; }
};

class RelabelingNode : public SvpVecTreeNode {
    public:
        string getClassName() const { return "Relabeling"; }
        RelabelingNode(SvpVec *v) : SvpVecTreeNode(v) { }

};

class ProgressKwdNode : public TreeNode {
    public:
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

