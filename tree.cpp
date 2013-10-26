#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "tree.hpp"
#include "driver.hpp"
#include "utils.hpp"
#include "unresolved.hpp"

using namespace std;
using namespace yy;


string int2string(int x);
int string2int(const string& s);
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
        string val;

        if (!c.ctx.lookup(vn->res, val)) {
            res = -1;
            return; // TODO error
        }
        res = string2int(val);
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

/* This recursive method can be used to compute the set of action defined
   by an arbitrary complex label expression, e.g.
        'a[i:1..2].b.{h,j,k}.c[3][j:i..2*i][j*i+3]'
    The caller should pass a SetValue object built using the
    default constructor (e.g. an empty SetValue) to 'base', and 0 to 'idx'.
    The elements vector contains pointers to strings, sets or action ranges.
*/
SetValue yy::TreeNode::computeActionLabels(FspDriver& c, SetValue base,
                                           const vector<TreeNode*>& elements,
                                           unsigned int idx)
{
    assert(idx < elements.size());

    /* Here we do the translation that was deferred in the lower layers.
       This is necessary because of context expansion: When an action range
       defines a variable in the middle of a label expression, that variable
       can influence the translation of the expression elements which are on
       the right of the variable definition: In these cases, we need to
       retranslate those elements on the right many times, once for each
       possibile variable value.
    */
    elements[idx]->translate(c);
    if (idx == 0) {
        /* This is the first element of a label expression. We set 'base'
           to its initial value. */
        DTCS(StringTreeNode, strn, elements[0]);
        DTCS(SetNode, setn, elements[0]);

        base = SetValue();
        if (strn) {
            /* Single action. */
            base += strn->res;
         } else if (setn) {
            /* A set of actions. */
            base += setn->res;
        } else {
            assert(FALSE);
        }
    } else {
        /* Here we are in the middle (or the end) of a label expression.
           We use the dotcat() or indexize() method to extend the current
           'base'. */
        DTCS(StringTreeNode, strn, elements[idx]);
        DTCS(SetNode, setn, elements[idx]);
        DTCS(ActionRangeNode, an, elements[idx]);

        if (strn) {
            base.dotcat(strn->res);
        } else if (setn) {
            base.dotcat(setn->res);
        } else if (an) {
            if (!an->res.hasVariable() || idx+1 >= elements.size()) {
                /* When an action range doesn't define a variable, or when
                   such a declaration is useless since this is the end of
                   the expression, we just extend the current 'base'. */
                base.indexize(an->res);
            } else {
                /* When an action range does define a variable, we must split
                   the computation in N parts, one for each action in the
                   action range, and then concatenate all the results. For
                   each part, we extend the current 'base' with only an
                   action, insert the variable into the context and do a
                   recursive call. */
                SetValue ret;
                SetValue next_base;
                bool ok;

                for (unsigned int j=0; j<an->res.actions.size(); j++) {
                    next_base = base;
                    next_base.indexize(an->res.actions[j]);
                    if (!c.ctx.insert(an->res.variable, an->res.actions[j])) {
                        cout << "ERROR: ctx.insert()\n";
                    }
                    ret += computeActionLabels(c, next_base,
                                               elements, idx+1);
                    ok = c.ctx.remove(an->res.variable);
                    assert(ok);
                }
                return ret;
            }
        } else {
            assert(FALSE);
        }
    }

    if (idx+1 >= elements.size()) {
        /* If there are no more elements to scan, return what we have
           collected so far: The current base, which has been extended
           by the code above. */
        return base;
    }

    /* It there are more elements to scan, extend the current base with
       the rest of the elements and return the result. */
    return computeActionLabels(c, base, elements, idx+1);
}

void yy::SetElementsNode::translate(FspDriver& c)
{
    translate_children(c);

    /* Here we have a list of ActionLabels. We compute the set of actions
       corresponding to each element by using the computeActionLabels()
       protected method, and concatenate all the results. */

    res = SetValue();
    for (unsigned int i=0; i<children.size(); i+=2) {
        DTC(ActionLabelsNode, an, children[i]);

        res += computeActionLabels(c, SetValue(), an->res, 0);
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
    /* Given an arbitrary complex label expression, e.g.
            'a[i:1..2].b.{h,j,k}.c[3][j:i..2*i][j*i+3]'
       this function collects all the children that make up the expression,
       i.e. a list of TreeNode* pointing to instances of LowerCaseIdNode,
       SetNode or ActionRangeNode.
       It's not necessary to call translate the children, since they
       will be translated in the upper layers.
    */

    res.clear();

    /* The leftmost part of the label expression: A single action
       or a set of actions. */
    do {
        DTCS(StringTreeNode, strn, children[0]);
        DTCS(SetNode, setn, children[0]);

        if (strn) {
            /* Single action. */
            res.push_back(strn);
        } else if (setn) {
            /* A set of actions. */
            res.push_back(setn);
        } else {
            assert(FALSE);
        }
    } while (0);

    /* The rest of the expression. */
    for (unsigned int i=1; i<children.size();) {
        DTCS(PeriodNode, pn, children[i]);
        DTCS(OpenSquareNode, sqn, children[i]);

        if (pn) {
            /* Recognize the "." operator; Can follow a string or a set. */
            DTCS(StringTreeNode, strn, children[i+1]);
            DTCS(SetNode, setn, children[i+1]);

            if (strn) {
                res.push_back(strn);
            } else if (setn) {
                res.push_back(setn);
            } else {
                assert(FALSE);
            }
            i += 2;
        } else if (sqn) {
            /* Recognize the "[]" operator; Must follow an action range. */
            DTC(ActionRangeNode, an, children[i+1]);

            res.push_back(an);
            i += 3;
        } else {
            assert(FALSE);
            break;
        }
    }
}

bool next_set_indexes(const vector<TreeNode *>& elements,
                      vector<unsigned int>& indexes)
{
    unsigned int j = indexes.size() - 1;

    assert(elements.size() == indexes.size());

    if (!elements.size()) {
        return false;
    }

    for (;;) {
        DTCS(StringTreeNode, strn, elements[j]);
        DTCS(SetNode, setn, elements[j]);
        DTCS(ActionRangeNode, an, elements[j]);

        if (strn) {
            /* This element contains just one action, and so there is
               noting to iterate over. In other words, we always wraparound.
               Just pass to the next element. */
        } else if (setn) {
            indexes[j]++;
            if (indexes[j] == setn->res.actions.size()) {
                /* Wraparaund: continue with the next element. */
                indexes[j] = 0;
            } else {
                /* No wraparound: stop here. */
                break;
            }
        } else if (an) {
            indexes[j]++;
            if (indexes[j] == an->res.actions.size()) {
                /* Wraparaund: continue with the next element. */
                indexes[j] = 0;
            } else {
                /* No wraparound: stop here. */
                break;
            }
        } else {
            assert(FALSE);
        }
        /* Continue with the next element, unless we are at the very
           last one: In the last case tell the caller that all the
           element combinations have been scanned. At this point
           'indexes' contain all zeroes. */
        if (j == 0) {
            return false;
        }
        j--;
    }

    return true; /* There are more combinations. */
}

yy::Lts yy::TreeNode::computePrefixActions(FspDriver& c,
                                           const vector<TreeNode *>& als,
                                           unsigned int idx,
                                           vector<NewContext>& ctxcache)
{
    assert(idx < als.size());
    DTC(ActionLabelsNode, an, als[idx]);
    const vector<TreeNode *>& elements = an->res;
    vector<unsigned int> indexes(elements.size());
    Lts lts(LtsNode::Normal, &c.actions);
    NewContext ctx = c.ctx;

    /* Initialize the 'indexes' vector. */
    for (unsigned int j=0; j<elements.size(); j++) {
        indexes[j] = 0;
    }

    do {
        string label;

        /* Scan the expression from the left to the right, computing the
           action label corresponding to 'indexes'. */
        for (unsigned int j=0; j<elements.size(); j++) {
            /* Here we do the translation that was deferred in the lower
               layers. This is necessary because of context expansion: When
               an action range defines a variable in the middle of a label
               expression, that variable can influence the translation of the
               expression elements which are on the right of the variable
               definition: In these cases, we need to retranslate those
               elements on the right many times, once for each
               possibile variable value.
            */
            elements[j]->translate(c);
            if (j == 0) {
                /* This is the first element of a label expression. */
                DTCS(StringTreeNode, strn, elements[j]);
                DTCS(SetNode, setn, elements[j]);

                if (strn) {
                    /* Single action. */
                    label = strn->res;
                } else if (setn) {
                    /* A set of actions. */
                    label = setn->res.actions[ indexes[j] ];
                } else {
                    assert(FALSE);
                }
            } else {
                /* Here we are in the middle (or the end) of a label
                   expression. */
                DTCS(StringTreeNode, strn, elements[j]);
                DTCS(SetNode, setn, elements[j]);
                DTCS(ActionRangeNode, an, elements[j]);

                if (strn) {
                    label += "." + strn->res;
                } else if (setn) {
                    label += "." + setn->res.actions[ indexes[j] ];
                } else if (an) {
                    label += "[" + an->res.actions[ indexes[j] ] + "]";
                    if (an->res.hasVariable()) {
                        if (!c.ctx.insert(an->res.variable,
                                    an->res.actions[ indexes[j] ])) {
                            cout << "ERROR: ctx.insert()\n";
                        }
                    }
                } else {
                    assert(FALSE);
                }
            }
        }

        Lts next;
        if (idx+1 >= als.size()) {
            /* This was the last ActionLabels in the chain: We create an
               incomplete node which represent an Lts which is the result
               of a LocalProcessNode we will translate later (in the
               ActionPrefixNode::translate method). However, we have to
               save now the context that will be used in the deferred
               translation. The incomplete node stores an index which refers
               to a context in the 'ctxcache' array of saved contexts. */
            if (!ctxcache.size() || c.ctx != ctxcache.back()) {
                /* Optimization: Avoid to duplicate the last inserted
                   context. */
                ctxcache.push_back(c.ctx);
            }
            next = Lts(LtsNode::Incomplete, &c.actions);
            /* Store the index in the 'priv' field. */
            next.set_priv(0, ctxcache.size() - 1);
        } else {
            /* This was not the last ActionLabels in the chain. Get
               the result of the remainder of the chain. */
            next = computePrefixActions(c, als, idx + 1, ctxcache);
        }

        /* Attach 'next' to 'lts' using 'label'. */
        lts.zerocat(next, label);

        /* Restore the saved context. */
        c.ctx = ctx;

        /* Increment indexes for the next 'label', and exits if there
           are no more combinations. */
    } while (next_set_indexes(elements, indexes));

    return lts;
}

void yy::PrefixActionsNode::translate(FspDriver& c)
{
    translate_children(c);

    res.clear();

    /* Here we have a chain of ActionLabels, e.g.
            't[1..2] -> g.y7 -> f[j:1..2][9] -> a[j+3].a.y'

       From such a chain we want to build an incomplete LTS, e.g. and LTS
       that lacks of some connections that will be completed by the upper
       ActionPrefix node.
    */
    for (unsigned int i=0; i<children.size(); i+=2) {
        DTC(ActionLabelsNode, an, children[i]);

        res.push_back(an);
    }
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
        DTC(ProcessIdNode, in, children[0]);
        unsigned int ui;

        /* TODO process_id indices_OPT. */
        res = Lts(LtsNode::Unresolved, &c.actions);
        ui = c.unres.add(in->res);
        res.set_priv(0, ui);
        assert(FALSE);
    }
}

void yy::ChoiceNode::translate(FspDriver& c)
{
    translate_children(c);

    assert(children.size());

    do {
        DTC(ActionPrefixNode, apn, children[0]);

        res = apn->res;
    } while (0);

    for (unsigned int i=2; i<children.size(); i+=2) {
        DTC(ActionPrefixNode, apn, children[i]);

        res.zeromerge(apn->res);
    }
}

void yy::LocalProcessNode::translate(FspDriver& c)
{
    translate_children(c);

    if (children.size() == 1) {
        DTCS(BaseLocalProcessNode, b, children[0]);

        if (b) {
            res = b->res;
        } else {
            /* TODO all the other cases. */
            res = Lts(LtsNode::Error, &c.actions);
        }
    } else if (children.size() == 3) {
        /* ( choice ) */
        DTC(ChoiceNode, cn, children[1]);

        res = cn->res;
    } else if (children.size() == 5) {
        /* IF expression THEN local_process else_OPT. */
        DTC(ExpressionNode, en, children[1]);
        DTC(LocalProcessNode, pn, children[3]);
        DTCS(ProcessElseNode, pen, children[4]);

        if (en->res) {
            res = pn->res;
        } else if (pen) {
            res = pen->res;
        } else {
            res = Lts(LtsNode::Normal, &c.actions);
        }
    } else {
        res = Lts(LtsNode::Error, &c.actions);
        assert(IMPLEMENT);
    }
}

void yy::ProcessElseNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(LocalProcessNode, pn, children[1]);

    res = pn->res;
}

void yy::ActionPrefixNode::translate(FspDriver& c)
{
    vector<NewContext> ctxcache;
    NewContext saved_ctx = c.ctx;

    translate_children(c);

    DTCS(GuardNode, gn, children[0]);
    DTC(PrefixActionsNode, pn, children[1]);
    DTC(LocalProcessNode, lp, children[3]);

    if (!gn || gn->res) {
        vector<Lts> processes;

        /* Compute an incomplete Lts, and the context related to
           each incomplete node (ctxcache). */
        res = computePrefixActions(c, pn->res, 0, ctxcache);
        /* Translate 'lp' under all the contexts in ctxcache. */
        for (unsigned int i=0; i<ctxcache.size(); i++) {
            c.ctx = ctxcache[i];
            lp->translate(c);
            processes.push_back(lp->res);
        }

        /* Connect the incomplete Lts to the computed translations. */
        res.incompcat(processes);
    }

    c.ctx = saved_ctx;
}

void yy::ProcessBodyNode::translate(FspDriver& c)
{
    translate_children(c);

    if (children.size() == 1) {
        DTC(LocalProcessNode, pn, children[0]);

        res = pn->res;
    } else if (children.size() == 3) {
        // TODO local process_defs
        res = Lts(LtsNode::Error, &c.actions);
    } else {
        assert(FALSE);
    }
}

void yy::AlphaExtNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(SetNode, sn, children[1]);

    res = sn->res;
}

void yy::RelabelDefNode::translate(FspDriver& c)
{
    translate_children(c);

    assert(children.size() == 3);

    DTCS(ActionLabelsNode, left, children[0]);
    DTCS(ForallNode, fan, children[0]);

    res = NewRelabelingValue();

    if (left) {
        DTC(ActionLabelsNode, right, children[2]);

        res.add(computeActionLabels(c, SetValue(), left->res, 0),
                computeActionLabels(c, SetValue(), right->res, 0));
    } else if (fan) {
        // TODO FORALL index_ranges braces_relabel_defs
    } else {
        assert(FALSE);
    }
}

void yy::RelabelDefsNode::translate(FspDriver& c)
{
    translate_children(c);

    do {
        DTC(RelabelDefNode, rn, children[0]);

        res = rn->res;
    } while (0);

    for (unsigned int i=2; i<children.size(); i++) {
        DTC(RelabelDefNode, rn, children[i]);

        res.merge(rn->res);
    }
}

void yy::BracesRelabelDefsNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(RelabelDefsNode, rn, children[1]);

    res = rn->res;
}

void yy::RelabelingNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(BracesRelabelDefsNode, bd, children[1]);

    res = bd->res;
}

void yy::HidingInterfNode::translate(FspDriver& c)
{
    translate_children(c);

    DTCS(HidingNode, hn, children[0]);
    DTCS(InterfNode, in, children[0]);
    DTC(SetNode, sn, children[1]);

    res = NewHidingValue();

    if (hn) {
        res.interface = false;
    } else if (in) {
        res.interface = true;
    } else {
        assert(FALSE);
    }

    res.setv = sn->res;
}

void yy::ParameterNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ParameterIdNode, in, children[0]);
    DTC(ExpressionNode, en, children[2]);
    ConstValue *cvp = new ConstValue;

    /* TODO overriding stuff
    SymbolValue *svp;
    if (c.identifiers.lookup(in->res, svp)) {
        c.overridden_names.push_back(in->res);
        c.overridden_values.push_back();
    } */

    /* Insert the parameter into the identifiers table. */
    cvp->value = en->res;
    if (!c.identifiers.insert(in->res, cvp)) {
        // TODO manage the error
        delete cvp;
    }
    /* Save the parameter name for subsequent removal. */
    c.param_names.push_back(in->res);
}

void yy::IndexRangesNode::translate(FspDriver& c)
{
    translate_children(c); // XXX useless because of deferred translation

    res.clear();

    for (unsigned int i=0; i<children.size(); i+=3) {
        DTC(ActionRangeNode, arn, children[i+1]);

        res.push_back(arn);
    }
}

void yy::LocalProcessDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ProcessIdNode, in, children[0]);
    DTC(IndexRangesNode, irn, children[1]);
    DTC(LocalProcessNode, lp, children[3]);

    const vector<TreeNode *>& elements = irn->res;
    vector<unsigned int> indexes(elements.size());
    NewContext ctx = c.ctx;
    bool first = true;

    /* Initialize the 'indexes' vector, used to iterate over all the
       possible index combinations. */
    for (unsigned int j=0; j<elements.size(); j++) {
        indexes[j] = 0;
    }

    do {
        string index_string;
        unsigned int ui;

        /* Scan the ranges from the left to the right, computing the
           '[x][y][z]...' string corresponding to 'indexes'. */
        for (unsigned int j=0; j<elements.size(); j++) {
            /* Here we do the translation that was deferred in the lower
               layers. */
            elements[j]->translate(c);
            DTC(ActionRangeNode, an, elements[j]);

            if (an) {
                index_string += "[" + an->res.actions[ indexes[j] ] + "]";
                if (an->res.hasVariable()) {
                    if (!c.ctx.insert(an->res.variable,
                                an->res.actions[ indexes[j] ])) {
                        cout << "ERROR: ctx.insert()\n";
                    }
                }
            } else {
                assert(FALSE);
            }
        }

        /* Translate the LocalProcess using the current context. */
        lp->translate(c);

        /* The name of a local process name is the concatenation of
           'process_id' and the 'index_string', e.g. 'P' + '[3][1]'. */
        ui = c.unres.add(in->res + index_string);
        lp->res.set_priv(0, ui);

        if (first) {
            first = false;
            res = lp->res;
        } else {
            res.append(lp->res, 0);
        }

        /* Restore the saved context. */
        c.ctx = ctx;

        /* Increment 'indexes' for the next 'index_string', and exits if
           there are no more combinations. */
    } while (next_set_indexes(elements, indexes));
}

void yy::LocalProcessDefsNode::translate(FspDriver& c)
{
    translate_children(c);
}

void yy::ProcessDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ProcessIdNode, idn, children[0]);
    DTC(ProcessBodyNode, bn, children[3]);
    DTCS(AlphaExtNode, aen, children[4]);
    DTCS(RelabelingNode, rn, children[5]);
    DTCS(HidingInterfNode, hin, children[6]);
    unsigned int ui;

    /* The base is the process body. */
    res = bn->res;

    ui = c.unres.add(idn->res);
    res.set_priv(0, ui);

    res.resolve();

    /* Extend the alphabet. */
    if (aen) {
        SetValue& sv = aen->res;

        for (unsigned int i=0; i<sv.actions.size(); i++) {
            res.updateAlphabet(c.actions.insert(sv.actions[i]));
        }
    }

    /* TODO Merge sequential processes actions into the alphabet. */

    /* Apply the relabeling operator. */
    if (rn) {
        NewRelabelingValue& rlv = rn->res;

        for (unsigned int i=0; i<rlv.size(); i++) {
            res.relabeling(rlv.new_labels[i], rlv.old_labels[i]);
        }
    }

    /* Apply the hiding/interface operator. */
    if (hin) {
        NewHidingValue& hv = hin->res;

        res.hiding(hv.setv, hv.interface);
    }
    res.graphvizOutput("temp.lts");
}

