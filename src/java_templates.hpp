/*
 *  Code generator for fspc - templates for java code generation
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

#ifndef __FSPC_CODEGEN_JAVA_TEMPLATES__HH
#define __FSPC_CODEGEN_JAVA_TEMPLATES__HH

#include <string>

namespace codegen {
namespace java {

/*This namespace contains template strings used to produce Java code.*/

static const std::string limitation_of_responsibility =
"/*\n"
" * WARNING: this code has been generated by a highly experimental tool.\n"
" * You are strongly encouraged to carefully inspect this code before using it.\n"
" * The author takes no responsibility for any inconvenient this code may cause.\n"
" * Use this code at your own risk.\n"
" */\n\n";

static const std::string monitor_template =
"$header"
"import java.util.concurrent.locks.Condition;\n\
import java.util.concurrent.locks.Lock;\n\
import java.util.concurrent.locks.ReentrantLock;\n\n\
public class $monitor_identifier\n{\n\
private int state = 0;\n\
private final Lock lock = new ReentrantLock(true);\n\
$condition_variable_declarations\
$internal_action_definitions\
public $monitor_identifier(/*fill in parameters*/)\n{\n/*fill in body*/\n}\n\
$interactions\
}";

static const std::string condition_variable_declaration_template =
    "private final Condition $interaction_condition = lock.newCondition();\n";

static const std::string internal_action_declaration_template =
    "private void $internal_action(/*fill in parameters*/)\n{\n/*fill in body*/\n}\n";

static const std::string interaction_template =
    "public void $interaction(/*fill in parameters*/) throws InterruptedException\n{\n\
lock.lock();\n\
try {\n\
while ($wait_condition) {\n\
$interaction_condition.await();\n\
}\n\
switch (state) {\n\
$state_consequences\
}\n\
} finally {lock.unlock();}\n\
return;\n\
}\n";

static const std::string wait_condition_minterm_template = "(state != $state)";

static const std::string state_consequence_template =
    "case $state: {\n\
$internal_actions_sequence\
state = $next_state;\n\
$condition_signaling\
} break;\n";

static const std::string internal_action_invocation =
    "$internal_action(/*fill in parameters*/);\n";

static const std::string condition_signaling_template =
    "$interaction_condition.signalAll();\n";

static const std::string thread_template =
"$header"
"public class $thread_identifier extends Thread\n{\n\
private int state = 0;\n\
$monitor_members\
$private_methods\
$choice_methods\
public $thread_identifier($monitor_parameters /*fill in other parameters*/)\n\
{\n\
$monitor_references_init\
}\n\
$run_method\
}";

static const std::string monitor_member_template =
    "private $monitor_class $local_monitor_identifier;\n";

static const std::string private_method_template =
    "private void $internal_action(/*fill in parameters*/)\n{\n/*fill in body*/\n}\n";

static const std::string choice_method_template =
    "private int choice$state(/*fill in parameters*/)\n{\n/*fill in body*/\n}\n";

static const std::string monitor_parameter_template =
    "$monitor_class _$monitor_reference";

static const std::string monitor_reference_init_template =
    "$local_monitor_identifier = _$monitor_reference;\n";

static const std::string run_method_template =
    "public void run()\n{\n\
try{\n\
while(true){\n\
switch(state){\n\
$cases\
}\n\
}\n\
} catch(InterruptedException e) {/*Manage InterruptedException*/}\n\
}\n";

static const std::string simple_case_template =
    "case $state:{\n\
$action_sequence\
$transition\n\
}\n";

static const std::string state_transition_template =
    "state = $next_state;";

static const std::string final_transition_template =
    "return;";

static const std::string interaction_invocation =
    "$local_monitor_identifier.$interaction(/*fill in parameters*/);\n";

static const std::string choice_case_template =
    "case $state:{\n\
int choice = choice$state(/*fill in parameters*/);\n\
switch(choice){\n\
$cases\
}\n\
}\n";

}
}

#endif
