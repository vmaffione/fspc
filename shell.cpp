#include <sstream>
#include "shell.hpp"
#include "lts.hpp"


void Shell::ls(const vector<string> &args)
{
    map<string, SymbolValue *>::iterator it;

    cout << "Compiled FSPs:\n";
    for (it=c.processes.table.begin(); it!=c.processes.table.end(); it++) {
	cout << "   " << it->first << "\n";
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

void Shell::help(const vector<string> &args)
{
    cout << "	ls: show a list of compiled FSPs\n";
    cout << "	safety [FSP_NAME]: run deadlock/error analysis on \
the specified FSP or on every FSP\n";
    cout << "	progress [FSP_NAME]: run progress analysis on \
the specified FSP or on every FSP\n";
    cout << "	simulate FSP_NAME: run an interactive simulation of \
the specified FSP\n";
    cout << "	help: show this help\n";
    cout << "	quit: exit the shell\n";
}


int Shell::run()
{
    for (;;) {
	string line;
	string token;
	vector<string> tokens;

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
	    cout << "\n";
	    continue;
	}

	token = tokens[0];
	tokens.erase(tokens.begin());
	if (token == "ls")
	    ls(tokens);
	else if (token == "safety")
	    safety(tokens);
	else if (token == "progress")
	    progress(tokens);
	else if (token == "simulate")
	    simulate(tokens);
	else if (token == "help")
	    help(tokens);
	else if (token == "quit" || token == "exit")
	    return 0;
	else {
	    cout << "unrecognized command\n";
	}
    }
    
}

