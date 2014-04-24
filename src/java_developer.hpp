/*
 *  Code generator for fspc - java developer
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

#ifndef __FSPC_CODEGEN_JAVA_DEVELOPER__HH
#define __FSPC_CODEGEN_JAVA_DEVELOPER__HH

#include <vector>
#include <array>

#include "fspc_experts.hpp"
#include "monitor_analyst.hpp"

namespace codegen {
namespace java {

/**
 * @class JavaSourceFile
 * @brief A JavaSourceFile is a SourceFile with Java indentation conventions.
 **/
class JavaSourceFile : public SourceFile {
    protected:
        virtual void indent();
    public:
        JavaSourceFile(std::string& outfile, std::string& code);
};

/**
 * @class JavaDeveloper
 * @brief Developer of Java code
 *
 * A JavaDeveloper is a FspcDeveloper, namely a Developer having access to the
 * fspc environment. It understands several formalisms (that is, it can visit
 * several specification types, which are Information(s)) an produces as output
 * Java SourceCode (which is an Information).
 **/
class JavaDeveloper :
    public FspcDeveloper,
    public Visits<MonitorSpecification>
/* public Visits<SemaphoreSpecification>,
 * public Visits<RPCSpecification>,
 * public Visits<LockfreeSpecification> and so on... */
{
    protected:
        /**
         * Characters which may appear in FSP names but can't in Java names.
         **/
        static constexpr std::array<char, 5> invalid_chars
        {{'(', ')', '[', ']', '.'}};
        /**
         * Utility method which substitutes each occurrence of an invalid
         * character with a '_'.
         * @param s The original string
         * @return The modified string
         **/
        std::string _(const std::string& s);
        /**
         * Utility method which substitutes each occurrence of an invalid
         * character with a '_' saving the last dot.
         * @param s The original string
         * @return The modified string
         **/
        std::string _saving_last_dot(const std::string& s);
        /**
         * Method which maps the specification of a monitor into Java code.
         * @param specification The complete specification adhering to the
         * monitor formalism
         * @param identifier The identifier of the FSP process to be translated
         * as a Java monitor class
         * @param interactions A list of the interactions offered by the process
         * (which will be translated as synchronized Java methods)
         * @return The produced Java monitor class
         **/
        heap<SourceFile> monitor_code(
            MonitorSpecification& specification,
            const MonitorIdentifier& identifier,
            const list<Interaction>& interactions
        );
        /**
         * Method which maps the specification of a thread into Java code.
         * @param specification The complete specification adhering to the
         * monitor formalism
         * @param identifier The identifier of the FSP process to be translated
         * as a Java thread class
         * @return The produced Java thread class
         **/
        heap<SourceFile> thread_code(
            MonitorSpecification& specification,
            const ThreadIdentifier& identifier
        );
        /**
         * Method which produces a monitor class by template instantiation.
         * @param identifier Name for the produced class
         * @param model Structure describing the monitor in terms of
         * Monitor Normal Form
         * @param synchpoints List of synchronized methods
         * @return The code is returned by reference
         **/
        void instantiate_monitor_template(
            const MonitorIdentifier& identifier,
            MNF& model, const std::list<std::string>& synchpoints,
            std::string& monitor_code
        );
        /**
         * Method which produces a thread class by template instantiation.
         * @param identifier Name for the produced class
         * @param tnf Structure describing the monitor in terms of Thread Normal
         * Form
         * @param interaction_instance_map For each interaction (synchronized
         * method) used by the thread this map identifies the reference name of
         * the monitor object offering that interaction
         * @param instance_type_map For each monitor instance used by the thread
         * this map identifies its class
         * @param thread_alphabet Set of all the actions and interactions
         * performed by the thread
         * @return The code is returned by reference
         **/
        void instantiate_thread_template(
            const ThreadIdentifier& identifier,
            TNF& tnf,
            std::list<MonitorInstance> used_instances,
            std::map<Interaction, std::pair<MonitorInstance, bool>>
            interaction_instance_map,
            std::map<MonitorInstance, MonitorIdentifier>
            instance_type_map,
            std::set<Label> thread_alphabet,
            std::string& thread_code
        );
    public:
        /**
         * Method which maps a MonitorSpecification to the corresponding
         * Java SourceCode (which is an Information).
         * @param ms Specification adhering to the monitor formalism
         * @return Java source code that follows the specification
         **/
        virtual heap<Information> visit(MonitorSpecification& ms);
        JavaDeveloper(FspDriver& _c);
};

}
}

#endif
