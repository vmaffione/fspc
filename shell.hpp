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

using namespace std;


class FspDriver;

class Shell {
	FspDriver& c;
	map<string, const char*> help_map;
	typedef void (Shell::*ShellCmdFunc)(const vector<string>& args, stringstream& ss);
	map<string, ShellCmdFunc> cmd_map;
	istream& in;
	bool interactive;  /* True if in is an interactive input. */

	void common_init();
	void getline_ncurses(string& line, const char * prompt);

	void ls(const vector<string> &args, stringstream& ss);
	void safety(const vector<string> &args, stringstream& ss);
	void progress(const vector<string> &args, stringstream& ss);
	void simulate(const vector<string> &args, stringstream& ss);
	void basic(const vector<string> &args, stringstream& ss);
	void alpha(const vector<string> &args, stringstream& ss);
	void see(const vector<string> &args, stringstream& ss);
	void print(const vector<string> &args, stringstream& ss);
	void help(const vector<string> &args, stringstream& ss);

    public:
	Shell(FspDriver& cr, istream& inr);
	Shell(FspDriver& cr, ifstream& inr);

	int run();
	void readline(string& line);
	void putsstream(stringstream& ss, bool eol);

	~Shell();
};


#endif

