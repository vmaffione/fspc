#include "sh_driver.hpp"

/* Interactive shell */
#include "shell.hpp"

/* Bison generated parser. */
#include "sh_parser.hpp"

#include <cassert>

using namespace std;


/* Global functions exported by sh_scanner.cpp. */
void sh_scan_begin(const string& s, int trace_scanning);
void sh_scan_end();


/* ============================== FspDriver ============================= */

ShDriver::ShDriver(const Shell& shell) : sh(shell)
{
    trace_scanning = trace_parsing = false;
    result = 0;
}

ShDriver::~ShDriver()
{
}

int ShDriver::parse(const string& expression)
{
    int ret;

    sh_scan_begin(expression, trace_scanning);
    sh::ShParser parser(*this);
    parser.set_debug_level(trace_parsing);
    ret = parser.parse();
    sh_scan_end();

    return ret;
}

/*
void FspDriver::error(const fsp::location& l, const std::string& m)
{
    print_error_location_pretty(l);
    cerr << m << endl;
}*/

void ShDriver::error(const std::string& m)
{
#if 0
    cerr << m << endl;
#endif
}

bool ShDriver::lookup_variable(const string* name, int& val)
{
    assert(name);

    return sh.lookup_variable(*name, val);
}

