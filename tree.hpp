#ifndef __TREE_HPP__
#define __TREE_HPP__

#include <iostream>
#include <vector>
#include <string>

#include "symbols_table.hpp"
#include "lts.hpp"
#include "location.hh"


namespace yy {

class TreeNode {
        unsigned int type;
        vector<TreeNode *> children;

    public:
        TreeNode(unsigned int t) : type(t) { }
        virtual ~TreeNode() { }
        void addChild(TreeNode *n) { children.push_back(n); }
        void addChild(unsigned int t) {
            children.push_back(new TreeNode(t));
        }
        void stealChildren(TreeNode& n) {
            for (unsigned int i=0; i<n.children.size(); i++) {
                children.push_back(n.children[i]);
            }
            n.children.clear();
        }

        /* All the possible type for a parse tree node. */
        enum Type {
            BaseExpression,
            Integer,
            VariableId,
            ConstantId,
            RangeId,
            SetId,
            ConstParameterId,
            ParameterId,
            ProcessId,
            ProgressId,
            Variable,
            Expression,
            Bang,
            Minus,
            Plus,
            Modulus,
            Divide,
            Times,
            RightShift,
            GOE,
            LOE,
            Greater,
            Less,
            NotEqual,
            Equal,
            BitAnd,
            BitXor,
            BitOr,
            LogicAnd,
            LogicOr,
            OpenParen,
            CloseParen,
            ProgressDef,
            PropertyDef,
            Property,
            HidingInterf,
            Hiding,
            Interf,
            RelabelDef,
            Slash,
            Forall,
            RelabelDefs,
            Comma,
            OpenCurly,
            CloseCurly,
            BracesRelabelDefs,
            Parameter,
            Assign,
            ParameterList,
            Param,
            Colon,
            Labeling,
            DoubleColon,
            Sharing,
            ProcessRef,
            ParallelComp,
            CompositeElse,
            Else,
            CompositeBody,
            If,
            Then,
            CompositeDef,
            ArgumentList,
            Arguments,
            ProcessRefSeq,
            SeqProcessList,
            Semicolon,
            SeqComp,
            IndexRanges,
            OpenSquare,
            CloseSquare,
            Indices,
            Guard,
            When,
            PrefixActions,
            Arrow,
            ActionPrefix,
            Choice,
            BaseLocalProcess,
            End,
            Stop,
            Error,
            ProcessElse,
            LocalProcess,
            AlphaExt,
            LocalProcessDef,
            LocalProcessDefs,
            ProcessBody,
            ProcessDef,
            Period,
            SetElements,
            SetDef,
            SetKwd,
            RangeDef,
            RangeKwd,
            DotDot,
            ConstDef,
            ConstKwd,
            RangeExpr,
            ActionRange,
            Range,
            SetExpr,
            Set,
            ActionLabels,
            LowerCaseId,
            UpperCaseId,
            Root,
            Priority,
            LeftShift,
            Relabeling,
            ProgressKwd,
        };
};

class IntTreeNode : public TreeNode {
    public:
        int value;

        IntTreeNode(unsigned int t, int v) : TreeNode(t), value(v) { }
};

class FloatTreeNode : public TreeNode {
    public:
        float value;

        FloatTreeNode(unsigned int t, float v) : TreeNode(t), value(v) { }
};

class StringTreeNode : public TreeNode {
    public:
        std::string *value;

        StringTreeNode(unsigned int t, std::string *v)
                                            : TreeNode(t), value(v) { }
};

class SymbolTreeNode : public TreeNode {
    public:
        class SymbolValue *value;

        SymbolTreeNode(unsigned int t, class SymbolValue *v)
                                            : TreeNode(t), value(v) { }
};

class SvpVecTreeNode : public TreeNode {
    public:
        class SvpVec *value;

        SvpVecTreeNode(unsigned int t, class SvpVec *v)
                                            : TreeNode(t), value(v) { }
};

class PvecTreeNode : public TreeNode {
    public:
        class Pvec *value;

        PvecTreeNode(unsigned int t, class Pvec *v)
                                            : TreeNode(t), value(v) { }
};

class LtsTreeNode : public TreeNode {
    public:
        class yy::Lts *value;

        LtsTreeNode(unsigned int t, class yy::Lts *v)
                                            : TreeNode(t), value(v) { }
};

template <class T>
T* tree_downcast(TreeNode *n)
{
    T *ret = dynamic_cast<T*>(n);

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

#define DO_DOWNCAST(_p, _t)  \
    ( { \
        TreeNode *n = _p; \
        _t *tmp = dynamic_cast<_t*>(n); \
        assert(tmp); \
        tmp; \
    } )

#endif

} /* namespace yy */

