/*
 *  Code generator for fspc - monitor analyst
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

#include <algorithm>

using namespace std;

#include "lts.hpp"
#include "monitor_analyst.hpp"

using namespace fsp;
using namespace codegen;

string lowercase(const string& u)
{
    string l(u);
    transform(u.begin(), u.end(), l.begin(), ::tolower);
    return l;
}

#ifdef MONITOR_DEBUG
void MonitorSpecification::print()
{
    ActionsTable act(ActionsTable::getref());
    cout << "------------------------------------------<MONITORS>------------------------------------------"
         << endl;

    for (auto& m : monitors) {
        cout << "\t<" + m.first + "> offering interactions [";

        for (auto& i : m.second) cout <<  act.lookup(i) + ", ";

        cout << "] modeled by" << endl;

        for (auto& mnf : monitor_models[m.first]) {
            for (auto& i : mnf.second) {
                MNFAntecedent a(mnf.first, i.first);
                cout << "\t\t[" << a.state << "]" <<"=|" << act.lookup(a.interaction) << "|=>";

                for (auto k : i.second.internal_actions) {
                    cout << "o" << "-" << act.lookup(k) << "->";
                }

                cout << "(" << i.second.next_state << ")" << endl;
            }
        }
    }

    cout << endl;
    cout << "-------------------------------------<MONITOR INSTANCES>-------------------------------------"
         << endl;

    for (auto& instAndType : instance_type_map) {
        cout << "\t<" << instAndType.first << "> of type " << instAndType.second;
        list<string> l;

        for (auto& interAndInst : interaction_instance_map) {
            if (instAndType.first.compare(interAndInst.second.first) == 0) {
                l.push_back((interAndInst.second.second)?
                            (lowercase(interAndInst.second.first) + "." +
                             act.lookup(interAndInst.first))
                            :
                            act.lookup(interAndInst.first)
                           );
            }
        }

        cout << " offers [";

        for (auto& in : l) {
            cout << in << ", ";
        }

        cout << "]" << endl;
    }

    cout << endl;
    cout << "------------------------------------------<THREADS>------------------------------------------"
         << endl;

    for (auto& t : threads) {
        cout << "\t<" + t.first + "> depending on [";

        for (auto& i : t.second) cout <<  i + ", ";

        cout << "] modeled by" << endl;

        for (auto& tnf : thread_models[t.first]) {
            for (auto& path : tnf.second) {
                cout << "\t\t(" << tnf.first << ")";

                for (auto& action : path.first) {
                    cout << "-" << act.lookup(action) << "->";
                }

                cout << "(" << path.second << ")" << endl;
            }
        }
    }

    cout << endl;
    return;
}
#endif

MonitorAnalyst::NodeClassifier::NodeClassifier
(const fsp::Lts& _lts, const list<Interaction>& _interactions)
    : interactions(_interactions)
{
    set<Label> labels(_lts.getAlphabet());

    for (auto& i : interactions) {
        if (labels.find(i) == labels.end()) {
            throw runtime_error(string("Process ") + _lts.name +
                                string(" doesn't offer the requested labels. \
                                Use \"alpha ") + _lts.name +
                                string("\" to check the process' alphabet"));
        }
    }

    interactions.sort();
}

MonitorAnalyst::NodeType MonitorAnalyst::NodeClassifier::node_type
(int node, const vector<Edge>& edges)
{
    if (cache.find(node) != cache.end()) return cache[node];

    unsigned int interactions_count = 0;
    unsigned int edges_number = edges.size();

    if (edges_number == 0) { /*If the visited state is a terminal state*/
        return (cache[node] = ZERO_DIMENSION);
    }

    for (unsigned int i = 0; i < edges_number; i++) {
        if (binary_search(interactions.begin(), interactions.end(),
                          edges[i].action)) {
            interactions_count++;
        }
    }

    if (interactions_count == edges_number) {
        return (cache[node] = SYNCHRONIZATION_POINT);
    }

    if (interactions_count == 0 && edges_number == 1) {
        return (cache[node] = TRANSITORY);
    }

    if (interactions_count == 0 && edges_number > 1) {
        return (cache[node] = STOCHASTIC);
    }

    return (cache[node] = HYBRID);
}

MonitorAnalyst::MNFExtractor::MNFExtractor(const fsp::Lts& _lts,
        const list<Interaction>& _interactions) :
    actions(fsp::ActionsTable::getref()),
    lts(_lts), interactions(_interactions),
    seen(lts.numStates(), false),
    classifier(_lts, _interactions) {}

void MonitorAnalyst::MNFExtractor::internal_actions_lookahead(State state,
        MNF& mnf)
{
    unsigned int marked_state = state;
    vector<Edge> edges = lts.get_children(state);

    while (true) {
        switch (classifier.node_type(state, edges)) {
            case TRANSITORY: {
                if (edges[0].dest != marked_state) {
                    mnf[antecedent.state][antecedent.interaction]
                    .internal_actions.push_back(edges[0].action);
                    state = edges[0].dest;
                    edges = lts.get_children(state);
                } else {
                    throw runtime_error(fail_reasons[internal_actions_cycle]);
                }
            }
            break;

            case SYNCHRONIZATION_POINT: {
                mnf[antecedent.state][antecedent.interaction]
                .next_state = state;
                return;
            }

            case STOCHASTIC: {
                throw runtime_error(fail_reasons[stochastic_node]);
            }

            case HYBRID: {
                throw runtime_error(fail_reasons[hybrid_interactions]);
            }

            case ZERO_DIMENSION: {
                throw runtime_error(fail_reasons[end_or_fail_state_detected]);
            }

            default: {
                throw runtime_error(fail_reasons[unknown_state]);
            }
        }
    }

    assert(0);
    return;
}

void MonitorAnalyst::MNFExtractor::remap_states(MNF& mnf)
{
    mnf.erase(-1);
    State state_generator = 0;
    map<State, State> state_map;
    MNF::iterator i = mnf.begin();

    for (; i != mnf.end(); i++) {
        state_map[i->first] = state_generator++;
    }

    map<State, State>::iterator k = state_map.begin();

    for (; k != state_map.end(); k++) {
        if (k->first != k->second) mnf[k->second] = mnf[k->first];

        map<Interaction, MNFConsequent>::iterator j = (mnf[k->second]).begin();

        for (; j != (mnf[k->second]).end(); j++) {
            (j->second).next_state = state_map[(j->second).next_state];
        }

        if (k->first != k->second) mnf.erase(k->first);
    }

    return;
}

void MonitorAnalyst::MNFExtractor::operator()(State state, MNF& mnf,
        bool recursive_call)
{
    if (!lts.isDeterministic()) {
        throw runtime_error(fail_reasons[stochastic_node]);
    }

    const vector<Edge>& edges = lts.get_children(state);

    switch (classifier.node_type(state, edges)) {
        case HYBRID:
            throw runtime_error(fail_reasons[hybrid_interactions]);

        case STOCHASTIC: {
            throw runtime_error(fail_reasons[stochastic_node]);
        }

        case SYNCHRONIZATION_POINT: {
            mnf[antecedent.state][antecedent.interaction].next_state = state;

            /*the first time -1 -1 is used as antecedent (default constructed)*/
            if (seen[state]) return;

            seen[state] = true;

            for (auto& e : edges) {
                antecedent.state = state;
                antecedent.interaction = e.action;
                this->operator()(e.dest, mnf, true);
            }

            if (!recursive_call) remap_states(mnf);

            return;
        }

        case TRANSITORY: {
            /*If the first state we visit is deterministic, the monitor
             * initial state would not be a point of synchronization, so the
             * monitor would expose an active behaviour*/
            if (state == 0) throw runtime_error(fail_reasons[active_control]);

            if (seen[state]) {
                /*We've reached an already seen state through an interal action,
                 * that is, passing through a TRANSITORY. These are the possible
                 * scenarios:
                 * - the reached state is a synchronization point -> OK
                 * - the reached state is transitory but leads to a
                 * synchronization point -> OK, append the path to the
                 * antecedent
                 * - the reached state is a TRANSITORY and there's a cycle of
                 * internal actions -> throw an error, not in Monitor Normal Form!*/
                internal_actions_lookahead(state, mnf);

                if (!recursive_call) remap_states(mnf);

                return;
            }

            seen[state] = true;
            mnf[antecedent.state][antecedent.interaction]
            .internal_actions.push_back(edges[0].action);
            this->operator()(edges[0].dest, mnf, true);

            if (!recursive_call) remap_states(mnf);

            return;
        }

        case ZERO_DIMENSION:
            throw runtime_error(fail_reasons[end_or_fail_state_detected]);

        default:
            assert(0);
    }

    assert(0);
    return;
}

MonitorAnalyst::TNFExtractor::TNFExtractor(const fsp::Lts& _lts) :
    lts(_lts), seen(lts.numStates(), false), currentRoot(-1) {}

/* node is guaranteed (am I sure?) to be a transitory node (just 1 exiting action) */
void MonitorAnalyst::TNFExtractor::complete_path(State node, TNF& tnf)
{
    ActionSequence path = ActionSequence();
    State entry = node;
    vector<Edge> edges;
    set<State> visited;

    do { /*TODO: what happens if we have to complete a path for a END state?*/
        visited.insert(node);
        edges = lts.get_children(node);

        if (edges.size() > 0) {
            path.push_back(edges[0].action);
            node = edges[0].dest;
        }
    } while (tnf.find(node) == tnf.end() && visited.find(node) == visited.end());

    tnf[entry][path] = node;
    return;
}

void MonitorAnalyst::TNFExtractor::remap_states(TNF& tnf)
{
    tnf.erase(-1);
    State state_generator = 0;
    map<State, State> state_map;
    TNF::iterator i = tnf.begin();

    for (; i != tnf.end(); i++) {
        state_map[i->first] = state_generator++;
    }

    map<State, State>::iterator k = state_map.begin();

    for (; k != state_map.end(); k++) {
        if (k->first != k->second) tnf[k->second] = tnf[k->first];

        map<ActionSequence, State>::iterator j = (tnf[k->second]).begin();

        for (; j != (tnf[k->second]).end(); j++) {
            if (j->second != -1) j->second = state_map[j->second];
        }

        if (k->first != k->second) tnf.erase(k->first);
    }

    return;
}

void MonitorAnalyst::TNFExtractor::operator()(int node, TNF& tnf,
        bool recursive_call)
{
    if (!lts.isDeterministic())
        throw runtime_error(fail_reasons[non_deterministic_thread]);

    if (seen[node]) {
        tnf[currentRoot][currentPath] = node;

        if (tnf.find(node) == tnf.end()) {
            complete_path(node, tnf);
        }

        if (!recursive_call) remap_states(tnf);

        return;
    }

    seen[node] = true;
    const vector<Edge>& edges = lts.get_children(node);

    switch (edges.size()) {
        case 0: {
            tnf[currentRoot][currentPath] = -1;

            if (!recursive_call) remap_states(tnf);

            return;
        }

        case 1: {
            if (tnf.empty()) {
                tnf[currentRoot][currentPath] = node;
                currentRoot = node;
                currentPath = ActionSequence();
            }

            currentPath.push_back(edges[0].action);
            this->operator()(edges[0].dest, tnf, true);

            if (!recursive_call) remap_states(tnf);

            return;
        }

        default: {
            tnf[currentRoot][currentPath] = node;

            for (auto& e : edges) {
                currentRoot = node;
                currentPath = ActionSequence();
                currentPath.push_back(e.action);
                this->operator()(e.dest, tnf, true);
            }

            if (!recursive_call) remap_states(tnf);

            return;
        }
    }

    assert(0);
    return;
}

void MonitorAnalyst::monitor(const Lts& lts,
                             const list<Interaction>& interactions, MNF& mnf)
{
    MNFExtractor extractor(lts, interactions);
    extractor(0, mnf);
    return;
}

void MonitorAnalyst::thread(const fsp::Lts& lts, TNF& tnf)
{
    TNFExtractor extractor(lts);
    extractor(0, tnf);
    return;
}

MonitorAnalyst::MonitorAnalyst(FspDriver& _c) : FspcAnalyst(_c) {}

heap<Information> MonitorAnalyst::visit(const UserRequirements& r)
{
    heap<MonitorSpecification> specification(new MonitorSpecification());
    MonitorIdentifier currentMonitorName;
    ThreadIdentifier currentThreadName;
    MonitorIdentifier currentInstanceType;
    const Requirements& args(r.userRequirements);
    ParseMode mode = monitor_thread_instance;
    map<MonitorInstance, MonitorIdentifier> instances;
    map<MonitorInstance, MonitorIdentifier> implicit_instances;
    unsigned int current = 0;

    while (current < args.size()) {
        switch (mode) {
            case monitor_thread_instance: {
                if (args[current].compare("-m") == 0) {
                    mode = monitorType;
                } else if (args[current].compare("-t") == 0) {
                    mode = threadType;
                } else if (args[current].compare("-i") == 0) {
                    mode = instanceType;
                } else {
                    throw runtime_error(string("Expected -m, -t or -i flag, ") +
                                        args[current] + string(" found"));
                }
            }
            break;

            case monitorType: {
                currentMonitorName = string(args[current]);
                specification->monitors.insert(
                    pair<MonitorIdentifier, list<Interaction>>
                    (currentMonitorName, list<Interaction>())
                );
                mode = monitor_thread_instance_monitorInteraction;
            }
            break;

            case threadType: {
                currentThreadName = string(args[current]);
                specification->threads.insert(
                    pair<ThreadIdentifier, list<MonitorInstance>>
                    (currentThreadName, list<MonitorInstance>())
                );
                mode = monitor_thread_instance_monitorInstance;
            }
            break;

            case instanceType: {
                currentInstanceType = string(args[current]);
                mode = monitor_thread_instance_instanceIdentifier;
            }
            break;

            case monitor_thread_instance_monitorInteraction: {
                if (args[current].compare("-m") == 0) {
                    mode = monitorType;
                } else if (args[current].compare("-t") == 0) {
                    mode = threadType;
                } else if (args[current].compare("-i") == 0) {
                    mode = instanceType;
                } else {
                    specification->monitors[currentMonitorName]
                    .push_back(actions.lookup(args[current]));
                }
            }
            break;

            case monitor_thread_instance_monitorInstance: {
                if (args[current].compare("-m") == 0) {
                    mode = monitorType;
                } else if (args[current].compare("-t") == 0) {
                    mode = threadType;
                } else if (args[current].compare("-i") == 0) {
                    mode = instanceType;
                } else {
                    specification->threads[currentThreadName]
                    .push_back(args[current]);
                }
            }
            break;

            case monitor_thread_instance_instanceIdentifier: {
                if (args[current].compare("-m") == 0) {
                    mode = monitorType;
                } else if (args[current].compare("-t") == 0) {
                    mode = threadType;
                } else if (args[current].compare("-i") == 0) {
                    mode = instanceType;
                } else {
                    instances[args[current]] = currentInstanceType;
                }
            }
            break;
        }

        current++;
    }

    if (mode == monitorType || mode == threadType || mode == instanceType) {
        throw runtime_error("Process expected");
    }

    for (auto const& m : specification->monitors) {
        implicit_instances[lowercase(m.first)] = m.first;
        specification->instance_type_map[lowercase(m.first)] = m.first;
        MNF mnf;
        SmartPtr<Lts> lts = c.getLts(m.first, true);

        if (!lts) {
            string not_found(fail_reasons[process_not_found]);
            instantiate_template(not_found, "$process", m.first);
            throw runtime_error(not_found);
        }

        monitor(*lts, specification->monitors[m.first], mnf);
        specification->monitor_models[m.first] = mnf;
    }

    for (auto const& i : instances) {
        if (specification->monitors.find(i.second) ==
                specification->monitors.end())
            throw runtime_error(string("Instance ") + i.first +
                                string(" not of monitor type"));

        specification->instance_type_map[i.first] = i.second;
    }

    for (auto const& t: specification->threads) {
        TNF tnf;
        SmartPtr<Lts> lts = c.getLts(t.first, true);

        if (!lts) {
            string not_found(fail_reasons[process_not_found]);
            instantiate_template(not_found, "$process", t.first);
            throw runtime_error(not_found);
        }

        thread(*lts, tnf);
        specification->thread_models[t.first] = tnf;

        for (auto& i : t.second) {
            if (instances.find(i) != instances.end()) {
                specification->instance_type_map[i] = instances[i];
                set<Interaction> instance_interactions =
                    (c.getLts(instances[i], true))->getAlphabet();

                for (auto& j : instance_interactions) {
                    Interaction complete = actions.lookup(i + "." + actions.lookup(j));
                    specification->interaction_instance_map[complete] =
                        pair<MonitorInstance, bool>(i, false);
                }
            } else if (implicit_instances.find(i) != implicit_instances.end()) {
                set<Interaction> instance_interactions =
                    (c.getLts(implicit_instances[i], true))->getAlphabet();

                for (auto& j : instance_interactions) {
                    specification->interaction_instance_map[j] =
                        pair<MonitorInstance, bool>(i, true);
                }
            } else {
                throw runtime_error(string("Undeclared monitor instance: ") + i);
            }
        }
    }

#ifdef MONITOR_DEBUG
    specification->print();
#endif
    return dynamic_pointer_cast<Information>(specification);
}
