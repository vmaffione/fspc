#include <iostream>
#include <vector>

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
    "Bang",
    "Minus",
    "Plus",
    "Modulus",
    "Divide",
    "Times",
    "RightShift",
    "GOE",
    "LOE",
    "Greater",
    "Less",
    "NotEqual",
    "Equal",
    "BitAnd",
    "BitXor",
    "BitOr",
    "LogicAnd",
    "LogicOr",
    "OpenParen",
    "CloseParen",
    "ProgressDef",
    "PropertyDef",
    "Property",
    "HidingInterf",
    "Hiding",
    "Interf",
    "RelabelDef",
    "Slash",
    "Forall",
    "RelabelDefs",
    "Comma",
    "OpenCurly",
    "CloseCurly",
    "BracesRelabelDefs",
    "Parameter",
    "Assign",
    "ParameterList",
    "Param",
    "Colon",
    "Labeling",
    "DoubleColon",
    "Sharing",
    "ProcessRef",
    "ParallelComp",
    "CompositeElse",
    "Else",
    "CompositeBody",
    "If",
    "Then",
    "CompositeDef",
    "ArgumentList",
    "Arguments",
    "ProcessRefSeq",
    "SeqProcessList",
    "Semicolon",
    "SeqComp",
    "IndexRanges",
    "OpenSquare",
    "CloseSquare",
    "Indices",
    "Guard",
    "When",
    "PrefixActions",
    "Arrow",
    "ActionPrefix",
    "Choice",
    "BaseLocalProcess",
    "End",
    "Stop",
    "Error",
    "ProcessElse",
    "LocalProcess",
    "AlphaExt",
    "LocalProcessDef",
    "LocalProcessDefs",
    "ProcessBody",
    "ProcessDef",
    "Period",
    "SetElements",
    "SetDef",
    "SetKwd",
    "RangeDef",
    "RangeKwd",
    "DotDot",
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
    "LeftShift",
    "Relabeling",
    "ProgressKwd",
};

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

void yy::TreeNode::print()
{
    vector<TreeNode *> frontier;

    frontier.push_back(this);

    cout << names[this->type] << "\n";
}

