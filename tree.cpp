#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "tree.hpp"
#include "driver.hpp"
#include "utils.hpp"

using namespace std;
using namespace yy;

string int2string(int x);
/*
void int2string(int x, string& s)
{
    ostringstream oss;

    oss << x;
    s = oss.str();
}*/

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
                label = int2string(in->value);
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
        SymbolValue *svp;
        ConstValue *cvp;

        if (!c.identifiers.lookup(cn->res, svp)) {
            return; // TODO
        }
        cvp = err_if_not_const(c, svp, loc);
        res = cvp->value;
    } else {
        assert(FALSE);
    }
}

void yy::RangeExprNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ExpressionNode, l, children[0]);
    DTC(ExpressionNode, r, children[2]);

    /* Build a range from two expressions. */
    res.low = l->res;
    res.high = r->res;
}

void yy::RangeNode::translate(FspDriver& c)
{
    translate_children(c);

    DTCS(RangeIdNode, ri, children[0]);
    DTCS(RangeExprNode, re, children[0]);

    if (ri) {
        /* Lookup the range identifier. */
        SymbolValue *svp;
        RangeValue *rvp;

        if (!c.identifiers.lookup(ri->res, svp)) {
            return; //TODO
        }
        rvp = err_if_not_range(c, svp, loc);
        res = *rvp;
    } else if (re) {
        /* Return the range expression. */
        res = re->res;
    } else {
        assert(FALSE);
    }
}

void yy::SetElementsNode::translate(FspDriver& c)
{
    translate_children(c);

    res = SetValue();

    do {
        DTC(ActionLabelsNode, an, children[0]);

        res += an->res;
    } while (0);

    for (unsigned int i=1; i<children.size(); i+=2) {
        DTC(ActionLabelsNode, an, children[i+1]);

        res += an->res;
    }
}

void yy::SetExprNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(SetElementsNode, sn, children[1]);

    res = sn->res;
}

void yy::SetNode::translate(FspDriver& c)
{
    translate_children(c);

    DTCS(SetIdNode, si, children[0]);
    DTCS(SetExprNode, se, children[0]);

    if (si) {
        /* Lookup the set identifier. */
        SymbolValue *svp;
        SetValue *setvp;

        if (!c.identifiers.lookup(si->res, svp)) {
            return; //TODO
        }
        setvp = err_if_not_set(c, svp, loc);
        res = *setvp;
    } else if (se) {
        /* Return the set expression. */
        res = se->res;
    } else {
        assert(FALSE);
    }
}

void yy::ActionRangeNode::translate(FspDriver& c)
{
    translate_children(c);

    res = SetValue();

    if (children.size() == 1) {
        /* Build a set of actions from an integer, a range or a set. */
        DTCS(ExpressionNode, en, children[0]);
        DTCS(RangeNode, rn, children[0]);
        DTCS(SetNode, sn, children[0]);

        if (en) {
            res += int2string(en->res);
        } else if (rn) {
            rn->res.set(res);
        } else if (sn) {
            res = sn->res;
        } else {
            assert(FALSE);
        }

    } else if (children.size() == 3) {
        /* Do the same with variable declarations. */
        DTC(VariableIdNode, vn, children[0]);
        DTCS(RangeNode, rn, children[2]);
        DTCS(SetNode, sn, children[2]);

        if (rn) {
            rn->res.set(res);
            res.variable = vn->res;
        } else if (sn) {
            res = sn->res;
            res.variable = vn->res;
        } else {
            assert(FALSE);
        }
if (res.actions.size() == 0) { // XXX remove a.s.a.p.
    cout << "XXX2\n";
    res += "xxx2";
}
    } else {
        assert(FALSE);
    }
}

void yy::ActionLabelsNode::translate(FspDriver& c)
{
    translate_children(c);

    /* This function builds a set of actions from an arbitrary complex
       label expression, e.g.
       a[1..2].b.{h,j,k}.c[3]
    */

    res = SetValue();

    /* The leftmost part of the label expression: Fill the set with
       a single action or a set of actions. */
    do {
        DTCS(StringTreeNode, strn, children[0]);
        DTCS(SetNode, setn, children[0]);

        if (strn) {
            /* Single action. */
            res += strn->res;
        } else if (setn) {
            /* A set of actions. */
            res += setn->res;
        } else {
            assert(FALSE);
        }
    } while (0);

    /* The rest of the expression. */
    for (unsigned int i=1; i<children.size();) {
        DTCS(PeriodNode, pn, children[i]);
        DTCS(OpenSquareNode, sqn, children[i]);

        if (pn) {
            /* Apply the "." operator. */
            DTCS(StringTreeNode, strn, children[i+1]);
            DTCS(SetNode, setn, children[i+1]);

            if (strn) {
                res.dotcat(strn->res);
            } else if (setn) {
                res.dotcat(setn->res);
            } else {
                assert(FALSE);
            }
            i += 2;
        } else if (sqn) {
            /* Apply the "[]" operator. */
            DTC(ActionRangeNode, an, children[i+1]);

            res.indexize(an->res);
            i += 3;
        } else {
            assert(FALSE);
            break;
        }
    }
    //cout << "Actions labels: "; res.print(); cout << "\n"; // XXX debug
}

