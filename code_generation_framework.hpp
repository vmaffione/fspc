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

#ifndef __FSPC_CODEGEN_CODE_GENERATOR_FRAMEWORK__HH
#define __FSPC_CODEGEN_CODE_GENERATOR_FRAMEWORK__HH

#include <memory>
#include <string>
#include <fstream>
#include <vector>

#include "scalable_visitor.hpp"

namespace codegen {

/**
 * @class Serializable
 * @brief Serializable interface
 **/
class Serializable {
    protected:
        std::string filename;
    public:
        Serializable(std::string f);
        virtual void serialize() = 0;
};

/**Alias for shared_ptr<T>*/
template<typename T> using heap = std::shared_ptr<T>;

/*TODO: FIND A WAY TO ALLOW COVARIANT RETURN TYPES IN ACCEPT AND VISIT*/

/**
 * @class Information
 * @brief A generic information
 *
 * An information is a visitable element (ie. it exposes an accept method).
 * The result of a visit is a new information.
 **/
class Information : public virtual visitor::Element<heap<Information>> {
    public:
        virtual ~Information() = 0;
};

/**Alias for AddAccept<T, heap<Information>>*/
template<typename T> using MakeVisitable =
    visitor::AddAccept<T, heap<Information>>;
/**Alias for AddVisit<T, heap<Information>>*/
template<typename T> using Visits = visitor::AddVisit<T, heap<Information>>;

/**
 * Requirements are sequences of words.
 **/
typedef std::vector<std::string> Requirements;
/**
 * @class UserRequirements
 * @brief This struct models user requirements
 *
 * UserRequirements is an Information, so it is a a visitable class. The result
 * of a visit on an Information is another Information. The class wraps a vector
 * of strings, which contains the user specification.
 **/
struct UserRequirements : Information, MakeVisitable<const UserRequirements> {
    Requirements userRequirements;
    UserRequirements(const Requirements& r);
};

/**
 * @class SourceFile
 * @brief This class models a source file
 *
 * A SourceFile is a Serializable Information. It wraps a string containing
 * code, which can be serialized using the Serializable interface.
 **/
class SourceFile :
    public Information,
    public Serializable,
    public MakeVisitable<SourceFile> {
    protected:
        std::string code;
        /**
         * This procedure can be redefined in subclasses with proper indentation
         * logic (language dependant). It is called before serialization
         * happens.
         **/
        virtual void indent() {};
    public:
        SourceFile(std::string f, std::string c);
        virtual void serialize();
        virtual ~SourceFile() {};
};

/**
 * @class SourceCode
 * @brief SourceCode models a collection of SourceFile
 *
 * SourceCode is a Serializable Information modeling a set of SourceFiles.
 * Serializing a SourceCode means to serialize each composing SourceFile.
 **/
class SourceCode :
    public Information,
    public Serializable,
    public MakeVisitable<SourceCode> {
    private:
        vector<heap<SourceFile>> sourceFiles;
    public:
        SourceCode();
        void add_file(heap<SourceFile> f);
        virtual void serialize();
};

/**
 * @class Analyst
 * @brief Visitor which can visit UserRequirements
 *
 * An Analyst can visit UserRequirements, which is an Information. The result of
 * a visit is another Information. In particular, a ParticularAnalyst (a class
 * inheriting from Analyst) must define its visit(const UserRequirements&)
 * method as a mapping from UserRequirements (which is an Information) to a
 * ParticularModel (which has to be an Information).
 **/
class Analyst :
    public visitor::Visitor,
    public Visits<const UserRequirements> {};

/**
 * @class Developer
 * @brief Developer concept, a Visitor
 *
 * A Developer is a Visitor. A ParticularDeveloper must extend
 * Visits<ParticularModel> for each ParticularModel (which is an Information) it
 * can handle. Each exposed visit has to map a ParticularModel to SourceCode
 * (which is an Information).
 **/
class Developer : public visitor::Visitor {};

}

#endif
