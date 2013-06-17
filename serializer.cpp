/*
 *  fspc serialization support
 *
 *  Copyright (C) 2013  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include "serializer.hpp"

using namespace std;


/* ============================= Serializer ============================== */
const char Serializer::Integer = 'I';
const char Serializer::String = 'S';
const char Serializer::Byte = 'B';
const char Serializer::ActionsTable = 'T';
const char Serializer::Lts = 'L';
const char Serializer::SetValue = 'U';

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

void Serializer::integer(uint32_t v, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::Integer),
						sizeof(char));
    }

    v = htole32(v);
    fout.write(reinterpret_cast<const char *>(&v), sizeof(v));
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

void Serializer::actions_table(const struct ActionsTable& at, bool raw)
{
    map<string, int>::const_iterator it;

    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::ActionsTable),
						sizeof(char));
    }

    this->stl_string(at.name, 1);
    this->integer(at.table.size(), 1);
    for (it=at.table.begin(); it!=at.table.end(); it++) {
	this->stl_string(it->first, 1);
	this->integer(it->second, 1);
    }
    this->integer(at.serial, 1);
}

void Deserializer::actions_table(ActionsTable &at, bool raw)
{
    char type;
    uint32_t size, x;
    string s;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::ActionsTable) {
	    cout << "Error: expected actions table\n";
	    exit(-1);
	}
    }

    this->stl_string(at.name, 1);
    this->integer(size, 1);
    at.reverse.resize(size);
    for (int i=0; i<size; i++) {
	this->stl_string(s, 1);
	this->integer(x, 1);
	at.table.insert(pair<string, int>(s, x));
	at.reverse[x] = s;
    }
    this->integer(x, 1);
    at.serial = x;
    
}

void serializeLtsVisitFunction(int state, const struct LtsNode& node, void * opaque)
{
    Serializer * serp = static_cast<Serializer *>(opaque);

    for (int i=0; i<node.children.size(); i++) {
	serp->integer(state, 1);
	serp->integer(node.children[i].action, 1);
	serp->integer(node.children[i].dest, 1);
    }
}

void Serializer::lts(const class Lts &lts, bool raw)
{
    LtsVisitObject lvo;
    uint32_t end = ~0;
    uint32_t error = ~0;

    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::Lts),
						sizeof(char));
    }

    this->stl_string(lts.name, 1);
    this->integer(lts.ntr, 1);
    this->integer(lts.nodes.size(), 1);
    for (int i=0; i<lts.nodes.size(); i++) {
	switch (lts.nodes[i].type) {
	    case LtsNode::End:
		end = i;
		break;
	    case LtsNode::Error:
		error = i;
		break;
	}
    }
    this->integer(end, 1);
    this->integer(error, 1);

    lvo.vfp = serializeLtsVisitFunction;
    lvo.opaque = this;
    lts.visit(lvo);

    this->integer(lts.alphabet.size(), 1);
    for (set<int>::iterator it=lts.alphabet.begin();
			it!=lts.alphabet.end(); it++) {
	this->integer(*it, 1);
    }
}

void Deserializer::lts(class Lts &lts, bool raw)
{
    char type;
    uint32_t x, y, z, end, error;
    Edge e;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::Lts) {
	    cout << "Error: expected LTS\n";
	    exit(-1);
	}
    }

    lts.terminal_sets_computed = false;

    this->stl_string(lts.name, 1);
    this->integer(x, 1); lts.ntr = x;
    this->integer(x, 1); lts.nodes.resize(x);
    this->integer(end, 1);
    this->integer(error, 1);
    for (int i=0; i<lts.nodes.size(); i++)
	lts.nodes[i].type = LtsNode::Normal;
    if (end != ~0)
	lts.nodes[end].type = LtsNode::End;
    if (error != ~0) // XXX if (~error)
	lts.nodes[error].type = LtsNode::Error;

    for (int i=0; i<lts.ntr; i++) {
	this->integer(x, 1);
	this->integer(y, 1);
	this->integer(z, 1);
	e.dest = z;
	e.action = y;
	lts.nodes[x].children.push_back(e);
    }

    this->integer(x, 1);
    for (int i=0; i<x; i++) {
	this->integer(y, 1);
	lts.updateAlphabet(y);
    }
}

void Serializer::set_value(const struct SetValue& setv, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::SetValue),
						sizeof(char));
    }

    this->integer(setv.actions.size(), 1);
    for (int i=0; i<setv.actions.size(); i++) {
	this->stl_string(setv.actions[i], 1);
    }
}

void Deserializer::set_value(struct SetValue& setv, bool raw)
{
    char type;
    uint32_t x;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::SetValue) {
	    cout << "Error: expected SetValue\n";
	    exit(-1);
	}
    }

    this->integer(x, 1);
    setv.actions.resize(x);
    for (int i=0; i<setv.actions.size(); i++) {
	this->stl_string(setv.actions[i], 1);
    }
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

/* XXX old test code
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
*/
