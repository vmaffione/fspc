#include "driver.hpp"

/* Inline utilities. */
#include "utils.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

/* Serialization and deserialization support. */
#include "serializer.hpp"

/* Interactive shell */
#include "shell.hpp"

/* Preprocessor */
#include "preproc.hpp"

/* Bison generated parser. */
#include "parser.hpp"

/* The parse tree structures. */
#include "tree.hpp"


using namespace std;


//#define DEBUG
#ifdef DEBUG
#define IFD(x) x
#else
#define IFD(x)
#endif


/* ============================== FspDriver ============================= */

FspDriver::FspDriver() : actions("Global actions table")
{
    trace_scanning = trace_parsing = false;
    tree = NULL;
}

FspDriver::~FspDriver()
{
    this->clear();
}

void FspDriver::clear()
{
    if (remove_file.size()) {
	remove(remove_file.c_str());
	remove_file = "";
    }
    if (tree) {
        delete tree;
        tree = NULL;
    }
}

void FspDriver::translateDeclarations()
{
    vector<string> classes;
    vector<yy::TreeNode *> declarations;

    assert(tree);
    /* Find all the tree nodes that don't correspond to process
       definitions (e.g. declarations). Observe that a property
       definition is considered to be a process definition. */
    classes.push_back(yy::ConstantDefNode::className());
    classes.push_back(yy::RangeDefNode::className());
    classes.push_back(yy::SetDefNode::className());
    classes.push_back(yy::ProgressDefNode::className());
    classes.push_back(yy::MenuDefNode::className());
    tree->getNodesByClasses(classes, declarations);

    /* Translate the declarations found. This results in filling
       in the following symbols tables: 'identifiers', 'progresses'
       and 'menus'. */
    for (unsigned int i = 0; i < declarations.size(); i++) {
        declarations[i]->translate(*this);
    }
}

void FspDriver::translateProcessesDefinitions()
{
    vector<string> classes;
    vector<yy::TreeNode *> definitions;

    assert(tree);
    /* Find all the tree nodes that correspond to process definitions
       (either simple or composite). */
    classes.push_back(yy::ProcessDefNode::className());
    classes.push_back(yy::CompositeDefNode::className());
    tree->getNodesByClasses(classes, definitions);

    for (unsigned int i = 0; i < definitions.size(); i++) {
        /* For each process definition, find all the parameters name
           and default values, building a ParametricProcess object.
           This object is then stored into the 'parametric_process'
           symbols table. */
        DTCS(yy::ProcessDefNode, pdn, definitions[i]);
        DTCS(yy::CompositeDefNode, cdn, definitions[i]);
        yy::LtsTreeNode *ltn;
        yy::ProcessIdNode *pid;
        yy::ParamNode *pan;
        ParametricProcess *pp = new ParametricProcess;

        /* Find the ProcessIdNode and ParamNode depending on the process
           definition type. */
        if (pdn) {
            pid = yy::tree_downcast<yy::ProcessIdNode>(pdn->getChild(1));
            pan = yy::tree_downcast_null<yy::ParamNode>(pdn->getChild(2));
            pp->set_translator(pdn);
            ltn = pdn;
        } else if (cdn) {
            pid = yy::tree_downcast<yy::ProcessIdNode>(cdn->getChild(1));
            pan = yy::tree_downcast_null<yy::ParamNode>(cdn->getChild(2));
            pp->set_translator(cdn);
            ltn = cdn;
        } else {
            assert(0);
        }

        pid->translate(*this);  /* Compute the name. */

        if (pan) {
            /* Find the parameters. */
            DTC(yy::ParameterListNode, pln, pan->getChild(1));

            for (unsigned int i = 0; i < pln->numChildren(); i += 2) {
                DTC(yy::ParameterNode, p, pln->getChild(i));
                DTC(yy::ParameterIdNode, in, p->getChild(0));
                DTC(yy::ExpressionNode, en, p->getChild(2));

                in->translate(*this);
                en->translate(*this);

                if (!pp->insert(in->res, en->res)) {
                    stringstream errstream;
                    errstream << "parameter " << in->res << " declared twice";
                    semantic_error(*this, errstream, p->getLocation());
                }
            }
        }

        /* Insert into the symbol table. */
        if (!this->parametric_processes.insert(pid->res, pp)) {
            stringstream errstream;

            delete pp;
            errstream << "Parametric process " << pid->res
                        << " already declared";
            semantic_error(*this, errstream, pid->getLocation());
        }

        /* Translate the process definition, filling in the 'processes'
           symbol table. Note that we don't call 'ltn->translate(*this)'
           directly, but we use the 'process_ref_translate()' wrapper
           function (which is also used with process references like ).
           This function also setups and restore the translator context,
           taking care of the default process parameters and overridden
           identifiers. */
        ltn->process_ref_translate(*this, pid->res, NULL, &ltn->res);
    }
}

int FspDriver::parse(const CompilerOptions& co)
{
    Serializer * serp = NULL;
    Deserializer * desp = NULL;
    stringstream ss;

    if (co.input_type == CompilerOptions::InputTypeFsp) {
	string orig(co.input_file);
	string temp = ".fspcc." + orig;
        int ret;

	for (unsigned int i=0; i<temp.size(); i++)
	    if (temp[i] == '\\' || temp[i] == '/')
		temp[i] = '.';

	/* Preprocess the input file, producing a temporary file. */
	preprocess(orig, temp);
	this->remove_file = temp;

	/* Parse the preprocessed temporary file. */
	scan_begin(temp.c_str());
	yy::FspParser parser(*this);
	parser.set_debug_level(trace_parsing);
	ret = parser.parse();
	scan_end();

	/* Remove the temporary file. */
	remove(temp.c_str());
	this->remove_file = "";

        if (ret) {
            /* On error, the parser returns 1. */
            return ret;
        }

        assert(tree);
#if 0
        /* Output a GraphViz representation of the parse tree. */
        ofstream treef("tree.gv");
        tree->print(treef);
        treef.close();
#endif
        /* Translate the declarations (e.g. range, set, progress, ...). */
        translateDeclarations();

        /* Collect and translate the process definitions. */
        translateProcessesDefinitions();

        if (co.output_file) {
	    serp = new Serializer(co.output_file);
        }
    } else { /* Load the processes table from an LTS file. */
	uint32_t nlts, nprogr;

	desp = new Deserializer(co.input_file);

	desp->actions_table(actions, 0);
	desp->integer(nlts, 0);
	for (uint32_t i=0; i<nlts; i++) {
	    yy::LtsPtr lts = new yy::Lts(LtsNode::End, &actions);

	    desp->lts(*lts, 0);
	    /* Insert lts into the global 'processes' table. */
	    if (!processes.insert(lts->name, lts.delegate())) {
                /* This should never happen: Two LTSs are saved with the same name
                   into the compiled file. */
		assert(0);
	    }
	}

	desp->integer(nprogr, 0);
	for (uint32_t i=0; i<nprogr; i++) {
	    ProgressS *pv = new ProgressS;
	    string name;

	    desp->stl_string(name, 0);
	    desp->progress_value(*pv, 0);
	    if (!progresses.insert(name, pv)) {
		assert(0);
	    }
	}
    }

    /* Scan the 'processes' symbols table. For each process, output
       the associated LTS and do the deadlock analysis. */
    map<string, Symbol *>::iterator it;
    map<string, Symbol *>::iterator jt;
    yy::LtsPtr lts;
    ProgressS *pv;

    if (serp) {
	serp->actions_table(actions, 0);
	serp->integer(processes.table.size(), 0);
    }
    for (it=processes.table.begin(); it!=processes.table.end(); it++) {
	lts = is<yy::Lts>(it->second);

	/* We output an LTS file only if the input is not an LTS file. */
	if (serp) {
	    serp->lts(*lts, 0);
	}

	if (co.deadlock) {
	    lts->deadlockAnalysis(ss);
	}

	if (co.graphviz) {
	    lts->graphvizOutput((lts->name + ".gv").c_str());
	}
    }

    if (serp) {
	serp->integer(progresses.table.size(), 0);
    }
    /* Do each progress check against all the global processes. */
    for (it=progresses.table.begin(); it!=progresses.table.end();
	    it++) {
	pv = is<ProgressS>(it->second);
	if (co.progress) {
	    for (jt=processes.table.begin();
		    jt!=processes.table.end(); jt++) {
		lts = is<yy::Lts>(jt->second);
		lts->progress(it->first, *pv, ss);
	    }
	}

	/* Output the property if the input is not an LTS file. */
	if (serp) {
	    serp->stl_string(it->first, 0);
	    serp->progress_value(*pv, 0);
	}
    }

    if (serp)
	delete serp;
    if (desp)
	delete desp;

    /* Flush out program output. */
    cout << ss.str();

    /* Run an LTS analysis script if the user asked for that. */
    if (co.script) {
	ifstream fin(co.script_file, ios::in);
	int ret;

	if (fin.fail()) {
	    cerr << co.script_file << ": no such script file\n";
	    exit(-1);
	}

	ret = Shell(*this, fin).run();
	fin.close();
	if (ret) {
	    return ret;
	}
    }

    /* Run the interactive shell if the user asked for that. */
    if (co.shell) {
	return Shell(*this, std::cin).run();
    }

    return 0;
}

void FspDriver::nesting_save()
{
    /* Save the current context and push it on the
       nesting stack. */
    nesting_stack.push_back(NestingContext());
    nesting_stack.back().ctx = ctx;
    nesting_stack.back().unres = unres;
    nesting_stack.back().parameters = parameters;
    nesting_stack.back().overridden_names = overridden_names;
    nesting_stack.back().overridden_values = overridden_values;

    /* Clean the current context. */
    ctx.clear();
    unres.clear();
    parameters.clear();
    overridden_names.clear();
    overridden_values.clear();
}

void FspDriver::nesting_restore()
{
    /* Remove the parameters identifiers that have been used
       during the last process translation. */
    for (unsigned int i=0; i<parameters.names.size(); i++) {
        identifiers.remove(parameters.names[i]);
    }
    /* Restore overridden identifiers. */
    for (unsigned int i=0; i<overridden_names.size(); i++) {
        if (!identifiers.insert(overridden_names[i],
                    overridden_values[i])) {
            assert(0);
            delete overridden_values[i];
        }
    }

    /* Pop the last saved context. */
    assert(nesting_stack.size());
    ctx = nesting_stack.back().ctx;
    unres = nesting_stack.back().unres;
    parameters = nesting_stack.back().parameters;
    overridden_names = nesting_stack.back().overridden_names;
    overridden_values = nesting_stack.back().overridden_values;
    nesting_stack.pop_back();
}

void FspDriver::error(const yy::location& l, const std::string& m)
{
    print_error_location_pretty(l);
    cerr << m << endl;
}

void FspDriver::error(const std::string& m)
{
    cerr << m << endl;
}

