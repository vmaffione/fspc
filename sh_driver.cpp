#include "sh_driver.hpp"

/* Interactive shell */
#include "shell.hpp"

/* Bison generated parser. */
#include "sh_parser.hpp"

using namespace std;


/* ============================== FspDriver ============================= */

ShDriver::ShDriver()
{
    trace_scanning = trace_parsing = false;
}

ShDriver::~ShDriver()
{
}

int ShDriver::parse()
{
    int ret;
    string inp = "3+2*4";

    scan_begin(inp);
    sh::ShParser parser(*this);
    parser.set_debug_level(trace_parsing);
    ret = parser.parse();
    scan_end();

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
    cerr << m << endl;
}

