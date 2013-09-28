#ifndef __TREE_HPP__
#define __TREE_HPP__

#include <iostream>
#include <vector>
#include <string>

#include "symbols_table.hpp"
#include "lts.hpp"
#include "location.hh"


class TreeNode {
        unsigned int type;

    public:
        TreeNode(unsigned int t) : type(t) { }

        static const int TreeNodeInt = 0;
        static const int TreeNodeFloat = 1;
        static const int TreeNodeString = 2;
        static const int TreeNodeSymbol = 3;
        static const int TreeNodeSvpVec = 4;
        static const int TreeNodePvec = 5;
        static const int TreeNodeLts = 6;
};

class IntTreeNode : public TreeNode {
        int value;

    public:
        IntTreeNode(unsigned int t, int v) : TreeNode(t), value(v) { }
};

class FloatTreeNode : public TreeNode {
        float value;

    public:
        FloatTreeNode(unsigned int t, float v) : TreeNode(t), value(v) { }
};

class StringTreeNode : public TreeNode {
        std::string *value;

    public:
        StringTreeNode(unsigned int t, std::string *v) : TreeNode(t), value(v) { }
};

class SymbolTreeNode : public TreeNode {
        class SymbolValue *value;

    public:
        SymbolTreeNode(unsigned int t, class SymbolValue *v) : TreeNode(t), value(v) { }
};

class SvpVecTreeNode : public TreeNode {
        class SvpVec *value;

    public:
        SvpVecTreeNode(unsigned int t, class SvpVec *v) : TreeNode(t), value(v) { }
};

class PvecTreeNode : public TreeNode {
        class Pvec *value;

    public:
        PvecTreeNode(unsigned int t, class Pvec *v) : TreeNode(t), value(v) { }
};

class LtsTreeNode : public TreeNode {
        class yy::Lts *value;

    public:
        LtsTreeNode(unsigned int t, class yy::Lts *v) : TreeNode(t), value(v) { }
};

#endif

