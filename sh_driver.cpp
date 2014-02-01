#include "sh_driver.hpp"

/* Interactive shell */
#include "shell.hpp"

/* Bison generated parser. */
#include "sh_parser.hpp"

using namespace std;


/* Global functions exported by sh_scanner.cpp. */
void sh_scan_begin(const string& s, int trace_scanning);
void sh_scan_end();


/* ============================== FspDriver ============================= */

ShDriver::ShDriver()
{
    trace_scanning = trace_parsing = false;
    result = 0;
}

ShDriver::~ShDriver()
{
}

int ShDriver::parse()
{
    int ret;
    string inp = "3+2*4";

    sh_scan_begin(inp, trace_scanning);
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
    cerr << m << endl;
}

