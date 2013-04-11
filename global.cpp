#include "global.hpp"


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

