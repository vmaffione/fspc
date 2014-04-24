/*
 *  Code generator for fspc - code generator
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

#include <stdexcept>

using namespace std;

#include "monitor_analyst.hpp"
#include "java_developer.hpp"
#include "code_generator.hpp"

using namespace codegen;
using namespace java;

CodeGenerator::CodeGenerator(FspDriver& _c) : c(_c)
{
    /*TODO: dynamic expert loading from .so/.dll libraries*/
    typedef heap<Expert> Profile;
    typedef pair<Competence, Profile> Hire;
    experts.insert
    (Hire(Competence("monitor"), Profile(new MonitorAnalyst(c))));
    experts.insert
    (Hire(Competence("java"), Profile(new JavaDeveloper(c))));
}

heap<Information> CodeGenerator::visit(const UserRequirements& r)
{
    /*The first two words in the requirements must identify a formalism and a
     * programming language*/
    if (r.userRequirements.size() < 2) {
        throw runtime_error("Formalism and programming language expected");
    }

    /*The right Analyst and Developer are chosen from the expert pool*/
    heap<Analyst> analyst(get_expert<Analyst>(r.userRequirements[0]));
    heap<Developer> developer(get_expert<Developer>(r.userRequirements[1]));

    /*If the formalism or the language is unknown (ie. the related expert has'n
     * been hired yet), then throw an exception*/
    if (!analyst) throw runtime_error(string("Unknown formalism: ") +
                                          r.userRequirements[0]);

    if (!developer) throw runtime_error(string("Unknown programming language: ") +
                                            r.userRequirements[1]);

    /*The first two requirements have been processed, so they can be cut away*/
    UserRequirements specification(Requirements(r.userRequirements.begin() + 2,
                                   r.userRequirements.end()));

    /*Code generation pipeline: the specification is processed by the analyst
     * who produces a model; then the model is processed by the developer who
     * produces source code.*/
    return (&specification)->accept(*analyst)->accept(*developer);
}
