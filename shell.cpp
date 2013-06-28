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
#include <cctype>	/* isprint() */
#include <unistd.h>	/* fork() */
#include <sys/wait.h>	/* waitpid() */
#include <ncurses.h>
#include "shell.hpp"
#include "lts.hpp"



#define DEBUG
#ifdef DEBUG
#define IFD(x)	(x)
#else
#define IFD(x)
#endif


void Shell::common_init()
{
    /* Initialize the help map. */

    help_map["ls"] = "ls: show a list of compiled FSPs";
    help_map["safety"] = "safety [FSP_NAME]: run deadlock/error analysis on \
the specified FSP or on every FSP";
    help_map["progress"] = "progress [FSP_NAME]: run progress analysis on \
the specified FSP or on every FSP";
    help_map["simulate"] = "simulate FSP_NAME: run an interactive simulation of \
the specified FSP";
    help_map["basic"] = "basic FSP_NAME FILE_NAME: write a basic process \
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

Shell::Shell(FspCompiler& cr, istream& inr) : c(cr), in(inr)
{
    common_init();
    interactive = true;

    /* Curses initialization. */
    initscr();
    cbreak(); //raw();
    noecho();
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);

    //scrollok(stdscr, TRUE); /* Use original terminal scrolling. */
}

Shell::Shell(FspCompiler& cr, ifstream& inr) : c(cr), in(inr)
{
    common_init();
    interactive = false;
}

Shell::~Shell()
{
    if (interactive)
	endwin();   /* Exit curses mode. */
}

static void scroll_screen(int n, int y, int x)
{
    int rows, cols;
    int r_i, c_i;
    chtype ch;

    getmaxyx(stdscr, rows, cols);
    if (n >= rows)
	return;

    /* Shift everything n rows up. */
    for (r_i=n; r_i<rows; r_i++) {
	for (c_i=0; c_i<cols; c_i++) {
	    ch = mvinch(r_i, c_i);
	    mvaddch(r_i-n, c_i, ch);
	}
    }

    /* Clear the last n rows. */
    move(rows-n, 0);
    clrtobot();

    /* Move the cursor to the required position. */
    move(y, x);
    refresh();
}

/* eol: if false a newline will not be printed together with the last
	line in 'ss'. */
void Shell::putsstream(stringstream& ss, bool eol) {

    if (interactive) {
	string line;
	int y, x;
	int rows, cols;
	bool first_line = true;

	getmaxyx(stdscr, rows, cols);

	/* Split the stringstream content in lines (using '\n' as
	   separator. */
	while (getline(ss, line)) {
	    int rows_required;

	    if (!first_line) {
		/* Newline attached to the line printed in the previous
		   iteration: We need to defer the newline insertion because
		   of the 'eol' parameter. */
		printw("\n");
		refresh();
	    }
	    first_line = false;

	    /* Compute the number of rows that are necessary to print the
	       line. */
	    rows_required = line.size() / cols;
	    if (line.size() % cols)
		rows_required++;

	    getyx(stdscr, y, x);
	    /* If there is not enough empty rows in the screen, we scroll as
	       many times as needed to make enough room, and move the
	       cursor at the beginning of the first empty row after
	       scrolling. */
	    if (y + rows_required >= rows)
		scroll_screen(y + rows_required - rows + 1,
				rows - rows_required-1, 0);
	    /* Finally print the line. */
	    printw("%s", line.c_str());
	}

	/* Put a newline after the last line in 'ss' only if asked by
	   the user. */
	if (eol)
	    printw("\n");

	refresh();
    }
    else {
	cout << ss.str();
    }
}

void Shell::getline_ncurses(string& line, const char * prompt)
{
    int ch;
    int y, x;
    int prompt_y, prompt_x;
    int frontier_y, frontier_x;
    int tmp_y, tmp_x;
    int rows, cols;
    int str_cursor = 0;

    getmaxyx(stdscr, rows, cols);
    if (prompt) {
	attron(COLOR_PAIR(2));
	printw(prompt);
	attroff(COLOR_PAIR(2));
	refresh();
    }
    getyx(stdscr, prompt_y, prompt_x);
    frontier_y = prompt_y;
    frontier_x = prompt_x;

    line = "";
    for (;;) {
	ch = getch();
	getyx(stdscr, y, x);

	switch (ch) {
	    case '\r':
	    case '\n':
		move(frontier_y, frontier_x);
		if (frontier_y == rows-1) {
		    scroll_screen(1, frontier_y-1,frontier_x);
		}
		printw("\n");
		refresh();
		return;

	    case KEY_UP:
	    case KEY_DOWN:
		/* Still not implemented. */
		break;

	    case KEY_LEFT:
		if (y > prompt_y || x > prompt_x) {
		    if (x) {
			x--;
		    } else {
			y--;
			x = cols-1;
		    }
		    move(y, x);
		    str_cursor--;
		}
		break;

	    case KEY_RIGHT:
		if (y < frontier_y || x < frontier_x) {
		    if (x == cols-1) {
			x = 0;
			y++;
		    } else {
			x++;
		    }
		    move(y, x);
		    str_cursor++;
		}
		break;

	    case KEY_HOME:
		move(prompt_y, prompt_x);
		str_cursor = 0;
		break;

	    case KEY_END:
		move(frontier_y, frontier_x);
		str_cursor = line.size();
		break;

	    case 127:	/* Backspace. */
		if (str_cursor) {
		    /* Compute the next cursor position. */
		    if (x) {
			x--;
		    } else {
			y--;
			x = cols-1;
		    }
		    str_cursor--;
		    /* Use the same implementation of KEY_DC. */
		} else break;

	    case KEY_DC:    /* Canc */
		if (line.size()) {
		    line.erase(str_cursor, 1);

		    /* Update the frontier. */
		    if (frontier_x) {
			frontier_x--;
		    } else {
			frontier_x = cols-1;
			frontier_y--;
		    }

		    /* Reflush the command string. */
		    move(prompt_y, prompt_x);
		    printw("%s", line.c_str());

		    /* Clear up to the end of the screen, in order to remove
		       old trailing character (they have been shifted). */
		    clrtobot();

		    /* Restore the cursor position. */
		    move(y, x);
		}
		break;

	    default:
		if (isprint(ch)) {
		    /* Insert a character in the command string at the
		       current cursor position. */
		    line.insert(str_cursor, 1, static_cast<char>(ch));

		    /* Compute the new cursor position. */
		    if (x == cols-1) {
			x = 0;
			if (y == rows-1) {
			    prompt_y--;
			    scroll_screen(1, rows - 1, 0);
			    frontier_y = rows - 2;
			} else {
			    y++;
			}
		    } else {
			x++;
		    }
		    str_cursor++;

		    /* Reflush the whole command string. */
		    move(prompt_y, prompt_x);
		    printw("%s", line.c_str());

		    /* Update the frontier. */
		    getyx(stdscr, tmp_y, tmp_x);
		    if (tmp_y > frontier_y) {
			frontier_y = tmp_y;
			frontier_x = 0;
		    }
		    if (tmp_y == frontier_y)
			frontier_x = max(frontier_x, tmp_x);

		    /* Restore the cursor position. */
		    move(y, x);
		} else {
		    IFD(printw("??%d??", ch));
		}
	}
	refresh();
    }

    assert(rows < 10000);
}

void Shell::readline(string& line)
{
    if (interactive) {
	getline_ncurses(line, NULL);
    } else {
	in >> line;
    }
}

void Shell::ls(const vector<string> &args, stringstream& ss)
{
    map<string, SymbolValue *>::iterator it;
    Lts * lts;

    ss << "Compiled FSPs:\n";
    for (it=c.processes.table.begin(); it!=c.processes.table.end(); it++) {
	lts = is_lts(it->second);
	ss << "   " << it->first << ": " << lts->numStates()
	    << " states, " << lts->numTransitions() << " transitions, "
	    << lts->alphabetSize() << " actions in alphabet\n";
    }
}

void Shell::safety(const vector<string> &args, stringstream& ss)
{
    map<string, SymbolValue *>::iterator it;
    SymbolValue * svp;
    Lts * lts;

    if (args.size()) {
	/* Deadlock analysis on args[0]. */
	if (!c.processes.lookup(args[0], svp)) {
	    ss << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    lts->deadlockAnalysis(ss);
	}
    } else {
	/* Deadlock analysis on every process. */
	for (it=c.processes.table.begin();
		    it!=c.processes.table.end(); it++) {
	    lts = is_lts(it->second);
	    lts->deadlockAnalysis(ss);
	}
    }
}

void Shell::progress(const vector<string> &args, stringstream& ss)
{
    map<string, SymbolValue *>::iterator it;
    map<string, SymbolValue *>::iterator jt;
    SymbolValue * svp;
    SetValue * setvp;
    Lts * lts;

    if (args.size()) {
	/* Progress analysis on args[0]. */
	if (!c.processes.lookup(args[0], svp)) {
	    ss << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    for (it=c.progresses.table.begin();
		    it!=c.progresses.table.end(); it++) {
		setvp = is_set(it->second);
		lts->progress(it->first, *setvp, ss);
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
		lts->progress(jt->first, *setvp, ss);
	    }
	}
    }
}

void Shell::simulate(const vector<string> &args, stringstream& ss)
{
    SymbolValue * svp;
    Lts * lts;

    if (args.size()) {
	if (!c.processes.lookup(args[0], svp)) {
	    ss << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    lts->simulate(*this);
	}
    } else {
	ss << "Invalid command: try 'help'\n";
    }
}

void Shell::basic(const vector<string> &args, stringstream& ss)
{
    string outfile;
    SymbolValue * svp;
    Lts * lts;

    if (!args.size()) {
	ss << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	ss << "Process " << args[0] << " not found\n";
	return;
    }

    if (args.size() >= 2) {
	outfile = args[1];
    } else {
	outfile = args[0] + ".bfsp";
    }

    lts = is_lts(svp);
    lts->basic(outfile, ss);
}

void Shell::alpha(const vector<string> &args, stringstream& ss)
{
    SymbolValue * svp;
    Lts * lts;

    if (!args.size()) {
	ss << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	ss << "Process " << args[0] << " not found\n";
	return;
    }

    lts = is_lts(svp);
    lts->printAlphabet(ss);
}

void Shell::see(const vector<string> &args, stringstream& ss)
{
    SymbolValue * svp;
    Lts * lts;

    if (!interactive) {
	// XXX do we really need this restriction ?
	ss << "Cannot use 'see' command in scripts\n";
	return;
    }

    if (!args.size()) {
	ss << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	ss << "Process " << args[0] << " not found\n";
	return;
    }

    lts = is_lts(svp);
    lts->graphvizOutput(".tmp.gv");

    /* UNIX-specific section. */
    pid_t drawer = fork();

    switch (drawer) {
	case -1:
	    ss << "fork() error\n";
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

void Shell::help(const vector<string> &args, stringstream& ss)
{
    map<string, const char *>::iterator it;

    if (args.size()) {
	it = help_map.find(args[0]);
	if (it == help_map.end()) {
	    ss << "	No command named like that\n";
	    return;
	}
	ss << "   " << it->second << "\n";
    } else {
	/* Show the help for every command. */
	for (it=help_map.begin(); it!=help_map.end(); it++) {
	    ss << "   " << it->second << "\n";
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
	stringstream ss;

	if (interactive)
	    getline_ncurses(line, "fspcc >> ");
	else
	    getline(in, line);

	if (in.eof()) {
	    return 0;
	}
	if (in.fail()) {
	    cerr << "Shell input error\n";
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
	    ss << "	Unrecognized command\n";
	} else {
	    ShellCmdFunc fp = it->second;

	    (this->*fp)(tokens, ss);
	}
	putsstream(ss, true);
    }
    
}

