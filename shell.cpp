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
#include <ncurses.h>

#include "shell.hpp"

/* FspDriver class to access the compiler structure. */
#include "driver.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

/* Helper functions. */
#include "helpers.hpp"

#define DEBUG
#ifdef DEBUG
#define IFD(x)	(x)
#else
#define IFD(x)
#endif


#define HISTORY_MAX_COMMANDS    20

/* =========================== Shell implementation ==================== */
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
    help_map["print"] = "print FSP_NAME {png | pdf}: print the GraphViz "
                        "representation of the specified fsp into a file "
                        "FSP_NAME.FORMAT";
    help_map["lsprop"] = "lsprop: show a list of compiled properties";
    help_map["lsmenu"] = "lsmenu: show a list of available menus";
    help_map["help"] = "help: show this help";
    help_map["quit"] = "quit: exit the shell";

    cmd_map["ls"] = &Shell::ls;
    cmd_map["safety"] = &Shell::safety;
    cmd_map["progress"] = &Shell::progress;
    cmd_map["simulate"] = &Shell::simulate;
    cmd_map["basic"] = &Shell::basic;
    cmd_map["alpha"] = &Shell::alpha;
    cmd_map["see"] = &Shell::see;
    cmd_map["print"] = &Shell::print;
    cmd_map["lsprop"] = &Shell::lsprop;
    cmd_map["lsmenu"] = &Shell::lsmenu;
    cmd_map["help"] = &Shell::help;
}

/* This function must be called after common_init(). */
void Shell::fill_completion()
{
    map<string, SymbolValue *>::iterator it;
    map<string, const char *>::iterator jt;

    /* Process names. */
    for (it=c.processes.table.begin(); it!=c.processes.table.end(); it++) {
	completion.insert(it->first);
    }

    /* fspc shell command names. */
    for (jt = help_map.begin(); jt != help_map.end(); jt++) {
        completion.insert(jt->first);
    }
}

Shell::Shell(FspDriver& cr, istream& inr) : c(cr), in(inr)
{
    common_init();
    interactive = true;
    history_enabled = true;

    /* Autocompletion initialization. */
    fill_completion();

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

Shell::Shell(FspDriver& cr, ifstream& inr) : c(cr), in(inr)
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

/* The more portable way I found out to implement 'isprint'. */
static bool is_printable(int ch)
{
    return ch>=32 && ch<=126;
}

static void right_split(string& base, string& back)
{
    size_t p;

    back.clear();

    p = base.find_last_of(' ');
    if (p == string::npos) {
        base.swap(back);
        return;
    }

    back = base.substr(p + 1);
    base = base.substr(0, p + 1);
}

void Shell::getline_ncurses(string& line, const char * prompt)
{
    int ch;

    /* Position of the cursor. */
    int y, x;

    /* Position of the prompt. */
    int prompt_y, prompt_x;

    /* Position of the last char of the command string. */
    int frontier_y, frontier_x;

    /* Command string index of the cursor. */
    int str_cursor = 0;

    /* Number of rows and columns in the screen. */
    int rows, cols;

    int tmp_y, tmp_x;
    string arg;

    getmaxyx(stdscr, rows, cols);

    /* Print the prompty (if any), and initialize the prompt position
       properly. */
    if (prompt) {
	attron(COLOR_PAIR(2));
	printw(prompt);
	attroff(COLOR_PAIR(2));
	refresh();
    }
    getyx(stdscr, prompt_y, prompt_x);

    /* Initially the frontier is the same as the prompt. */
    frontier_y = prompt_y;
    frontier_x = prompt_x;

    /* Always start with an empty string. */
    line = string();

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
                if (history_enabled) {
                    /* Update the history. */
                    history.add_command(line);
                }
		return;

	    case KEY_UP:
	    case KEY_DOWN:
                if (!history_enabled) {
                    break;
                }
                if (ch == KEY_UP) {
                    history.up();
                } else /* (ch == KEY_DOWN) */ {
                    history.down();
                }
                history.get_current(line);
                move(prompt_y, prompt_x);
                printw("%s", line.c_str());
		getyx(stdscr, frontier_y, frontier_x);
                move(frontier_y, frontier_x);
                str_cursor = line.size();
		clrtobot();
		break;

            case '\t':
                /* Split the current 'line' into 'line' + 'arg', so that
                   'arg' contains the last command word. */
                right_split(line, arg);
                /* Try to complete 'arg'. */
                if (completion.lookup(arg)) {
                    /* The string 'arg' has been extended because of
                       auto-completion. Reflush the command string. */
                    line += arg;
                    move(prompt_y, prompt_x);
                    printw("%s", line.c_str());
                    getyx(stdscr, frontier_y, frontier_x);
                    move(frontier_y, frontier_x);
                    str_cursor = line.size();
                    clrtobot();
                } else {
                    /* The string 'arg' was not modified: Let's undo the
                       splitting. */
                    line += arg;
                }
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
	    case KEY_BACKSPACE:
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
		if (is_printable(ch)) {
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
        getline(in, line);
    }
}

void Shell::ls(const vector<string> &args, stringstream& ss)
{
    map<string, SymbolValue *>::iterator it;
    yy::Lts * lts;

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
    yy::Lts * lts;

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
    SymbolValue *svp;
    ActionSetValue *asv;
    yy::Lts *lts;

    if (args.size()) {
	/* Progress analysis on args[0]. */
	if (!c.processes.lookup(args[0], svp)) {
	    ss << "Process " << args[0] << " not found\n";
	} else {
	    lts = is_lts(svp);
	    for (it=c.progresses.table.begin();
		    it!=c.progresses.table.end(); it++) {
		asv = is_actionset(it->second);
		lts->progress(it->first, *asv, ss);
	    }
	}
    } else {
	/* Progress analysis on every process. */
	for (it=c.processes.table.begin(); 
		    it!=c.processes.table.end(); it++) {
	    lts = is_lts(it->second);
	    for (jt=c.progresses.table.begin();
		    jt!=c.progresses.table.end(); jt++) {
		asv = is_actionset(jt->second);
		lts->progress(jt->first, *asv, ss);
	    }
	}
    }
}

void Shell::simulate(const vector<string> &args, stringstream& ss)
{
    SymbolValue * svp;
    yy::Lts * lts;
    ActionSetValue *menu = NULL;

    if (!args.size()) {
	ss << "Invalid command: try 'help'\n";
        return;
    }

    if (!c.processes.lookup(args[0], svp)) {
        ss << "Process " << args[0] << " not found\n";
        return;
    }
    lts = is_lts(svp);

    if (args.size() >= 2) {
        if (!c.menus.lookup(args[1], svp)) {
            ss << "Menu " << args[1] << " not found\n";
            return;
        }
        menu = is_actionset(svp);
    }

    history_enable(false);
    lts->simulate(*this, menu);
    history_enable(true);
}

void Shell::basic(const vector<string> &args, stringstream& ss)
{
    string outfile;
    SymbolValue * svp;
    yy::Lts * lts;

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
    yy::Lts * lts;

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
    yy::Lts * lts;
    string tmp_name;
    pid_t drawer;

    if (!interactive) {
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

    /* Generate the graphivz output into a temporary file (whose name does
       not collide with other fspc instances). */
    tmp_name = "." + int2string(getpid()) + ".gv";
    lts->graphvizOutput(tmp_name.c_str());

    /* UNIX-specific section. */
    drawer = fork();

    switch (drawer) {
	case -1:
	    ss << "fork() error\n";
	    return;
	    break;
	case 0:
	    execlp("ltsee", "ltsee", tmp_name.c_str(), NULL);
	    exit(0);
	    break;
	default:
	    int status;

	    waitpid(-1, &status, 0);
    }

    remove(tmp_name.c_str());
}

void Shell::print(const vector<string> &args, stringstream& ss)
{
    SymbolValue * svp;
    yy::Lts * lts;
    string format = "png";
    string filename;

    if (!args.size()) {
	ss << "Invalid command: try 'help'\n";
	return;
    }

    if (!c.processes.lookup(args[0], svp)) {
	ss << "Process " << args[0] << " not found\n";
	return;
    }
    filename = args[0] + ".gv";

    lts = is_lts(svp);
    lts->graphvizOutput(filename.c_str());

    if (args.size() > 1) {
        if (args[1] != "png" && args[1] != "pdf") {
            ss << "Format '" << args[1] << "' unknown\n";
            return;
        }
        format = args[1];
    }

    /* UNIX-specific section. */
    pid_t drawer = fork();

    switch (drawer) {
	case -1:
	    ss << "fork() error\n";
	    return;
	    break;
	case 0:
	    execl("ltsimg", "ltsimg", filename.c_str(), format.c_str(), NULL);
	    exit(1);
	    break;
	default:
	    int status;

	    waitpid(-1, &status, 0);
    }

    remove(filename.c_str());
}

void Shell::lsprop(const vector<string> &args, stringstream& ss)
{
    map<string, SymbolValue *>::iterator it;
    ActionSetValue *as;

    ss << "Progresses:\n";
    for (it=c.progresses.table.begin(); it!=c.progresses.table.end(); it++) {
        SetValue setv;

        as = is_actionset(it->second);
        as->toSetValue(c.actions, setv);
	ss << "   " << it->first << ": ";
        setv.output(ss);
        ss << "\n";
    }
}

void Shell::lsmenu(const vector<string> &args, stringstream& ss)
{
    map<string, SymbolValue *>::iterator it;
    ActionSetValue *as;

    ss << "Menus:\n";
    for (it=c.menus.table.begin(); it!=c.menus.table.end(); it++) {
        SetValue setv;

        as = is_actionset(it->second);
        as->toSetValue(c.actions, setv);
	ss << "   " << it->first << ": ";
        setv.output(ss);
        ss << "\n";
    }
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

void Shell::history_enable(bool enable)
{
    history_enabled = enable;
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


/* ====================== CommandHistory implementation ================= */
CommandHistory::CommandHistory() {
    /* The history is initialized as containing an empty command string. */
    commands.push_back(string());
    cur = 0;
}

/* Navigate the history backward. */
void CommandHistory::up()
{
    if (cur) {
        cur--;
    }
}

/* Navigate the history forward. */
void CommandHistory::down()
{
    if (cur < commands.size()) {
        cur++;
    }
}

/* Return the current command in the history. */
void CommandHistory::get_current(string& s)
{
    if (cur < commands.size()) {
        s = commands[cur];
    } else /* if (cur == commands.size()) */ {
        s = string();
    }
}

/* Insert a new command into the history. This has always the effect
   of resetting the "current" to the most recent command. The new command
   is actually added to the story only if non-empty and if not equal to
   the most recent command. */
void CommandHistory::add_command(const string& s)
{
    if (s.size() && s != commands.back()) {
        commands.push_back(s);
        if (commands.size() > HISTORY_MAX_COMMANDS) {
            commands.erase(commands.begin());
        }
    } else {
        /* Merge to last entry, if matches. */
    }
    /* Reset the current. */
    cur = commands.size();
}


/* =================== AutoCompletion implementation ===================== */
AutoCompletion::AutoCompletion()
{
    /* The trie data structure initially contains only one node with a NULL
       character. This represents the empty string. */
    trie.push_back(TrieElem('\0', false));
}

/* Insert a string into the trie. */
void AutoCompletion::insert(const string& s)
{
    unsigned int ti = 0;

    /* Scan the input string and the trie in parallel. New nodes are
       inserted into the trie when necessary. */
    for (unsigned int si = 0; si < s.size(); si++) {
        const TrieElem& elem = trie[ti];
        unsigned int j = 0;
        unsigned int n = elem.next.size();

        /* Match the current input character 's[si]' with a trie node
           in the 'next' of the current trie node. */
        for (; j < n; j++) {
            if (trie[ elem.next[j] ].ch == s[si]) {
                /* A match is found: Keep navigating the trie. */
                ti = elem.next[j];
                break;
            }
        }
        if (j == n) {
            /* No match has been found. Create a new trie node containing
               's[si]' and add it to the next of the current trie node. */
            trie.push_back(TrieElem(s[si], si == s.size() - 1));
            trie[ti].next.push_back(trie.size() - 1);
            ti = trie.size() - 1;
        }
    }
}

/* Find the subset of the strings in the trie that 's' is prefix of. If
   the subset is not empty, modify 's' so that is equal to the longest
   common prefix of all the strings in the subset.

   Returns true if 's' has been modified.
*/
bool AutoCompletion::lookup(string& s) const
{
    unsigned int ti = 0;
    bool modified = false;

    /* Scanning similar to the insert() method. */
    for (unsigned int si = 0; si < s.size(); si++) {
        const TrieElem& elem = trie[ti];
        unsigned int j = 0;
        unsigned int n = elem.next.size();

        for (; j < n; j++) {
            if (trie[ elem.next[j] ].ch == s[si]) {
                ti = elem.next[j];
                break;
            }
        }
        if (j == n) {
            /* The input string is not a prefix of any of the strings
               contained into the trie. */
            return modified;
        }
    }

    /* Keep scanning (and updating the input string) the trie as long as
       the sub-trie does not have branches and we don't run into an EOW. */
    for (;;) {
        const TrieElem& elem = trie[ti];

        if (elem.next.size() != 1 || elem.eow) {
            /* The trie is finished or there is a branch: Stop here. */
            break;
        }
        ti = elem.next[0];
        s.push_back(trie[ti].ch);
        modified = true;
    }

    return modified;
}

/* Depth first search visit. Since the trie is a tree structure, it's
   not necessary to keep track of visited nodes or worry about cycles. */
void AutoCompletion::print(unsigned int idx, unsigned int level)
{
    for (unsigned int j = 0; j < level; j++) {
        cout << "  ";
    }
    cout << trie[idx].ch << "\n";

    for (unsigned int j = 0; j < trie[idx].next.size(); j++) {
        this->print(trie[idx].next[j], level + 1);
    }
}

TrieElem::TrieElem(char c, bool end_of_word)
{
    ch = c;
    eow = end_of_word;
}

