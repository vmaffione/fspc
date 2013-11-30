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
    replay = false;
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
	this->current_file = orig; /* TODO redundant (remove_file)*/
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
        /* Translate the parse tree. */
        tree->translate(*this);
        delete tree;
        tree = NULL;

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
	    if (!processes.insert(lts->name, lts)) {
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

void FspDriver::nesting_save(bool r)
{
    /* Save the current context and push it on the
       nesting stack. */
    nesting_stack.push_back(NestingContext());
    nesting_stack.back().ctx = ctx;
    nesting_stack.back().unres = unres;
    nesting_stack.back().paramproc = paramproc;
    nesting_stack.back().overridden_names = overridden_names;
    nesting_stack.back().overridden_values = overridden_values;
    nesting_stack.back().replay = replay;

    /* Clean the current context. */
    ctx.clear();
    unres.clear();
    paramproc.clear();
    overridden_names.clear();
    overridden_values.clear();
    replay = r;
}

void FspDriver::nesting_restore()
{
    /* Remove the parameters identifiers. */
    for (unsigned int i=0; i<paramproc.names.size(); i++) {
        identifiers.remove(paramproc.names[i]);
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
    paramproc = nesting_stack.back().paramproc;
    overridden_names = nesting_stack.back().overridden_names;
    overridden_values = nesting_stack.back().overridden_values;
    replay = nesting_stack.back().replay;
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

