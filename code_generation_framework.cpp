/*
 *  Code generation framework
 *
 *  Copyright (C) 2014  Cosimo Sacco
 *  Email contact: <cosimosacco@gmail.com>
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

#include <string>
#include <fstream>
#include <vector>

using namespace std;

#include "code_generation_framework.hpp"

using namespace codegen;

Serializable::Serializable(string f) : filename(f) {}

Information::~Information() {}

UserRequirements::UserRequirements(const Requirements& r)
        : userRequirements(r) {}

SourceFile::SourceFile(string f, string c)
        : Serializable(f), code(c) {}

void SourceFile::serialize()
{
    fstream file;
    this->indent();
    file.open(filename.c_str(), ios::out);

    if (!file.is_open() || !(file << code))
        throw runtime_error(string("Could not write on ") + filename);

    file.close();
    return;
}

SourceCode::SourceCode() : Serializable("") {}

void SourceCode::add_file(heap<SourceFile> f)
{
    sourceFiles.push_back(f);
}

void SourceCode::serialize()
{
    for (auto& s: sourceFiles) s->serialize();
}
