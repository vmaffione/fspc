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


#ifndef __SERIALIZER__HH
#define __SERIALIZER__HH

#include <fstream>
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <endian.h>

#include "symbols_table.hpp"  /* struct ActionsTable */
#include "lts.hpp"

using namespace std;


class Serializer {
	ofstream fout;

    public:
	Serializer(const char * filename);
	void byte(uint8_t i, bool raw);
	void integer(uint32_t i, bool raw);
	void stl_string(const string& s, bool raw);
	void actions_table(const struct ActionsTable& at, bool raw);
	void lts(const class yy::Lts& lts, bool raw);
	void set_value(const struct SetValue& setv, bool raw);
	~Serializer();

	static const char Integer;
	static const char String;
	static const char Byte;
	static const char ActionsTable;
	static const char Lts;
	static const char SetValue;
};

class Deserializer {

        ifstream fin;

    public:
	Deserializer(const char * filename);
	void byte(uint8_t &v, bool raw);
	void integer(uint32_t &v, bool raw);
	void stl_string(string& s, bool raw);
	void actions_table(struct ActionsTable& at, bool raw);
	void lts(class yy::Lts& lts, bool raw);
	void set_value(struct SetValue& setv, bool raw);
	~Deserializer();
};

#endif

