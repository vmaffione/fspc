#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <endian.h>

using namespace std;


class Serializer {
	ofstream fout;

    public:
	Serializer(const char * filename);
	void byte(uint8_t i, bool raw);
	void integer(uint32_t i, bool raw);
	void stl_string(const string& s, bool raw);
	~Serializer();

	static const char Integer;
	static const char String;
	static const char Byte;
};

class Deserializer {

        ifstream fin;

    public:
	Deserializer(const char * filename);
	void byte(uint8_t &v, bool raw);
	void integer(uint32_t &v, bool raw);
	void stl_string(string& s, bool raw);
	~Deserializer();
};


/* ============================= Serializer ============================== */
const char Serializer::Integer = 'I';
const char Serializer::String = 'S';
const char Serializer::Byte = 'B';

Serializer::Serializer(const char * filename)
{
    fout.open(filename, ios::binary);
}

Serializer::~Serializer()
{
    fout.close();
}

void Serializer::byte(uint8_t v, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::Byte),
						sizeof(char));
    }

    fout.write(reinterpret_cast<const char *>(&v), sizeof(v));
}

void Serializer::integer(uint32_t v, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::Integer),
						sizeof(char));
    }

    v = htole32(v);
    fout.write(reinterpret_cast<const char *>(&v), sizeof(v));
}

void Serializer::stl_string(const string &s, bool raw)
{
    const char * cs = s.c_str();
    uint8_t len = s.size();

    if (s.size() > 255) {
	cout << "Error: string is too long (>255)\n";
	exit(-1);
    }

    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::String),
						sizeof(char));
    }

    this->byte(len, 1);
    fout.write(cs, len);
}


/* ============================ Deserializer ============================= */
Deserializer::Deserializer(const char * filename)
{
    fin.open(filename, ios::binary);
}

Deserializer::~Deserializer()
{
    fin.close();
}

void Deserializer::byte(uint8_t &v, bool raw)
{
    char type;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::Byte) {
	    cout << "Error: expected byte\n";
	    exit(-1);
	}
    }

    fin.read(reinterpret_cast<char *>(&v), sizeof(v));
}

void Deserializer::integer(uint32_t &v, bool raw)
{
    char type;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::Integer) {
	    cout << "Error: expected integer\n";
	    exit(-1);
	}
    }

    fin.read(reinterpret_cast<char *>(&v), sizeof(v));
    v = le32toh(v);
}

void Deserializer::stl_string(string &s, bool raw)
{
    char type;
    uint8_t len;
    char buf[256 + 1];

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::String) {
	    cout << "Error: expected string\n";
	    exit(-1);
	}
    }

    this->byte(len, 1);
    fin.read(buf, len);
    buf[len] = '\0';
    s = string(buf);
}







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
