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


/* ================================ Aliases ============================== */

void Aliases::insert(const string& left, const string& right) {
    int left_index = -1;
    bool left_was_assigned = false;
    int right_index = -1;
    int index;

    /* Firstly we look up both process names to discover what sets they
       are contained in (if any). */
    for (unsigned int i=0; i<groups.size(); i++)
	for (unsigned int j=0; j<groups[i].size(); j++) {
	    if (groups[i][j].name == left) {
		left_index = i;
		left_was_assigned = groups[i][j].assigned;
		groups[i][j].assigned = true;
	    }
	    if (groups[i][j].name == right)
		right_index = i;
	}

    if (left_index != -1 && left_was_assigned) {
	stringstream errstream;
	errstream << "Process " << left << " defined twice";
	semantic_error(tr.dr, errstream, tr.locations[0]);
    }

    if (left_index == -1 && right_index == -1) {
	/* Neither 'left' nor 'right' was found. We create another set
	   that will contain both of them. */
	vector<AliasElement> v;
	groups.push_back(v);
	index = groups.size() - 1;
    } else if (left_index == -1 && right_index != -1) {
	/* The alias 'left' will added to the same set of 'right'. */
	index = right_index;
    } else if (left_index != -1 && right_index == -1) {
	/* The alias 'right' will added to the same set of 'left'. */
	index = left_index;
    } else {
	/* Both names were found. If they aren't already in the same
	   set we merge the two sets. */
	if (left_index != right_index) {
	    /* Merge the two sets. */
	    for (unsigned int j=0; j<groups[right_index].size(); j++)
		groups[left_index].push_back(groups[right_index][j]);
	    groups.erase(groups.begin() + right_index);
	}
    }

    /* Do the real insertion work if necessary (see above). */
    if (left_index == -1)
	groups[index].push_back(AliasElement(left, true));
    if (right_index == -1)
	groups[index].push_back(AliasElement(right, false));
}

void Aliases::fill_process_table(SymbolsTable& pt)
{
    for (unsigned int i=0; i<groups.size(); i++) {
	int found = 0;
	unsigned int index = ~0U;
	SymbolValue * svp;
	ProcessValue * pvp;

	/* For each set, we have to find the unique process (if any) that
	   is already in 'pt', e.g. a process that has been defined as a
	   proper process (with a non-NULL ProcessNode*) and not just as
	   an alias. */
	for (unsigned int j=0; j<groups[i].size(); j++) {
	    if (pt.lookup(groups[i][j].name, svp)) {
		found++;
		index = j;
	    } else if (!groups[i][j].assigned) {
		/* We make sure that every non-properly-defined process
		   has been assigned somewhere as an alias.*/
		stringstream errstream;
		errstream << "Process " << groups[i][j].name << " undefined";
		semantic_error(tr.dr, errstream, tr.locations[0]);
	    }
	}
	assert(found <= 1);
	if (found == 0) {
	    cerr << "Warning, aliases cycle found: {";
	    for (unsigned int j=0; j<groups[i].size(); j++)
		cerr << groups[i][j].name << ", ";
	    cerr << "}\n";
	    cerr << "    A new STOP process will be associated to these aliases\n";
	    pvp = new ProcessValue;
	    pvp->pnp = new ProcessNode;
	    svp = pvp;
	    pt.insert(groups[i][0].name, svp);
	    index = 0;
	}
	pvp = err_if_not_process(tr.dr, svp, tr.locations[0]);
	for (unsigned int j=0; j<groups[i].size(); j++)
	    if (j != index) {
		ProcessValue * npvp = new ProcessValue;

		npvp->pnp = pvp->pnp;
		if (!pt.insert(groups[i][j].name, npvp)) {
		    /* This really can't happen. */
		    stringstream errstream;
		    errstream << "Impossible duplicate (BUG)\n";
		    semantic_error(tr.dr, errstream, tr.locations[0]);
		}
		IFD(cout << "Process " << groups[i][j].name << " defined (" << npvp->pnp << ")\n");
	    }
    }
}

void Aliases::clear()
{
    groups.clear();
}

void Aliases::print()
{
    IFD(cout << "Aliases:\n");
    for (unsigned int i=0; i<groups.size(); i++)
	for (unsigned int j=0; j<groups[i].size(); j++)
	    IFD(cout << i << ": " << groups[i][j].name << ", " << groups[i][j].assigned << "\n");
}


/*============================== FspTranslator ============================*/

void FspTranslator::init_fakenode() {
    vector<ProcessEdge> cv;
    struct ProcessEdge e;
    struct FrontierElement fe;
    vector<FrontierElement> frontier;

    /* The fakenode must have a child for each contexts. In this way the 
       actions associated to the rule 'prefix_actions-->action_labels' can
       correctly expand each context and return a ProcessNode* for each
       child. */
    fe.pnp = &fakenode;
    for (unsigned int c=0; c<current_contexts().size(); c++) {
	e.dest = NULL;
	e.rank = c;	/* The child will combine with the c-th context. */
	e.action = 0;	/* Not significant. */
	cv.push_back(e);
	/* Put the child in the frontier. */
	fe.child = c;
	frontier.push_back(fe);
    }
    fakenode.children = cv;
    current_contexts().frontier = frontier;
}

void FspTranslator::print_fakenode_forest() {
    cout << "Current ProcessNode fakenode forest:\n";
    for (unsigned int i=0; i<fakenode.children.size(); i++)
	if (fakenode.children[i].dest)
	    fakenode.children[i].dest->print(&dr.actions);
}


/* ============================== FspDriver ============================= */

FspDriver::FspDriver() : actions("Global actions table"), tr(*this)
{
    trace_scanning = trace_parsing = false;
    record_mode_on = 0;
    parametric = new ParametricProcess;
    tree = NULL;
    iterated_tree = true;
}

FspDriver::~FspDriver()
{
    this->clear();
}

void FspDriver::clear()
{
    if (parametric)
	delete parametric;

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
        ActionsTable tmp_at = actions; /* XXX remove when everthing works. */
        assert(tree);
        ofstream treef("tree.gv");
        tree->print(treef);
        treef.close();
        tree->translate(*this);
        delete tree;
        tree = NULL;
        actions = tmp_at;

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

void FspDriver::error(const yy::location& l, const std::string& m)
{
    cerr << l << ": " << m << endl;
}

void FspDriver::error(const std::string& m)
{
    cerr << m << endl;
}

