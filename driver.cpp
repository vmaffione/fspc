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
    int produceLts = (co.input_type != CompilerOptions::InputTypeLts);
    stringstream ss;

    if (co.input_type == CompilerOptions::InputTypeFsp) {
	string orig(co.input_file);
	string temp = ".fspcc." + orig;

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
	parser.parse();
	scan_end();

	/* Remove the temporary file. */
	remove(temp.c_str());
	this->remove_file = "";

        /* Output a GraphViz representation of the parse tree. */
        assert(tree);
        ofstream treef("tree.gv");
        tree->print(treef);
        treef.close();
        tree->translate(*this);
        delete tree;
        tree = NULL;

	serp = new Serializer(co.output_file);
    } else { /* Load the processes table from an LTS file. */
	uint32_t nlts, nprogr;

	desp = new Deserializer(co.input_file);

	desp->actions_table(actions, 0);
	desp->integer(nlts, 0);
	for (uint32_t i=0; i<nlts; i++) {
	    yy::Lts * lts = new yy::Lts(LtsNode::End, &actions);

	    desp->lts(*lts, 0);
	    /* Insert lts into the global 'processes' table. */
	    if (!processes.insert(lts->name, lts)) {
		assert(0);  //XXX Should be an invalid LTS file
	    }
	}

	desp->integer(nprogr, 0);
	for (uint32_t i=0; i<nprogr; i++) {
	    SetValue * setvp = new SetValue;
	    string name;

	    desp->stl_string(name, 0);
	    desp->set_value(*setvp, 0);
	    if (!progresses.insert(name, setvp)) {
		assert(0);
	    }
	}
    }

    /* Scan the 'processes' symbols table. For each process, output
       the associated LTS and do the deadlock analysis. */
    map<string, SymbolValue *>::iterator it;
    map<string, SymbolValue *>::iterator jt;
    yy::Lts * lts;
    SetValue * setvp;    

    if (produceLts) {
	serp->actions_table(actions, 0);
	serp->integer(processes.table.size(), 0);
    }
    for (it=processes.table.begin(); it!=processes.table.end(); it++) {
	lts = is_lts(it->second);

	/* We output an LTS file only if the input is not an LTS file. */
	if (produceLts) {
	    serp->lts(*lts, 0);
	}

	if (co.deadlock) {
	    lts->deadlockAnalysis(ss);
	}

	if (co.graphviz) {
	    lts->graphvizOutput((lts->name + ".gv").c_str());
	}
    }

    if (produceLts) {
	serp->integer(progresses.table.size(), 0);
    }
    /* Do each progress check against all the global processes. */
    for (it=progresses.table.begin(); it!=progresses.table.end();
	    it++) {
	setvp = is_set(it->second);
	if (co.progress) {
	    for (jt=processes.table.begin();
		    jt!=processes.table.end(); jt++) {
		lts = is_lts(jt->second);
		lts->progress(it->first, *setvp, ss);
	    }
	}

	/* Output the property if the input is not an LTS file. */
	if (produceLts) {
	    serp->stl_string(it->first, 0);
	    serp->set_value(*setvp, 0);
	}
    }

    if (serp)
	delete serp;
    if (desp)
	delete desp;

    /* Flush out program output. */
    cout << ss.str();

    /* Run a LTS analysis script if the user asked for that. */
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
    cerr << l << ": " << m << endl;
}

void FspDriver::error(const std::string& m)
{
    cerr << m << endl;
}

