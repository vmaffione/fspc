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
#include <map>

#include "translator.hpp"

using namespace std;


class Shell {
	FspCompiler& c;
	map<string, const char*> help_map;
	typedef void (Shell::*ShellCmdFunc)(const vector<string>& args);
	map<string, ShellCmdFunc> cmd_map;

	void ls(const vector<string> &args);
	void safety(const vector<string> &args);
	void progress(const vector<string> &args);
	void simulate(const vector<string> &args);
	void basic(const vector<string> &args);
	void alpha(const vector<string> &args);
	void see(const vector<string> &args);
	void help(const vector<string> &args);

    public:
	Shell(FspCompiler &cr);
	int run();
};


#endif

