#include <iostream>
#include <vector>
#include <fstream>

#include "tree.hpp"

using namespace std;
using namespace yy;


void int2string(int x, string& s)
{
    ostringstream oss;

    oss << x;
    s = oss.str();
}

yy::TreeNode::~TreeNode()
{
    for (unsigned int i=0; i<children.size(); i++)
        if (children[i])
            delete children[i];
}

string yy::TreeNode::getClassName() const
{
    return "TreeNode";
}

void yy::TreeNode::addChild(yy::TreeNode *n)
{
    children.push_back(n);
}

void yy::TreeNode::addChild(unsigned int t)
{
    children.push_back(new yy::TreeNode());
}

void yy::TreeNode::stealChildren(yy::TreeNode& n)
{
    for (unsigned int i=0; i<n.children.size(); i++) {
        children.push_back(n.children[i]);
    }
    n.children.clear();
}

void yy::TreeNode::print(ofstream& os)
{
    vector<TreeNode *> frontier;
    TreeNode *current;
    unsigned int pop = 0;
    bool print_nulls = false;  /* Do we want to print null nodes? (i.e. optional symbols) */

    os << "digraph G {\n";
    frontier.push_back(this);

    while (pop != frontier.size()) {
        string label = "NULL";

        current = frontier[pop];
        if (current) {
            LowerCaseIdNode *ln;
            UpperCaseIdNode *un;
            IntTreeNode *in;

            label = current->getClassName();
            ln = tree_downcast_safe<LowerCaseIdNode>(current);
            un = tree_downcast_safe<UpperCaseIdNode>(current);
            in = tree_downcast_safe<IntTreeNode>(current);
            assert(!(in && ln) && !(ln && un) && !(un && in));
            if (ln) {
                label = ln->saved;
            } else if (un) {
                label = un->saved;
            } else if (in) {
                int2string(in->value, label);
            }
        }
        if (current || print_nulls)
            os << pop << " [label=\"" << label << "\", style=filled];\n";
        if (current) {
            for (unsigned int i=0; i<current->children.size(); i++) {
                if (current->children[i] || print_nulls) {
                    frontier.push_back(current->children[i]);
                    os << pop << " -> " << frontier.size()-1 << " [label=\"\"];\n";
                }
            }
        }
        pop++;
    }

    os << "}\n";
}

int yy::TreeNode::translate_children()
{
    int ret = 0;

    for (unsigned int i=0; i<children.size(); i++) {
        if (children[i]) {
            ret = children[i]->translate();
            if (ret) {
                break;
            }
        }
    }

    return ret;
}

int yy::TreeNode::translate()
{
    return translate_children();
}

/* ============================== Translation methods =========================== */

int yy::RootNode::translate()
{
    return translate_children();
}

int yy::ProcessDefNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::ProcessIdNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::ProcessBodyNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::LocalProcessNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::ChoiceNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::ActionPrefixNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::PrefixActionsNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::BaseLocalProcessNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

int yy::ActionLabelsNode::translate()
{
    int ret = translate_children();

    if (ret)
        return ret;

    return 0;
}

