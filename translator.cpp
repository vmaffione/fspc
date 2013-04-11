#include "translator.hpp"


void Aliases::insert(const string& left, const string& right) {
    int left_index = -1;
    bool left_was_assigned = false;
    int right_index = -1;
    int index;

    /* Firstly we look up both process names to discover what sets they
       are contained in (if any). */
    for (int i=0; i<groups.size(); i++)
	for (int j=0; j<groups[i].size(); j++) {
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
	errstream << "$$Process " << left << " defined twice";
	semantic_error(errstream);
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
	    for (int j=0; j<groups[right_index].size(); j++)
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
    for (int i=0; i<groups.size(); i++) {
	int found = 0;
	int index = -1;
	SymbolValue * svp;
	ProcessValue * pvp;

	/* For each set, we have to find the unique process (if any) that
	   is already in 'pt', e.g. a process that has been defined as a
	   proper process (with a non-NULL ProcessNode*) and not just as
	   an alias. */
	for (int j=0; j<groups[i].size(); j++) {
	    if (pt.lookup(groups[i][j].name, svp)) {
		found++;
		index = j;
	    } else if (!groups[i][j].assigned) {
		/* We make sure that every non-properly-defined process
		   has been assigned somewhere as an alias.*/
		stringstream errstream;
		errstream << "$* Process " << groups[i][j].name
		    << " undefined";
		semantic_error(errstream);
	    }
	}
	assert(found <= 1);
	if (found == 0) {
	    cerr << "Warning, aliases cycle found: {";
	    for (int j=0; j<groups[i].size(); j++)
		cerr << groups[i][j].name << ", ";
	    cerr << "}\n";
	    cerr << "    A new STOP process will be associated to these aliases\n";
	    pvp = new ProcessValue;
	    pvp->pnp = new ProcessNode;
	    svp = pvp;
	    pt.insert(groups[i][0].name, svp);
	    index = 0;
	}
	pvp = err_if_not_process(svp);
	for (int j=0; j<groups[i].size(); j++)
	    if (j != index) {
		ProcessValue * npvp = new ProcessValue;

		npvp->pnp = pvp->pnp;
		if (!pt.insert(groups[i][j].name, npvp)) {
		    /* This really can't happen. */
		    stringstream errstream;
		    errstream << "Impossible duplicate (BUG)\n";
		    semantic_error(errstream);
		}
		cout << "Process " << groups[i][j].name << " defined (" << npvp->pnp << ")\n";
	    }
    }
}

void Aliases::clear()
{
    groups.clear();
}

void Aliases::print()
{
    cout << "Aliases:\n";
    for (int i=0; i<groups.size(); i++)
	for (int j=0; j<groups[i].size(); j++)
	    cout << i << ": " << groups[i][j].name << ", "
		<< groups[i][j].assigned << "\n";
}

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
    for (int c=0; c<current_contexts().size(); c++) {
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
    for (int i=0; i<fakenode.children.size(); i++)
	if (fakenode.children[i].dest)
	    fakenode.children[i].dest->print(&actions);
}

