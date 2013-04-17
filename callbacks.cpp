/* Inline utilities. */
#include "utils.hpp"

/* Context, ContextsSet and ContextsSetStack. */
#include "context.hpp"

/* Definition of the FspTranslator class. */
#include "translator.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

#include "callbacks.hpp"


#define DEBUG
#ifdef DEBUG
#define PROX(x) cout<<"PROX: ";x;cout<<"\n"
#define PROP(x) cout<<"PROP:==============================================================\n      " << x << "\n"
#else
#define PROP(x)
#define PROX(x)
#endif

/* Replay a sequence of callbacks under a local translator and a local
   stack. */
ProcessValue * ParametricProcess::replay(struct FspCompiler& c,
					    const vector<int>& values)
{
    vector<void *> stack;
    FspTranslator tr(c);
    ConstValue * cvp;
    SymbolValue * svp;

    cout << "Replay!!\n";
    assert(values.size() == parameter_defaults.size());
    assert(parameter_defaults.size() == parameter_names.size());

    /* Load the parameters into c.identifiers. */
    for (int i=0; i<parameter_names.size(); i++) {
	cvp = new ConstValue;
	cvp->value = values[i];
	svp = cvp;
	if (!c.identifiers.insert(parameter_names[i], svp)) {
	    stringstream errstream;
	    errstream << "identifier " << parameter_names[i]
			    << " already declared";
	    semantic_error(errstream);
	}
    }

    /* Replay the callbacks. */
    for (int i=0; i<record.size(); i++) {
	cout << record[i]->is_void() << ", " << stack.size() << " ";
	record[i]->print();
	void * ret = record[i]->execute(tr, stack);
	if (!(record[i]->is_void()))
	    stack.push_back(ret);
    }
    assert(stack.size() == 1);

    /* Remove the parameters from c.identifiers. */
    for (int i=0; i<parameter_names.size(); i++)
	c.identifiers.remove(parameter_names[i]);

    return static_cast<ProcessValue *>(stack.back());
}

void ParametricProcess::print() const
{
    cout << "Parameters (defaults):\n";
    for (int i=0; i<parameter_names.size(); i++)
	cout << parameter_names[i] << " (" << parameter_defaults[i] << ")\n";
    cout << "   List of " << record.size() << " callbacks\n";
}


ParametricProcess* err_if_not_parametric(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::ParametricProcess) {
	stringstream errstream;
	errstream << "Parametric process expected";
	semantic_error(errstream);
    }

    return (ParametricProcess *)svp;
}

/* Helper function used with T=string and T=int. */
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
	    cout << "Unref " << pnp << ": "
		<< trp->cr.actions.reverse[e.action]
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

/* Data structure associated to the visit function 'check_if_sequential'. */
struct ChifseqData {
    bool sequential;
    ProcessNode * tail;
    ProcessNode * tail_prev;
};

/* Return true in opaque->sequential if the process is sequential. Before
   invoking the visit, opaque->sequential must be initialized to true.
   Also returns a pointer to the last node in opaque->tail and a pointer
   to the one before the last node in opaque->tail_prev.*/
static void check_if_sequential(ProcessNode * pnp, void * opaque)
{
    ChifseqData * d = static_cast<ChifseqData *>(opaque);

    d->sequential = d->sequential && (pnp->children.size() == 1 || 
	    (pnp->children.size() == 0 && pnp->type == ProcessNode::End));
    d->tail_prev = d->tail;
    d->tail = pnp;
}



/* =========================== CALLBACKS =============================== */
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
    if (!tr.cr.identifiers.lookup(*one, svp)) {
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

    if (!tr.cr.identifiers.lookup(*two, svp)) {
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

void * callback__14(FspTranslator& tr, string * one)
{
    /* Parameters have been pushed inside the 'param_OPT' rule.
       A cleaner approach would be to get a list of parameters from
       the rule and push the parameters into the 'tr.cr.identifiers' table
       in this action. */
    tr.init_fakenode();

    return NULL;
}

ProcessValue * callback__15(FspTranslator& tr, string * one, Pvec * two,
			SvpVec * three)
{
    PROP("process_def --> ... process_body ...");
    PROX(cout<<*one<<" = "; two->v[0]->print(&tr.cr.actions));

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

    PROX(cout<<"resolved: "; pvp->pnp->print(&tr.cr.actions));

    if (three) {
	if (three->v.size() != 1) {
	    stringstream errstream;
	    errstream << "Multiset alphabet extension";
	    semantic_error(errstream);
	}
	pvp->setvp = err_if_not_set(three->v[0]);
	three->detach(0);
	delete three;
    } else
	pvp->setvp = NULL;

    /* Clear 'tr.local_processes' and 'tr.aliases'. */
    tr.local_processes.clear();
    tr.aliases.clear();
    delete one;
    // TODO implement everything is OPT

    return pvp;
}

void * callback__16(FspTranslator& tr, string * one)
{
    /* The following nonterminal 'index_ranges_OPT' may result
       in contexts ramifications, and so tr.css.update() must be called. We
       therefore push a clone of the current ContextsSet, which is the
       initial empty one, so that the initial one is not modified. */
    tr.css.push_clone();

    return NULL;
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
	cvp = err_if_not_const(one->v[c]);
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
	cvp = err_if_not_const(one->v[c]);
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
	cvp = err_if_not_const(one->v[c]);
	if (cvp->value)
	    pvec->v.push_back(two->v[c]);
	else if (three)
	    pvec->v.push_back(three->v[c]);
	else
	    /* If $7 == NULL there is no else branch, and so we insert
	       a STOP state. */
	    pvec->v.push_back(new ProcessNode);
    }
    delete one;

    return pvec;
}

void * callback__22(FspTranslator& tr)
{
    /* Replicate the CSS stack top so that it can be used by 'choice'. */
    tr.css.push_clone();

    return NULL;
}

Pvec * callback__23(FspTranslator& tr)
{
    Pvec * pvec = new Pvec;
    ProcessNode * pnp = new ProcessNode(ProcessNode::End);

    for (int c=0; c<tr.current_contexts().size(); c++)
	pvec->v.push_back(pnp);

    return pvec;
}

Pvec * callback__24(FspTranslator& tr)
{
    Pvec * pvec = new Pvec;
    ProcessNode * pnp = new ProcessNode;

    for (int c=0; c<tr.current_contexts().size(); c++)
	pvec->v.push_back(pnp);

    return pvec;
}

Pvec * callback__25(FspTranslator& tr)
{
    Pvec * pvec = new Pvec;
    ProcessNode * pnp = new ProcessNode(ProcessNode::Error);

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
	setvp = err_if_not_set(two->v[c]);
	assert(setvp->actions.size() == 1);
	name = *one + setvp->actions[0];
	cout << "looking for " << name << endl;
	/* If the process referenced is already defined, return it.
	   Otherwise return a new UnresolvedProcess object, so that the
	   upper level sees that 'name' is unresolved. */
	if (tr.local_processes.lookup(name, svp))
	    pbp = ((ProcessValue *)svp)->pnp;
	else
	    pbp = new UnresolvedProcess(name);
	pvec->v.push_back(pbp);
    }

    return pvec;
}

Pvec * callback__27(FspTranslator& tr, Pvec * one)
{
    /* Pop (and destroy) the replicated top and everything above. */
    tr.css.pop();
    
    return one;
}

void * callback__28(FspTranslator& tr, Pvec * one)
{
    /* Replicate the CSS stack top so that it can be used by
       'action_prefix'. */
    tr.css.push_clone();

    return NULL;
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
	    cvp = err_if_not_const(one->v[c]);
	    if (!cvp->value)
		tr.current_contexts().rule_out(c);
	}
	cout << "filtered\n"; tr.current_contexts().print();
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
	    pnp->children[child].dest =
		err_if_not_procnode(three->v[rank]);
	else
	    /* If $5 is an unresolved reference, we scan the current
	       frontier recording the reference itself, so that this
	       reference will be fixed by the upper levels. */
	    pnp->children[child].unresolved_reference =
		((UnresolvedProcess *)three->v[rank])->reference;
    }

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
		new ProcessNode(ProcessNode::Normal);
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
	    setvp = err_if_not_set(one->v[c]);
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
	ProcessBase * pbp = new ConnectedProcess();
	for (int c=0; c<max_rank+1; c++)
	    pvec->v.push_back(pbp);
    }
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
		    new ProcessNode(ProcessNode::Normal);
		npnp = pnp->children[child].dest;
		rank = pnp->children[child].rank;
		for (int c=0; c<two->v.size(); c++) {
		    if (tr.current_contexts().is_ruled_out(c)) continue;
		    setvp = err_if_not_set(two->v[c]);
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
	setvp = new SetValue;
	cvp = err_if_not_const(one->v[c]);
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
	setvp = err_if_not_set(one->v[c]);
	cvp = err_if_not_const(two->v[c]);
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
    SvpVec * vp;
    SetValue * setvp;
    ConstValue * cvp;

    /* Here we are sure to have only the empty context. */
    assert(tr.current_contexts().size() == 1 && one->v.size() == 1);

    if (one->v[0]->type() == SymbolValue::Set) {
	stringstream errstream;
	errstream << "Unexpected set";
	semantic_error(errstream);
    }
    setvp = new SetValue;
    setvp->actions.push_back("");
    vp = new SvpVec;
    vp->v.push_back(setvp);

    return indexize_svpvec(&tr, vp, one);
}

SvpVec * callback__39(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(two->v.size() == one->v.size());
    if (two->v[0]->type() == SymbolValue::Set) {
	stringstream errstream;
	errstream << "Unexpected set";
	semantic_error(errstream);
    }

    return indexize_svpvec(&tr, one, two);
}

SvpVec * callback__41(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = vpl->value && vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__42(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = vpl->value && vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__43(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value |= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__44(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value ^= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__45(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value &= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__46(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = (vpl->value == vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__47(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = (vpl->value != vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__48(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = (vpl->value < vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__49(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = (vpl->value > vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__50(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = (vpl->value <= vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__51(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = (vpl->value >= vpr->value);
    }
    delete two;

    return one;
}

SvpVec * callback__52(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value <<= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__53(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value >>= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__54(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	//cout << vpl->value << " + " << vpr->value << "\n";
	vpl->value += vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__55(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	//cout << vpl->value << " - " << vpr->value << "\n";
	vpl->value -= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__56(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	//cout << vpl->value << " * " << vpr->value << "\n";
	vpl->value *= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__57(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	//cout << vpl->value << " / " << vpr->value << "\n";
	vpl->value /= vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__58(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    assert(one->v.size() == two->v.size());
    for (int i=0; i<one->v.size(); i++) {
	ConstValue * vpl = err_if_not_const(one->v[i]);
	ConstValue * vpr = err_if_not_const(two->v[i]);
	vpl->value = vpl->value % vpr->value;
    }
    delete two;

    return one;
}

SvpVec * callback__59(FspTranslator& tr, SvpVec * one)
{
    ConstValue * cvp;

    for (int i=0; i<one->v.size(); i++) {
	cvp = err_if_not_const(one->v[i]);
	cvp->value *= -1;
    }

    return one;
}

SvpVec * callback__60(FspTranslator& tr, SvpVec * one)
{
    ConstValue * cvp;

    for (int i=0; i<one->v.size(); i++) {
	cvp = err_if_not_const(one->v[i]);
	cvp->value = !(cvp->value);
	//cout << "UU ! " << cvp->value << "\n";
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
	    errstream << "variable " << *one << " undeclared\n";
	    semantic_error(errstream);
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
	semantic_error(errstream);
    }
    delete one;

    /* Return a ConstValue* for each context (in non shared mode). */
    vp = new SvpVec;
    vp->shared = false;
    for (int c=0; c<tr.current_contexts().size(); c++)
	vp->v.push_back(svp->clone());

    return vp;
}

Pvec * callback__64(FspTranslator& tr, SvpVec * one, Pvec * two)
{
    assert(one->v.size() == two->v.size() &&
	    two->v.size() == tr.current_contexts().size());
    Pvec * pvec = new Pvec;

    for (int c=0; c<one->v.size(); c++) {
	ProcnodePairValue * pair = err_if_not_procnodepair(one->v[0]);
	ProcessBase * pbp = two->v[c];

	assert(!pbp->connected());
	assert(pair->second->children.size() == 1);
	delete pair->second->children[0].dest;
	if (pbp->unresolved()) {
	    pair->second->children[0].dest = NULL;
	    pair->second->children[0].unresolved_reference =
		((UnresolvedProcess *)pbp)->reference;
	} else {
	    ProcessNode * pnp = err_if_not_procnode(pbp);
	    pair->second->children[0].dest = pnp;
	}
	pvec->v.push_back(pair->first);
    }
    delete one;
    delete two;

    return pvec;
}

SvpVec * callback__65(FspTranslator& tr, SvpVec * one, SvpVec * two)
{
    for (int c=0; c<one->v.size(); c++) {
	ProcnodePairValue * lpair = err_if_not_procnodepair(one->v[c]);
	ProcnodePairValue * rpair = err_if_not_procnodepair(two->v[c]);

	/* Concatenate the two processes. */
	assert(lpair->second->children.size() == 1);
	delete lpair->second->children[0].dest;
	lpair->second->children[0].dest = rpair->first;
	lpair->second = rpair->second;
    }
    delete two;

    return one;
}

SvpVec * callback__66(FspTranslator& tr, string * one, SvpVec * two)
{
    SymbolValue * svp;
    Lts * lts;
    ParametricProcess * ppp;
    ProcessValue * pvp;
    SvpVec * vp = new SvpVec;
    SvpVec * argvp = two;
    ArgumentsValue * avp;

    /* Lookup 'process_id' in the 'parametric_process' table. */
    if (!tr.cr.parametric_processes.lookup(*one, svp)) {
	stringstream errstream;
	errstream << "Process " << *one << " undeclared\n";
	semantic_error(errstream);
    }
    ppp = err_if_not_parametric(svp);

    if (!argvp) {

	argvp = new SvpVec;
	for (int k=0; k<tr.current_contexts().size(); k++) {
	    avp = new ArgumentsValue;
	    avp->args = ppp->parameter_defaults;
	    argvp->v.push_back(avp);
	}
    }

    for (int k=0; k<argvp->v.size(); k++) {
	string name = *one;
	struct ProcessVisitObject f;
	ChifseqData vd;

	avp = err_if_not_arguments(argvp->v[k]);
	if (avp->args.size() != ppp->parameter_names.size()) {
	    stringstream errstream;
	    errstream << "Parameters mismatch\n";
	    semantic_error(errstream);
	}
	pvp = ppp->replay(tr.cr, avp->args);
	/* Compute the process name. TODO */

	/* Check that the process is sequential. */
	f.vfp = &check_if_sequential;
	vd.sequential = true;
	vd.tail = vd.tail_prev = NULL;
	f.opaque = &vd;
	pvp->pnp->visit(f);
	if (!vd.sequential) {
	    stringstream errstream;
	    errstream << "Process " << *one << " undeclared\n";
	    semantic_error(errstream);
	}
	vp->v.push_back(new ProcnodePairValue(pvp->pnp, vd.tail_prev));
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
	cvp = err_if_not_const(one->v[c]);
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
	cvp = err_if_not_const(two->v[c]);
	avp = err_if_not_arguments(one->v[c]);
	avp->args.push_back(cvp->value);
    }
    delete two;

    return one;
}

