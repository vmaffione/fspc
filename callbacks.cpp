/*
 *  fspc semantic action callbacks
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


/* Inline utilities. */
#include "utils.hpp"

/* Context, ContextsSet and ContextsSetStack. */
#include "context.hpp"

/* Definition of the FspTranslator class. */
#include "translator.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

#include "callbacks.hpp"


//#define DEBUG
#ifdef DEBUG
#define PROX(x) cout<<"PROX: ";x;cout<<"\n"
#define PROP(x) cout<<"PROP:==============================================================\n      " << x << "\n"
#else
#define PROP(x)
#define PROX(x)
#endif


/* Replay a sequence of callbacks under a local translator and a local
   stack. */
Lts * ParametricProcess::replay(struct FspCompiler& c,
					    const vector<int>& values)
{
    vector<void *> stack;
    FspTranslator tr(c);
    ConstValue * cvp;
    SymbolValue * svp;
    vector<string> overridden_names;
    vector<SymbolValue *> overridden_values;

    PROX(cout << "START Replay!!\n");
    assert(values.size() == parameter_defaults.size());
    assert(parameter_defaults.size() == parameter_names.size());

    /* Load the parameters into c.identifiers. */
    for (int i=0; i<parameter_names.size(); i++) {
	/* Save the overridden identifiers, if any. */
	if (c.identifiers.lookup(parameter_names[i], svp)) {
	    overridden_names.push_back(parameter_names[i]);
	    overridden_values.push_back(svp->clone());
	    c.identifiers.remove(parameter_names[i]);
	}

	cvp = new ConstValue;
	cvp->value = values[i];
	if (!c.identifiers.insert(parameter_names[i], cvp)) {
	    /* This should never happen. */
	    assert(0);
	}
    }

    /* Replay the callbacks. */
    for (int i=0; i<record.size(); i++) {
	PROX(cout << record[i]->is_void() << ", " << stack.size() << " ";record[i]->print());
	void * ret = record[i]->execute(tr, stack);
	if (!(record[i]->is_void()))
	    stack.push_back(ret);
    }
    assert(stack.size() == 1);

    /* Remove the parameters from c.identifiers. */
    for (int i=0; i<parameter_names.size(); i++)
	c.identifiers.remove(parameter_names[i]);

    /* Restore overridden parameters, if any. */
    for (int i=0; i<overridden_names.size(); i++)
	if(!c.identifiers.insert(overridden_names[i],
					    overridden_values[i])) {
	    /* This should never happen. */
	    assert(0);
	}
    PROX(cout << "END Replay!!\n");

    return static_cast<class Lts *>(stack.back());
}

void ParametricProcess::print() const
{
    cout << "Parameters (defaults):\n";
    for (int i=0; i<parameter_names.size(); i++)
	cout << parameter_names[i] << " (" << parameter_defaults[i] << ")\n";
    cout << "   List of " << record.size() << " callbacks\n";
}


ParametricProcess* err_if_not_parametric(SymbolValue * svp,
						const struct YYLTYPE& loc)
{
    if (svp->type() != SymbolValue::ParametricProcess) {
	stringstream errstream;
	errstream << "Parametric process expected";
	semantic_error(errstream, loc);
    }

    return static_cast<ParametricProcess *>(svp);
}


void lts_name_extension(const vector<int>& values, string& extension)
{
    extension = "";

    if (values.size()) {
	int i = 0;

	extension = "(";
	for (; i<values.size() - 1; i++) {
	   extension += int2string(values[i]) + ",";
	}
	extension += int2string(values[i]) + ")";
    }
}

/* Helper function used with T=string and T=int. */
template <class T>
static Context* extended_context(struct FspTranslator& tr, Context * ctx,
				const string& var, const T& val)
{
    ctx = new Context(*ctx);
    if (!ctx->insert(var, val)) {
	stringstream errstream;
	errstream << "Variable " << var << " declared twice";
	semantic_error(errstream, tr.locations[0]);
    }

    return ctx;
}

/* Left contains a SetValue*, while right is the result of 'action_range'.
   Returns a SetValue*. */
SvpVec * indexize_svpvec(struct FspTranslator& tr, SvpVec * left,
							    SvpVec * right)
{
    SvpVec * vp;
    SetValue * setvp;
    SetValue * rsetvp;
    RangeValue * rvp;
    ConstValue * cvp;
    ContextsSet * csp; 
    Context * cxp;

    /* type() and 'variable' is the same for all the elements in 
       action_range. */
    switch (right->v[0]->type()) {
	case SymbolValue::Range:
	    rvp = is_range(right->v[0]);
	    if (rvp->variable != "") {
		/* In this case each element in action_range is in the
		   form 'var:R', where R is a range_id or a range_expr. */
		csp = new ContextsSet;
		csp->frontier = tr.current_contexts().frontier;
		vp = new SvpVec;
		for (int c=0; c<left->v.size(); c++) {
		    setvp = is_set(left->v[c]);
		    rvp = is_range(right->v[c]);
		    for (int j=rvp->low; j<=rvp->high; j++) {
			SetValue * new_setvp = new SetValue(*setvp);
			new_setvp->indexize(j);
			/* When a context spread happens, each new_setvp
			   inherits the rank of the generating setvp. */
			new_setvp->rank = setvp->rank;
			/* Append the new SetValue to associate to this
			   context.*/
			vp->v.push_back(new_setvp);
			/* Create a new context (extending the previous
			   one) */
			cxp = extended_context(tr, tr.current_contexts()[c],
				rvp->variable, j);
			csp->append(cxp);
		    }
		}
		tr.css.update(csp);
		delete left;
		delete right;
		return vp;
	    } else {
		/* In this case each element in action_range is in the
		   form "R", where R is a range_id or a range_expr. */
		for (int c=0; c<left->v.size(); c++) {
		    setvp = is_set(left->v[c]);
		    rvp = is_range(right->v[c]);
		    setvp->indexize(rvp->low, rvp->high);
		    /* Rank is inherited. */
		}
		delete right;
		return left;
	    }
	    break;

	case SymbolValue::Const:
	    for (int c=0; c<left->v.size(); c++) {
		setvp = is_set(left->v[c]);
		cvp = is_const(right->v[c]);
		setvp->indexize(cvp->value);
		/* Rank is inherited. */
	    }
	    delete right;
	    return left;
	    break;
	case SymbolValue::Set:
	    setvp = is_set(right->v[0]);
	    if (setvp->variable != "") {
		/* In this case each element in action_range is in the
		   form 'var:R', where R is a set_id or a set_expr. */
		csp = new ContextsSet;
		csp->frontier = tr.current_contexts().frontier;
		/* We need a new SvpVec for a new ContextSet. */
		vp = new SvpVec;
		for (int c=0; c<left->v.size(); c++) {
		    setvp = is_set(left->v[c]);
		    rsetvp = is_set(right->v[c]);
		    for (int j=0; j<rsetvp->actions.size(); j++) {
			SetValue * new_setvp = new SetValue(*setvp);
			new_setvp->dotcat(rsetvp->actions[j]);
			/* When a context spread happens, each new_setvp
			   inherits the rank of the generating setvp. */
			new_setvp->rank = setvp->rank;
			/* Append the new SetValue to associate to this
			   context.*/
			vp->v.push_back(new_setvp);
			/* Create a new context (extending the previous
			   one) */
			cxp = extended_context(tr, tr.current_contexts()[c],
				rsetvp->variable, rsetvp->actions[j]);
			/* Append the new Context to the new ContextSet. 
			 */
			csp->append(cxp);
		    }
		}
		tr.css.update(csp);
		delete left;
		delete right;
		return vp;
	    } else {
		for (int c=0; c<left->v.size(); c++) {
		    setvp = is_set(left->v[c]);
		    rsetvp = is_set(right->v[c]);
		    /* Create a new SetValue that combines setvp and
		       rsetvp. No context ramification happens here. */
		    setvp->indexize(*rsetvp);
		    /* Rank is inherited. */
		}
		delete right;
		return left;
	    }
	    break;

	default:
	    /* We should not reach this point. */
	    assert(0);
	    exit(1);
    }
}

void merge_lts_by_rank(FspTranslator& tr, SvpVec * ltsv, SvpVec& result)
{
    int rank = -1;
    Lts * merged = NULL;
    Lts * lts;

    assert(result.v.size() == 0);
    assert(ltsv->v.size() == tr.current_contexts().size());
    for (int k=0; k<ltsv->v.size(); k++) {
	lts = is_lts(ltsv->v[k]);
	if (lts->rank != rank) {
	    rank = lts->rank;
	    merged = lts;
	    ltsv->detach(k);
	    result.v.push_back(merged);
	} else
	    merged->compose(*lts);
    }
}


void relabel_one(SymbolValue * r, SymbolValue * l)
{
    Lts * lts = is_lts(l);
    RelabelingValue * rlv = is_relabeling(r);

    /* Apply relabeling. */
    if (rlv) {
	for (int i=0; i<rlv->size(); i++)
	    lts->relabeling(*(rlv->new_labels[i]),
		    *(rlv->old_labels[i]));
    }
}

void relabel(FspTranslator& tr, SvpVec *relabv, SvpVec * ltsv)
{
    assert(ltsv->v.size() == tr.current_contexts().size());

    if (relabv) {
	assert(ltsv->v.size() == relabv->v.size());
	for (int c=0; c<relabv->v.size(); c++)
	    relabel_one(relabv->v[c], ltsv->v[c]);
    }
}

void merge_ltscomp_by_rank(FspTranslator& tr, SvpVec * ltscompv,
							    SvpVec& result)
{
    int rank = -1;
    LtsComposition * merged = NULL;
    LtsComposition * lcp;
    Lts * lts;

    assert(result.v.size() == 0);
    assert(ltscompv->v.size() == tr.current_contexts().size());
    for (int k=0; k<ltscompv->v.size(); k++) {
	lcp = is_ltscomposition(ltscompv->v[k]);
	if (lcp->rank != rank) {
	    rank = lcp->rank;
	    merged = lcp;
	    ltscompv->detach(k);
	    result.v.push_back(merged);
	} else {
	    for (int i=0; i<merged->lts.size(); i++)
		merged->lts[i]->compose(*(lcp->lts[i]));
	}
    }
}

SvpVec * priority_callback(FspTranslator& tr, SvpVec * one, bool low)
{
    PriorityValue * prv;

    assert(one->v.size() == tr.current_contexts().size());
    for (int c=0; c<one->v.size(); c++) {
	prv = new PriorityValue;
	prv->setvp = is_set(one->v[c]);
	prv->low = low;
	one->v[c] = prv;
    }

    return one;
}


/* Fix unresolved ProcessNode references due to cyclic processes. */
static void fix_unresolved_references(ProcessNode * pnp, void * opaque)
{
    FspTranslator * trp = (FspTranslator *)opaque;
    SymbolValue * svp;
    ProcessValue * pvp;

    for (int i=0; i<pnp->children.size(); i++) {
	ProcessEdge& e = pnp->children[i];

	if (e.dest == NULL) {
	    PROX(cout << "Unref " << pnp << ": " << trp->cr.actions.reverse[e.action] << " -> " << e.unresolved_reference << "\n");
	    if (!trp->local_processes.lookup(e.unresolved_reference, svp)) {
		stringstream errstream;
		errstream << "Local process " << e.unresolved_reference
			    << " undeclared";
		semantic_error(errstream, trp->locations[0]);
		/* TODO improve localization by saving the location of the
		   reference in the ProcessEdge e. */
	    }
	    pvp = is_process(svp);
	    e.dest = pvp->pnp;
	}
    }
}


/* Data structure associated to the visit function 'aggregate_END_ERROR_states'. */
struct AggrStatesData {
    ProcessNode * end;
    ProcessNode * error;
    bool end_found;
    bool end_init;
    string reference;
};

/* This visit function looks for END and ERROR states. If there are more
   END states, the first END state found will be elected to be the END state,
   and all the pointers to and END state are modified so that they point to
   the elected state. The same applies to ERROR states. The filed 'end' and
   'error' of the data structure must be initialized to NULL before the visit
   starts. When the visit is completed, if 'end' is NULL we can conclude
   that there are no END states (e.g. the process is sequential). */
static void aggregate_end_error_states(ProcessNode * pnp, void *opaque)
{
    if (pnp->type == ProcessNode::Normal) {
	ProcessNode * dest;
	AggrStatesData * d = static_cast<AggrStatesData *>(opaque);

	for (int i=0; i<pnp->children.size(); i++) {
	    dest = pnp->children[i].dest;
	    assert(dest);
	    if (dest->type == ProcessNode::End) {
		d->end_found = true;
		if (d->end_init)
		    pnp->children[i].dest = d->end;
		else {
		    d->end = dest;
		    d->end_init = true;
		}
		if (d->reference != "")
		    pnp->children[i].unresolved_reference = d->reference;
	    } else if (dest->type == ProcessNode::Error) {
		if (d->error)
		    pnp->children[i].dest = d->error;
		else
		    d->error = dest;
	    }
	}
    }
}

void * Callback_pop_push::exe(FspTranslator& tr, int pp, int psh) {
    for (int i=0; i<pp; i++)
	tr.css.pop();
    if (psh)
	tr.css.push_clone();

    return NULL;
}


/* ============================= CALLBACKS =============================== */
SvpVec * callback__1(FspTranslator& tr, string * one) 
{
    SvpVec * vp;
    SetValue * setvp; 

    vp = new SvpVec;
    vp->shared = false;
    for (int c=0; c<tr.current_contexts().size(); c++) {
	setvp = new SetValue;
	*setvp += *one;
	/* We assign a different rank to each setvp in vp (and so one
	   per context). Note that this assignment happens before
	   parsing anithing else in the composite action. For istance,
	   if we have to parse 'a[i:R][S][i+1]', this action is executed
	   as soon as 'a' is parsed, and so before parsing the rest
	   of the composite action. */
	setvp->rank = c;
	vp->v.push_back(setvp);
    }
    delete one;

    return vp;
}

SvpVec * callback__2(FspTranslator& tr, SvpVec * one, string * two)
{
    SetValue * setvp;

    assert(one->v.size() == tr.current_contexts().size());
    for (int c=0; c<one->v.size(); c++) {
	setvp = is_set(one->v[c]);
	setvp->dotcat(*two);
	/* Rank is inherited. */
    }
    delete two;

    return one;
}

SvpVec * callback__3(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size() &&
	    one->v.size() == tr.current_contexts().size());
    SetValue * setvp;
    SetValue * rsetvp;
    for (int c=0; c<one->v.size(); c++) {
	setvp = is_set(one->v[c]);
	rsetvp = is_set(two->v[c]);
	/* Combine setvp with rsetvp. No context ramification
	   happens here. */
	setvp->dotcat(*rsetvp);
	/* Rank is inherited. */
    }
    delete two;

    return one;
}

SvpVec * callback__4(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size() &&
	    one->v.size() == tr.current_contexts().size());
    return indexize_svpvec(tr, one, two);
}

SvpVec * callback__5(FspTranslator& tr, string * one)
{
    SvpVec * vp = new SvpVec;
    SymbolValue * svp;
    SetValue * setvp;
    if (!tr.cr.identifiers.lookup(*one, svp)) {
	stringstream errstream;
	errstream << "set " << *one << " undeclared";
	semantic_error(errstream, tr.locations[0]);
    }
    delete one;
    setvp = err_if_not_set(svp, tr.locations[0]);
    svp = svp->clone();
    vp->shared = true;
    for (int c=0; c<tr.current_contexts().size(); c++)
	vp->v.push_back(svp);

    return vp;
}

SvpVec * callback__6(FspTranslator& tr, string * one, string * two)
{
    SvpVec * vp = new SvpVec;
    SymbolValue * svp;

    if (!tr.cr.identifiers.lookup(*two, svp)) {
	stringstream errstream;
	errstream << "range/set " << *two << " undeclared";
	semantic_error(errstream, tr.locations[0]);
    }
    if (!(svp->type() == SymbolValue::Range ||
		svp->type() == SymbolValue::Set)) {
	stringstream errstream;
	errstream << "range/set expected";
	semantic_error(errstream, tr.locations[0]);
    }
    svp = svp->clone();

    /* Pass the variable name to the upper levels.*/
    svp->setVariable(*one);
    delete one;
    vp->shared = true;
    for (int c=0; c<tr.current_contexts().size(); c++)
	vp->v.push_back(svp);

    return vp;
}

SvpVec * callback__7(FspTranslator& tr, string * one, SvpVec * two)
{
    RangeValue * rvp;
    for (int i=0; i<two->v.size(); i++) {
	rvp = is_range(two->v[i]);
	/* Pass the variable to the upper levels.*/
	rvp->setVariable(*one);
    }
    delete one;

    return two;
}

SvpVec * callback__8(FspTranslator& tr, string * one, SvpVec * two)
{
    SetValue * setvp;
    for (int i=0; i<two->v.size(); i++) {
	setvp = is_set(two->v[i]);
	/* Pass the variable to the upper levels.*/
	setvp->setVariable(*one);
    }
    delete one;

    return two;
}

SvpVec * callback__9(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    RangeValue * rvp;
    ConstValue * cvp;
    SvpVec * vp = new SvpVec;

    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	rvp = new RangeValue;
	cvp = is_const(one->v[i]);
	rvp->low = cvp->value;
	cvp = is_const(two->v[i]);
	rvp->high = cvp->value;
	vp->v.push_back(rvp);
    }
    delete one;
    delete two;

    return vp;
}

void merge_by_rank(SvpVec * sets, SvpVec& result) {
    SetValue * merged;
    SetValue * setvp;
    int rank = -1;

    assert(result.v.size() == 0);

    for (int c=0; c<sets->v.size(); c++) {
	setvp = is_set(sets->v[c]);
	if (setvp->rank != rank) {
	    rank = setvp->rank;
	    merged = setvp;
	    sets->detach(c);
	    result.v.push_back(merged);
	} else
	    *merged += *setvp;
    }
    /* Note: Deleting 'sets' is up to the caller. */
}

SvpVec * callback__10(FspTranslator& tr, SvpVec * one)
{
    SvpVec * vp = new SvpVec;

    merge_by_rank(one, *vp);
    delete one;

    return vp;
}

SvpVec * callback__13(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    SvpVec two_merged;

    merge_by_rank(two, two_merged);
    delete two;

    assert(one->v.size() == two_merged.v.size());
    for (int c=0; c<one->v.size(); c++) {
	SetValue * setvp = is_set(one->v[c]);
	SetValue * rsetvp = is_set(two_merged.v[c]);
	*setvp += *rsetvp;
    }

    return one;
}

void * callback__14(FspTranslator& tr, string * one)
{
    /* Parameters have been pushed inside the 'param_OPT' rule.
       A cleaner approach would be to get a list of parameters from
       the rule and push the parameters into the 'tr.cr.identifiers' table
       in this action. */
    tr.init_fakenode();

    return NULL;
}

class Lts * callback__15(FspTranslator& tr, string * one, Pvec * two,
			SvpVec * three, SvpVec * four, SvpVec * five)
{
    PROP("process_def --> ... process_body ...");
    PROX(cout<<*one<<" = "; two->v[0]->print(&tr.cr.actions));

    ProcessBase * pbp = two->v[0];
    SymbolValue * svp;
    ProcessValue * pvp = NULL;
    Lts * lts;

    assert(two->v.size() == 1);

    if (tr.local_processes.lookup(*one, svp)) {
	/* This happens if "*one" is defined both as a local processes
	   and a global process in the same process_def statement. */
	stringstream errstream;
	errstream << "process " << *one << " declared twice";
	semantic_error(errstream, tr.locations[0]);
    }

    if (pbp->unresolved()) {
	tr.aliases.insert(*one, ((UnresolvedProcess* )pbp)->reference);
	tr.aliases.print();
    } 
    tr.aliases.fill_process_table(tr.local_processes);
    if (pbp->unresolved()) {
	if(!tr.local_processes.lookup(*one, svp)) {
	    // XXX can we reach here??
	    stringstream errstream;
	    errstream << "process " << *one << " undeclared";
	    semantic_error(errstream, tr.locations[0]);
	}
	pvp = is_process(svp);
    } else {
	/* Insert a new ProcessValue in the symbol table, attaching the
	   process_body to it. */
	pvp = new ProcessValue;
	pvp->pnp = is_procnode(pbp);
	if (!tr.local_processes.insert(*one, pvp)) {
	    stringstream errstream;
	    errstream << "process " << *one << " declared twice";
	    semantic_error(errstream, tr.locations[0]);
	}
    }

    PROX(cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<< Process " << *one << " defined >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    /* Try to resolve all the undefined references into this process. */
    struct ProcessVisitObject f;
    f.vfp = &fix_unresolved_references;
    f.opaque = &tr;
    pvp->pnp->visit(f, true);

    /* Aggregate END and ERROR states. */
    struct AggrStatesData d;
    d.error = NULL;
    d.end_init = d.end_found = false;
    f.vfp = &aggregate_end_error_states;
    f.opaque = &d;
    pvp->pnp->visit(f, false);

    PROX(cout<<"resolved: "; pvp->pnp->print(&tr.cr.actions));

    /* Convert the collection of ProcessNodes in an Lts object. */
    lts = new Lts(pvp->pnp, &tr.cr.actions);

    /* Note: We don't delete pnp, since this has been inserted in
       'local_processes', and will be thereafter deleted when the symbol
       table will be destroyed. */

    /* Extend the alphabet if it is the case. */
    if (three) {
	SetValue * setvp;

	assert(three->v.size() == 1);
	setvp = is_set(three->v[0]);
	for (int i=0; i<setvp->actions.size(); i++)
	    lts->updateAlphabet(tr.cr.actions.insert(setvp->actions[i]));
    }

    /* Apply relabeling. */
    if (four) {
	RelabelingValue * rlv;

	assert(four->v.size() == 1);
	rlv = is_relabeling(four->v[0]);
	for (int i=0; i<rlv->size(); i++)
	    lts->relabeling(*(rlv->new_labels[i]), *(rlv->old_labels[i]));
    }

    /* Apply hiding. */
    if (five) {
	HidingValue * hvp = is_hiding(five->v[0]);

	assert(five->v.size() == 1);
	lts->hiding(*(hvp->setvp), hvp->interface);
    }

    lts->name = *one;

    delete one;
    delete three;
    delete four;
    delete five;

    return lts;
}

void * callback__17(FspTranslator& tr, string * one, SvpVec * two)
{
    tr.init_fakenode();

    return NULL;
}

Pvec * callback__18(FspTranslator& tr, string * one, SvpVec * two,
							    Pvec * three)
{
    ProcessValue * pvp;
    SetValue * setvp;
    string procname;

    for (int c=0; c<two->v.size(); c++) {
	setvp = is_set(two->v[c]);
	for (int j=0; j<setvp->actions.size(); j++) {
	    procname = *one + setvp->actions[j];
	    if (three->v[c]->unresolved()) {
		tr.aliases.insert(procname,
		(static_cast<UnresolvedProcess *>(three->v[c]))->reference);
		tr.aliases.print();
	    } else {
		pvp = new ProcessValue;
		pvp->pnp = is_procnode(three->v[c]);
		if (j)
		    pvp->pnp = pvp->pnp->clone();
		if (!tr.local_processes.insert(procname, pvp)) {
		    stringstream errstream;
		    errstream << "process " << procname
			<< " declared twice";
		    semantic_error(errstream, tr.locations[0]);
		}
		PROX(cout << "Process " << procname << " defined (" << pvp->pnp << ")\n");
	    }
	}
    }
    /* Pop out the cloned (maybe dirty) ContextsState, so that we
       correctly restore the empty initial state. */
    tr.css.pop();
    delete one;
    delete two;

    return three;
}

void * callback__19(FspTranslator& tr, SvpVec * one)
{
    ConstValue * cvp;

    /* Prepare a clone of the current ContextsSet to rule out those
       contexts that makes 'expression' false. In this way when executing
       the translation of the following 'local_process', only the
       selected contexts are expanded, while the others are not
       translated (e.tr. are filtered out). */
    tr.css.push_clone();
    for (int c=0; c<tr.current_contexts().size(); c++) {
	cvp = is_const(one->v[c]);
	if (!cvp->value) {
	    tr.current_contexts().rule_out(c);
	}
    }

    return NULL;
}

void * callback__20(FspTranslator& tr, SvpVec * one, Pvec * two)
{
    ConstValue * cvp;

    tr.css.pop();
    tr.css.push_clone();
    /* Clean the previous clone and prepare another one for the else
       branch, in the same way descripted above. */
    for (int c=0; c<tr.current_contexts().size(); c++) {
	cvp = is_const(one->v[c]);
	if (cvp->value)
	    tr.current_contexts().rule_out(c);
    }

    return NULL;
}

Pvec * callback__21(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three)
{
    Pvec * pvec = new Pvec;
    ConstValue * cvp;

    tr.css.pop();
    /* Clean the previous cloned ContextsSet and fill the return vector
     *pvec, depending on the truth values contained in $2. */
    for (int c=0; c<tr.current_contexts().size(); c++) {
	cvp = is_const(one->v[c]);
	if (cvp->value)
	    pvec->v.push_back(two->v[c]);
	else if (three)
	    pvec->v.push_back(three->v[c]);
	else
	    /* If $7 == NULL there is no else branch, and so we insert
	       a STOP state. */
	    pvec->v.push_back(tr.cr.pna.allocate(ProcessNode::Normal));
    }
    delete one;
    delete two;
    delete three;

    return pvec;
}

Pvec * callback__23(FspTranslator& tr)
{
    Pvec * pvec = new Pvec;
    ProcessNode * pnp = tr.cr.pna.allocate(ProcessNode::End);

    for (int c=0; c<tr.current_contexts().size(); c++)
	pvec->v.push_back(pnp);

    return pvec;
}

Pvec * callback__24(FspTranslator& tr)
{
    Pvec * pvec = new Pvec;
    ProcessNode * pnp = tr.cr.pna.allocate(ProcessNode::Normal);

    for (int c=0; c<tr.current_contexts().size(); c++)
	pvec->v.push_back(pnp);

    return pvec;
}

Pvec * callback__25(FspTranslator& tr)
{
    Pvec * pvec = new Pvec;
    ProcessNode * pnp = tr.cr.pna.allocate(ProcessNode::Error);

    for (int c=0; c<tr.current_contexts().size(); c++)
	pvec->v.push_back(pnp);

    return pvec;
}

Pvec * callback__26(FspTranslator& tr, string * one, SvpVec * two)
{
    SymbolValue * svp;
    Pvec * pvec = new Pvec;
    ProcessBase * pbp;
    SetValue * setvp;
    string name;

    assert(two->v.size() == tr.current_contexts().size());
    for (int c=0; c<two->v.size(); c++) {
	setvp = is_set(two->v[c]);
	assert(setvp->actions.size() == 1);
	name = *one + setvp->actions[0];
	PROX(cout << "looking for " << name << endl);
	/* If the process referenced is already defined, return it.
	   Otherwise return a new UnresolvedProcess object, so that the
	   upper level sees that 'name' is unresolved. */
	if (tr.local_processes.lookup(name, svp))
	    pbp = ((ProcessValue *)svp)->pnp;
	else
	    pbp = tr.cr.pna.allocate_unresolved(name);
	pvec->v.push_back(pbp);
    }
    delete one;
    delete two;

    return pvec;
}

Pvec * callback__27(FspTranslator& tr, Pvec * one)
{
    /* Pop (and destroy) the replicated top and everything above. */
    tr.css.pop();
    
    return one;
}

Pvec * callback__29(FspTranslator& tr, Pvec * one, Pvec * two)
{
    /* Pop (and destroy) the replicated top and everything above. */
    tr.css.pop();

    return one;
}

void * callback__30(FspTranslator& tr, SvpVec * one)
{
    PROP("action_prefix --> guard_OPT (cont)");
    ConstValue * cvp;
    if (one) {
	for (int c=0; c<tr.current_contexts().size(); c++) {
	    cvp = is_const(one->v[c]);
	    if (!cvp->value)
		tr.current_contexts().rule_out(c);
	}
	PROX(cout << "filtered\n"; tr.current_contexts().print());
    }

    return NULL;
}

Pvec * callback__31(FspTranslator& tr, SvpVec * one, Pvec * two, Pvec * three)
{
    PROP("action_prefix --> (cont) prefix_actions -> local_process");
    /* If $5 is an istance of ConnectedProcess, it means that its first 
       nodes have already been connected to the frontier of $3 in the
       lower level rule 'prefix_actions'. Therefore there is nothing to
       be done here. */

    for (int i=0; i<tr.current_contexts().frontier.size(); i++) {
	ProcessNode * pnp = tr.current_contexts().frontier[i].pnp;
	int child = tr.current_contexts().frontier[i].child;
	int rank = pnp->children[child].rank;
	if (three->v[rank]->connected()) continue;
	if (!three->v[rank]->unresolved())
	    /* If $5 is not an unresolved reference, we connect the
	       current frontier (e.tr. the frontier of $3) to the process
	       $5. */
	    pnp->children[child].dest = is_procnode(three->v[rank]);
	else
	    /* If $5 is an unresolved reference, we scan the current
	       frontier recording the reference itself, so that this
	       reference will be fixed by the upper levels. */
	    pnp->children[child].unresolved_reference =
	    (static_cast<UnresolvedProcess *>(three->v[rank]))->reference;
    }
    delete one;
    delete three;

    PROX(tr.current_contexts().print(); cout<<"$$ = "; tr.print_fakenode_forest());
    return two;
}

Pvec * callback__32(FspTranslator& tr, SvpVec * one)
{
    PROP("prefix_actions --> action_labels");
    PROX(cout<<"$1 = "; one->print());

    /* Note that this is executed (obviously) after 'one' has been
       parsed, and so after all the context ramifications implied by
       the latter have been executed. */

    /* If the frontier is not empty, it means that, although at the
       beginning of an 'action_prefix', we are in the middle of
       a process definition (e.tr. "P=(a->b->(c->..." or
       "P=(a-b->(c->d->P|e->..."). In this case we have consider all
       the edges in the frontier (e.tr. the edge "b").
       */
    vector<FrontierElement> new_frontier;
    Pvec * pvec = new Pvec;
    ProcessNode * pnp;
    ProcessNode * npnp;
    ProcessEdge e;
    SetValue * setvp;
    int rank;
    int child;
    struct FrontierElement fe;
    bool zero;
    int max_rank = 0;

    zero = true;
    for (int i=0; i<tr.current_contexts().frontier.size(); i++)
	if (tr.current_contexts().frontier[i].pnp != &tr.fakenode) {
	    zero = false;
	    break;
	}

    for (int i=0; i<tr.current_contexts().frontier.size(); i++) {
	pnp = tr.current_contexts().frontier[i].pnp;
	child = tr.current_contexts().frontier[i].child;
	/* If the edge i is NULL-pointing, it means that we are
	   at the beginning of a choice construct (e.tr.
	   "P=(a->b->(c->..."): We then create a node for that
	   edge (in our example we create a node for "b"). If the
	   edge is not NULL-pointing, it means that we are in 
	   the middle of a choice construct (e.tr.
	   "P=(a-b->(c->d->P|e->...") and so the node has already
	   been created when processing the first choice element.
	   There is no need to create anythintr.
	   */
	if (pnp->children[child].dest == NULL)
	    pnp->children[child].dest = 
		tr.cr.pna.allocate(ProcessNode::Normal);
	npnp = pnp->children[child].dest;

	/* Once we get the node pointed by the edge i (npnp), we
	   have to combine that edge with the sets in 'one'. We
	   have a combination when the rank of the edge is equal
	   to the rank of the set. When we find a combination, we
	   add to npnp a NULL-pointing edge for each element in 
	   the matching set.
	   */
	rank = pnp->children[child].rank;
	if (rank > max_rank) max_rank = rank;
	for (int c=0; c<one->v.size(); c++) {
	    if (tr.current_contexts().is_ruled_out(c)) continue;
	    setvp = is_set(one->v[c]);
	    if (setvp->rank == rank) {
		for (int k=0; k<setvp->actions.size(); k++) {
		    e.action = tr.cr.actions.insert(setvp->actions[k]);
		    e.dest = NULL;
		    /* We set e.rank to the index in the SvpVec
		       'one' of setvp, so that this edge will
		       combine with actions in the next level
		       that have the same rank. */
		    e.rank = c;
		    npnp->children.push_back(e);
		    /* We add the edge to the new frontier, so that
		       it will be used for combination with the next
		       level actions. */
		    fe.pnp = npnp;
		    fe.child = npnp->children.size() - 1;
		    new_frontier.push_back(fe);
		}
	    }
	}
    }

    /* Update the frontier. */
    tr.current_contexts().frontier = new_frontier;
    if (zero) {
	assert(max_rank+1 == tr.fakenode.children.size());
	for (int c=0; c<max_rank+1; c++)
	    pvec->v.push_back(tr.fakenode.children[c].dest);
    }
    else {
	/* Tell the upper level rule (action_prefix) that the beginning
	   of this prefix_actions has already been connected, and so
	   'local_process' results already connected (see above). */
	ProcessBase * pbp = tr.cr.pna.allocate_connected();
	for (int c=0; c<max_rank+1; c++)
	    pvec->v.push_back(pbp);
    }
    delete one;

    PROX(tr.current_contexts().print(); cout<<"$$ = \n"; tr.print_fakenode_forest());

    return pvec;
}

Pvec * callback__33(FspTranslator& tr, Pvec * one, SvpVec * two)
{
	PROP("prefix_actions --> prefix_actions -> action_labels");
	PROX(cout<<"$3 = "; two->print());
	ProcessNode * pnp;
	ProcessNode * npnp;
	ProcessEdge e;
	SetValue * setvp;
	int rank;
	int child;
	struct FrontierElement fe;
	vector<FrontierElement> new_frontier;

	/* Here we are in the middle of a 'prefix_actions' 
	   (e.tr. "...->b->c->d->.."), and so we have to connect each edge in
	   the frontier with the action sets in 'two' (e.tr. we have a frontier
	   that contains a NULL-pointing edge "c" and 'two'={{d}}).
	*/
	for (int i=0; i<tr.current_contexts().frontier.size(); i++) {
	    pnp = tr.current_contexts().frontier[i].pnp;
	    child = tr.current_contexts().frontier[i].child;
	    /* Here we have to do the same combination descripted above,
	       but only if the edge in the frontier is NULL-pointing and
	       it has not an unresolved reference associated. If the
	       edge is not NULL-pointing, it means that we are in a 
	       situation like this "...->a->(b->c->...|d->e->...)". In
	       this case pnp is the node which has an edge "b" and an 
	       edge "d", and 'two'={{e}}: Therefore we don't have to consider
	       the edge "b" for combinations, since it has already been used
	       when processing the previous choice element. We only have to
	       consider "d". If an edge has an unresolved reference, 
	       it means that we are in a situation like this
	       "..a->(b->P|d->e->...)": this case is very similar to the
	       previous one, but here the edge "b" is NULL-pointing,
	       only because of the unresolved reference P: considering it
	       for combinations would be an error. */
	    if (pnp->children[child].dest == NULL &&
		    pnp->children[child].unresolved_reference == "") {
		pnp->children[child].dest = 
		    tr.cr.pna.allocate(ProcessNode::Normal);
		npnp = pnp->children[child].dest;
		rank = pnp->children[child].rank;
		for (int c=0; c<two->v.size(); c++) {
		    if (tr.current_contexts().is_ruled_out(c)) continue;
		    setvp = is_set(two->v[c]);
		    if (setvp->rank == rank) {
			for (int k=0; k<setvp->actions.size(); k++) {
			    e.action = tr.cr.actions.insert(setvp->actions[k]);
			    e.dest = NULL;
			    /* We set e.rank to the index in the SvpVec
			       'two', so that this edge will combine with
			       actions in the next level that have the
			       same rank. */
			    e.rank = c;
			    npnp->children.push_back(e);
			    fe.pnp = npnp;
			    fe.child = npnp->children.size() - 1;
			    new_frontier.push_back(fe);
			}
		    }
		}
	    }
	}
	tr.current_contexts().frontier = new_frontier;
	/* Note that 'one' is not used in this action, because we use the
	   frontier stored in tr.current_contexts(). As a result, 'one' is
	   the result of the previous action (and so a ProcessNode* or a
	   ConnectedProcess*. */
	delete two;

	PROX(tr.current_contexts().print(); cout<<"$$ = \n"; tr.print_fakenode_forest());

	return one;
}

SvpVec * callback__34(FspTranslator& tr)
{
    SvpVec * vp = new SvpVec;
    SetValue * setvp;

    for (int c=0; c<tr.current_contexts().size(); c++) {    
	setvp = new SetValue;
	setvp->actions.push_back("");
	vp->v.push_back(setvp);
    }
    return vp;
}

SvpVec * callback__35(FspTranslator& tr, SvpVec * one)
{
    SvpVec * vp = new SvpVec;
    SetValue * setvp;
    ConstValue * cvp;

    assert(one->v.size() == tr.current_contexts().size());
    for (int c=0; c<one->v.size(); c++) {
	cvp = is_const(one->v[c]);
	setvp = new SetValue;
	setvp->actions.push_back("[" + int2string(cvp->value) + "]");
	vp->v.push_back(setvp);
    }
    delete one;

    return vp;
}

SvpVec * callback__36(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    SetValue * setvp;
    ConstValue * cvp;

    assert(one->v.size() == two->v.size());
    for (int c=0; c<two->v.size(); c++) {
	setvp = is_set(one->v[c]);
	cvp = is_const(two->v[c]);
	setvp->indexize(cvp->value);
    }
    delete two;

    return one;
}

SvpVec * callback__37(FspTranslator& tr)
{
    SvpVec * vp = new SvpVec;
    SetValue * setvp = new SetValue;

    assert(tr.current_contexts().size() == 1);
    setvp->actions.push_back("");
    vp->v.push_back(setvp);

    return vp;
}

SvpVec * callback__38(FspTranslator& tr, SvpVec * one)
{
    SvpVec * vp = new SvpVec;
    SetValue * setvp;

    assert(tr.current_contexts().size() == one->v.size());

    if (one->v[0]->type() == SymbolValue::Set) {
	stringstream errstream;
	errstream << "Unexpected set";
	semantic_error(errstream, tr.locations[0]);
    }
    for (int c=0; c<one->v.size(); c++) {
	setvp = new SetValue;
	setvp->actions.push_back("");
	setvp->rank = c;
	vp->v.push_back(setvp);
    }

    return indexize_svpvec(tr, vp, one);
}

SvpVec * callback__39(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(two->v.size() == one->v.size());
    if (two->v[0]->type() == SymbolValue::Set) {
	stringstream errstream;
	errstream << "Unexpected set";
	semantic_error(errstream, tr.locations[0]);
    }

    return indexize_svpvec(tr, one, two);
}

SvpVec * callback__41(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = vpl->value && vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__42(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = vpl->value && vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__43(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value |= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__44(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value ^= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__45(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value &= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__46(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = (vpl->value == vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__47(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = (vpl->value != vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__48(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = (vpl->value < vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__49(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = (vpl->value > vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__50(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = (vpl->value <= vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__51(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = (vpl->value >= vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__52(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value <<= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__53(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value >>= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__54(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value += vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__55(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value -= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__56(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value *= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__57(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value /= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__58(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = is_const(one->v[i]);
	ConstValue * vpr = is_const(two->v[i]);
	vpl->value = vpl->value % vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__59(FspTranslator& tr, SvpVec * one)
{
    ConstValue * cvp;

    for (int i=0; i<one->v.size(); i++) {
	cvp = is_const(one->v[i]);
	cvp->value *= -1;
    }

    return one;
}

SvpVec * callback__60(FspTranslator& tr, SvpVec * one)
{
    ConstValue * cvp;

    for (int i=0; i<one->v.size(); i++) {
	cvp = is_const(one->v[i]);
	cvp->value = !(cvp->value);
    }

    return one;
}

SvpVec * callback__61(FspTranslator& tr, int one)
{
    SvpVec * vp;
    ConstValue * cvp;

    /* Return a ConstValue* for each context (in non shared mode). */
    vp = new SvpVec;
    vp->shared = false;
    for (int c=0; c<tr.current_contexts().size(); c++) {
	cvp = new ConstValue;
	cvp->value = one;
	vp->v.push_back(cvp);
    }

    return vp;
}

SvpVec * callback__62(FspTranslator& tr, string * one)
{
    /* Return a ConstValue* for each context (in non shared mode). */
    SvpVec * vp;

    vp = new SvpVec;
    vp->shared = false;
    for (int c=0; c<tr.current_contexts().size(); c++) {
	SymbolValue * svp;
	if (!(tr.current_contexts()[c]->lookup(*one, svp))) {
	    stringstream errstream;
	    errstream << "variable " << *one << " undeclared";
	    semantic_error(errstream, tr.locations[0]);
	}
	svp = svp->clone();
	vp->v.push_back(svp);
    }
    delete one;

    return vp;
}

SvpVec * callback__63(FspTranslator& tr, string * one)
{
    SymbolValue * svp;
    SvpVec * vp; 

    /* Lookup the identifier and clone the associated object. */
    if (!tr.cr.identifiers.lookup(*one, svp)) {
	stringstream errstream;
	errstream << "const/range/set/parameter " << *one << " undeclared";
	semantic_error(errstream, tr.locations[0]);
    }
    delete one;

    /* Return a ConstValue* for each context (in non shared mode). */
    vp = new SvpVec;
    vp->shared = false;
    for (int c=0; c<tr.current_contexts().size(); c++)
	vp->v.push_back(svp->clone());

    return vp;
}

Pvec * callback__64(FspTranslator& tr, Pvec * one, Pvec * two)
{
    assert(one->v.size() == two->v.size() &&
	    two->v.size() == tr.current_contexts().size());
    Pvec * pvec = new Pvec;
    struct AggrStatesData d;
    struct ProcessVisitObject f;

    for (int c=0; c<one->v.size(); c++) {
	ProcessNode * left = is_procnode(one->v[c]);
	ProcessBase * pbp = two->v[c];

	assert(!pbp->connected());

	if (pbp->unresolved()) {
	    d.end = NULL;
	    d.reference = (static_cast<UnresolvedProcess *>(pbp))->reference;
	} else {
	    d.end = is_procnode(pbp);
	}
	d.end_init = true;
	d.error = NULL;
	f.vfp = &aggregate_end_error_states;
	f.opaque = &d;
	left->visit(f, false);

	pvec->v.push_back(left);
    }
    delete one;
    delete two;

    return pvec;
}

Pvec * callback__65(FspTranslator& tr, Pvec * one, Pvec * two)
{
    struct AggrStatesData d;
    struct ProcessVisitObject f;

    for (int c=0; c<one->v.size(); c++) {
	ProcessNode * left = is_procnode(one->v[c]);
	ProcessNode * right = is_procnode(two->v[c]);

	/* Concatenate the two sequential processes and check that the
	   process is sequential. */
	d.end = right;
	d.end_init = true;
	d.end_found = false;
	d.error = NULL;
	f.vfp = &aggregate_end_error_states;
	f.opaque = &d;
	left->visit(f, false);
	if (!d.end_found) {
	    stringstream errstream;
	    errstream << "Process is not sequential";
	    semantic_error(errstream, tr.locations[0]);
	}
    }
    delete two;

    return one;
}


Lts * get_parametric_lts(FspTranslator& tr, const string& name,
		    ParametricProcess * ppp, ArgumentsValue * avp)
{
    string extension;
    SymbolValue * svp;
    Lts * lts;

    if (avp->args.size() != ppp->parameter_names.size()) {
	stringstream errstream;
	errstream << "Parameters mismatch";
	semantic_error(errstream, tr.locations[1]);
    }

    lts_name_extension(avp->args, extension);

    /* We first lookup the global processes table in order to see if
       we already have the requested LTS. */
    if (tr.cr.processes.lookup(name + extension, svp)) {
	/* If there is a cache hit, clone the LTS. */
	svp = svp->clone();
	lts = is_lts(svp);
    } else {
	/* If there is a cache miss, we have to compute the requested LTS
	   by replay and save it in the global processes table. */
	lts = ppp->replay(tr.cr, avp->args);

	lts->name += extension; /* Here lts->name is complete. */
	if (!tr.cr.processes.insert(name + extension, lts)) {
	    assert(0);
	}
	lts = static_cast<Lts *>(lts->clone());
    }

    return lts;
}


Pvec * callback__66(FspTranslator& tr, string * one, SvpVec * two)
{
    SymbolValue * svp;
    Lts * lts;
    ParametricProcess * ppp;
    Pvec * vp = new Pvec;
    SvpVec * argvp = two;
    ArgumentsValue * avp;

    /* Lookup 'process_id' in the 'parametric_process' table. */
    if (!tr.cr.parametric_processes.lookup(*one, svp)) {
	stringstream errstream;
	errstream << "Process " << *one << " undeclared";
	semantic_error(errstream, tr.locations[0]);
    }
    ppp = is_parametric(svp);

    if (!argvp) {
	/* 'argvp' is going to specify the default parameters for every
	   context. */
	argvp = new SvpVec;
	for (int k=0; k<tr.current_contexts().size(); k++) {
	    avp = new ArgumentsValue;
	    avp->args = ppp->parameter_defaults;
	    argvp->v.push_back(avp);
	}
    }

    for (int k=0; k<argvp->v.size(); k++) {
	struct ProcessVisitObject f;
	struct AggrStatesData d;

	avp = is_arguments(argvp->v[k]);
	lts = get_parametric_lts(tr, *one, ppp, avp);

	/* Note that here we don't c.pna.clear(). */

	vp->v.push_back(lts->toProcessNode(tr.cr.pna));
    }
    delete one;
    delete argvp;

    return vp;
}

SvpVec * callback__67(FspTranslator& tr, SvpVec * one)
{
    assert(one->v.size() == tr.current_contexts().size());
    SvpVec * vp = new SvpVec;
    ConstValue * cvp;
    ArgumentsValue * avp;

    for (int c=0; c<one->v.size(); c++) {
	cvp = is_const(one->v[c]);
	avp = new ArgumentsValue;
	avp->args.push_back(cvp->value);
	vp->v.push_back(avp);
    }
    delete one;

    return vp;
}

SvpVec * callback__68(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == tr.current_contexts().size() &&
	    two->v.size() == one->v.size());
    ConstValue * cvp;
    ArgumentsValue * avp;

    for (int c=0; c<two->v.size(); c++) {
	cvp = is_const(two->v[c]);
	avp = is_arguments(one->v[c]);
	avp->args.push_back(cvp->value);
    }
    delete two;

    return one;
}

SvpVec * callback__69(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    RelabelingValue * lrlv;
    RelabelingValue * rrlv;

    assert(one->v.size() == two->v.size());
    for (int c=0; c<one->v.size(); c++) {
	lrlv = is_relabeling(one->v[c]);
	rrlv = is_relabeling(two->v[c]);
	lrlv->merge(*rrlv);
    }
    delete two;

    return one;
}

SvpVec * callback__70(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    SvpVec * vp = new SvpVec;
    SvpVec old_merged;
    SvpVec new_merged;
    RelabelingValue * rlv;

    tr.css.pop();

    merge_by_rank(one, new_merged);
    delete one;

    merge_by_rank(two, old_merged);
    delete two;
    
    assert(old_merged.v.size() == new_merged.v.size());
    for (int i=0; i<old_merged.v.size(); i++) {
	rlv = new RelabelingValue;
	rlv->add(static_cast<SetValue * >(new_merged.v[i]),
			static_cast<SetValue *>(old_merged.v[i]));
	vp->v.push_back(rlv);
    }
    new_merged.detach_all();
    old_merged.detach_all();

    return vp;
}

SvpVec * hiding_callback(FspTranslator& tr, SvpVec * one, bool interface)
{
    SetValue * setvp;
    HidingValue * hvp;

    assert(one->v.size() == tr.current_contexts().size());
    for (int c=0; c<one->v.size(); c++) {
	setvp = is_set(one->v[c]);
	hvp = new HidingValue;
	hvp->interface = interface;
	hvp->setvp = setvp;
	one->v[c] = hvp;    
    }

    return one;
}

SvpVec * callback__71(FspTranslator& tr, SvpVec * one)
{
    return hiding_callback(tr, one, false);
}

SvpVec * callback__72(FspTranslator& tr, SvpVec * one)
{
    return hiding_callback(tr, one, true);
}

SvpVec * callback__73(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    SvpVec * vp = new SvpVec;
    SetValue * setvp;
    RelabelingValue * merged = NULL;
    RelabelingValue * rlv;
    int rank = -1;

    assert(one->v.size() == two->v.size() &&
	    two->v.size() == tr.current_contexts().size());
    for (int c=0; c<one->v.size(); c++) {
	setvp = is_set(one->v[c]);
	rlv = is_relabeling(two->v[c]);
	if (setvp->rank != rank) {
	    rank = setvp->rank;
	    merged = rlv;
	    two->detach(c);
	    vp->v.push_back(merged);
	} else
	    merged->merge(*rlv);
    }
    tr.css.pop();
    delete one;
    delete two;

    return vp;
}

SvpVec * callback__75(FspTranslator& tr, SvpVec * one)
{
    tr.css.pop();

    return one;
}

SvpVec * callback__76(FspTranslator& tr, SvpVec * one, SvpVec * two,
					    SvpVec * three)
{
    SvpVec * vp;
    Lts * lts;

    assert(three->v.size() == tr.current_contexts().size());
    if (two) {
	SetValue * setvp;

	assert(two->v.size() == three->v.size());
	/* Apply process labeling, for each context separately. */
	for (int k=0; k<two->v.size(); k++) {
	    setvp = is_set(two->v[k]);
	    lts = is_lts(three->v[k]);
	    lts->labeling(*setvp);
	    lts->rank = setvp->rank;
	}

	/* Merge LTSs by prefix-labeling rank. */
	vp = new SvpVec;
	merge_lts_by_rank(tr, three, *vp);
    } else
	vp = three;

    if (one) {
	SetValue * setvp;

	assert(one->v.size() == vp->v.size());
	/* Apply process sharing. */
	for (int k=0; k<one->v.size(); k++) {
	    lts = is_lts(vp->v[k]);
	    setvp = is_set(one->v[k]);
	    lts->sharing(*setvp);
	}

    }

    /* Clean up the dirty contexts caused by labeling_OPT. */
    tr.css.pop();
    tr.css.push_clone();

    return vp;
}

SvpVec * callback__77(FspTranslator& tr, SvpVec * one, SvpVec * two,
				SvpVec * three, SvpVec * four, SvpVec * five)
{
    relabel(tr, five, four);
    if (one) delete one;
    if (two) delete two;
    if (five) delete five;

    return four;
}

SvpVec * callback__78(FspTranslator& tr, SvpVec * one, SvpVec * two,
						    SvpVec * three)
{
    SvpVec * vp;
    LtsComposition * lcp;

    /* Restore the contexts modified by labeling_OPT. */
    tr.css.pop();

    assert(three->v.size() == tr.current_contexts().size());
    if (two) {
	SetValue * setvp;

	assert(two->v.size() == three->v.size());
	/* Apply process labeling, for each context separately. */
	for (int k=0; k<two->v.size(); k++) {
	    setvp = is_set(two->v[k]);
	    lcp = is_ltscomposition(three->v[k]);

	    /* Apply process labeling to each component process, before
	       performing the composition. */
	    for (int i=0; i<lcp->lts.size(); i++)
		lcp->lts[i]->labeling(*setvp);
	    lcp->rank = setvp->rank;
	}

	/* Merge LTS compositions by labeling rank. */
	vp = new SvpVec;
	merge_ltscomp_by_rank(tr, three, *vp);
    } else
	vp = three;

    if (one) {
	SetValue * setvp;

	assert(one->v.size() == vp->v.size());
	/* Apply process sharing, before performing the composition. */
	for (int k=0; k<one->v.size(); k++) {
	    setvp = is_set(one->v[k]);
	    lcp = is_ltscomposition(vp->v[k]);
	    for (int i=0; i<lcp->lts.size(); i++)
		lcp->lts[i]->sharing(*setvp);
	}

    }

    /* Restore the contexts to the state previous to labeling_OPT. */
    tr.css.pop();
    tr.css.push_clone();

    return vp;
}

SvpVec * callback__79(FspTranslator& tr, SvpVec * one, SvpVec * two,
				SvpVec * three, SvpVec * four, SvpVec * five)
{
    SvpVec * vp = new SvpVec;
    LtsComposition * lcp;
    Lts * lts;

    if (five) {
	/* Apply relabeling. */
	assert(four->v.size() == five->v.size());
	for (int c=0; c<five->v.size(); c++) {
	    lcp = is_ltscomposition(four->v[c]);

	    /* Apply relabeling to each component process, before
	       performing the composition. */
	    for (int i=0; i<lcp->lts.size(); i++)
		relabel_one(five->v[c], lcp->lts[i]);

	}
    }

    /* Perform parallel composition. */
    for (int c=0; c<four->v.size(); c++) {
	lcp = is_ltscomposition(four->v[c]);
	/* Compose all the processes contained in 'lcp'. */
	lts = lcp->lts[0];
	for (int i=1; i<lcp->lts.size(); i++) {
	    lts->compose(*(lcp->lts[i]));
	    delete lcp->lts[i];
	}
	vp->v.push_back(lts);
    }
    delete four;
    if (one) delete one;
    if (two) delete two;
    if (five) delete five;

    return vp;
}

SvpVec * callback__80(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    SvpVec * vp = new SvpVec;
    SetValue * setvp;
    Lts * lts;

    tr.css.pop();
    assert(one->v.size() == two->v.size());
    for (int k=0; k<one->v.size(); k++) {
	setvp = is_set(one->v[k]);
	lts = is_lts(two->v[k]);
	lts->rank = setvp->rank;
    }
    merge_lts_by_rank(tr, two, *vp);
    delete one;
    delete two;

    return vp;
}

SvpVec * callback__81(FspTranslator& tr, SvpVec * one, SvpVec * two,
							    SvpVec * three)
{
    /* TODO optimize: useless calculation of both branches. */
    ConstValue * cvp;
    SvpVec * vp = three;

    assert(one->v.size() == two->v.size() &&
	    one->v.size() == tr.current_contexts().size());
    assert(!three || one->v.size() == three->v.size());
    if (!vp) {
	vp = new SvpVec;
	for (int k=0; k<one->v.size(); k++)
	    vp->v.push_back(new Lts(LtsNode::Normal, &tr.cr.actions));
    }
    for (int c=0; c<one->v.size(); c++) {
	cvp = is_const(one->v[c]);
	if (!cvp->value) {
	    SymbolValue * tmp = two->v[c];
	    two->v[c] = vp->v[c];
	    vp->v[c] = tmp;
	}
    }
    delete vp;

    return two;
}

SvpVec * callback__82(FspTranslator& tr, SvpVec * one)
{
    SvpVec * vp = new SvpVec;
    LtsComposition * lcp;
    Lts * lts;

    for (int c=0; c<one->v.size(); c++) {
	lts = is_lts(one->v[c]);
	lcp = new LtsComposition;
	lcp->lts.push_back(lts);
	vp->v.push_back(lcp);
    }
    one->detach_all();
    delete one;

    return vp;
}

SvpVec * callback__83(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    LtsComposition * lcp;
    Lts * lts;

    assert(one->v.size() == two->v.size());
    for (int c=0; c<one->v.size(); c++) {
	lcp = is_ltscomposition(one->v[c]);
	lts = is_lts(two->v[c]);
	lcp->lts.push_back(lts);
    }
    two->detach_all();
    delete two;

    return one;
}

SvpVec * callback__84(FspTranslator& tr, string * one, SvpVec * two)
{
    SymbolValue * svp;
    Lts * lts;
    ParametricProcess * ppp;
    SvpVec * vp = new SvpVec;
    SvpVec * argvp = two;
    ArgumentsValue * avp;

    /* Lookup 'process_id' in the 'parametric_process' table. */
    if (!tr.cr.parametric_processes.lookup(*one, svp)) {
	stringstream errstream;
	errstream << "Process " << *one << " undeclared";
	semantic_error(errstream, tr.locations[0]);
    }
    ppp = is_parametric(svp);

    if (!argvp) {
	/* 'argvp' is going to specify the default parameters for every
	   context. */
	argvp = new SvpVec;
	for (int k=0; k<tr.current_contexts().size(); k++) {
	    avp = new ArgumentsValue;
	    avp->args = ppp->parameter_defaults;
	    argvp->v.push_back(avp);
	}
    }

    for (int k=0; k<argvp->v.size(); k++) {
	string name = *one;
	string extension;
	SymbolValue * svp;

	avp = is_arguments(argvp->v[k]);
	lts = get_parametric_lts(tr, *one, ppp, avp);

	/* Free all the nodes allocated by c.pna, and remove the from
	   the allocator itself. */
	tr.cr.pna.clear();

	vp->v.push_back(lts);
    }
    delete one;
    delete argvp;

    return vp;
}

SvpVec * callback__85(FspTranslator& tr, SvpVec * one)
{
    SvpVec * vp = new SvpVec;

    merge_by_rank(one, *vp);
    delete one;

    /* We have to clean up the contexts, since process sharing does not
       extend the contexts. */
    tr.css.pop();
    tr.css.push_clone();

    return vp;
}

SvpVec * callback__86(FspTranslator& tr, SvpVec * one)
{
    return priority_callback(tr, one, true);
}

SvpVec * callback__87(FspTranslator& tr, SvpVec * one)
{
    return priority_callback(tr, one, false);
}

Lts * callback__88(FspTranslator& tr, string * one, SvpVec * two,
					    SvpVec * three, SvpVec * four)
{
    Lts * lts;

    tr.css.pop();
    assert(two->v.size() == 1);
    lts = is_lts(two->v[0]);
    two->detach(0);

    /* Apply priority. */
    if (three) {
	PriorityValue * prv = is_priority(three->v[0]);

	assert(three->v.size() == 1);
	lts->priority(*(prv->setvp), prv->low);
    }

    /* Apply hiding. */
    if (four) {
	HidingValue * hvp = is_hiding(four->v[0]);

	assert(four->v.size() == 1);
	lts->hiding(*(hvp->setvp), hvp->interface);
    }
    lts->name = *one;
    delete one;
    delete two;
    delete three;
    delete four;

    return lts;
}
