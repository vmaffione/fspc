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
            StringTreeNode *sn;
            IntTreeNode *in;

            label = current->getClassName();
            sn = tree_downcast_safe<StringTreeNode>(current);
            in = tree_downcast_safe<IntTreeNode>(current);
            assert(!(sn && in));
            if (sn) {
                label = sn->saved;
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


int yy::TreeNode::translate()
{
#if 0
    switch (type) {
        case Root:
            for (unsigned int i=0; i<children.size(); i++) {
                int ret;

                assert(children[i]);
                ret = children[i]->translate();
                if (ret) {
                    return ret;
                }
            }
            break;

        default:
            cout << "Unimplemented: " << names[type] << "\n";
            return 0;
    }
#endif
    return 0;
}

