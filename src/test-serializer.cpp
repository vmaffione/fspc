#include "serializer.hpp"

#include <iostream>

using namespace std;


void serialize()
{
    Serializer ser("ser");
    string s1("miao");
    string s2("culo");

    ser.integer(4, 0);
    ser.integer(1973, 0);
    ser.stl_string(s1, 0);
    ser.stl_string(s2, 0);
    ser.integer(0, 0);
    ser.byte(65, 0);
    ser.integer(4123451, 0);
}

void deserialize()
{
    Deserializer des("ser");
    uint32_t a,b,c,d;
    uint8_t h;
    string s1, s2;

    des.integer(a, 0);
    des.integer(b, 0);
    des.stl_string(s1, 0);
    des.stl_string(s2, 0);
    des.integer(c, 0);
    des.byte(h, 0);
    des.integer(d, 0);

    cout << a << " " << b << " " << s1 << " " << s2 << " " << c << " " << h << " " << d << "\n";
}

int main() {
    serialize();
    deserialize();
}

