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
            for (int i=0; n.children.size(); i++) {
                children.push_back(n.children[i]);
            }
            n.children.clear();
        }

        static const int BaseExpression         =  0;
        static const int Integer                =  1;
        static const int VariableId             =  2;
        static const int ConstantId             =  3;
        static const int RangeId                =  4;
        static const int SetId                  =  5;
        static const int ConstParameterId       =  6;
        static const int ParameterId            =  7;
        static const int ProcessId              =  8;
        static const int ProgressId             =  9;
        static const int Variable               = 10;
        static const int Expression             = 11;
        static const int Bang                   = 12;
        static const int Minus                  = 13;
        static const int Plus                   = 14;
        static const int Modulus                = 15;
        static const int Divide                 = 16;
        static const int Times                  = 17;
        static const int RightShift             = 18;
        static const int GOE                    = 19;
        static const int LOE                    = 20;
        static const int Greater               = 21;
        static const int Less                   = 22;
        static const int NotEqual               = 23;
        static const int Equal                  = 24;
        static const int BitAnd                 = 25;
        static const int BitXor                 = 26;
        static const int BitOr                  = 27;
        static const int LogicAnd               = 28;
        static const int LogicOr                = 29;
        static const int OpenParen              = 30;
        static const int CloseParen             = 31;
        static const int ProgressDef            = 32;
        static const int PropertyDef            = 33;
        static const int Property               = 34;
        static const int HidingInterf           = 35;
        static const int Hiding                 = 36;
        static const int Interf                 = 37;
        static const int RelabelDef             = 38;
        static const int Slash                  = 39;
        static const int Forall                 = 40;
        static const int RelabelDefs            = 41;
        static const int Comma                  = 42;
        static const int OpenCurly              = 43;
        static const int CloseCurly             = 44;
        static const int BracesRelabelDefs      = 45;
        static const int Parameter              = 46;
        static const int Assign                 = 47;
        static const int ParameterList          = 48;
        static const int Param                  = 49;
        static const int Colon                  = 50;
        static const int Labeling               = 51;
        static const int DoubleColon            = 52;
        static const int Sharing                = 53;
        static const int ProcessRef             = 54;
        static const int ParallelComp           = 55;
        static const int CompositeElse          = 56;
        static const int Else                   = 57;
        static const int CompositeBody          = 58;
        static const int If                     = 59;
        static const int Then                   = 60;
        static const int CompositeDef           = 61;
        static const int ArgumentList           = 62;
        static const int Arguments              = 63;
        static const int ProcessRefSeq          = 64;
        static const int SeqProcessList         = 65;
        static const int Semicolon              = 66;
        static const int SeqComp                = 67;
        static const int IndexRanges            = 68;
        static const int OpenSquare             = 69;
        static const int CloseSquare            = 70;
        static const int Indices                = 71;
        static const int Guard                  = 72;
        static const int When                   = 73;
        static const int PrefixActions          = 74;
        static const int Arrow                  = 75;
        static const int ActionPrefix           = 76;
        static const int Choice                 = 77;
        static const int BaseLocalProcess       = 78;
        static const int End                    = 79;
        static const int Stop                   = 80;
        static const int Error                  = 81;
        static const int ProcessElse            = 82;
        static const int LocalProcess           = 83;
        static const int AlphaExt               = 84;
        static const int LocalProcessDef        = 85;
        static const int LocalProcessDefs       = 86;
        static const int ProcessBody            = 87;
        static const int ProcessDef             = 88;
        static const int Period                 = 89;
        static const int SetElements            = 90;
        static const int SetDef                 = 91;
        static const int SetKwd                 = 92;
        static const int RangeDef               = 93;
        static const int RangeKwd               = 94;
        static const int DotDot                 = 95;
        static const int ConstDef               = 96;
        static const int ConstKwd               = 97;
        static const int RangeExpr              = 98;
        static const int ActionRange            = 99;
        static const int Range                  = 100;
        static const int SetExpr                = 101;
        static const int Set                    = 102;
        static const int ActionLabels           = 103;
        static const int LowerCaseId            = 104;
        static const int UpperCaseId            = 105;
        static const int Root                   = 106;
        static const int Priority               = 107;
        static const int LeftShift              = 108;
        static const int Relabeling             = 109;
        static const int ProgressKwd            = 110;
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

        StringTreeNode(unsigned int t, std::string *v) : TreeNode(t), value(v) { }
};

class SymbolTreeNode : public TreeNode {
    public:
        class SymbolValue *value;

        SymbolTreeNode(unsigned int t, class SymbolValue *v) : TreeNode(t), value(v) { }
};

class SvpVecTreeNode : public TreeNode {
    public:
        class SvpVec *value;

        SvpVecTreeNode(unsigned int t, class SvpVec *v) : TreeNode(t), value(v) { }
};

class PvecTreeNode : public TreeNode {
    public:
        class Pvec *value;

        PvecTreeNode(unsigned int t, class Pvec *v) : TreeNode(t), value(v) { }
};

class LtsTreeNode : public TreeNode {
    public:
        class yy::Lts *value;

        LtsTreeNode(unsigned int t, class yy::Lts *v)
                                    : TreeNode(t), value(v) { }
};

template <class T>
T* do_upcast(TreeNode * n)
{
    T *ret = dynamic_cast<T*>(n);

    assert(ret);

    return ret;
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

