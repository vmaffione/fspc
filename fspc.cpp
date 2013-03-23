#include <iostream>
#include "strings_table.hpp"
#include "parser.hpp"
#include "lts.hpp"

using namespace std;



struct SymbolsTable identifiers;
struct SymbolsTable processes;


int main ()
{
    int ret;
    Lts p1();

    ret = parser();

    return ret;
}
