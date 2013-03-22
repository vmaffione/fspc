#include <iostream>
#include <sstream>

#include "strings_set.hpp"



string int2string(int x)
{
    stringstream sstr;
    sstr << x;
    return sstr.str();
}


StringsSet& StringsSet::dotcat(const string& s)
{
    for (int i=0; i<strings.size(); i++)
	strings[i] += "." + s;

    return *this;
}

StringsSet& StringsSet::dotcat(const StringsSet& ss)
{
    int n = strings.size();
    int nss = ss.strings.size();

    strings.resize(n * nss);

    for (int j=1; j<nss; j++)
	for (int i=0; i<n; i++)
	    strings[j*n+i] = strings[i] + "." + ss.strings[j];
    
    for (int i=0; i<n; i++)
	strings[i] += "." + ss.strings[0];

    return *this;
}

StringsSet& StringsSet::indexize(int index)
{
    stringstream sstr;
    sstr << index;
    string suffix = "[" + sstr.str() + "]";

    for (int i=0; i<strings.size(); i++)
	strings[i] += suffix;

    return *this;
}

StringsSet& StringsSet::indexize(const StringsSet& ss)
{
    return dotcat(ss);
}

StringsSet& StringsSet::indexize(int low, int high)
{
    int n = strings.size();
    int nr = high - low + 1;

    strings.resize(n * nr);

    for (int j=1; j<nr; j++)
	for (int i=0; i<n; i++)
	    strings[j*n+i] = strings[i] + "[" + int2string(low + j)  + "]";
    
    for (int i=0; i<n; i++)
	strings[i] += "[" + int2string(low) + "]";

    return *this;
    
}

StringsSet& StringsSet::operator +=(const StringsSet& ss)
{
    for (int i=0; i<ss.strings.size(); i++)
	strings.push_back(ss.strings[i]);

    return *this;
}

StringsSet& StringsSet::operator +=(const string& s)
{
    strings.push_back(s);

    return *this;
}

StringsSet& operator+(const StringsSet& ss1, const StringsSet& ss2)
{
    StringsSet * res = new StringsSet;

    for (int i=0; i<ss1.strings.size(); i++)
	res->strings.push_back(ss1.strings[i]);
    for (int i=0; i<ss2.strings.size(); i++)
	res->strings.push_back(ss2.strings[i]);

    return *res;
}

void StringsSet::print() const
{
    cout << "{";
    for (int i=0; i<strings.size(); i++)
	cout << strings[i] << ", ";
    cout << "}\n";
}


int test()
{
    StringsSet ss1("ciao"), ss2("p1");
    StringsSet ss3("kk");

    ss1 += StringsSet("miao");
    ss1.print();
    ss2 += StringsSet("p2");
    ss2.print();
    
    ss3 += StringsSet("yy");

    ss1.dotcat(ss2).dotcat("olla").indexize(3).indexize(ss3).indexize(8,9);
    ss1.print();

    return 0;
}
