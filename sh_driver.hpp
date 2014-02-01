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


using namespace std;


class ShDriver
{
    public:
        ShDriver();
        virtual ~ShDriver();

        /* Handling the scanner. */
        bool trace_scanning;

        /* Run the parser.  Return 0 on success. */
        int parse();
        bool trace_parsing;
        int result;

        /* Error handling. */
        void error(const std::string& m);
};

#endif // ! __SH_DRIVER__HH

