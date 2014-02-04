/*
 *  fspc shell support
 *
 *  Copyright (C) 2013-2014  Vincenzo Maffione
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
#include <stack>
#include "code_generator.hpp"//cosimo

using namespace std;

#include "symbols_table.hpp"


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


/* Using the if/elif/else/fi constructs, a script can nest conditional
   command sequences to create an arbitrarily complex tree of if branches.

   In this example:

        B0  // a block of commands
        if COND
            B1
            if COND
                B2
            elif COND
                B3
            else
                B4
            fi
        else
            B5
            if COND
                B6
                if COND
                    B7
                    if COND
                        B8
                    fi
                elif
                    B9
                    if COND
                        B10
                    else
                        B11
                    fi
                fi
            fi
        fi

    the tree is

        B0 --- B1 --- B2
            |      |
            |      -- B3
            |      |
            |      -- B4
            |
            -- B5 --- B6 --- B7 --- B8
                          |
                          -- B9 --- B10
                                 |
                                 -- B11

    The IfFrame structure keeps track of a single level of nesting: To be
    more precise, it keeps track of a group of siblings. In the example,
    the groups of siblings are {B1, B5}, {B2, B3, B4}, {B6}, {B7, B9},
    {B8} and {B10, B11}. Note that B8 and B10 are not siblings, even if
    they are on the same tree level (the same holds for B6 w.r.t. B2,
    B3 and B4).

    Let's say we have a group of siblings {b1, b2, ... bn}, in the specified
    order.
    Each block of commands comes with a explicit condition (if/elif), or an
    implicit true condition (else) in the case of the last block. Only the
    first block associated to a true condition will be executed.

    The IfFrame instance associated to the group is initialized when
    evaluating the 'if' statement associated to 'b1' and is updated by the
    'elif' and 'else' statements associated to the other siblings.

    The 'accepted' field is true when we have already met a true condition.
    As an example, if 'c3' (condition associated to 'b3') is true, while
    'c1' and 'c2' are not, 'accepted' would become true during the
    evaluation of 'elif c3' and remain true as long as the IfFrame instance
    lives.
    The 'accepting' field is true while we are scanning/executing the commands
    of the block selected for execution ('b3' in the example).
    The 'else_met' field becomes true when an 'else' statement is met,
    and is useful to catch semantic errors.
 */
struct IfFrame {
    bool accepting;
    bool accepted;
    bool else_met;

    IfFrame(bool accing, bool acced, bool emet) :
                accepting(accing), accepted(acced), else_met(emet) {
    }
};


class FspDriver;

namespace codegen {
    class CodeGenerator;
}

class Shell {
        FspDriver& c;
        codegen::CodeGenerator coder;

        /* A mapping of command names to the help strings. */
        map<string, const char*> help_map;

        typedef int (Shell::*ShellCmdFunc)(const vector<string>& args,
                                            stringstream& ss);

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

        /* Shell variables (values are integers). */
        map<string, int> variables;

        /* Support for bash-like conditional statements (if/elif/else/fi).
           The evolution of the stack follows the ramification of the
           shell script. */
        stack<IfFrame> ifframes;

        /* Shell return value, set by the "exit" command. */
        int return_value;

        void common_init();
        void fill_completion();
        void getline_ncurses(string& line, const char *prompt);

        int ls(const vector<string>& args, stringstream& ss);
        int safety(const vector<string>& args, stringstream& ss);
        int progress(const vector<string>& args, stringstream& ss);
        int simulate(const vector<string>& args, stringstream& ss);
        int basic(const vector<string>& args, stringstream& ss);
        int monitor(const vector<string>& args, stringstream& ss);
        int code(const vector<string>& args, stringstream& ss);
        int alpha(const vector<string>& args, stringstream& ss);
        int see(const vector<string>& args, stringstream& ss);
        int print(const vector<string>& args, stringstream& ss);
        int lsprop(const vector<string>& args, stringstream& ss);
        int lsmenu(const vector<string>& args, stringstream& ss);
        int minimize(const vector<string>& args, stringstream& ss);
        int traces(const vector<string>& args, stringstream& ss);
        int printvar(const vector<string>& args, stringstream& ss);
        int if_(const vector<string>& args, stringstream& ss);
        int elif_(const vector<string>& args, stringstream& ss);
        int else_(const vector<string>& args, stringstream& ss);
        int fi_(const vector<string>& args, stringstream& ss);
        int exit_(const vector<string>& args, stringstream& ss);
        int help(const vector<string>& args, stringstream& ss);

    public:
        Shell(FspDriver& cr, istream& inr);
        Shell(FspDriver& cr, ifstream& inr);

        int run();
        void readline(string& line);
        void putsstream(stringstream& ss, bool eol);
        void history_enable(bool enable);
        bool lookup_variable(const string& name, int& val) const;

        ~Shell();
};


#endif

