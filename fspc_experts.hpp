/*
 *  Code generator for fspc - expert classes
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

#ifndef __FSPC_CODEGEN_FSPC_EXPERTS__HH
#define __FSPC_CODEGEN_FSPC_EXPERTS__HH

#include <string>
#include <list>

#include "fsp_driver.hpp"
#include "symbols_table.hpp"
#include "code_generation_framework.hpp"

namespace codegen {

/**
 * @class FspcExpert
 * @tparam CompetenceClass The class to extend granting access to fspc
 * @brief An Expert which can use the fspc environment
 *
 * This class empowers an Expert giving it access to the fspc environment.
 **/
template <typename CompetenceClass>
class FspcExpert : public CompetenceClass {
    protected:
        FspDriver& c;
        fsp::ActionsTable& actions;
        /**
         * Utility method to instantiate template strings.
         * @param template_string A template string is a string containing
         * parameters meant to be instantiated by string substitution
         * @param parameter The specific parameter to instantiate into the
         * template string
         * @param value The value to use in template string instantiation
         * @return The instantiated template string is returned by reference
         * through the first parameter
         **/
        void instantiate_template(std::string& template_string,
                                  const std::string& parameter,
                                  const std::string& value)
        {
            const string::size_type value_size(value.size());
            const string::size_type parameter_size(parameter.size());
            string::size_type n = 0;

            while (string::npos != (n = template_string.find(parameter, n))) {
                template_string.replace(n, parameter_size, value);
                n += value_size;
            }
        }
    public:
        FspcExpert(FspDriver& _c) :
            c(_c), actions(fsp::ActionsTable::getref()) {}
};

/**
 * An Expert is someone being able to map some kind information into other kinds
 * of information.
 **/
typedef visitor::Visitor Expert;
/**
 * Each expert is competend about something (a simple string).
 **/
typedef std::string Competence;
/**
 * FspcAnalyst is an Analyst which can access the fspc environment.
 **/
typedef FspcExpert<Analyst> FspcAnalyst;
/**
 * FspcDeveloper is a Developer which can access the fspc environment.
 **/
typedef FspcExpert<Developer> FspcDeveloper;

/*Some useful typedefs*/
typedef int State;
typedef int Label;
typedef int Interaction;
typedef int InternalAction;
typedef std::string MonitorInstance;
typedef std::string MonitorIdentifier;
typedef std::string ThreadIdentifier;
typedef std::list<Label> ActionSequence;

/*Error codes and error messages*/
static const int internal_actions_cycle = 0;
static const int hybrid_interactions = 1;
static const int active_control = 2;
static const int end_or_fail_state_detected = 3;
static const int unknown_state = 4;
static const int stochastic_node = 5;
static const int label_not_found = 6;
static const int non_deterministic_thread = 7;
static const int process_not_found = 8;
static constexpr const char* fail_reasons[9] = {
    "Endless execution of local actions",
    "Hybrid choice between local action and interaction",
    "Candidate monitor engages in active control",
    "End or fail state detected",
    "Unknown state",
    "Candidate monitor exhibits stochastic behaviour",
    "Label not found",
    "Candidate thread exhibits stochastic behaviour",
    "Process $process not found"
};

}

#endif
