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

#include "symbols_table.hpp"  /* struct ActionsTable */
#include "lts.hpp"            /* class Lts */

/* Serialization support. */
#include "serializer.hpp"

using namespace std;



/* ============================= Serializer ============================== */
const char Serializer::SerInteger = 'I';
const char Serializer::SerString = 'S';
const char Serializer::SerByte = 'B';
const char Serializer::SerActionsTable = 'T';
const char Serializer::SerLts = 'L';
const char Serializer::SerSetValue = 'U';
const char Serializer::SerActionSetValue = 'A';
const char Serializer::SerProgressValue = 'P';

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
	fout.write(static_cast<const char *>(&Serializer::SerByte),
						sizeof(char));
    }

    fout.write(reinterpret_cast<const char *>(&v), sizeof(v));
}

void Deserializer::byte(uint8_t &v, bool raw)
{
    char type;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::SerByte) {
	    cout << "Error: expected byte\n";
	    exit(-1);
	}
    }

    fin.read(reinterpret_cast<char *>(&v), sizeof(v));
}

void Serializer::integer(uint32_t v, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::SerInteger),
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
	if (type != Serializer::SerInteger) {
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
	fout.write(static_cast<const char *>(&Serializer::SerString),
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
	if (type != Serializer::SerString) {
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
	fout.write(static_cast<const char *>(&Serializer::SerActionsTable),
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
	if (type != Serializer::SerActionsTable) {
	    cout << "Error: expected actions table\n";
	    exit(-1);
	}
    }

    this->stl_string(at.name, 1);
    this->integer(size, 1);
    at.reverse.resize(size);
    for (unsigned int i=0; i<size; i++) {
	this->stl_string(s, 1);
	this->integer(x, 1);
	at.table.insert(pair<string, int>(s, x));
	at.reverse[x] = s;
    }
    this->integer(x, 1);
    at.serial = x;
    
}

void serializeLtsVisitFunction(int state, const yy::Lts& lts,
                               const struct LtsNode& node, void * opaque)
{
    Serializer * serp = static_cast<Serializer *>(opaque);

    for (unsigned int i=0; i<node.children.size(); i++) {
	serp->integer(state, 1);
	serp->integer(node.children[i].action, 1);
	serp->integer(node.children[i].dest, 1);
    }
}

void Serializer::lts(const yy::Lts &lts, bool raw)
{
    yy::LtsVisitObject lvo;
    uint32_t end = ~0;
    uint32_t error = ~0;

    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::SerLts),
						sizeof(char));
    }

    this->stl_string(lts.name, 1);
    this->integer(lts.numTransitions(), 1);
    this->integer(lts.nodes.size(), 1);
    for (unsigned int i=0; i<lts.nodes.size(); i++) {
	switch (lts.get_type(i)) {
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

void Deserializer::lts(yy::Lts &lts, bool raw)
{
    char type;
    uint32_t x, y, z, end, error;
    Edge e;
    int ntr;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::SerLts) {
	    cout << "Error: expected LTS\n";
	    exit(-1);
	}
    }

    lts.terminal_sets_computed = false;

    this->stl_string(lts.name, 1);
    this->integer(x, 1); ntr = x;
    this->integer(x, 1); lts.nodes.resize(x);
    this->integer(end, 1);
    this->integer(error, 1);
    for (unsigned int i=0; i<lts.nodes.size(); i++)
	lts.set_type(i, LtsNode::Normal);
    if (end != ~0U)
	lts.set_type(end, LtsNode::End);
    if (error != ~0U) // XXX if (~error)
	lts.set_type(error, LtsNode::Error);

    for (int i=0; i<ntr; i++) {
	this->integer(x, 1);
	this->integer(y, 1);
	this->integer(z, 1);
	e.dest = z;
	e.action = y;
	lts.nodes[x].children.push_back(e);
    }

    this->integer(x, 1);
    for (uint32_t i=0; i<x; i++) {
	this->integer(y, 1);
	lts.updateAlphabet(y);
    }
}

void Serializer::set_value(const struct SetS& setv, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::SerSetValue),
						sizeof(char));
    }

    this->integer(setv.size(), 1);
    for (unsigned int i=0; i<setv.size(); i++) {
	this->stl_string(setv[i], 1);
    }
}

void Deserializer::set_value(struct SetS& setv, bool raw)
{
    char type;
    uint32_t x;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::SerSetValue) {
	    cout << "Error: expected SetS\n";
	    exit(-1);
	}
    }

    this->integer(x, 1);
    setv.actions.resize(x);
    for (unsigned int i=0; i<setv.size(); i++) {
	this->stl_string(setv[i], 1);
    }
}

void Serializer::action_set_value(const struct ActionSetS& asv, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::SerActionSetValue),
						sizeof(char));
    }

    this->integer(asv.actions.size(), 1);
    for (set<unsigned int>::iterator it = asv.actions.begin();
                    it != asv.actions.end(); it++) {
	this->integer(*it, 1);
    }
}

void Deserializer::action_set_value(struct ActionSetS& asv, bool raw)
{
    char type;
    uint32_t x, y;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::SerActionSetValue) {
	    cout << "Error: expected ActionSetS\n";
	    exit(-1);
	}
    }

    this->integer(x, 1);
    asv.clear();
    for (unsigned int i=0; i<x; i++) {
	this->integer(y, 1);
        asv.add(y);
    }
}

void Serializer::progress_value(const struct ProgressS& pv, bool raw)
{
    if (!raw) {
	fout.write(static_cast<const char *>(&Serializer::SerProgressValue),
						sizeof(char));
    }

    this->action_set_value(pv.set, 1);
    this->byte(pv.conditional ? 1 : 0, 1);
    if (pv.conditional) {
        this->action_set_value(pv.condition, 1);
    }
}

void Deserializer::progress_value(struct ProgressS& pv, bool raw)
{
    char type;
    uint8_t x;

    if (!raw) {
	fin.read(static_cast<char *>(&type), sizeof(char));
	if (type != Serializer::SerProgressValue) {
	    cout << "Error: expected ProgressS\n";
	    exit(-1);
	}
    }

    pv.set.clear();
    this->action_set_value(pv.set, 1);
    this->byte(x, 1);
    pv.conditional = (x != 0);
    if (pv.conditional) {
        this->action_set_value(pv.condition, 1);
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

