#include <iostream>
#include <cstdlib>
#include <cstring>
#include "lts.hpp"

using namespace std;

extern int parser();

struct CompilerOptions {
    const char * input_file;
    int input_type;
    const char * output_file;
    bool deadlock;
    bool progress;
    bool graphviz;

    static const int InputTypeFsp = 0;
    static const int InputTypeLts = 1;
};


void process_args(CompilerOptions& co, int argc, char **argv)
{
    int i = 1;
    int il_options = 0;

    /* Set default values. */
    co.input_file = "input.fsp";
    co.input_type = CompilerOptions::InputTypeFsp;
    co.output_file = "output.lts";
    co.deadlock = false;
    co.progress = false;
    co.graphviz = false;

    /* Scan input arguments. */
    while (i < argc) {
	int len = strlen(argv[i]);
	const char * arg = argv[i];
	if (len != 2 || len == 2 && arg[0] != '-') {
	    cerr << "Error: Unrecognized option\n";
	    exit(-1);
	}
	switch (arg[1]) {
	    case 'a':
		co.deadlock = co.progress = co.graphviz = true;
	    case 'd':
		co.deadlock = true;
		break;
	    case 'p':
		co.progress = true;
		break;
	    case 'g':
		co.graphviz = true;
		break;
	    case 'i':
	    case 'l':
	    case 'o':
		i++;
		if (i == argc) {
		    cerr << "Error: Expected filename after -"<< arg[1] 
			    << " option\n";
		    exit(-1);
		}
		switch (arg[1]) {
		    case 'i':
			il_options++;
			co.input_file = argv[i];
			break;
		    case 'l':
			il_options++;
			co.input_file = argv[i];
			co.input_type = CompilerOptions::InputTypeLts;
			break;
		    case 'o':
			co.output_file = argv[i];
			break;
		}
		break;

	    default:
		cerr << "Unrecognized option " << arg[1] << "\n";
		exit(-1);
	}
	i++;
    }

    if (il_options > 1) {
	cerr << "Error: Cannot specify both 'i' and 'l' options\n";
	exit(-1);
    }

#if 1
    cout << "PROGRAM OPTIONS\n";
    cout << "	input file:	    " << co.input_file << endl;
    cout << "	input type:	    " << ((co.input_type == CompilerOptions::InputTypeFsp) ? "fsp": "lts") << endl;
    cout << "	output file:	    " << co.output_file << endl;
    cout << "	deadlock check:	    " << co.deadlock << endl;
    cout << "	progress check:	    " << co.progress << endl;
    cout << "	graphviz output:    " << co.graphviz << endl;
    cout << "\n";
#endif
}

int main (int argc, char ** argv)
{
    int ret;
    CompilerOptions co;
    
    process_args(co, argc, argv);

    ret = parser();

    return ret;
}
