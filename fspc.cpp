#include <iostream>
#include "strings_table.hpp"
#include "parser.hpp"
#include "lts.hpp"

using namespace std;



struct SymbolsTable identifiers;


int main ()
{
    int ret;
    Lts p1("P1");

    ret = parser();

    return ret;
}
