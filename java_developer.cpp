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

#include <string>

using namespace std;

#include "java_developer.hpp"
#include "java_templates.hpp"

using namespace codegen;
using namespace java;

JavaSourceFile::JavaSourceFile(std::string& outfile, std::string& code)
    : SourceFile(outfile, code) {}

void JavaSourceFile::indent()
{
    int level = 0;
    string::size_type size = code.size();

    for (string::size_type i = 0; i < size; i++) {
        switch (code[i]) {
            case '{': {
                level++;

                if (i < size-1 && code[i+1] == '}') level--;
            }
            break;

            case '\n': {
                if (i < size-1 && code[i+1] == '}') level--;

                code.insert(i+1, string(level, '\t'));
                size += level;
                i += level;
            }
            break;

            default:
                if (i < size-1 && code[i+1] == '}') level--;
        }
    }

    return;
}

constexpr array<char, 5> JavaDeveloper::invalid_chars;

string JavaDeveloper::_(const string& s)
{
    string r(s);

    for (auto c : invalid_chars) instantiate_template(r, string(1, c), "_");

    return r;
}

string JavaDeveloper::_saving_last_dot(const string& s)
{
    string r(s);
    auto last_dot_position = r.find_last_of('.');

    if (last_dot_position != string::npos) { /*if found*/
        r[last_dot_position] = '%';
    }

    r = _(r);
    r[last_dot_position] = '.';

    return r;
}

heap<SourceFile> JavaDeveloper::monitor_code(
    MonitorSpecification& specification,
    const MonitorIdentifier& identifier,
    const list<Interaction>& interactions)
{
    string code;
    std::string outfile(identifier + ".java");
    list<string> synch_points;

    for (auto const& in : interactions) {
        /*TODO ATTENZIONE, LOOKUP HA ASSERT CHE FALLISCONO!*/
        synch_points.push_back(actions.lookup(in));
    }

    instantiate_monitor_template(identifier,
                                 specification.monitor_models[identifier],
                                 synch_points, code);
    return heap<SourceFile>(new JavaSourceFile(outfile, code));
}

heap<SourceFile> JavaDeveloper::thread_code(
    MonitorSpecification& specification,
    const ThreadIdentifier& identifier)
{
    string code;
    std::string outfile(identifier + ".java");
    fsp::SmartPtr<fsp::Lts> lts;
    lts = c.getLts(identifier, true);

    if (!lts) {
        string not_found(fail_reasons[process_not_found]);
        instantiate_template(not_found, "$process", identifier);
        throw runtime_error(not_found);
    }

    instantiate_thread_template(identifier,
                                specification.thread_models[identifier],
                                specification.threads[identifier],
                                specification.interaction_instance_map,
                                specification.instance_type_map,
                                lts->getAlphabet(),
                                code);
    return heap<SourceFile>(new JavaSourceFile(outfile, code));
}

void JavaDeveloper::instantiate_monitor_template(const MonitorIdentifier&
        monitor_identifier,
        MNF& model,
        const list<string>& synchpoints,
        string& monitor_code)
{
    map<Interaction, set<State>> activation_states;
    set<Interaction> interactions;
    set<InternalAction> internal_actions;

    for (MNF::const_iterator i = model.begin(); i != model.end(); i++) {
        map<Interaction, MNFConsequent>::const_iterator j = (i->second).begin();

        for (; j != (i->second).end(); j++) {
            interactions.insert(j->first);
            activation_states[j->first].insert(i->first);
            list<InternalAction>::const_iterator k = (j->second).internal_actions.begin();

            for (; k != (j->second).internal_actions.end(); k++) {
                internal_actions.insert(*k);
            }
        }
    }

    monitor_code = string(monitor_template);
    instantiate_template(monitor_code, "$monitor_identifier",
                         _(monitor_identifier));
    string condition_variable_declarations("");
    set<Interaction>::iterator i = interactions.begin();

    for (; i != interactions.end(); i++) {
        string new_declaration(condition_variable_declaration_template);
        instantiate_template(new_declaration,
                             "$interaction", _(actions.lookup(*i)));
        condition_variable_declarations += new_declaration;
    }

    instantiate_template(monitor_code, "$condition_variable_declarations",
                         condition_variable_declarations);
    string internal_action_definitions("");
    set<InternalAction>::iterator l = internal_actions.begin();

    for (; l != internal_actions.end(); l++) {
        string new_definition(internal_action_declaration_template);
        instantiate_template(new_definition,
                             "$internal_action", _(actions.lookup(*l)));
        internal_action_definitions += new_definition;
    }

    instantiate_template(monitor_code, "$internal_action_definitions",
                         internal_action_definitions);
    string interaction_definitions("");
    set<Interaction>::iterator m = interactions.begin();

    for (; m != interactions.end(); m++) {
        string interaction_definition(interaction_template);
        instantiate_template(interaction_definition, string("$interaction"),
                             _(actions.lookup(*m)));
        string wait_condition("");
        set<State>::iterator n = activation_states[*m].begin();
        set<State>::size_type minterm_counter = 1;

        for (; n != activation_states[*m].end(); n++) {
            string minterm(wait_condition_minterm_template);
            instantiate_template(minterm, "$state", to_string(*n));
            wait_condition += minterm;

            if (minterm_counter < activation_states[*m].size()) {
                wait_condition += " && ";
                minterm_counter++;
            }
        }

        instantiate_template(interaction_definition, "$wait_condition", wait_condition);
        string state_consequences("");
        set<State>::iterator c = activation_states[*m].begin();

        for (; c != activation_states[*m].end(); c++) {
            string case_definition(state_consequence_template);
            instantiate_template(case_definition, "$state", to_string(*c));
            string internal_actions_sequence("");
            list<InternalAction>::iterator ia = model[*c][*m].internal_actions.begin();

            for (; ia != model[*c][*m].internal_actions.end(); ia++) {
                string invocation(internal_action_invocation);
                instantiate_template(invocation, "$internal_action", _(actions.lookup(*ia)));
                internal_actions_sequence += invocation;
            }

            int next_state = model[*c][*m].next_state;
            instantiate_template(case_definition, "$internal_actions_sequence",
                                 internal_actions_sequence);
            instantiate_template(case_definition, "$next_state",
                                 to_string(model[*c][*m].next_state));
            string condition_signaling("");
            map<Interaction, MNFConsequent>::iterator sig = model[next_state].begin();

            for (; sig != model[next_state].end(); sig++) {
                string signal(condition_signaling_template);
                instantiate_template(signal, "$interaction", _(actions.lookup(sig->first)));
                condition_signaling += signal;
            }

            instantiate_template(case_definition, "$condition_signaling",
                                 condition_signaling);
            state_consequences += case_definition;
        }

        instantiate_template(interaction_definition, "$state_consequences",
                             state_consequences);
        interaction_definitions += interaction_definition;
    }

    instantiate_template(monitor_code, "$interactions", interaction_definitions);
    instantiate_template(monitor_code, "$header", limitation_of_responsibility);
}

void JavaDeveloper::instantiate_thread_template(
    const ThreadIdentifier& identifier,
    TNF& tnf,
    list<MonitorInstance> used_instances,
    map<Interaction, pair<MonitorInstance, bool>> interaction_instance_map,
    map<MonitorInstance, MonitorIdentifier> instance_type_map,
    set<Label> thread_alphabet,
    string& thread_code)
{
    thread_code = string(thread_template);
    string monitor_members("");
    string monitor_member("");
    string monitor_parameters("");
    string monitor_parameter("");
    string monitor_references_init("");
    string monitor_reference_init("");
    string private_methods("");

    for (auto& i : thread_alphabet) {
        if (interaction_instance_map.find(i) == interaction_instance_map.end()) {
            string private_method(private_method_template);
            instantiate_template(private_method, "$internal_action", _(actions.lookup(i)));
            private_methods += private_method;
        }
    }

    bool firstIteration = true;

    for (auto& i : used_instances) {
        monitor_member = string(monitor_member_template);
        monitor_parameter = string(monitor_parameter_template);
        monitor_reference_init = string(monitor_reference_init_template);
        string inst(_(i));
        string type(_(instance_type_map[i]));
        instantiate_template(monitor_member, "$monitor_class", type);
        instantiate_template(monitor_parameter, "$monitor_class", type);
        instantiate_template(monitor_parameter, "$monitor_reference", inst);
        instantiate_template(monitor_member, "$local_monitor_identifier", inst);
        instantiate_template(monitor_reference_init, "$monitor_reference", inst);
        instantiate_template(monitor_reference_init, "$local_monitor_identifier", inst);
        monitor_members += monitor_member;
        monitor_parameters +=
            (firstIteration)?
            monitor_parameter : (string(", ") + monitor_parameter);
        monitor_references_init += monitor_reference_init;
        firstIteration = false;
    }

    instantiate_template(thread_code, "$thread_identifier", _(identifier));
    instantiate_template(thread_code, "$monitor_members", monitor_members);
    instantiate_template(thread_code, "$private_methods", private_methods);
    instantiate_template(thread_code, "$monitor_parameters", monitor_parameters);
    instantiate_template(thread_code, "$monitor_references_init",
                         monitor_references_init);
    string run_method(run_method_template);
    string state_cases("");
    string choice_methods("");

    for (auto const& i : tnf) {
        string state_case("");

        if (i.second.size() == 1) {
            state_case = string(simple_case_template);
            instantiate_template(state_case, "$state", to_string(i.first));
            string action_sequence("");
            string action("");
            string transition("");
            map<ActionSequence, State>::const_iterator consequent = i.second.begin();

            for (auto const& j : consequent->first) {
                map<Interaction, pair<MonitorInstance, bool>>::const_iterator
                        access = interaction_instance_map.find(j);

                if (access == interaction_instance_map.end()) { /*if it's an internal action*/
                    action = _(actions.lookup(j)) + string("();\n");
                } else if (
                    access->second.second) { /*if the name must be composed with the instance*/
                    action = string(
                                 access->second.first + "." + _(actions.lookup(j)) + "();\n");
                } else {

                    action = _saving_last_dot(actions.lookup(j)) + string("();\n");
                }

                action_sequence += action;
            }

            if (consequent->second == -1) {
                transition = string(final_transition_template);
            } else {
                transition = state_transition_template;
                instantiate_template(transition, "$next_state", to_string(consequent->second));
            }

            instantiate_template(state_case, "$action_sequence", action_sequence);
            instantiate_template(state_case, "$transition", transition);
        } else if (i.second.size() > 1) {
            string choice_method(choice_method_template);
            instantiate_template(choice_method, "$state", to_string(i.first));
            choice_methods += choice_method;
            state_case = string(choice_case_template);
            string cases("");
            instantiate_template(state_case, "$state", to_string(i.first));
            int choice_counter = 0;

            for (auto const& k : i.second) {
                string simple_case(simple_case_template);
                string action_sequence("");
                string action("");
                string transition("");

                for (auto const& a : k.first) {
                    map<Interaction, pair<MonitorInstance, bool>>::const_iterator
                            access = interaction_instance_map.find(a);

                    if (access == interaction_instance_map.end()) {
                        action = _(actions.lookup(a)) + string("();\n");
                    } else if (access->second.second) { /*if the name must be composed with the instance*/
                        action = string(
                            access->second.first + "." + _(actions.lookup(a)) + "();\n");
                    } else {
                        action = _(actions.lookup(a)) + string("();\n");
                    }

                    action_sequence += action;
                }

                if (k.second == -1) {
                    transition = string(final_transition_template);
                } else {
                    transition = state_transition_template;
                    instantiate_template(transition, "$next_state", to_string(k.second));
                }

                instantiate_template(simple_case, "$state", to_string(choice_counter));
                choice_counter++;
                instantiate_template(simple_case, "$action_sequence", action_sequence);
                instantiate_template(simple_case, "$transition", transition);
                cases += simple_case;
            }

            instantiate_template(state_case, "$cases", cases);
        } else assert(0);

        state_cases += state_case;
    }

    instantiate_template(run_method, "$cases", state_cases);
    instantiate_template(thread_code, "$choice_methods", choice_methods);
    instantiate_template(thread_code, "$run_method", run_method);
    instantiate_template(thread_code, "$header", limitation_of_responsibility);
    return;
}

heap<Information> JavaDeveloper::visit(MonitorSpecification& ms)
{
    heap<SourceCode> output(new SourceCode());

    for (auto& m : ms.monitors) {
        output->add_file(monitor_code(ms, m.first, m.second));
    }

    for (auto const& t: ms.threads) {
        output->add_file(thread_code(ms, t.first));
    }

    return dynamic_pointer_cast<Information>(output);
}

JavaDeveloper::JavaDeveloper(FspDriver& _c) : FspcDeveloper(_c) {}
