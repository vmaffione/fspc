/* Inline utilities. */
#include "utils.hpp"

/* Context, ContextsSet and ContextsSetStack. */
#include "context.hpp"

/* Definition of the FspTranslator class. */
#include "translator.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"



#define DEBUG
#ifdef DEBUG
#define PROX(x) cout<<"PROX: ";x;cout<<"\n"
#define PROP(x) cout<<"PROP:==============================================================\n      " << x << "\n"
#else
#define PROP(x)
#define PROX(x)
#endif



template <class T>
static Context* extended_context(Context * ctx,
				const string& var, const T& val)
{
    ctx = new Context(*ctx);
    if (!ctx->insert(var, val)) {
	stringstream errstream;
	errstream << "Variable " << var << " declared twice\n";
	semantic_error(errstream);
    }

    return ctx;
}

/* Left contains a SetValue*, while right is the result of 'action_range'. */
SvpVec * indexize_svpvec(struct FspTranslator * gp, SvpVec * left, SvpVec * right)
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
	    rvp = err_if_not_range(right->v[0]);
	    if (rvp->variable != "") {
		/* In this case each element in action_range is in the
		   form 'var:R', where R is a range_id or a range_expr. */
		csp = new ContextsSet;
		csp->frontier = gp->current_contexts().frontier;
		vp = new SvpVec;
		for (int c=0; c<left->v.size(); c++) {
		    setvp = err_if_not_set(left->v[c]);
		    rvp = err_if_not_range(right->v[c]);
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
			cxp = extended_context(gp->current_contexts()[c],
				rvp->variable, j);
			csp->append(cxp);
		    }
		}
		cout << "Contexts ramification\n";
		gp->css.update(csp);
		delete left;
		delete right;
		return vp;
	    } else {
		/* In this case each element in action_range is in the
		   form "R", where R is a range_id or a range_expr. */
		for (int c=0; c<left->v.size(); c++) {
		    setvp = err_if_not_set(left->v[c]);
		    rvp = err_if_not_range(right->v[c]);
		    setvp->indexize(rvp->low, rvp->high);
		    /* Rank is inherited. */
		}
		delete right;
		return left;
	    }
	    break;

	case SymbolValue::Const:
	    for (int c=0; c<left->v.size(); c++) {
		setvp = err_if_not_set(left->v[c]);
		cvp = err_if_not_const(right->v[c]);
		setvp->indexize(cvp->value);
		/* Rank is inherited. */
	    }
	    delete right;
	    return left;
	    break;
	case SymbolValue::Set:
	    setvp = err_if_not_set(right->v[0]);
	    if (setvp->variable != "") {
		/* In this case each element in action_range is in the
		   form 'var:R', where R is a set_id or a set_expr. */
		csp = new ContextsSet;
		csp->frontier = gp->current_contexts().frontier;
		/* We need a new SvpVec for a new ContextSet. */
		vp = new SvpVec;
		for (int c=0; c<left->v.size(); c++) {
		    setvp = err_if_not_set(left->v[c]);
		    rsetvp = err_if_not_set(right->v[c]);
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
			cxp = extended_context(gp->current_contexts()[c],
				rsetvp->variable, rsetvp->actions[j]);
			/* Append the new Context to the new ContextSet. 
			 */
			csp->append(cxp);
		    }
		}
		cout << "Contexts ramification\n";
		gp->css.update(csp);
		delete left;
		delete right;
		return vp;
	    } else {
		for (int c=0; c<left->v.size(); c++) {
		    setvp = err_if_not_set(left->v[c]);
		    rsetvp = err_if_not_set(right->v[c]);
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
	    cerr << "I should not reach this point.\n";
	    assert(0);
	    exit(1);
    }
}

/* Fix unresolved ProcessNode references due to cyclic processes. */
static void fix_unresolved_references(ProcessNode * pnp, void * opaque)
{
    FspTranslator * trp = (FspTranslator *)opaque;

    for (int i=0; i<pnp->children.size(); i++) {
	ProcessEdge& e = pnp->children[i];
	SymbolValue * svp;
	if (e.dest == NULL) {
	    cout << "Unref " << pnp << ": " << trp->actions.reverse[e.action]
		<< " -> " << e.unresolved_reference << "\n";				
	    if (!trp->local_processes.lookup(e.unresolved_reference, svp)) {
		stringstream errstream;
errstream << "Local process " << pnp << ": "
		    << e.unresolved_reference << " undeclared\n";
		semantic_error(errstream);
	    }
	    e.dest = ((ProcessValue *)svp)->pnp;
	}
    }
}


/* action_labels: LowerCaseID */
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
	setvp = err_if_not_set(one->v[c]);
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
	setvp = err_if_not_set(one->v[c]);
	rsetvp = err_if_not_set(two->v[c]);
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
    return indexize_svpvec(&tr, one, two);
}

SvpVec * callback__5(FspTranslator& tr, string * one)
{
    SvpVec * vp = new SvpVec;
    SymbolValue * svp;
    SetValue * setvp;
    if (!tr.identifiers.lookup(*one, svp)) {
	stringstream errstream;
	errstream << "set " << *one << " undeclared";
	semantic_error(errstream);
    }
    delete one;
    setvp = err_if_not_set(svp);
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

    if (!tr.identifiers.lookup(*two, svp)) {
	stringstream errstream;
	errstream << "range/set " << *two << " undeclared";
	semantic_error(errstream);
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
	rvp = err_if_not_range(two->v[i]);
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
	setvp = err_if_not_set(two->v[i]);
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
	cvp = err_if_not_const(one->v[i]);
	rvp->low = cvp->value;
	cvp = err_if_not_const(two->v[i]);
	rvp->high = cvp->value;
	vp->v.push_back(rvp);
    }
    delete one;
    delete two;

    return vp;
}

SvpVec * callback__13(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int c=0; c<one->v.size(); c++) {
	SetValue * setvp = err_if_not_set(one->v[c]);
	SetValue * rsetvp = err_if_not_set(two->v[c]);
	*setvp += *rsetvp;
    }
    delete two;

    return one;
}

SvpVec * callback__14(FspTranslator& tr, string * one)
{
    /* Parameters have been pushed inside the 'param_OPT' rule.
       A cleaner approach would be to get a list of parameters from
       the rule and push the parameters into the 'tr.identifiers' table
       in this action. */
    tr.init_fakenode();

    return NULL;
}

void * callback__15(FspTranslator& tr, string * one, Pvec * two,
			SvpVec * three)
{
    PROP("process_def --> ... process_body ...");
    PROX(cout<<*one<<" = "; two->v[0]->print(&tr.actions));

    ProcessBase * pbp = two->v[0];
    SymbolValue * svp;
    ProcessValue * pvp = NULL;

    assert(two->v.size() == 1);
    if (pbp->unresolved()) {
	tr.aliases.insert(*one, ((UnresolvedProcess* )pbp)->reference);
	tr.aliases.print();
    } 
    tr.aliases.fill_process_table(tr.local_processes);
    if (pbp->unresolved()) {
	if(!tr.local_processes.lookup(*one, svp)) {
	    stringstream errstream;
	    errstream << "process " << *one << " undeclared";
	    semantic_error(errstream);
	}
	pvp = err_if_not_process(svp);
    } else {
	/* Insert a new ProcessValue in the symbol table, attaching the
	   process_body to it. */
	pvp = new ProcessValue;
	pvp->pnp = err_if_not_procnode(pbp);
	if (!tr.local_processes.insert(*one, pvp)) {
	    stringstream errstream;
	    errstream << "process " << *one << " declared twice";
	    semantic_error(errstream);
	}
	cout << "Process " << *one << " defined (" << pvp->pnp << ")\n";
    }

    PROX(cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<< Process " << *one << " defined >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    /* Try to resolve all the undefined references into this process. */
    struct ProcessVisitObject f;
    f.vfp = &fix_unresolved_references;
    f.opaque = &tr;
    pvp->pnp->visit(f);

    PROX(cout<<"resolved: "; pvp->pnp->print(&tr.actions));

    /* Convert the collection of ProcessNodes in an Lts object. */
    Lts lts(pvp->pnp, &tr.actions);

    /* Now we can free the graph pointed by pvp->pnp. */
    freeProcessNodeGraph(pvp->pnp);

    /* Extend the alphabet if is the case. */
    if (three) {
	SetValue * setvp;
	if (three->v.size() != 1) {
	    stringstream errstream;
	    errstream << "Multiset alphabet extension";
	    semantic_error(errstream);
	}
	setvp = err_if_not_set(three->v[0]);
	for (int i=0; i<setvp->actions.size(); i++)
	    lts.updateAlphabet(tr.actions.insert(setvp->actions[i]));
	delete setvp;
    }

    /* Remove process parameters from the tr.identifiers. */
    for (int i=0; i<tr.parameters.size(); i++) {
	tr.identifiers.remove(*(tr.parameters[i]));
	delete tr.parameters[i];
    }
    tr.parameters.clear();

    /* Clear 'tr.local_processes' and 'tr.aliases'. */
    tr.local_processes.clear();
    tr.aliases.clear();

    lts.print();
    lts.graphvizOutput((*one += ".gv").c_str());
    delete one;

    // TODO implement everything is OPT
}

void * callback__16(FspTranslator& tr, string * one)
{
    /* The following nonterminal 'index_ranges_OPT' may result
       in contexts ramifications, and so tr.css.update() must be called. We
       therefore push a clone of the current ContextsSet, which is the
       initial empty one, so that the initial one is not modified. */
    tr.css.push_clone();
}

void * callback__17(FspTranslator& tr, string * one, SvpVec * two)
{
    tr.init_fakenode();
}

Pvec * callback__18(FspTranslator& tr, string * one, SvpVec * two,
							    Pvec * three)
{
    ProcessValue * pvp;
    SetValue * setvp;
    string procname;

    for (int c=0; c<two->v.size(); c++) {
	setvp = err_if_not_set(two->v[c]);
	for (int j=0; j<setvp->actions.size(); j++) {
	    procname = *one + setvp->actions[j];
	    if (three->v[c]->unresolved()) {
		tr.aliases.insert(procname,
			((UnresolvedProcess* )three->v[c])->reference);
		tr.aliases.print();
	    } else {
		pvp = new ProcessValue;
		pvp->pnp = err_if_not_procnode(three->v[c]);
		if (j)
		    pvp->pnp = pvp->pnp->clone();
		if (!tr.local_processes.insert(procname, pvp)) {
		    stringstream errstream;
		    errstream << "process " << procname
			<< " declared twice";
		    semantic_error(errstream);
		}
		cout << "Process " << procname << " defined (" << pvp->pnp << ")\n";
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

