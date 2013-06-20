/*
 *  fspc shell support
 *
 *  Copyright (C) 2013  Vincenzo Maffione
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


#include <sstream>
#include <cstdio>
#include <unistd.h>	/* fork() */
#include <sys/wait.h>	/* waitpid() */
#include "shell.hpp"
#include "lts.hpp"


Shell::Shell(FspCompiler& cr, istream& inr) : c(cr), in(inr)
{
    /* Initialize the help map. */
    help_map["ls"] = "ls: show a list of compiled FSPs";
    help_map["safety"] = "safety [FSP_NAME]: run deadlock/error analysis on \
the specified FSP or on every FSP";
    help_map["progress"] = "progress [FSP_NAME]: run progress analysis on \
the specified FSP or on every FSP";
    help_map["simulate"] = "simulate FSP_NAME: run an interactive simulation of \
the specified FSP";
    help_map["basic"] = "basic FSP_NAME FILE_NAME: writes a basic process \
description of the specified FSP into the specified output file";
    help_map["alpha"] = "alpha FSP_NAME: show the alphabet of the specified \
FSP";
    help_map["see"] = "see FSP_NAME: show a graphical representation of the \
specified FSP using GraphViz";
    help_map["help"] = "help: show this help";
    help_map["quit"] = "quit: exit the shell";

    cmd_map["ls"] = &Shell::ls;
    cmd_map["safety"] = &Shell::safety;
    cmd_map["progress"] = &Shell::progress;
    cmd_map["simulate"] = &Shell::simulate;
    cmd_map["basic"] = &Shell::basic;
    cmd_map["alpha"] = &Shell::alpha;
    cmd_map["see"] = &Shell::see;
    cmd_map["help"] = &Shell::help;
}

void Shell::ls(const vector<string> &args)
{
    map<string, SymbolValue *>::iterator it;
    Lts * lts;

    cout << "Compiled FSPs:\n";
    for (it=c.processes.table.begin(); it!=c.processes.table.end(); it++) {
	lts = is_lts(it->second);
	cout << "   " << it->first << ": " << lts->numStates()
	    << " states, " << lts->numTransitions() << " transitions, "
	    << lts->alphabetSize() << " actions in alphabet\n";
    }
}

void Shell::safety(const vector<string> &args)
{
    map<string, SymbolValue *>::iterator it;
    SymbolValue * svp;
    Lts * lts;

    if (args.size()) {
	/* Deadlock analysis on args[0]. */
	if (!c.processes.lookup(args[0], svp)) {
	    cout << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    lts->deadlockAnalysis();
	}
    } else {
	/* Deadlock analysis on every process. */
	for (it=c.processes.table.begin();
		    it!=c.processes.table.end(); it++) {
	    lts = is_lts(it->second);
	    lts->deadlockAnalysis();
	}
    }
}

void Shell::progress(const vector<string> &args)
{
    map<string, SymbolValue *>::iterator it;
    map<string, SymbolValue *>::iterator jt;
    SymbolValue * svp;
    SetValue * setvp;
    Lts * lts;

    if (args.size()) {
	/* Progress analysis on args[0]. */
	if (!c.processes.lookup(args[0], svp)) {
	    cout << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    for (it=c.progresses.table.begin();
		    it!=c.progresses.table.end(); it++) {
		setvp = is_set(it->second);
		lts->progress(it->first, *setvp);
	    }
	}
    } else {
	/* Progress analysis on every process. */
	for (it=c.processes.table.begin(); 
		    it!=c.processes.table.end(); it++) {
	    lts = is_lts(it->second);
	    for (jt=c.progresses.table.begin();
		    jt!=c.progresses.table.end(); jt++) {
		setvp = is_set(jt->second);
		lts->progress(jt->first, *setvp);
	    }
	}
    }
}

void Shell::simulate(const vector<string> &args)
{
    SymbolValue * svp;
    Lts * lts;

    if (args.size()) {
	if (!c.processes.lookup(args[0], svp)) {
	    cout << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    lts->simulate();
	}
    } else {
	cout << "Invalid command: try 'help'\n";
    }
}

void Shell::basic(const vector<string> &args)
{
    string outfile;
    SymbolValue * svp;
    Lts * lts;

    if (!args.size()) {
	cout << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	cout << "Process " << args[0] << " not found\n";
	return;
    }

    if (args.size() >= 2) {
	outfile = args[1];
    } else {
	outfile = args[0] + ".bfsp";
    }

    lts = is_lts(svp);
    lts->basic(outfile);
}

void Shell::alpha(const vector<string> &args)
{
    SymbolValue * svp;
    Lts * lts;

    if (!args.size()) {
	cout << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	cout << "Process " << args[0] << " not found\n";
	return;
    }

    lts = is_lts(svp);
    lts->printAlphabet();
}

void Shell::see(const vector<string> &args)
{
    SymbolValue * svp;
    Lts * lts;

    if (!args.size()) {
	cout << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	cout << "Process " << args[0] << " not found\n";
	return;
    }

    lts = is_lts(svp);
    lts->graphvizOutput(".tmp.gv");

    /* XXX UNIX-specific section. */
    pid_t drawer = fork();

    switch (drawer) {
	case -1:
	    cout << "fork() error\n";
	    return;
	    break;
	case 0:
	    execl("ltsee", "ltsee", ".tmp.gv", NULL);
	    exit(1);
	    break;
	default:
	    int status;

	    waitpid(-1, &status, 0);
    }

    remove(".tmp.gv");
}

void Shell::help(const vector<string> &args)
{
    map<string, const char *>::iterator it;

    if (args.size()) {
	it = help_map.find(args[0]);
	if (it == help_map.end()) {
	    cout << "	No command named like that\n";
	    return;
	}
	cout << "   " << it->second << "\n";
    } else {
	/* Show the help for every command. */
	for (it=help_map.begin(); it!=help_map.end(); it++) {
	    cout << "   " << it->second << "\n";
	}
    }
}

int Shell::run()
{
    for (;;) {
	string line;
	string token;
	vector<string> tokens;
	map<string, ShellCmdFunc>::iterator it;

	cout << "fspcc >> ";
	cout.flush();
	getline(cin, line);
	if (cin.fail()) {
	    cout << "Shell input error\n";
	    return -1;
	}

	istringstream iss(line);

	while (iss >> token) {
	    tokens.push_back(token);
	}

	if (!tokens.size()) {
	    continue;
	}

	token = tokens[0];
	tokens.erase(tokens.begin());

	if (token == "quit" || token == "exit")
	    return 0;
	it = cmd_map.find(token);
	if (it == cmd_map.end()) {
	    cout << "	Unrecognized command\n";
	} else {
	    ShellCmdFunc fp = it->second;

	    (this->*fp)(tokens);
	}
    }
    
}

