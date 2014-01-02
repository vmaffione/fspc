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

using namespace std;


namespace fsp {
    class Lts;
    class ActionsTable;
}

class Serializer {
	ofstream fout;

    public:
	Serializer(const char * filename);
	void byte(uint8_t i, bool raw);
	void integer(uint32_t i, bool raw);
	void stl_string(const string& s, bool raw);
	void actions_table(const class fsp::ActionsTable& at, bool raw);
	void lts(const fsp::Lts& lts, bool raw);
	void set_value(const struct fsp::SetS& setv, bool raw);
        void action_set_value(const struct fsp::ActionSetS& as, bool raw);
        void progress_value(const struct fsp::ProgressS& pv, bool raw);
	~Serializer();

	static const char SerInteger;
	static const char SerString;
	static const char SerByte;
	static const char SerActionsTable;
	static const char SerLts;
	static const char SerSetValue;
	static const char SerActionSetValue;
	static const char SerProgressValue;
};

class Deserializer {

        ifstream fin;

    public:
	Deserializer(const char * filename);
	void byte(uint8_t &v, bool raw);
	void integer(uint32_t &v, bool raw);
	void stl_string(string& s, bool raw);
	void actions_table(class fsp::ActionsTable& at, bool raw);
	void lts(fsp::Lts& lts, bool raw);
	void set_value(struct fsp::SetS& setv, bool raw);
        void action_set_value(struct fsp::ActionSetS& asv, bool raw);
        void progress_value(struct fsp::ProgressS& pv, bool raw);
	~Deserializer();
};

#endif

