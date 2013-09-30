#include <iostream>
#include <vector>
#include <fstream>

#include "tree.hpp"

using namespace std;


static const char* names[] = {
    "BaseExpression",
    "Integer",
    "VariableId",
    "ConstantId",
    "RangeId",
    "SetId",
    "ConstParameterId",
    "ParameterId",
    "ProcessId",
    "ProgressId",
    "Variable",
    "Expression",
    "!",
    "-",
    "+",
    "%",
    "/",
    "*",
    ">>",
    ">=",
    "<=",
    ">",
    "<",
    "!=",
    "==",
    "&",
    "^",
    "|",
    "&&",
    "||",
    "(",
    ")",
    "ProgressDef",
    "PropertyDef",
    "Property",
    "HidingInterf",
    "Hiding",
    "Interf",
    "RelabelDef",
    "Slash",
    "forall",
    "RelabelDefs",
    ",",
    "{",
    "}",
    "BracesRelabelDefs",
    "Parameter",
    "=",
    "ParameterList",
    "Param",
    ":",
    "Labeling",
    "::",
    "Sharing",
    "ProcessRef",
    "ParallelComp",
    "CompositeElse",
    "else",
    "CompositeBody",
    "if",
    "then",
    "CompositeDef",
    "ArgumentList",
    "Arguments",
    "ProcessRefSeq",
    "SeqProcessList",
    ";",
    "SeqComp",
    "IndexRanges",
    "[",
    "]",
    "Indices",
    "Guard",
    "when",
    "PrefixActions",
    "->",
    "ActionPrefix",
    "Choice",
    "BaseLocalProcess",
    "END",
    "STOP",
    "ERROR",
    "ProcessElse",
    "LocalProcess",
    "AlphaExt",
    "LocalProcessDef",
    "LocalProcessDefs",
    "ProcessBody",
    "ProcessDef",
    ".",
    "SetElements",
    "SetDef",
    "SetKwd",
    "RangeDef",
    "RangeKwd",
    "..",
    "ConstDef",
    "ConstKwd",
    "RangeExpr",
    "ActionRange",
    "Range",
    "SetExpr",
    "Set",
    "ActionLabels",
    "LowerCaseId",
    "UpperCaseId",
    "Root",
    "Priority",
    "<<",
    "Relabeling",
    "ProgressKwd",
};

void int2string(int x, string& s)
{
    ostringstream oss;

    oss << x;
    s = oss.str();
}

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

            switch (current->type) {
                case UpperCaseId:
                case LowerCaseId:
                    sn = tree_downcast<StringTreeNode>(current);
                    label = sn->saved;
                    break;

                case Integer:
                    in = tree_downcast<IntTreeNode>(current);
                    int2string(in->value, label);
                    break;

                default:
                    label = names[current->type];
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

