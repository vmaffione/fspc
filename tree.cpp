#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "tree.hpp"
#include "driver.hpp"

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

void yy::TreeNode::translate_children(FspDriver& c)
{
    for (unsigned int i=0; i<children.size(); i++) {
        if (children[i]) {
            children[i]->translate(c);
        }
    }
}

void yy::TreeNode::translate(FspDriver& c)
{
    translate_children(c);
}

/* ========================== Translation methods ======================== */

#define FALSE 3  /* TODO when everything works, switch to "0". */
#define IMPLEMENT 718  /* TODO when everything works, switch to "0". */

void yy::RootNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::ProcessDefNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::ProcessIdNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::ProcessBodyNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::LocalProcessNode::translate(FspDriver& c)
{
    translate_children(c);

    if (children.size() == 1) {
        DTCS(BaseLocalProcessNode, b, children[0]);

        if (b) {
            res = b->res;
        } else {
            assert(FALSE);
        }
    } else {
        assert(IMPLEMENT);
    }
}

void yy::ChoiceNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::ActionPrefixNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::PrefixActionsNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::BaseLocalProcessNode::translate(FspDriver& c)
{
    translate_children(c);

    DTCS(EndNode, en, children[0]);
    DTCS(StopNode, sn, children[0]);
    DTCS(ErrorNode, ern, children[0]);

    if (en) {
        res = Lts(LtsNode::End, &c.actions);
    } else if (sn) {
        res = Lts(LtsNode::Normal, &c.actions);
    } else if (ern) {
        res = Lts(LtsNode::Error, &c.actions);
    } else {
        assert(FALSE);
    }
}

void yy::ActionLabelsNode::translate(FspDriver& c)
{
    translate_children(c);

    if (children.size() == 1) {
        //StringTreeNode *sn = tree_downcast_safe<StringTreeNode>(children[0]);
    } else {
        assert(IMPLEMENT);
    }
}

void yy::ExpressionNode::translate(FspDriver& c)
{
    translate_children(c);

    if (children.size() == 1) {
        DTC(BaseExpressionNode, en, children[0]);

        res = en->res;
    } else if (children.size() == 2) {
        /* OPERATOR EXPR */
        DTC(OperatorNode, on, children[0]);
        DTC(BaseExpressionNode, en, children[1]);

        if (on->sign == "+") {
            res = en->res;
        } else if (on->sign == "-") {
            res = -en->res;
        } else if (on->sign == "!") {
            res = !en->res;
        } else {
            assert(FALSE);
        }
        
    } else if (children.size() == 3) {
        DTCS(OpenParenNode, pn, children[0]);

        if (pn) {
            /* ( EXPR ) */
            DTC(ExpressionNode, en, children[1]);

            res = en->res;
        } else {
            /* EXPR OPERATOR EXPR */
            DTC(ExpressionNode, l, children[0]);
            DTC(OperatorNode, o, children[1]);
            DTC(ExpressionNode, r, children[2]);

            if (o->sign == "||") {
                res = l->res || r->res;
            } else if (o->sign == "&&") {
                res = l->res && r->res;
            } else if (o->sign == "|") {
                res = l->res | r->res;
            } else if (o->sign == "^") {
                res = l->res ^ r->res;
            } else if (o->sign == "&") {
                res = l->res & r->res;
            } else if (o->sign == "==") {
                res = (l->res == r->res);
            } else if (o->sign == "!=") {
                res = (l->res != r->res);
            } else if (o->sign == "<") {
                res = (l->res < r->res);
            } else if (o->sign == "<=") {
                res = (l->res <= r->res);
            } else if (o->sign == ">=") {
                res = (l->res >= r->res);
            } else if (o->sign == "<<") {
                res = l->res << r->res;
            } else if (o->sign == ">>") {
                res = l->res >> r->res;
            } else if (o->sign == "+") {
                res = l->res + r->res;
            } else if (o->sign == "-") {
                res = l->res - r->res;
            } else if (o->sign == "*") {
                res = l->res * r->res;
            } else if (o->sign == "/") {
                res = l->res / r->res;
            } else if (o->sign == "%") {
                res = l->res % r->res;
            } else {
                assert(FALSE);
            }
        }
        
    } else {
        assert(FALSE);
    }
}

void yy::BaseExpressionNode::translate(FspDriver& c)
{
    translate_children(c);

    DTCS(IntTreeNode, in, children[0]);
    DTCS(VariableIdNode, vn, children[0]);
    DTCS(ConstParameterIdNode, cn, children[0]);

    if (in) {
        res = in->res;
    } else if (vn) {
        res = -1;
        if (!c.ctxset.lookup(vn->res, res)) {
            
        }
    } else if (cn) {
    } else {
        assert(FALSE);
    }
}

void yy::RangeExprNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ExpressionNode, l, children[0]);
    DTC(ExpressionNode, r, children[2]);

    res.low = l->res;
    res.high = r->res;
}
