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


#ifndef __SHELL_HH
#define __SHELL_HH

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>

using namespace std;


/* An helper class which implements the command history for the
   Shell class. */
class CommandHistory {
        vector<string> commands;
        unsigned int cur;

    public:
        CommandHistory();
        void up();
        void down();
        void get_current(string& s);
        void add_command(const string& s);
};


/* An element of a trie data structure: It contains a character and
   an array of pointers (indexes) to other trie elements. */
struct TrieElem {
    char ch;
    bool eow;   /* End Of Word: This trie element ends a word. */
    vector<unsigned int> next;

    TrieElem(char c, bool end_of_word);
};


/* An helper class which implements the auto-completion feature for
   the Shell class. It uses a trie data structure to store the set of
   strings that can be completed. */
class AutoCompletion {
        vector<TrieElem> trie;

    public:
        AutoCompletion();
        void insert(const string& s);
        bool lookup(string& s) const;
        void print(unsigned int idx, unsigned int level);
};


class FspDriver;

class Shell {
        FspDriver& c;

        /* A mapping of command names to the help strings. */
        map<string, const char*> help_map;

        typedef int (Shell::*ShellCmdFunc)(const vector<string>& args, stringstream& ss);

        /* A mapping of command names to command callbacks. */
        map<string, ShellCmdFunc> cmd_map;

        /* The input stream the shell reads from. */
        istream& in;

        /* Whethter the input stream 'in' is interactive or not. */
        bool interactive;

        /* Command history. */
        CommandHistory history;
        bool history_enabled;

        /* Auto-completion. */
        AutoCompletion completion;

        /* Keep trace of the last 'c.processes.size()' seen. */
        unsigned int trace_processes_size;

        void common_init();
        void fill_completion();
        void getline_ncurses(string& line, const char *prompt);

        int ls(const vector<string>& args, stringstream& ss);
        int safety(const vector<string>& args, stringstream& ss);
        int progress(const vector<string>& args, stringstream& ss);
        int simulate(const vector<string>& args, stringstream& ss);
        int basic(const vector<string>& args, stringstream& ss);
        int alpha(const vector<string>& args, stringstream& ss);
        int see(const vector<string>& args, stringstream& ss);
        int print(const vector<string>& args, stringstream& ss);
        int lsprop(const vector<string>& args, stringstream& ss);
        int lsmenu(const vector<string>& args, stringstream& ss);
        int minimize(const vector<string>& args, stringstream& ss);
        int traces(const vector<string>& args, stringstream& ss);
        int help(const vector<string>& args, stringstream& ss);

    public:
        Shell(FspDriver& cr, istream& inr);
        Shell(FspDriver& cr, ifstream& inr);

        int run();
        void readline(string& line);
        void putsstream(stringstream& ss, bool eol);
        void history_enable(bool enable);

        ~Shell();
};


#endif

