#include <string>
using namespace std;

#ifndef __JAVA_TEMPLATES__HH
#define __JAVA_TEMPLATES__HH

const string monitor_template =
"import java.util.concurrent.locks.Condition;\n\
import java.util.concurrent.locks.Lock;\n\
import java.util.concurrent.locks.ReentrantLock;\n\n\
class $monitor_identifier\n{\n\
private int state = 0;\n\
private final Lock lock = new ReentrantLock();\n\
$condition_variable_declarations\
$internal_action_definitions\
public $monitor_identifier(/*fill in parameters*/){/*fill in body*/}\n\
$interactions\
}";

const string condition_variable_declaration_template =
"private final Condition $interaction_condition = lock.newCondition();\n";

const string internal_action_declaration_template =
"private void $internal_action(/*fill in parameters*/)\n{\n/*fill in body*/\n}\n";

const string interaction_template =
"private void $interaction(/*fill in parameters*/)\n{\n\
lock.lock();\n\
try {\n\
while ($wait_condition) {\n\
$interaction_condition.await();\n\
}\n\
switch (state) {\n\
$state_consequences\
}\n\
} finally {lock.unlock();}\n\
}\n";

const string wait_condition_minterm_template = "(state != $state)";

const string state_consequence_template =
"case $state: {\n\
$internal_actions_sequence\
state = $next_state;\n\
} break;\n";

const string internal_action_invocation =
"$internal_action(/*fill in parameters*/);\n";

void instantiate_template_string(string& template_string,
                                 const string& parameter,
                                 const string& value)
{
    const string::size_type value_size(value.size());
    const string::size_type parameter_size(parameter.size());
    string::size_type n = 0;
    while (string::npos != (n = template_string.find(parameter, n))) {
        template_string.replace(n, parameter_size, value);
        n += value_size;
    }
}

#endif
