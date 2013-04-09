/* For semantic errors */
stringstream errstream;

static void semantic_error()
{
    cout << "Semantic error: " << errstream.str() << "\n";
    exit(-1);
}

template <class T>
static Context* extended_context(Context * ctx, const string& var, 
				const T& val)
{
    ctx = new Context(*ctx);
    if (!ctx->insert(var, val)) {
	errstream << "Variable " << var
	    << " declared twice\n";
	semantic_error();
    }

    return ctx;
}

/* Helper function used to get a ConstValue* from a SymbolValue*. If the
   object pointed is not a constant, a semantic error is issued. */
inline ConstValue* err_if_not_const(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Const) {
	errstream << "Const expected";
	semantic_error();
    }

    return (ConstValue *)svp;
}

inline RangeValue* err_if_not_range(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Range) {
	errstream << "Range expected";
	semantic_error();
    }

    return (RangeValue *)svp;
}

inline SetValue* err_if_not_set(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Set) {
	errstream << "Set expected";
	semantic_error();
    }

    return (SetValue *)svp;
}

inline ProcessValue* err_if_not_process(SymbolValue * svp)
{
    if (svp->type() != SymbolValue::Process) {
	errstream << "Process expected";
	semantic_error();
    }

    return (ProcessValue *)svp;
}

inline ProcessNode * err_if_not_procnode(ProcessBase * pbp) {
    if (pbp->unresolved() || pbp->connected()) {
	errstream << "ProcessNode expected";
	semantic_error();
    }
    return (ProcessNode *)pbp;
}

