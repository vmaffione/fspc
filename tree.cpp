/*
 *  fspc parse tree
 *
 *  Copyright (C) 2013  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>

#include "tree.hpp"
#include "driver.hpp"
#include "unresolved.hpp"
#include "helpers.hpp"
#include "utils.hpp"

using namespace std;
using namespace yy;


//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


/* Helper function used to update the 'unres' table.
   name: the name of the (possibly local) process to assign
         an alias
   lts:  the LTS corresponding the the (possibly local) process,
         which can also be a LTS containing a single unresolved
         node
   define: true if 'name' is on the left side of an FSP assignement
*/
static void update_unres(FspDriver& c, const string& name, yy::LtsPtr lts,
                         bool define, const yy::location& loc)
{
    unsigned int ui;

    if (define && c.unres.defined(name)) {
        /* A process name cannot be defined twice. */
        stringstream errstream;
        errstream << "Process " << name << " defined twice";
        semantic_error(c, errstream, loc);
    }

    if (lts->get_priv(0) == LtsNode::NoPriv) {
        /* If 'lts[0]' does not have its 'priv' set, we must be in one of
           the following two cases:
                - 'lts[0]' is an unresolved node, and so we have to assign
                  a new UnresolvedName alias (an 'idx') to it, for subsequent
                  name resolution (Lts::resolve). In this case 'define' is
                  false.
                - 'lts[0]' is not unresolved and so we have to assign a new
                   alias (an 'idx') to it, for subsequent name resolution
                  (Lts::resolve). In this case 'define' is true.
        */
        ui = c.unres.insert(name, define); /* Create a new entry for 'name' */
        lts->set_priv(0, ui);  /* Record the alias into the 'lts[0]' priv. */
    } else {
        /* If 'lts[0]' does have its 'priv' set, it means that 'lts[0]'
           is not unresolved, and there is already an alias assigned to it.

           Tell 'unres' that the 'lts[0]' priv field must be an alias also
           for 'name'. */
        ui = c.unres.append(name, lts->get_priv(0), define);
        /* Update all the 'priv' fields that have the 'idx' previously
           associated to 'name', if any. */
        if (ui != LtsNode::NoPriv) {
            lts->replace_priv(lts->get_priv(0), ui);
        }
    }
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

void yy::TreeNode::addChild(yy::TreeNode *n, const yy::location& loc)
{
    children.push_back(n);
    if (n) {
        n->loc = loc;
    }
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
            in = tree_downcast_safe<IntegerNode>(current);
            assert(!(in && ln) && !(ln && un) && !(un && in));
            if (ln && label == "LowerCaseId") {
                label = ln->res;
            } else if (un && label == "UpperCaseId") {
                label = un->res;
            } else if (in) {
                label = int2string(in->res);
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

void yy::TreeNode::clear_children()
{
    for (unsigned int i=0; i<children.size(); i++) {
        if (children[i]) {
            children[i]->clear();
        }
    }
}

Result *yy::TreeNode::translate(FspDriver& c)
{
    translate_children(c);

    return NULL;
}

void yy::TreeNode::getNodesByClasses(const vector<string>& classes,
                            vector<TreeNode *>& results)
{
    queue<TreeNode *> frontier;

    results.clear();
    frontier.push(this);

    while (!frontier.empty()) {
        TreeNode *cur = frontier.front();

        frontier.pop();

        for (unsigned int i = 0; i < classes.size(); i++) {
            if (cur->getClassName() == classes[i]) {
                results.push_back(cur);
                break;
            }
        }

        for (unsigned int i = 0; i < cur->children.size(); i++) {
            if (cur->children[i]) {
                frontier.push(cur->children[i]);
            }
        }
    }
}

/* ========================== Translation methods ======================== */

Result *yy::VariableIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::ConstantIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::RangeIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::SetIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::ConstParameterIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::ParameterIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::ProcessIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::ProgressIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::MenuIdNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[0]->translate(c));

    return id;
}

Result *yy::ExpressionNode::translate(FspDriver& c)
{
    if (children.size() == 1) {
        DRC(IntResult, expr, children[0]->translate(c));

        return expr;
    } else if (children.size() == 2) {
        /* OPERATOR EXPR */
        DTC(OperatorNode, on, children[0]);
        DRC(IntResult, expr, children[1]->translate(c));

        if (on->sign == "+") {
        } else if (on->sign == "-") {
            expr->val = -expr->val;
        } else if (on->sign == "!") {
            expr->val = !expr->val;
        } else {
            assert(0);
        }

        return expr;
    } else if (children.size() == 3) {
        DTCS(OpenParenNode, pn, children[0]);

        if (pn) {
            /* ( EXPR ) */
            DRC(IntResult, expr, children[1]->translate(c));

            return expr;
        } else {
            /* EXPR OPERATOR EXPR */
            DRC(IntResult, l, children[0]->translate(c));
            DTC(OperatorNode, o, children[1]);
            DRC(IntResult, r, children[2]->translate(c));
            IntResult *expr = new IntResult;

            if (o->sign == "||") {
                expr->val = l->val || r->val;
            } else if (o->sign == "&&") {
                expr->val = l->val && r->val;
            } else if (o->sign == "|") {
                expr->val = l->val | r->val;
            } else if (o->sign == "^") {
                expr->val = l->val ^ r->val;
            } else if (o->sign == "&") {
                expr->val = l->val & r->val;
            } else if (o->sign == "==") {
                expr->val = (l->val == r->val);
            } else if (o->sign == "!=") {
                expr->val = (l->val != r->val);
            } else if (o->sign == "<") {
                expr->val = (l->val < r->val);
            } else if (o->sign == ">") {
                expr->val = (l->val > r->val);
            } else if (o->sign == "<=") {
                expr->val = (l->val <= r->val);
            } else if (o->sign == ">=") {
                expr->val = (l->val >= r->val);
            } else if (o->sign == "<<") {
                expr->val = l->val << r->val;
            } else if (o->sign == ">>") {
                expr->val = l->val >> r->val;
            } else if (o->sign == "+") {
                expr->val = l->val + r->val;
            } else if (o->sign == "-") {
                expr->val = l->val - r->val;
            } else if (o->sign == "*") {
                expr->val = l->val * r->val;
            } else if (o->sign == "/") {
                expr->val = l->val / r->val;
            } else if (o->sign == "%") {
                expr->val = l->val % r->val;
            } else {
                assert(0);
            }
            delete l;
            delete r;

            return expr;
        }
    } else {
        assert(0);
    }

    return NULL;
}

Result *yy::BaseExpressionNode::translate(FspDriver& c)
{
    DTCS(IntegerNode, in, children[0]);
    DTCS(VariableIdNode, vn, children[0]);
    DTCS(ConstParameterIdNode, cn, children[0]);

    if (in) {
        DRC(IntResult, expr, children[0]->translate(c));

        return expr;
    } else if (vn) {
        DRC(StringResult, id, children[0]->translate(c));
        string val;

        if (!c.ctx.lookup(id->val, val)) {
            stringstream errstream;
            errstream << "variable " << id->val << " undeclared";
            semantic_error(c, errstream, loc);
        }
        delete id;

        return new IntResult(string2int(val));
    } else if (cn) {
        DRC(StringResult, id, children[0]->translate(c));
        Symbol *svp;
        ConstS *cvp;

        if (!c.identifiers.lookup(id->val, svp)) {
            stringstream errstream;
            errstream << "const/parameter " << id->val << " undeclared";
            semantic_error(c, errstream, loc);
        }
        cvp = err_if_not<ConstS>(c, svp, loc);
        delete id;

        return new IntResult(cvp->value);
    } else {
        assert(0);
    }

    return NULL;
}

Result *yy::RangeExprNode::translate(FspDriver& c)
{
    DRC(IntResult, l, children[0]->translate(c));
    DRC(IntResult, r, children[2]->translate(c));
    /* Build a range from two expressions. */
    RangeResult *range = new RangeResult(l->val, r->val);

    delete l;
    delete r;

    return range;
}

Result *yy::RangeNode::translate(FspDriver& c)
{
    DTCS(RangeIdNode, ri, children[0]);
    DTCS(RangeExprNode, re, children[0]);

    if (ri) {
        /* Lookup the range identifier. */
        DRC(StringResult, id, children[0]->translate(c));
        Symbol *svp;
        RangeS *rvp;
        RangeResult *range = new RangeResult;

        if (!c.identifiers.lookup(id->val, svp)) {
            stringstream errstream;
            errstream << "range " << id->val << " undeclared";
            semantic_error(c, errstream, loc);
        }
        rvp = err_if_not<RangeS>(c, svp, loc);
        range->val = *rvp;
        delete id;

        return range;
    } else if (re) {
        /* Return the range expression. */
        DRC(RangeResult, range, children[0]->translate(c));

        return range;
    } else {
        assert(0);
    }

    return NULL;
}

Result *yy::ConstantDefNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[1]->translate(c));
    DRC(IntResult, expr, children[3]->translate(c));
    ConstS *cvp = new ConstS;

    cvp->value = expr->val;
    if (!c.identifiers.insert(id->val, cvp)) {
        stringstream errstream;
        errstream << "const " << id->val << " declared twice";
        delete cvp;
        semantic_error(c, errstream, loc);
    }

    delete id;
    delete expr;

    return NULL;
}

Result *yy::RangeDefNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[1]->translate(c));
    DRC(IntResult, l, children[3]->translate(c));
    DRC(IntResult, r, children[5]->translate(c));
    RangeS *rvp = new RangeS;

    rvp->low = l->val;
    rvp->high = r->val;
    if (!c.identifiers.insert(id->val, rvp)) {
        stringstream errstream;
        errstream << "range " << id->val << " declared twice";
        delete rvp;
        semantic_error(c, errstream, loc);
    }

    delete id;
    delete l;
    delete r;

    return NULL;
}

Result *yy::SetDefNode::translate(FspDriver& c)
{
    DRC(StringResult, id, children[1]->translate(c));
    DRC(SetResult, se, children[3]->translate(c));
    SetS *svp = new SetS;

    *svp = se->val;
    if (!c.identifiers.insert(id->val, svp)) {
        stringstream errstream;
        errstream << "set " << id->val << " declared twice";
        delete svp;
        semantic_error(c, errstream, loc);
    }

    delete id;
    delete se;

    return NULL;
}

void yy::ProgressDefNode::combination(FspDriver& c, Result *r,
                                      string index, bool first)
{
    DRC(StringResult, id, children[1]->translate(c));
    ProgressS *pv = new ProgressS;
    string name = id->val + index;

    if (children.size() == 5) {
        /* Progress definition in unconditional form (normal form). */
        DRC(SetResult, se, children[4]->translate(c));

        pv->conditional = false;
        se->val.toActionSetValue(c.actions, pv->set);
        delete se;
    } else if (children.size() == 8) {
        /* Progress definition in conditional form. */
        DRC(SetResult, cse, children[5]->translate(c));
        DRC(SetResult, se, children[7]->translate(c));

        pv->conditional = true;
        cse->val.toActionSetValue(c.actions, pv->condition);
        se->val.toActionSetValue(c.actions, pv->set);
        delete se;
        delete cse;
    } else {
        assert(0);
    }

    if (!c.progresses.insert(name, pv)) {
        stringstream errstream;
        errstream << "progress " << name << " declared twice";
        semantic_error(c, errstream, loc);
    }

    delete id;
}

static bool next_set_indexes(const vector<TreeNode *>& elements,
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
            if (indexes[j] == setn->res.size()) {
                /* Wraparaund: continue with the next element. */
                indexes[j] = 0;
            } else {
                /* No wraparound: stop here. */
                break;
            }
        } else if (an) {
            indexes[j]++;
            if (indexes[j] == an->res.size()) {
                /* Wraparaund: continue with the next element. */
                indexes[j] = 0;
            } else {
                /* No wraparound: stop here. */
                break;
            }
        } else {
            assert(0);
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

static void for_each_combination(FspDriver& c, Result *r,
                                 const vector<TreeNode *>& elements,
                                 TreeNode *n)
{
    vector<unsigned int> indexes(elements.size());
    Context ctx = c.ctx;  /* Save the original context. */
    bool first = true;

    /* Initialize the 'indexes' vector, used to iterate over all the
       possible index combinations. */
    for (unsigned int j=0; j<elements.size(); j++) {
        indexes[j] = 0;
    }

    do {
        string index_string;

        /* Scan the ranges from the left to the right, computing the
           '[x][y][z]...' string corresponding to 'indexes'. */
        for (unsigned int j=0; j<elements.size(); j++) {
            /* Here we do the translation that was deferred in the lower
               layers. */
            elements[j]->translate(c);
            DTC(ActionRangeNode, an, elements[j]);

            if (an) {
                index_string += "." + an->res[ indexes[j] ];
                if (an->res.hasVariable()) {
                    if (!c.ctx.insert(an->res.variable,
                                an->res[ indexes[j] ])) {
                        cout << "ERROR: ctx.insert()\n";
                    }
                }
            } else {
                assert(0);
            }
        }

        n->combination(c, r, index_string, first);
        first = false;

        /* Restore the saved context. */
        c.ctx = ctx;

        /* Increment 'indexes' for the next 'index_string', and exits if
           there are no more combinations. */
    } while (next_set_indexes(elements, indexes));
}

Result *yy::ProgressDefNode::translate(FspDriver& c)
{
    DRC(TreeNodeVecResult, ir, children[2]->translate(c));

    for_each_combination(c, NULL, ir->val, this);

    delete ir;

    return NULL;
}

Result *yy::MenuDefNode::translate(FspDriver &c)
{
    /* menu_id set */
    DRC(StringResult, id, children[1]->translate(c));
    DRC(SetResult, se, children[3]->translate(c));
    ActionSetS *asv = new ActionSetS;

    /* Turn the SetS contained into the SetNode into an
       ActionSetS. */
    se->val.toActionSetValue(c.actions, *asv);

    if (!c.menus.insert(id->val, asv)) {
        stringstream errstream;
        errstream << "menu " << id->val << " declared twice";
        semantic_error(c, errstream, loc);
    }

    return NULL;
}

/* This recursive method can be used to compute the set of action defined
   by an arbitrary complex label expression, e.g.
        'a[i:1..2].b.{h,j,k}.c[3][j:i..2*i][j*i+3]'
    The caller should pass a SetS object built using the
    default constructor (e.g. an empty SetS) to 'base', and 0 to 'idx'.
    The elements vector contains pointers to strings, sets or action ranges.
*/
SetS yy::TreeNode::computeActionLabels(FspDriver& c, SetS base,
                                           const vector<TreeNode*>& elements,
                                           unsigned int idx)
{
    Result *r;

    assert(idx < elements.size());

    /* Here we do the translation that was deferred in the lower layers.
       This is necessary because of context expansion: When an action range
       defines a variable in the middle of a label expression, that variable
       can influence the translation of the expression elements which are on
       the right of the variable definition: In these cases, we need to
       retranslate those elements on the right many times, once for each
       possibile variable value.
    */
    r = elements[idx]->translate(c);
    if (idx == 0) {
        /* This is the first element of a label expression. We set 'base'
           to its initial value. */
        StringResult *str = result_downcast_safe<StringResult>(r);
        SetResult *se = result_downcast_safe<SetResult>(r);

        base = SetS();
        if (str) {
            /* Single action. */
            base += str->val;
         } else if (se) {
            /* A set of actions. */
            base += se->val;
        } else {
            assert(0);
        }
    } else {
        /* Here we are in the middle (or the end) of a label expression.
           We use the dotcat() or indexize() method to extend the current
           'base'. */
        DTCS(StringTreeNode, strn, elements[idx]);
        DTCS(SetNode, setn, elements[idx]);
        DTCS(ActionRangeNode, an, elements[idx]);

        if (strn) {
            StringResult *str = result_downcast_safe<StringResult>(r);

            base.dotcat(str->val);
        } else if (setn) {
            SetResult *se = result_downcast_safe<SetResult>(r);

            base.dotcat(se->val);
        } else if (an) {
            SetResult *ar = result_downcast_safe<SetResult>(r);

            if (!ar->val.hasVariable() || idx+1 >= elements.size()) {
                /* When an action range doesn't define a variable, or when
                   such a declaration is useless since this is the end of
                   the expression, we just extend the current 'base'. */
                base.indexize(ar->val);
            } else {
                /* When an action range does define a variable, we must split
                   the computation in N parts, one for each action in the
                   action range, and then concatenate all the results. For
                   each part, we extend the current 'base' with only an
                   action, insert the variable into the context and do a
                   recursive call. */
                SetS ret;
                SetS next_base;
                bool ok;

                for (unsigned int j=0; j<ar->val.size(); j++) {
                    next_base = base;
                    next_base.indexize(ar->val[j]);
                    if (!c.ctx.insert(ar->val.variable, ar->val[j])) {
                        cout << "ERROR: ctx.insert()\n";
                    }
                    ret += computeActionLabels(c, next_base,
                                               elements, idx+1);
                    ok = c.ctx.remove(ar->val.variable);
                    assert(ok);
                }
                delete r;

                return ret;
            }
        } else {
            assert(0);
        }
    }
    delete r;

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

Result *yy::SetElementsNode::translate(FspDriver& c)
{
    SetResult *se = new SetResult;

    /* Here we have a list of ActionLabels. We compute the set of actions
       corresponding to each element by using the computeActionLabels()
       protected method, and concatenate all the results. */

    for (unsigned int i = 0; i < children.size(); i += 2) {
        DRC(TreeNodeVecResult, al, children[i]->translate(c));

        se->val += computeActionLabels(c, SetS(), al->val, 0);
        delete al;
    }

    return se;
}

Result *yy::SetExprNode::translate(FspDriver& c)
{
    /* { SetElementsNode } */
    DRC(SetResult, se, children[1]->translate(c));

    return se;
}

Result *yy::SetNode::translate(FspDriver& c)
{
    DTCS(SetIdNode, sin, children[0]);
    DTCS(SetExprNode, sen, children[0]);

    if (sin) {
        /* Lookup the set identifier. */
        DRC(StringResult, id, children[0]->translate(c));
        Symbol *svp;
        SetS *setvp;
        SetResult *se = new SetResult;

        if (!c.identifiers.lookup(id->val, svp)) {
            stringstream errstream;
            errstream << "set " << id->val << " undeclared";
            semantic_error(c, errstream, loc);
        }
        setvp = err_if_not<SetS>(c, svp, loc);
        delete id;
        se->val = *setvp;

        return se;
    } else if (sen) {
        /* Return the set expression. */
        DRC(SetResult, se, children[0]->translate(c));

        return se;
    } else {
        assert(0);
    }

    return NULL;
}

Result *yy::ActionRangeNode::translate(FspDriver& c)
{
    SetResult *result = new SetResult;

    if (children.size() == 1) {
        /* Build a set of actions from an integer, a range or a set. */
        DRCS(IntResult, expr, children[0]->translate(c));
        DRCS(RangeResult, range, children[0]->translate(c));
        DRCS(SetResult, se, children[0]->translate(c));

        if (expr) {
            result->val += int2string(expr->val);
            delete expr;
        } else if (range) {
            range->val.set(result->val);
            delete range;
        } else if (se) {
            result->val = se->val;
            delete se;
        } else {
            assert(0);
        }
    } else if (children.size() == 3) {
        /* Do the same with variable declarations. */
        DRC(StringResult, id, children[0]->translate(c));
        DRCS(RangeResult, range, children[2]->translate(c));
        DRCS(SetResult, se, children[2]->translate(c));

        if (range) {
            range->val.set(result->val);
            delete range;
        } else if (se) {
            result->val = se->val;
            delete se;
        } else {
            assert(0);
        }
        result->val.variable = id->val;
        delete id;
    } else {
        assert(0);
    }

    return result;
}

Result *yy::ActionLabelsNode::translate(FspDriver& c)
{
    /* Given an arbitrary complex label expression, e.g.
            'a[i:1..2].b.{h,j,k}.c[3][j:i..2*i][j*i+3]'
       this function collects all the children that make up the expression,
       i.e. a list of TreeNode* pointing to instances of LowerCaseIdNode,
       SetNode or ActionRangeNode.
       It's not necessary to call translate the children, since they
       will be translated in the upper layers.
    */
    TreeNodeVecResult *result = new TreeNodeVecResult;

    /* The leftmost part of the label expression: A single action
       or a set of actions. */
    do {
        DTCS(StringTreeNode, strn, children[0]);
        DTCS(SetNode, setn, children[0]);

        if (strn) {
            /* Single action. */
            result->val.push_back(strn);
        } else if (setn) {
            /* A set of actions. */
            result->val.push_back(setn);
        } else {
            assert(0);
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
                result->val.push_back(strn);
            } else if (setn) {
                result->val.push_back(setn);
            } else {
                assert(0);
            }
            i += 2;
        } else if (sqn) {
            /* Recognize the "[]" operator; Must follow an action range. */
            DTC(ActionRangeNode, an, children[i+1]);

            result->val.push_back(an);
            i += 3;
        } else {
            assert(0);
            break;
        }
    }

    return result;
}

yy::LtsPtr yy::TreeNode::computePrefixActions(FspDriver& c,
                                           const vector<TreeNode *>& als,
                                           unsigned int idx,
                                           vector<Context>& ctxcache)
{
    assert(idx < als.size());
    DTC(ActionLabelsNode, an, als[idx]);
    const vector<TreeNode *>& elements = an->res;
    vector<unsigned int> indexes(elements.size());
    LtsPtr lts = new Lts(LtsNode::Normal, &c.actions);
    Context ctx = c.ctx;

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
            Result *r;

            r = elements[j]->translate(c);
            if (j == 0) {
                /* This is the first element of a label expression. */
                StringResult *str = result_downcast_safe<StringResult>(r);
                SetResult *se = result_downcast_safe<SetResult>(r);

                if (str) {
                    /* Single action. */
                    label = str->val;
                } else if (se) {
                    /* A set of actions. */
                    label = se->val[ indexes[j] ];
                } else {
                    assert(0);
                }
            } else {
                /* Here we are in the middle (or the end) of a label
                   expression. */
                DTCS(StringTreeNode, strn, elements[j]);
                DTCS(SetNode, setn, elements[j]);
                DTCS(ActionRangeNode, an, elements[j]);

                if (strn) {
                    StringResult *str = result_downcast_safe<StringResult>(r);

                    label += "." + str->val;
                } else if (setn) {
                    SetResult *se = result_downcast_safe<SetResult>(r);

                    label += "." + se->val[ indexes[j] ];
                } else if (an) {
                    SetResult *ar = result_downcast_safe<SetResult>(r);

                    label += "." + ar->val[ indexes[j] ];
                    if (ar->val.hasVariable()) {
                        if (!c.ctx.insert(ar->val.variable,
                                    ar->val[ indexes[j] ])) {
                            cout << "ERROR: ctx.insert()\n";
                        }
                    }
                } else {
                    assert(0);
                }
            }
            delete r;
        }

        LtsPtr next;
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
            next = new Lts(LtsNode::Incomplete, &c.actions);
            /* Store the index in the 'priv' field. */
            next->set_priv(0, ctxcache.size() - 1);
        } else {
            /* This was not the last ActionLabels in the chain. Get
               the result of the remainder of the chain. */
            next = computePrefixActions(c, als, idx + 1, ctxcache);
        }

        /* Attach 'next' to 'lts' using 'label'. */
        lts->zerocat(*next, label);

        /* Restore the saved context. */
        c.ctx = ctx;

        /* Increment indexes for the next 'label', and exits if there
           are no more combinations. */
    } while (next_set_indexes(elements, indexes));

    return lts;
}

Result *yy::PrefixActionsNode::translate(FspDriver& c)
{
    TreeNodeVecResult *result = new TreeNodeVecResult;

    /* Here we have a chain of ActionLabels, e.g.
            't[1..2] -> g.y7 -> f[j:1..2][9] -> a[j+3].a.y'

       From such a chain we want to build an incomplete LTS, e.g. and LTS
       that lacks of some connections that will be completed by the upper
       ActionPrefix node.
    */
    for (unsigned int i=0; i<children.size(); i+=2) {
        DTC(ActionLabelsNode, an, children[i]);

        result->val.push_back(an);
    }

    return result;
}

Result *yy::IndicesNode::translate(FspDriver& c)
{
    StringResult *result = new StringResult;

    /* [ EXPR ] [ EXPR ] ... [ EXPR ] */ 
    for (unsigned int i=0; i<children.size(); i+=3) {
        DRC(IntResult, expr, children[i+1]->translate(c));

        result->val += "." + int2string(expr->val);
        delete expr;
    }

    return result;
}

Result *yy::BaseLocalProcessNode::translate(FspDriver& c)
{
    DTCS(EndNode, en, children[0]);
    DTCS(StopNode, sn, children[0]);
    DTCS(ErrorNode, ern, children[0]);
    LtsResult *result = new LtsResult;

    if (en) {
        result->val = new Lts(LtsNode::End, &c.actions);
    } else if (sn) {
        result->val = new Lts(LtsNode::Normal, &c.actions);
    } else if (ern) {
        result->val = new Lts(LtsNode::Error, &c.actions);
    } else {
        /* process_id indices */
        DRC(StringResult, id, children[0]->translate(c));
        DRCS(StringResult, idx, children[1]->translate(c));
        string name = id->val;

        /* Create an LTS containing a single unresolved
           node. */
        result->val = new Lts(LtsNode::Unresolved, &c.actions);
        if (idx) {
            name += idx->val;
            delete idx;
        }
        /* Tell the unresolved names table that there
           is a new unresolved name (define is false
           because this is not a process definition,
           the process name is only referenced). */
        update_unres(c, name, result->val, false, loc);

        delete id;
    }

    return result;
}

Result *yy::ChoiceNode::translate(FspDriver& c)
{
    LtsResult *result;

    assert(children.size());

    /* action_prefix | action_prefix | ... | action_prefix */
    do {
        DRC(LtsResult, ap, children[0]->translate(c));

        result = ap;
    } while (0);

    for (unsigned int i=2; i<children.size(); i+=2) {
        DRC(LtsResult, ap, children[i]->translate(c));

        result->val->zeromerge(*ap->val);
        delete ap;
    }

    return result;
}

Result *yy::ArgumentListNode::translate(FspDriver& c)
{
    IntVecResult *result = new IntVecResult;

    /* EXPR , EXPR , ... , EXPR */
    for (unsigned int i = 0; i < children.size(); i += 2) {
        DRC(IntResult, expr, children[i]->translate(c));

        result->val.push_back(expr->val);
        delete expr;
    }

    return result;
}

Result *yy::ArgumentsNode::translate(FspDriver& c)
{
    /* ( argument_list ) */
    DRC(IntVecResult, argl, children[1]->translate(c));

    return argl;
}

void yy::TreeNode::process_ref_translate(FspDriver& c, const string& name,
                                            const vector<int> *args,
                                            yy::LtsPtr *res)
{
    Symbol *svp;
    ParametricProcess *pp;
    vector<int> arguments;
    string extension;

    assert(res);

    /* Lookup 'process_id' in the 'parametric_process' table. */
    if (!c.parametric_processes.lookup(name, svp)) {
	stringstream errstream;
	errstream << "Process " << name << " undeclared";
	semantic_error(c, errstream, loc);
    }
    pp = is<ParametricProcess>(svp);

    /* Find the arguments for the process parameters. */
    arguments = args ? *args : pp->defaults;

    if (arguments.size() != pp->defaults.size()) {
	stringstream errstream;
	errstream << "Parameters mismatch";
	semantic_error(c, errstream, loc);  // XXX tr.locations[1]
    }

    lts_name_extension(arguments, extension);

    /* We first lookup the global processes table in order to see whether
       we already have the requested LTS or we need to compute it. */
    IFD(cout << "Looking up " << name + extension << "\n");
    if (!c.processes.lookup(name + extension, svp)) {
        TreeNode *pdn;
        bool ok;

	/* If there is a cache miss, we have to compute the requested LTS
	   using the translate method and save it in the global processes
           table. */
        pdn = dynamic_cast<TreeNode *>(pp->translator);
        assert(pdn);
        /* Save and reset the compiler context. It must be called before
           inserting the parameters into 'c.parameters' (see the following
           for loop). */
        ok = c.nesting_save();
        if (!ok) {
            stringstream errstream;
            errstream << "Max reference depth exceeded while translating "
                        "process " << name + extension;
            general_error(c, errstream, loc);
        }
        /* Insert the arguments into the identifiers table, taking care
           of overridden names. */
        for (unsigned int i=0; i<pp->names.size(); i++) {
            Symbol *svp;
            ConstS *cvp = new ConstS();

            if (c.identifiers.lookup(pp->names[i], svp)) {
                /* If there is already an identifier with the same name as
                   the i-th parameter, override temporarly the identifier.
                */
                c.overridden_names.push_back(pp->names[i]);
                c.overridden_values.push_back(svp->clone());
                c.identifiers.remove(pp->names[i]);
            }

            cvp->value = arguments[i];
            if (!c.identifiers.insert(pp->names[i], cvp)) {
                assert(0);
                delete cvp;
            }
            /* Insert the parameter into 'c.parameters', which is part of
               the translator context. */
            c.parameters.insert(pp->names[i], arguments[i]);
        }
        /* Do the translation. The new LTS is stored in the 'processes'
           table by the translate function. */
        pdn->translate(c);
        /* Restore the previously saved compiler context. */
        c.nesting_restore();
    }

    /* Use the LTS stored in the 'processes' table. */
    if (c.processes.lookup(name + extension, svp)) {
	*res = is<yy::Lts>(svp->clone());
    } else {
        assert(0);
    }
}

Result *yy::ProcessRefSeqNode::translate(FspDriver& c)
{
    /* process_id arguments */
    DRC(StringResult, id, children[0]->translate(c));
    DRCS(IntVecResult, args, children[1]->translate(c));
    LtsResult *lts = new LtsResult;

    process_ref_translate(c, id->val, args ? &args->val : NULL, &lts->val);
    delete id;
    if (args) {
        delete args;
    }

    return lts;
}

Result *yy::SeqProcessListNode::translate(FspDriver& c)
{
    LtsResult *result;

    /* process_ref_seq ; process_ref_seq ; ... process_ref_seq */
    do {
        DRC(LtsResult, pr, children[0]->translate(c));

        result = pr;
    } while (0);

    for (unsigned int i=2; i<children.size(); i+=2) {
        DRC(LtsResult, pr, children[i]->translate(c));

        result->val->endcat(*pr->val);
        delete pr;
    }

    return result;
}

Result *yy::SeqCompNode::translate(FspDriver& c)
{
    /* seq_process_list ; base_local_process */
    DRC(LtsResult, plist, children[0]->translate(c));
    DRC(LtsResult, localp, children[2]->translate(c));

    plist->val->endcat(*localp->val);
    delete localp;

    return plist;
}

Result *yy::LocalProcessNode::translate(FspDriver& c)
{
    LtsResult *result;

    if (children.size() == 1) {
        DTCS(BaseLocalProcessNode, b, children[0]);
        DTCS(SeqCompNode, sc, children[0]);
        DRC(LtsResult, lts, children[0]->translate(c));

        assert(b || sc);
        result = lts;
    } else if (children.size() == 3) {
        /* ( choice ) */
        DRC(LtsResult, lts, children[1]->translate(c));

        result = lts;
    } else if (children.size() == 5) {
        /* IF expression THEN local_process else_OPT. */
        DRC(IntResult, expr, children[1]->translate(c));
        DTCS(ProcessElseNode, pen, children[4]);

        if (expr->val) {
            DRC(LtsResult, localp, children[3]->translate(c));

            result = localp;
        } else if (pen) {
            DRC(LtsResult, elsep, children[4]->translate(c));

            result = elsep;
        } else {
            result = new LtsResult;
            result->val = new Lts(LtsNode::Normal, &c.actions);
        }
        delete expr;
    } else {
        assert(0);
    }

    return result;
}

Result *yy::ProcessElseNode::translate(FspDriver& c)
{
    /* ELSE local_process */
    DRC(LtsResult, localp, children[1]->translate(c));

    return localp;
}

Result *yy::ActionPrefixNode::translate(FspDriver& c)
{
    vector<Context> ctxcache;
    Context saved_ctx = c.ctx;
    LtsResult *result = new LtsResult;

    /* guard_OPT prefix_actions local_process */
    DRCS(IntResult, guard, children[0]->translate(c));
    DRC(TreeNodeVecResult, pa, children[1]->translate(c));
    DTC(LocalProcessNode, lp, children[3]);

    /* Don't translate 'lp', since it will be translated into the loop,
       with proper context. */

    if (!guard || guard->val) {
        vector<Lts> processes; /* XXX can this be vector<LtsPtr> ?? */

        /* Compute an incomplete Lts, and the context related to
           each incomplete node (ctxcache). */
        result->val = computePrefixActions(c, pa->val, 0, ctxcache);
        /* Translate 'lp' under all the contexts in ctxcache. */
        for (unsigned int i=0; i<ctxcache.size(); i++) {
            LtsResult *lts;

            c.ctx = ctxcache[i];
            lts = result_downcast<LtsResult>(lp->translate(c));
            processes.push_back(*lts->val);
            delete lts;
        }

        /* Connect the incomplete Lts to the computed translations. */
        result->val->incompcat(processes);
    }
    if (guard) {
        delete guard;
    }
    delete pa;

    c.ctx = saved_ctx;

    return result;
}

Result *yy::ProcessBodyNode::translate(FspDriver& c)
{
    DRC(LtsResult, localp, children[0]->translate(c)); /* local_process */

    if (children.size() == 1) {
    } else if (children.size() == 3) {
        /* local_process , local_process_defs */
        DRC(LtsResult, ldefs, children[2]->translate(c));

        localp->val->append(*ldefs->val, 0);
        delete ldefs;
    } else {
        assert(0);
    }

    return localp;
}

Result *yy::AlphaExtNode::translate(FspDriver& c)
{
    /* + set */
    DRC(SetResult, se, children[1]->translate(c));

    return se;
}

void yy::RelabelDefNode::combination(FspDriver& c, Result *r,
                                     string index, bool first)
{
    DRC(RelabelingResult, relab, children[2]->translate(c));
    RelabelingResult *result = result_downcast<RelabelingResult>(r);

    if (first) {
        result->val = relab->val;
    } else {
        result->val.merge(relab->val);
    }
    delete relab;
}

Result *yy::RelabelDefNode::translate(FspDriver& c)
{
    assert(children.size() == 3);

    DTCS(ActionLabelsNode, left, children[0]);
    DTCS(ForallNode, fan, children[0]);
    RelabelingResult *relab = new RelabelingResult;

    if (left) {
        /* action_labels / action_labels */
        DRC(TreeNodeVecResult, l, children[0]->translate(c));
        DRC(TreeNodeVecResult, r, children[2]->translate(c));

        relab->val.add(computeActionLabels(c, SetS(), l->val, 0),
                       computeActionLabels(c, SetS(), r->val, 0));
        delete l;
        delete r;
    } else if (fan) {
        /* FORALL index_ranges braces_relabel_defs */
        DRC(TreeNodeVecResult, ir, children[1]->translate(c));

        /* Translate 'index_ranges' only, and rely on deferred translation
           for 'brace_relabel_defs'. */
        for_each_combination(c, relab, ir->val, this);
        delete ir;
    } else {
        assert(0);
    }

    return relab;
}

Result *yy::RelabelDefsNode::translate(FspDriver& c)
{
    RelabelingResult *result;

    /* relabel_def , relabel_def , ... , relabel_def */
    do {
        DRC(RelabelingResult, rl, children[0]->translate(c));

        result = rl;
    } while (0);

    for (unsigned int i = 2; i < children.size(); i += 2) {
        DRC(RelabelingResult, rl, children[i]->translate(c));

        result->val.merge(rl->val);
    }

    return result;
}

Result *yy::BracesRelabelDefsNode::translate(FspDriver& c)
{
    /* { relabel_defs }*/
    DRC(RelabelingResult, rl, children[1]->translate(c));

    return rl;
}

Result *yy::RelabelingNode::translate(FspDriver& c)
{
    /* / braces_relabel_defs */
    DRC(RelabelingResult, rl, children[1]->translate(c));

    return rl;
}

Result *yy::HidingInterfNode::translate(FspDriver& c)
{
    DTCS(HidingNode, hn, children[0]);
    DTCS(InterfNode, in, children[0]);
    DRC(SetResult, se, children[1]->translate(c));
    HidingResult *result = new HidingResult;

    if (hn) {
        result->val.interface = false;
    } else if (in) {
        result->val.interface = true;
    } else {
        assert(0);
    }

    result->val.setv = se->val;
    delete se;

    return result;
}

Result *yy::IndexRangesNode::translate(FspDriver& c)
{
    /* [ action_range ][ action_range] ... [action_range] */
    TreeNodeVecResult *result = new TreeNodeVecResult;

    /* Translation is deferred: Just collect the children. */
    for (unsigned int i=0; i<children.size(); i+=3) {
        DTC(ActionRangeNode, arn, children[i+1]);

        result->val.push_back(arn);
    }

    return result;
}

void yy::LocalProcessDefNode::combination(FspDriver& c, Result *r,
                                          string index, bool first)
{
    DRC(StringResult, id, children[0]->translate(c));
    /* Translate the LocalProcess using the current context. */
    DRC(LtsResult, lts, children[3]->translate(c));
    LtsResult *result = result_downcast<LtsResult>(r);

    /* Register the local process name (which is the concatenation of
       'process_id' and the 'index_string', e.g. 'P' + '[3][1]') into
       c.unres. The 'define' parameter is true since this is a (local)
       process definition. */
    update_unres(c, id->val + index, lts->val, true, loc);

    if (first) {
        result->val = lts->val;
    } else {
        result->val->append(*lts->val, 0);
    }

    delete id;
    delete lts;
}

Result *yy::LocalProcessDefNode::translate(FspDriver& c)
{
    DRC(TreeNodeVecResult, ir, children[1]->translate(c));
    LtsResult *lts = new LtsResult;

    /* Only translate 'index_ranges', while 'process_id' and 'local_process'
       will be translated in the loop below. */
    for_each_combination(c, lts, ir->val, this);
    delete ir;

    return lts;
}

Result *yy::LocalProcessDefsNode::translate(FspDriver& c)
{
    /* local_process_def , local_process_def, ... , local_process_def */
    LtsResult *result;

    do {
        DRC(LtsResult, lpd, children[0]->translate(c));

        result = lpd;
    } while (0);

    for (unsigned int i=2; i<children.size(); i+=2) {
        DRC(LtsResult, lpd, children[i]->translate(c));

        result->val->append(*lpd->val, 0);
        delete lpd;
    }

    return result;
}

void yy::TreeNode::post_process_definition(FspDriver& c, LtsPtr res,
                                           const string& name)
{
    string extension;

    res->name = name;
    res->cleanup();

    /* Compute the LTS name extension with the parameter values
       used with this translation. */
    lts_name_extension(c.parameters.defaults, extension);
    res->name += extension;

    /* Insert lts into the global 'processes' table. */
    IFD(cout << "Saving " << res->name << "\n");
    if (!c.processes.insert(res->name, res.delegate())) {
	stringstream errstream;

        delete res;
	errstream << "Process " << res->name + extension
                    << " already declared";
	semantic_error(c, errstream, loc);
    }
}

Result *yy::ProcessDefNode::translate(FspDriver& c)
{
    /* property_OPT process_id process_body alpha_ext_OPT
       relabeling_OPT hiding_OPT */
    DTCS(PropertyNode, prn, children[0]);
    DRC(StringResult, id, children[1]->translate(c));
    DRC(LtsResult, body, children[4]->translate(c));
    DRCS(SetResult, alpha, children[5]->translate(c));
    DRCS(RelabelingResult, rl, children[6]->translate(c));
    DRCS(HidingResult, hi, children[7]->translate(c));
    unsigned unres;

    /* The base is the process body. */

    /* Register the process name into c.unres (define is true since
       this is a process definition. */
    update_unres(c, id->val, body->val, true, loc);
#if 0
cout << "UnresolvedNames:\n";
for (unsigned int i=0; i<c.unres.size(); i++) {
    unsigned ui = c.unres.get_idx(i);

    if (ui != LtsNode::NoPriv) {
        cout << "   " << ui << " " << c.unres.get_name(i) << "\n";
    }
}
#endif

    /* Try to resolve all the unresolved nodes into the LTS. */
    unres = body->val->resolve();
    if (unres != LtsNode::NoPriv) {
        stringstream errstream;
        errstream << "process reference " << unres << " unresolved";
        semantic_error(c, errstream, loc);
    }

    /* Merge the End nodes. */
    body->val->mergeEndNodes();

    /* Extend the alphabet. */
    if (alpha) {
        SetS& sv = alpha->val;

        for (unsigned int i=0; i<sv.size(); i++) {
            body->val->updateAlphabet(c.actions.insert(sv[i]));
        }
        delete alpha;
    }

    /* Apply the relabeling operator. */
    if (rl) {
        RelabelingS& rlv = rl->val;

        for (unsigned int i=0; i<rlv.size(); i++) {
            body->val->relabeling(rlv.new_labels[i], rlv.old_labels[i]);
        }
        delete rl;
    }

    /* Apply the hiding/interface operator. */
    if (hi) {
        HidingS& hv = hi->val;

        body->val->hiding(hv.setv, hv.interface);
        delete hi;
    }

    if (prn) {
        /* Apply the property operator, if possible. */
        if (body->val->isDeterministic()) {
            body->val->property();
        } else {
            stringstream errstream;
            errstream << "Cannot apply the 'property' keyword since "
                << id->val << " is a non-deterministic process";
            semantic_error(c, errstream, loc);
        }
    }

    this->post_process_definition(c, body->val, id->val);
    delete body;
    delete id;

    return NULL;
}

Result *yy::ProcessRefNode::translate(FspDriver& c)
{
    /* process_id arguments */
    DRC(StringResult, id, children[0]->translate(c));
    DRCS(IntVecResult, args, children[1]->translate(c));
    LtsResult *lts = new LtsResult;

    process_ref_translate(c, id->val, args ? &args->val : NULL, &lts->val);
    delete id;
    if (args) {
        delete args;
    }

    return lts;
}

Result *yy::SharingNode::translate(FspDriver& c)
{
    /* action_labels :: */
    DRC(TreeNodeVecResult, al, children[0]->translate(c));
    SetResult *result = new SetResult;

    result->val = computeActionLabels(c, SetS(), al->val, 0);
    delete al;

    return result;
}

Result *yy::LabelingNode::translate(FspDriver& c)
{
    /* action_labels : */
    DRC(TreeNodeVecResult, al, children[0]->translate(c));
    SetResult *result = new SetResult;

    result->val = computeActionLabels(c, SetS(), al->val, 0);
    delete al;

    return result;
}

Result *yy::PrioritySNode::translate(FspDriver& c)
{
    DTC(OperatorNode, on, children[0]);
    DRC(SetResult, se, children[1]->translate(c));
    PriorityResult *result = new PriorityResult;

    if (on->sign == ">>") {
        result->val.low = true;
    } else if (on->sign == "<<") {
        result->val.low = false;
    } else {
        assert(0);
    }
    result->val.setv = se->val;
    delete se;

    return result;
}

void yy::CompositeBodyNode::combination(FspDriver& c, Result *r,
                                        string index, bool first)
{
    /* Translate the CompositedBodyNode using the current context. */
    DRC(LtsResult, cb, children[2]->translate(c));
    LtsResult *result = result_downcast<LtsResult>(r);

    if (first) {
        first = false;
        result->val = cb->val;
    } else {
        result->val->compose(*cb->val);
    }
    delete cb;
}

Result *yy::CompositeBodyNode::translate(FspDriver& c)
{
    if (children.size() == 4) {
        /* sharing_OPT labeling_OPT process_ref relabel_OPT */
        DRCS(SetResult, sh, children[0]->translate(c));
        DRCS(SetResult, lb, children[1]->translate(c));
        DRC(LtsResult, pr, children[2]->translate(c));
        DRCS(RelabelingResult, rl, children[3]->translate(c));

        /* Apply the process labeling operator. */
        if (lb) {
            pr->val->labeling(lb->val);
            delete lb;
        }

        /* Apply the process sharing operator. */
        if (sh) {
            pr->val->sharing(sh->val);
            delete sh;
        }

        /* Apply the relabeling operator. */
        if (rl) {
            RelabelingS& rlv = rl->val;

            for (unsigned int i=0; i<rlv.size(); i++) {
                pr->val->relabeling(rlv.new_labels[i], rlv.old_labels[i]);
            }
            delete rl;
        }

        return pr;
    } else if (children.size() == 5) {
        /* IF expression THEN composity_body composite_else_OPT */
        DRC(IntResult, expr, children[1]->translate(c));
        DTCS(CompositeElseNode, cen, children[4]);
        LtsResult *lts = NULL;

        if (expr->val) {
            DRC(LtsResult, cb, children[3]->translate(c));

            lts = cb;
        } else if (cen) {
            DRC(LtsResult, ce, children[4]->translate(c));

            lts = ce;
        } else {
            lts = new LtsResult;
            lts->val = new Lts(LtsNode::Normal, &c.actions);
        }
        delete expr;

        return lts;
    } else if (children.size() == 6) {
        /* sharing_OPT labeling_OPT ( parallel_composition ) relabeling_OPT
         */
        DRCS(SetResult, sh, children[0]->translate(c));
        DRCS(SetResult, lb, children[1]->translate(c));
        DRC(LtsVecResult, pc, children[3]->translate(c));
        DRCS(RelabelingResult, rl, children[5]->translate(c));
        LtsResult *lts = new LtsResult;

        /* Apply the process labeling operator to each component process
           separately, before parallel composition. */
        if (lb) {
            for (unsigned int k=0; k<pc->val.size(); k++) {
                pc->val[k]->labeling(lb->val);
            }
            delete lb;
        }

        /* Apply the process sharing operator (same way). */
        if (sh) {
            for (unsigned int k=0; k<pc->val.size(); k++) {
                pc->val[k]->sharing(sh->val);
            }
            delete sh;
        }
        /* Apply the relabeling operator (same way). */
        if (rl) {
            RelabelingS& rlv = rl->val;

            for (unsigned int k=0; k<pc->val.size(); k++) {
                for (unsigned int i=0; i<rlv.size(); i++) {
                    pc->val[k]->relabeling(rlv.new_labels[i],
                                           rlv.old_labels[i]);
                }
            }
            delete rl;
        }

        /* Apply parallel composition. */
        assert(pc->val.size());
        lts->val = pc->val[0];
        for (unsigned int k=1; k<pc->val.size(); k++) {
            lts->val->compose(*pc->val[k]);
        }
        delete pc;

        return lts;
    } else if (children.size() == 3) {
        /* FORALL index_ranges composite_body */
        DRC(TreeNodeVecResult, ir, children[1]->translate(c));
        LtsResult *lts = new LtsResult;

        /* Only translate 'index_ranges', while 'composite_body'
           will be translated in the loop below. */
        for_each_combination(c, lts, ir->val, this);
        delete ir;

        return lts;
    } else {
        assert(0);
    }

    return NULL;
}

Result *yy::CompositeElseNode::translate(FspDriver& c)
{
    /* ELSE composite_body */
    DRC(LtsResult, cb, children[1]->translate(c));

    return cb;
}

Result *yy::ParallelCompNode::translate(FspDriver& c)
{
    /* composite_body || composite_body || .. || composite_body */
    LtsVecResult *result = new LtsVecResult;

    for (unsigned int i=0; i<children.size(); i+=2) {
        DRC(LtsResult, cb, children[i]->translate(c));

        result->val.push_back(cb->val);
    }

    return result;
}

Result *yy::CompositeDefNode::translate(FspDriver& c)
{
    /* process_id composite_body priority_OPT hiding_OPT */
    DRC(StringResult, id, children[1]->translate(c));
    DRC(LtsResult, body, children[4]->translate(c));
    DRCS(PriorityResult, pr, children[5]->translate(c));
    DRCS(HidingResult, hi, children[6]->translate(c));

    /* The base is the composite body. */

    /* Apply the priority operator. */
    if (pr) {
        body->val->priority(pr->val.setv, pr->val.low);
        delete pr;
    }

    /* Apply the hiding/interface operator. */
    if (hi) {
        HidingS& hv = hi->val;

        body->val->hiding(hv.setv, hv.interface);
        delete hi;
    }

    this->post_process_definition(c, body->val, id->val);
    delete id;
    delete body;

    return NULL;
}

