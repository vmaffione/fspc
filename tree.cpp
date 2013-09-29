#include <iostream>
#include <vector>

#include "tree.hpp"

using namespace std;



void yy::TreeNode::addChild(yy::TreeNode *n)
{
    children.push_back(n);
}

void yy::TreeNode::addChild(unsigned int t)
{
    children.push_back(new yy::TreeNode(t));
}

void yy::TreeNode::stealChildren(yy::TreeNode& n)
{
    for (unsigned int i=0; i<n.children.size(); i++) {
        children.push_back(n.children[i]);
    }
    n.children.clear();
}

