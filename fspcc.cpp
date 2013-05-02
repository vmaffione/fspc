/*
 *  fspc - A Finite State Process compiler and LTS analyzer
 *
 *  Copyright (C) 2013  Vincenzo Maffione
 *  Email contact: <v.maffione@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <cstdlib>
#include <cstring>
#include "lts.hpp"

using namespace std;

#include "interface.hpp"


void help()
{
    cout << "fspc - A Finite State Process compiler and LTS analisys tool.\n";
    cout << "USAGE: fspc [-d] [-p] [-g] [-a] [-h] [-i FILE | -l FILE] [-o FILE]\n";
    cout << "	-i FILE : Specifies FILE as the input file containing FSP definitions.\n";
    cout << "	-l FILE : Specifies FILE as the input file containing compiled LTSs.\n";
    cout << "	-o FILE : Specifies the output FILE, e.g. the file that will contain the compiled LTSs.\n";
    cout << "	-d : Runs deadlock/error analysis on every FSP.\n";
    cout << "	-p : Runs all the specified progress verifications on every FSP.\n";
    cout << "	-g : Outputs a graphviz representation file of every FSP.\n";
    cout << "	-a : The same as '-d -p -g'\n";
    cout << "	-h : Shows this help.\n";
}


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
	    case 'h':
		help();
		exit(0);
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

#if 0
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

    ret = parser(co);

    return ret;
}
