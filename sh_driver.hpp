#ifndef __SH_DRIVER__HH
#define __SH_DRIVER__HH

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <set>
#include <vector>
#include <string>
#include <cstdio>

//#define NDEBUG
#include <assert.h>

#include "sh_parser.hpp"

#include "shell.hpp"

using namespace std;


class ShDriver
{
        const Shell& sh;

    public:
        ShDriver(const Shell& sh);
        virtual ~ShDriver();

        /* Handling the scanner. */
        bool trace_scanning;

        /* Run the parser.  Return 0 on success. */
        int parse(const string& expression);
        bool trace_parsing;
        int result;

        /* Error handling. */
        void error(const std::string& m);

        bool lookup_variable(const string *name, int& val);
};

#endif // ! __SH_DRIVER__HH

