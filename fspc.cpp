#include <iostream>
#include "lts.hpp"

using namespace std;

extern int parser();


int main ()
{
    int ret;
    Lts p1();

    ret = parser();

    return ret;
}
