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

#ifndef __FSPC_CODEGEN_GENERATOR__HH
#define __FSPC_CODEGEN_GENERATOR__HH

#include <stdexcept>
#include <string>

#include "fspc_experts.hpp"

namespace codegen {

/**
 * @class CodeGenerator
 * @brief CodeGenerator visitor for code generation
 *
 * A CodeGenerator is a Visitor which can visit UserRequirements (an
 * Information) producing SourceCode (an Information).
 **/
class CodeGenerator :
    public visitor::Visitor,
    public Visits<const UserRequirements> {
    private:
        /**
         * For each supported competence ("monitor", "fsp", "java",
         * "semaphores", "cplusplus"...) CodeGenerator "hires" an Expert and
         * registers it by competence.
         **/
        map<Competence, heap<Expert>> experts;
        /**
         * get_expert looks up the experts register (map) by competence.
         * The expert, if found, is cast to a more specific type, specified by
         * the template parameter Category, and then returned.
         * @param competence The competence profile to look for
         * @tparam Category The specific expert category to look for
         * @return A smart pointer to an expert of the right Category which
         * knows all about the required competence, or a smart pointer to
         * nullptr in case of error (no expert found or expert found in a
         * different Category).
         **/
        template <typename Category>
        heap<Category> get_expert(const Competence& competence)
        {
            return heap<Category>(
                       dynamic_pointer_cast<Category>(experts[competence]));
        }
        /**
         * Reference to the FspDriver structure, to gain access to the fspc
         * environment.
         **/
        FspDriver& c;
    public:
        CodeGenerator(FspDriver& _c);
        /**
         * A CodeGenerator visits UserRequirements (an Information) producing
         * SourceCode (an Information). The CodeGenerator organizes a pipeline
         * composed by one Analyst and one Developer. These Experts are chosen
         * by competences (which have to be specified by the user). The Analyst
         * visits the UserRequirements producing a ParticularModel (which is an
         * Information) whose actual type depends on the particular formalism
         * known by the Analyst. The produced Information is visited by the
         * Developer, which produces the final Information to return (the
         * SourceCode).
         * @param r User requirements
         * @return Source code
         **/
        virtual heap<Information> visit(const UserRequirements& r);
};

}

#endif
