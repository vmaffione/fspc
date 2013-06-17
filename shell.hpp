#ifndef __SHELL_HH
#define __SHELL_HH

#include <iostream>
#include <string>

#include "translator.hpp"

using namespace std;


class Shell {
	FspCompiler& c;

	void ls(const vector<string> &args);
	void safety(const vector<string> &args);
	void progress(const vector<string> &args);
	void simulate(const vector<string> &args);
	void help(const vector<string> &args);

    public:
	Shell(FspCompiler &cr) : c(cr) { }
	int run();
};

#endif

