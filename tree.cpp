#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "tree.hpp"
#include "driver.hpp"
#include "unresolved.hpp"
#include "helpers.hpp"

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
static void update_unres(FspDriver& c, const string& name,
                         Lts& lts, bool define, const yy::location& loc)
{
    unsigned int ui;

    if (define && c.unres.defined(name)) {
        /* A process name cannot be defined twice. */
        stringstream errstream;
        errstream << "Process " << name << " defined twice";
        semantic_error(c, errstream, loc);
    }

    if (lts.get_priv(0) == ~0U) {
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
        lts.set_priv(0, ui);  /* Record the alias into the 'lts[0]' priv. */
    } else {
        /* If 'lts[0]' does have its 'priv' set, it means that 'lts[0]'
           is not unresolved, and there is already an alias assigned to it.

           Tell 'unres' that the 'lts[0]' priv field must be an alias also
           for 'name'. */
        ui = c.unres.append(name, lts.get_priv(0), define);
        /* Update all the 'priv' fields that have the 'idx' previously
           associated to 'name', if any. */
        if (ui != ~0U) {
            lts.replace_priv(lts.get_priv(0), ui);
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

void yy::TreeNode::translate(FspDriver& c)
{
    translate_children(c);
}

/* ========================== Translation methods ======================== */

void yy::RootNode::translate(FspDriver& c)
{
    for (unsigned int i=0; i<children.size(); i++) {
        if (children[i]) {
            DTCS(ProcessDefNode, pdn, children[i]);
            DTCS(CompositeDefNode, cdn, children[i]);

            if (pdn || cdn) {
                /* Save the compiler context (in this case, an empty
                   context). */
                c.nesting_save(false);
            }
            children[i]->translate(c);
            if (pdn || cdn) {
                /* Restore the saved context (empty). */
                c.nesting_restore();
            }
        }
    }
}

void yy::VariableIdNode::translate(FspDriver& c)
{
    DTC(LowerCaseIdNode, lc, children[0]);

    res = lc->res;
}

void yy::ConstantIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::RangeIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::SetIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::ConstParameterIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::ParameterIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::ProcessIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::ProgressIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
}

void yy::MenuIdNode::translate(FspDriver& c)
{
    DTC(UpperCaseIdNode, uc, children[0]);

    res = uc->res;
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
            assert(0);
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
            } else if (o->sign == ">") {
                res = (l->res > r->res);
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
                assert(0);
            }
        }
        
    } else {
        assert(0);
    }
}

void yy::BaseExpressionNode::translate(FspDriver& c)
{
    translate_children(c);

    DTCS(IntegerNode, in, children[0]);
    DTCS(VariableIdNode, vn, children[0]);
    DTCS(ConstParameterIdNode, cn, children[0]);

    if (in) {
        res = in->res;
    } else if (vn) {
        string val;

        if (!c.ctx.lookup(vn->res, val)) {
            stringstream errstream;
            errstream << "variable " << vn->res << " undeclared";
            semantic_error(c, errstream, loc);
        }
        res = string2int(val);
    } else if (cn) {
        Symbol *svp;
        ConstS *cvp;

        if (!c.identifiers.lookup(cn->res, svp)) {
            stringstream errstream;
            errstream << "const/parameter " << cn->res << " undeclared";
            semantic_error(c, errstream, loc);
        }
        cvp = err_if_not<ConstS>(c, svp, loc);
        res = cvp->value;
    } else {
        assert(0);
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
        Symbol *svp;
        RangeS *rvp;

        if (!c.identifiers.lookup(ri->res, svp)) {
            stringstream errstream;
            errstream << "range " << ri->res << " undeclared";
            semantic_error(c, errstream, loc);
        }
        rvp = err_if_not<RangeS>(c, svp, loc);
        res = *rvp;
    } else if (re) {
        /* Return the range expression. */
        res = re->res;
    } else {
        assert(0);
    }
}

void yy::ConstantDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ConstantIdNode, id, children[1]);
    DTC(ExpressionNode, en, children[3]);
    ConstS *cvp = new ConstS;

    cvp->value = en->res;
    if (!c.identifiers.insert(id->res, cvp)) {
        stringstream errstream;
        errstream << "const " << id->res << " declared twice";
        delete cvp;
        semantic_error(c, errstream, loc);
    }
}

void yy::RangeDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(RangeIdNode, id, children[1]);
    DTC(ExpressionNode, len, children[3]);
    DTC(ExpressionNode, ren, children[5]);
    RangeS *rvp = new RangeS;

    rvp->low = len->res;
    rvp->high = ren->res;
    if (!c.identifiers.insert(id->res, rvp)) {
        stringstream errstream;
        errstream << "range " << id->res << " declared twice";
        delete rvp;
        semantic_error(c, errstream, loc);
    }
}

void yy::SetDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(SetIdNode, id, children[1]);
    DTC(SetExprNode, sen, children[3]);
    SetS *svp = new SetS;

    *svp = sen->res;
    if (!c.identifiers.insert(id->res, svp)) {
        stringstream errstream;
        errstream << "set " << id->res << " declared twice";
        delete svp;
        semantic_error(c, errstream, loc);
    }
}

void yy::PropertyDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ProcessDefNode, pdn, children[1]);
    Symbol *svp;
    yy::Lts *lts;

    /* Lookup the process name that has just been inserted by
       ProcessDefNode::translate. */
    if (!c.processes.lookup(pdn->res.name, svp)) {
        assert(0);
    }

    lts = is<yy::Lts>(svp);
    /* Apply the property operator, if possible. */
    if (lts->isDeterministic()) {
        lts->property();
    } else {
        stringstream errstream;
        errstream << "Cannot apply the 'property' keyword since "
            << lts->name << " is a non-deterministic process";
        semantic_error(c, errstream, loc);
    }
}

void yy::ProgressDefNode::combination(FspDriver& c, string index, bool first)
{
    DTC(ProgressIdNode, id, children[1]);
    ProgressS *pv = new ProgressS;
    string name = id->res + index;

    if (children.size() == 5) {
        /* Progress definition in unconditional form (normal form). */
        DTC(SetNode, sn, children[4]);

        /* Do the set translation here. */
        sn->translate(c);

        pv->conditional = false;
        sn->res.toActionSetValue(c.actions, pv->set);
    } else if (children.size() == 8) {
        /* Progress definition in conditional form. */
        DTC(SetNode, cn, children[5]);
        DTC(SetNode, sn, children[7]);

        /* Do the set translation here. */
        cn->translate(c);
        sn->translate(c);

        pv->conditional = true;
        cn->res.toActionSetValue(c.actions, pv->condition);
        sn->res.toActionSetValue(c.actions, pv->set);
    } else {
        assert(0);
    }

    if (!c.progresses.insert(name, pv)) {
        stringstream errstream;
        errstream << "progress " << name << " declared twice";
        semantic_error(c, errstream, loc);
    }
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

static void for_each_combination(FspDriver& c, IndexRangesNode *irn,
                                 TreeNode *n)
{
    const vector<TreeNode *>& elements = irn->res;
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

        n->combination(c, index_string, first);
        first = false;

        /* Restore the saved context. */
        c.ctx = ctx;

        /* Increment 'indexes' for the next 'index_string', and exits if
           there are no more combinations. */
    } while (next_set_indexes(elements, indexes));
}

void yy::ProgressDefNode::translate(FspDriver& c)
{
    DTC(ProgressIdNode, id, children[1]);
    DTCS(IndexRangesNode, irn, children[2]);

    id->translate(c);
    irn->translate(c);

    for_each_combination(c, irn, this);
}

void yy::MenuDefNode::translate(FspDriver &c)
{
    translate_children(c);

    DTC(MenuIdNode, id, children[1]);
    DTC(SetNode, sn, children[3]);
    ActionSetS *asv = new ActionSetS;

    /* Turn the SetS contained into the SetNode into an
       ActionSetS. */
    sn->res.toActionSetValue(c.actions, *asv);

    if (!c.menus.insert(id->res, asv)) {
        stringstream errstream;
        errstream << "menu " << id->res << " declared twice";
        semantic_error(c, errstream, loc);
    }
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

        base = SetS();
        if (strn) {
            /* Single action. */
            base += strn->res;
         } else if (setn) {
            /* A set of actions. */
            base += setn->res;
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
                SetS ret;
                SetS next_base;
                bool ok;

                for (unsigned int j=0; j<an->res.size(); j++) {
                    next_base = base;
                    next_base.indexize(an->res[j]);
                    if (!c.ctx.insert(an->res.variable, an->res[j])) {
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
            assert(0);
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

    res = SetS();
    for (unsigned int i=0; i<children.size(); i+=2) {
        DTC(ActionLabelsNode, an, children[i]);

        res += computeActionLabels(c, SetS(), an->res, 0);
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
        Symbol *svp;
        SetS *setvp;

        if (!c.identifiers.lookup(si->res, svp)) {
            stringstream errstream;
            errstream << "set " << si->res << " undeclared";
            semantic_error(c, errstream, loc);
        }
        setvp = err_if_not<SetS>(c, svp, loc);
        res = *setvp;
    } else if (se) {
        /* Return the set expression. */
        res = se->res;
    } else {
        assert(0);
    }
}

void yy::ActionRangeNode::translate(FspDriver& c)
{
    translate_children(c);

    res = SetS();

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
            assert(0);
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
            assert(0);
        }
if (res.size() == 0) { // XXX remove a.s.a.p.
    cout << "XXX2\n";
    res += "xxx2";
}
    } else {
        assert(0);
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
                res.push_back(strn);
            } else if (setn) {
                res.push_back(setn);
            } else {
                assert(0);
            }
            i += 2;
        } else if (sqn) {
            /* Recognize the "[]" operator; Must follow an action range. */
            DTC(ActionRangeNode, an, children[i+1]);

            res.push_back(an);
            i += 3;
        } else {
            assert(0);
            break;
        }
    }
}

yy::Lts yy::TreeNode::computePrefixActions(FspDriver& c,
                                           const vector<TreeNode *>& als,
                                           unsigned int idx,
                                           vector<Context>& ctxcache)
{
    assert(idx < als.size());
    DTC(ActionLabelsNode, an, als[idx]);
    const vector<TreeNode *>& elements = an->res;
    vector<unsigned int> indexes(elements.size());
    Lts lts(LtsNode::Normal, &c.actions);
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
                    label = setn->res[ indexes[j] ];
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
                    label += "." + strn->res;
                } else if (setn) {
                    label += "." + setn->res[ indexes[j] ];
                } else if (an) {
                    label += "." + an->res[ indexes[j] ];
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

void yy::IndicesNode::translate(FspDriver& c)
{
    translate_children(c);

    res = string();

    for (unsigned int i=0; i<children.size(); i+=3) {
        DTC(ExpressionNode, en, children[i+1]);

        res += "." + int2string(en->res);
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
        DTCS(IndicesNode, ixn, children[1]);
        string name = in->res;

        /* Create an LTS containing a single unresolved
           node. */
        res = Lts(LtsNode::Unresolved, &c.actions);
        if (ixn) {
            name += ixn->res;
        }
        /* Tell the unresolved names table that there
           is a new unresolved name (define is false
           because this is not a process definition,
           the process name is only referenced). */
        update_unres(c, name, res, false, loc);
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

void yy::ArgumentListNode::translate(FspDriver& c)
{
    translate_children(c);

    res.clear();
    for (unsigned int i=0; i<children.size(); i++) {
        DTC(ExpressionNode, en, children[i]);

        res.push_back(en->res);
    }
}

void yy::ArgumentsNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ArgumentListNode, al, children[1]);

    res = al->res;
}

void yy::TreeNode::process_ref_translate(FspDriver& c, yy::Lts& res)
{
    translate_children(c);

    DTC(ProcessIdNode, in, children[0]);
    DTCS(ArgumentsNode, an, children[1]);
    Symbol *svp;
    ParametricProcess *pp;
    vector<int> arguments;
    string extension;

    /* Lookup 'process_id' in the 'parametric_process' table. */
    if (!c.parametric_processes.lookup(in->res, svp)) {
	stringstream errstream;
	errstream << "Process " << in->res << " undeclared";
	semantic_error(c, errstream, loc);
    }
    pp = is<ParametricProcess>(svp);

    /* Find the arguments for the process parameters. */
    arguments = an ? an->res : pp->defaults;

    if (arguments.size() != pp->defaults.size()) {
	stringstream errstream;
	errstream << "Parameters mismatch";
	semantic_error(c, errstream, loc);  // XXX tr.locations[1]
    }

    lts_name_extension(arguments, extension);

    /* We first lookup the global processes table in order to see if
       we already have the requested LTS. */
    IFD(cout << "Looking up " << in->res+extension << "\n");
    if (c.processes.lookup(in->res + extension, svp)) {
	/* If there is a cache hit, use the stored LTS. */
	res = *(is<yy::Lts>(svp));
    } else {
        ProcessDefNode *pdn;
        CompositeDefNode *cdn;

	/* If there is a cache miss, we have to compute the requested LTS
	   using the translate method and save it in the global processes
           table. */
        pdn = dynamic_cast<ProcessDefNode *>(pp->translator);
        cdn = dynamic_cast<CompositeDefNode *>(pp->translator);
        assert(pdn || cdn);
        /* Save and reset the compiler context. It must be called before
           inserting the parameters into c.paramproc (see the following
           for loop). */
        c.nesting_save(true);
        /* Insert the arguments into the identifiers table, taking care
           of overridden names. */
        for (unsigned int i=0; i<pp->names.size(); i++) {
            Symbol *svp;
            ConstS *cvp = new ConstS();

            if (c.identifiers.lookup(pp->names[i], svp)) {
                /* If there is already an identifier with the same name as
                   the i-th parameter, override temporarly the identifier. */
                c.overridden_names.push_back(pp->names[i]);
                c.overridden_values.push_back(svp->clone());
                c.identifiers.remove(pp->names[i]);
            }

            cvp->value = arguments[i];
            if (!c.identifiers.insert(pp->names[i], cvp)) {
                assert(0);
                delete cvp;
            }
            c.paramproc.insert(pp->names[i], arguments[i]);
        }
        /* Do the translation an grab the result. The new LTS is
           stored in the 'processes' table by the translate function. */
        if (pdn) {
            pdn->translate(c);
            res = pdn->res;
        } else {
            cdn->translate(c);
            res = cdn->res;
        }
        /* Restore the previously saved compiler context. */
        c.nesting_restore();
    }
}

void yy::ProcessRefSeqNode::translate(FspDriver& c)
{
    process_ref_translate(c, res);
}

void yy::SeqProcessListNode::translate(FspDriver& c)
{
    translate_children(c);

    do {
        DTC(ProcessRefSeqNode, pr, children[0]);

        res = pr->res;
    } while (0);

    for (unsigned int i=2; i<children.size(); i+=2) {
        DTC(ProcessRefSeqNode, pr, children[i]);

        res.endcat(pr->res);
    }
}

void yy::SeqCompNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(SeqProcessListNode, pl, children[0]);
    DTC(BaseLocalProcessNode, lp, children[2]);

    res = pl->res;
    res.endcat(lp->res);
}

void yy::LocalProcessNode::translate(FspDriver& c)
{
    translate_children(c);

    if (children.size() == 1) {
        DTCS(BaseLocalProcessNode, b, children[0]);
        DTCS(SeqCompNode, sc, children[0]);

        if (b) {
            res = b->res;
        } else if (sc) {
            res = sc->res;
        } else {
            assert(0);
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
        assert(0);
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
    vector<Context> ctxcache;
    Context saved_ctx = c.ctx;

    DTCS(GuardNode, gn, children[0]);
    DTC(PrefixActionsNode, pn, children[1]);
    DTC(LocalProcessNode, lp, children[3]);

    /* Don't translate 'lp', since it will be translated into the loop,
       with proper context. */
    if (gn) {
        gn->translate(c);
    }
    pn->translate(c);

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

    DTC(LocalProcessNode, pn, children[0]);

    if (children.size() == 1) {
        res = pn->res;
    } else if (children.size() == 3) {
        DTC(LocalProcessDefsNode, lpd, children[2]);

        res = pn->res;
        res.append(lpd->res, 0);
    } else {
        assert(0);
    }
}

void yy::AlphaExtNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(SetNode, sn, children[1]);

    res = sn->res;
}

void yy::RelabelDefNode::combination(FspDriver& c, string index, bool first)
{
    DTC(BracesRelabelDefsNode, br, children[2]);

    br->translate(c);

    if (first) {
        res = br->res;
    } else {
        res.merge(br->res);
    }
}

void yy::RelabelDefNode::translate(FspDriver& c)
{
    assert(children.size() == 3);

    DTCS(ActionLabelsNode, left, children[0]);
    DTCS(ForallNode, fan, children[0]);

    res = RelabelingS();

    if (left) {
        DTC(ActionLabelsNode, right, children[2]);

        /* Translate here, because we don't want to translate everything in
           the other branch. */
        translate_children(c);

        res.add(computeActionLabels(c, SetS(), left->res, 0),
                computeActionLabels(c, SetS(), right->res, 0));
    } else if (fan) {
        /* FORALL index_ranges braces_relabel_defs */
        DTC(IndexRangesNode, irn, children[1]);

        /* Translate 'index_ranges' only, and rely on deferred translation
           for 'brace_relabel_defs'. */
        irn->translate(c);
        for_each_combination(c, irn, this);
    } else {
        assert(0);
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

    res = HidingS();

    if (hn) {
        res.interface = false;
    } else if (in) {
        res.interface = true;
    } else {
        assert(0);
    }

    res.setv = sn->res;
}

void yy::ParameterNode::translate(FspDriver& c)
{
    if (c.replay) {
        /* When c.replay is true, it means that we are translating a process
           because of a process reference (e.g. ProcessRefSeqNode). In
           this case we don't want to translate the parse tree nodes
           corresponding to parameters, because the identifiers corresponding
           to the process arguments have already been inserted into the
           identifiers table. */
        return;
    }

    translate_children(c);

    DTC(ParameterIdNode, in, children[0]);
    DTC(ExpressionNode, en, children[2]);
    ConstS *cvp;
    Symbol *svp;

    /* Save the parameter name for subsequent removal. If there is
       already a parameter with the same name we report an error. */
    if (!c.paramproc.insert(in->res, en->res)) {
        stringstream errstream;
        errstream << "parameter " << in->res << " declared twice";
        semantic_error(c, errstream, loc);
    }

    if (c.identifiers.lookup(in->res, svp)) {
        /* If there is already an identifier with the same name as
           this parameter, override temporarly the identifier. */
        c.overridden_names.push_back(in->res);
        c.overridden_values.push_back(svp->clone());
        c.identifiers.remove(in->res);
    }

    /* Insert the parameter into the identifiers table. Here we cannot
       fail because of of the previous two operations. */
    cvp = new ConstS;
    cvp->value = en->res;
    if (!c.identifiers.insert(in->res, cvp)) {
        assert(0);
    }
}

void yy::IndexRangesNode::translate(FspDriver& c)
{
    /* Translation is deferred: Just collect the children. */
    res.clear();

    for (unsigned int i=0; i<children.size(); i+=3) {
        DTC(ActionRangeNode, arn, children[i+1]);

        res.push_back(arn);
    }
}

void yy::LocalProcessDefNode::combination(FspDriver& c, string index,
                                          bool first)
{
    DTC(ProcessIdNode, in, children[0]);
    DTC(LocalProcessNode, lp, children[3]);

    /* Translate the LocalProcess using the current context. */
    lp->translate(c);

    /* Register the local process name (which is the concatenation of
       'process_id' and the 'index_string', e.g. 'P' + '[3][1]') into
       c.unres. The 'define' parameter is true since this is a (local)
       process definition. */
    update_unres(c, in->res + index, lp->res, true, loc);

    if (first) {
        res = lp->res;
    } else {
        res.append(lp->res, 0);
    }
}

void yy::LocalProcessDefNode::translate(FspDriver& c)
{
    DTC(ProcessIdNode, in, children[0]);
    DTC(IndexRangesNode, irn, children[1]);

    /* Only translate 'process_id' and 'index_ranges', while 'local_process'
       will be translated in the loop below. */
    in->translate(c);
    irn->translate(c);

    for_each_combination(c, irn, this);
}

void yy::LocalProcessDefsNode::translate(FspDriver& c)
{
    translate_children(c);

    do {
        DTC(LocalProcessDefNode, lpd, children[0]);

        res = lpd->res;
    } while (0);

    for (unsigned int i=2; i<children.size(); i+=2) {
        DTC(LocalProcessDefNode, lpd, children[i]);

        res.append(lpd->res, 0);
    }
}

void yy::TreeNode::post_process_definition(FspDriver& c, Lts& res,
                                           const string& name)
{
    string extension;
    Symbol *res_clone;
    ParametricProcess *pp_clone = is<ParametricProcess>(c.paramproc.clone());

    res.name = name;

    if (!c.replay) {
        /* Store c.paramproc in parametric_processes. */
        pp_clone->set_translator(this);
        if (!c.parametric_processes.insert(res.name, pp_clone)) {
            stringstream errstream;

            delete pp_clone;
            errstream << "Parametric process " << res.name
                << " already declared";
            semantic_error(c, errstream, loc);
        }
    }

    /* Compute the LTS name extension. */
    lts_name_extension(c.paramproc.defaults, extension);
    res.name += extension;

    res_clone = res.clone();
    /* Insert lts into the global 'processes' table. */
    IFD(cout << "Saving " << res.name << "\n");
    if (!c.processes.insert(res.name, res_clone)) {
	stringstream errstream;

        delete res_clone;
	errstream << "Process " << res.name + extension
                    << " already declared";
	semantic_error(c, errstream, loc);
    }
}

void yy::ProcessDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ProcessIdNode, idn, children[0]);
    DTC(ProcessBodyNode, bn, children[3]);
    DTCS(AlphaExtNode, aen, children[4]);
    DTCS(RelabelingNode, rn, children[5]);
    DTCS(HidingInterfNode, hin, children[6]);
    unsigned unres;

    /* The base is the process body. */
    res = bn->res;

    /* Register the process name into c.unres (define is true since
       this is a process definition. */
    update_unres(c, idn->res, res, true, loc);
#if 0
cout << "UnresolvedNames:\n";
for (unsigned int i=0; i<c.unres.size(); i++) {
    unsigned ui = c.unres.get_idx(i);

    if (ui != ~0U) {
        cout << "   " << ui << " " << c.unres.get_name(i) << "\n";
    }
}
#endif

    /* Try to resolve all the unresolved nodes into the LTS. */
    unres = res.resolve();
    if (unres != ~0U) {
        stringstream errstream;
        errstream << "process reference " << unres << " unresolved";
        semantic_error(c, errstream, loc);
    }

    /* Merge the End nodes. */
    res.mergeEndNodes();

    /* Extend the alphabet. */
    if (aen) {
        SetS& sv = aen->res;

        for (unsigned int i=0; i<sv.size(); i++) {
            res.updateAlphabet(c.actions.insert(sv[i]));
        }
    }

    /* Apply the relabeling operator. */
    if (rn) {
        RelabelingS& rlv = rn->res;

        for (unsigned int i=0; i<rlv.size(); i++) {
            res.relabeling(rlv.new_labels[i], rlv.old_labels[i]);
        }
    }

    /* Apply the hiding/interface operator. */
    if (hin) {
        HidingS& hv = hin->res;

        res.hiding(hv.setv, hv.interface);
    }

    this->post_process_definition(c, res, idn->res);
}

void yy::ProcessRefNode::translate(FspDriver& c)
{
    process_ref_translate(c, res);
}

void yy::SharingNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ActionLabelsNode, an, children[0]);

    res = computeActionLabels(c, SetS(), an->res, 0);
}

void yy::LabelingNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ActionLabelsNode, an, children[0]);

    res = computeActionLabels(c, SetS(), an->res, 0);
}

void yy::PrioritySNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(OperatorNode, on, children[0]);
    DTC(SetNode, sn, children[1]);

    if (on->sign == ">>") {
        res.low = true;
    } else if (on->sign == "<<") {
        res.low = false;
    } else {
        assert(0);
    }
    res.setv = sn->res;
}

void yy::CompositeBodyNode::combination(FspDriver& c, string index,
                                        bool first)
{
    DTC(CompositeBodyNode, cb, children[2]);

    /* Translate the LocalProcess using the current context. */
    cb->translate(c);

    if (first) {
        first = false;
        res = cb->res;
    } else {
        res.compose(cb->res);
    }
}

void yy::CompositeBodyNode::translate(FspDriver& c)
{
    if (children.size() != 3) {
        /* When children.size() == 3, we have deferred translation, and so
           we don't have to do the translation here. */
        translate_children(c);
    }

    if (children.size() == 4) {
        /* sharing_OPT labeling_OPT process_ref relabel_OPT */
        DTCS(SharingNode, shn, children[0]);
        DTCS(LabelingNode, lbn, children[1]);
        DTC(ProcessRefNode, prn, children[2]);
        DTCS(RelabelingNode, rln, children[3]);

        res = prn->res;

        /* Apply the process labeling operator. */
        if (lbn) {
            res.labeling(lbn->res);
        }

        /* Apply the process sharing operator. */
        if (shn) {
            res.sharing(shn->res);
        }

        /* Apply the relabeling operator. */
        if (rln) {
            RelabelingS& rlv = rln->res;

            for (unsigned int i=0; i<rlv.size(); i++) {
                res.relabeling(rlv.new_labels[i], rlv.old_labels[i]);
            }
        }
    } else if (children.size() == 5) {
        /* IF expression THEN composity_body composite_else_OPT */
        DTC(ExpressionNode, en, children[1]);
        DTC(CompositeBodyNode, cb, children[3]);
        DTCS(CompositeElseNode, ce, children[4]);

        if (en->res) {
            res = cb->res;
        } else if (ce) {
            res = ce->res;
        } else {
            res = Lts(LtsNode::Normal, &c.actions);
        }
    } else if (children.size() == 6) {
        /* sharing_OPT labeling_OPT ( parallel_composition ) relabeling_OPT
         */
        DTCS(SharingNode, shn, children[0]);
        DTCS(LabelingNode, lbn, children[1]);
        DTC(ParallelCompNode, pcn, children[3]);
        DTCS(RelabelingNode, rln, children[5]);

        /* Apply the process labeling operator to each component process
           separately, before parallel composition. */
        if (lbn) {
            for (unsigned int k=0; k<pcn->res.size(); k++) {
                pcn->res[k].labeling(lbn->res);
            }
        }

        /* Apply the process sharing operator (same way). */
        if (shn) {
            for (unsigned int k=0; k<pcn->res.size(); k++) {
                pcn->res[k].sharing(shn->res);
            }
        }
        /* Apply the relabeling operator (same way). */
        if (rln) {
            RelabelingS& rlv = rln->res;

            for (unsigned int k=0; k<pcn->res.size(); k++) {
                for (unsigned int i=0; i<rlv.size(); i++) {
                    pcn->res[k].relabeling(rlv.new_labels[i],
                                           rlv.old_labels[i]);
                }
            }
        }

        /* Apply parallel composition. */
        assert(pcn->res.size());
        res = pcn->res[0];
        for (unsigned int k=1; k<pcn->res.size(); k++) {
            res.compose(pcn->res[k]);
        }

    } else if (children.size() == 3) {
        /* FORALL index_ranges composite_body */
        DTC(IndexRangesNode, irn, children[1]);

        /* Only translate 'index_ranges', while 'composite_body'
           will be translated in the loop below. */
        irn->translate(c);

        for_each_combination(c, irn, this);
    } else {
        assert(0);
    }
}

void yy::CompositeElseNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(CompositeBodyNode, cb, children[1]);

    res = cb->res;
}

void yy::ParallelCompNode::translate(FspDriver& c)
{
    translate_children(c);

    res.clear();

    for (unsigned int i=0; i<children.size(); i+=2) {
        DTC(CompositeBodyNode, cb, children[i]);

        res.push_back(cb->res);
    }
}

void yy::CompositeDefNode::translate(FspDriver& c)
{
    translate_children(c);

    DTC(ProcessIdNode, idn, children[1]);
    DTC(CompositeBodyNode, cb, children[4]);
    DTCS(PrioritySNode, pr, children[5]);
    DTCS(HidingInterfNode, hin, children[6]);

    /* The base is the composite body. */
    res = cb->res;

    /* Apply the priority operator. */
    if (pr) {
        res.priority(pr->res.setv, pr->res.low);
    }

    /* Apply the hiding/interface operator. */
    if (hin) {
        HidingS& hv = hin->res;

        res.hiding(hv.setv, hv.interface);
    }

    this->post_process_definition(c, res, idn->res);
}

