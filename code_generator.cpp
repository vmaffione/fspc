/*
 *  fspc code generator
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
#include <vector>
#include <list>
#include <sstream>

#include "code_generator.hpp"
#include "java.hpp"

using namespace std;
using namespace fsp;
using namespace codegen;

bool codegen::operator<(const MNFAntecedent& a, const MNFAntecedent& b)
{
    return a.state < b.state? true :
           a.state == b.state? a.interaction < b.interaction : false;
}

ostream& codegen::operator<<(ostream& os, const MNFAntecedent& mnf_antecedent)
{
    ActionsTable& actions = ActionsTable::getref();
    if(mnf_antecedent.state == -1 && mnf_antecedent.interaction == -1){
        os << "Starting from state: ";
        return os;
    }
    os << "[" << mnf_antecedent.state << "]"
       <<"=|" << actions.lookup(mnf_antecedent.interaction) << "|=>";
    return os;
}

ostream& codegen::operator<<(ostream& os, const MNFConsequent& mnf_consequent)
{
    ActionsTable& actions = ActionsTable::getref();
    for(list<int>::const_iterator i = mnf_consequent.internal_actions.begin();
        i != mnf_consequent.internal_actions.end(); i++){
        os << "o" << "-" << actions.lookup(*i) << "->";
    }
    os << "(" << mnf_consequent.next_state << ")";
    return os;
}

ostream& codegen::operator<<(ostream& os, const MNF& mnf)
{
    for(MNF::const_iterator i = mnf.begin(); i != mnf.end(); i++){
        map<Interaction, MNFConsequent>::const_iterator j = (i->second).begin();
        for(; j != (i->second).end(); j++){
            MNFAntecedent a(i->first, j->first);
            os << a << j->second << endl;
        }
    }
    return os;
}





const int internal_actions_cycle = 0;
const int hybrid_interactions = 1;
const int active_control = 2;
const int end_or_fail_state_detected = 3;
const int unknown_state = 4;
const int stochastic_node = 5;
const char* MFN_fail_reasons[6] = {
    "Endless execution of local actions",
    "Hybrid choice between local action and interaction",
    "Candidate monitor engages in active control",
    "End or fail state detected",
    "Unknown state",
    "Candidate monitor exhibits stochastic behaviour"
};

MNFExtractor::MNFExtractor(const fsp::Lts& _lts,
                           const vector<uint32_t>& _interactions) :
                           actions(fsp::ActionsTable::getref()),
                           lts(_lts), interactions(_interactions),
                           seen(lts.size(), false),
                           nest_lev(0){
    sort(interactions.begin(), interactions.end());
}

codegen::NodeType MNFExtractor::node_type(int node, const vector<Edge>& edges)
{
    if(cache.find(node) != cache.end()) return cache[node];
    unsigned int interactions_count = 0;
    unsigned int edges_number = edges.size();
    if(edges_number == 0){//If the visited state is a terminal state
        return (cache[node] = ZERO_DIMENSION);
    }
    for(unsigned int i = 0; i < edges_number; i++){
        if(binary_search(interactions.begin(), interactions.end(),
                         edges[i].action)){
            interactions_count++;
        }
    }
    if(interactions_count == edges_number){
        return (cache[node] = SYNCHRONIZATION_POINT);
    }
    if(interactions_count == 0 && edges_number == 1){
        return (cache[node] = TRANSITORY);
    }
    if(interactions_count == 0 && edges_number > 1){
        return (cache[node] = STOCHASTIC);
    }
    return (cache[node] = HYBRID);
}

bool MNFExtractor::internal_actions_lookahead(int state, MNF& mnf, int& reason)
{
    unsigned int marked_state = state;
    vector<Edge> edges = lts.get_children(state);
    while(true){
        switch(node_type(state, edges)){
            case TRANSITORY:{
                if(edges[0].dest != marked_state){
                    mnf[antecedent.state][antecedent.interaction]
                        .internal_actions.push_back(edges[0].action);
                    state = edges[0].dest;
                    edges = lts.get_children(state);
                } else {
                    reason = internal_actions_cycle;
                    return false;
                }
            }
            break;
            case SYNCHRONIZATION_POINT:{
                mnf[antecedent.state][antecedent.interaction]
                    .next_state = state;
                return true;
            }
            break;
            case STOCHASTIC:{
                reason = stochastic_node;
                return false;
            }
            case HYBRID:{
                reason = hybrid_interactions;
                return false;
            }
            case ZERO_DIMENSION:{
                reason = end_or_fail_state_detected;
                return false;
            }
            default:{
                reason = unknown_state;
                return false;
            }
        }
    }
    return false;
}

bool MNFExtractor::operator()(int state, MNF& mnf, int& reason, int nest_lev){
    string nest(nest_lev, ' ');
    //cout << nest << "state " << state << endl;
    const vector<Edge>& edges = lts.get_children(state);
    switch(node_type(state, edges))
    {
        case HYBRID:{
            reason = hybrid_interactions;
            return false;
        } break;
        case STOCHASTIC:{
            reason = stochastic_node;
            return false;
        } break;
        case SYNCHRONIZATION_POINT: {
            //cout << nest << " synchronization point" << endl;
            mnf[antecedent.state][antecedent.interaction].next_state = state;
            //the first time the antecedent -1 -1 is used (default constructed)
            //cout << nest << " antecedent: " << antecedent << endl;
            if(seen[state]){
                //cout << nest << " already visited" << endl;
                return true;
            }
            seen[state] = true;
            for(unsigned int i = 0; i < edges.size(); i++){
                antecedent.state = state;
                antecedent.interaction = edges[i].action;
                /*cout << nest << " recursive call on ("
                     << edges[i].dest << ", " << actions.lookup(edges[i].action)
                     << ") with antecedent " << antecedent << endl;*/
                if(!(this->operator()(edges[i].dest, mnf, reason, nest_lev))){
                    return false;
                }
            }
            return true;
        } break;
        case TRANSITORY:{
            if(state == 0){//first time
                /*If the first state we visit is deterministic, the monitor
                 * initial state would not be a point of synchronization, so the
                 * monitor would expose an active behaviour*/
                reason = active_control;
                return false;
            }
            if(seen[state]){
                //Sono giunto ad uno stato gia' visitato passando da una azione
                //interna, quindi passando da un TRANSITORY. Scenari possibili:
                //- lo stato raggiunto e' un punto di sincronizzazione -> OK
                //- lo stato raggiunto e' transitorio ma conduce a un punto di
                //  sincronizzazione -> ok, appendi il path all'antecedente
                //- lo stato raggiunto e' un transitorio in ciclo diretto ->
                // errore! Ciclo infinito di azioni interne!
                // Se entro qui, devo discernere quale dei tre casi e' vero,
                // nel terzo dare errore, nel secondo fare lookahead del path,
                // nel primo e' sufficiente fare ricorsione. NB: dai un'occhiata
                // all'algoritmo di Tarjan
                return internal_actions_lookahead(state, mnf, reason);
            }
            seen[state] = true;
            /*cout << nest << " appending internal action "
                 << actions.lookup(edges[0].action) << " to antecedent "
                 << antecedent << endl;*/
            mnf[antecedent.state][antecedent.interaction]
                .internal_actions.push_back(edges[0].action);
            /*cout << nest << " recursive call on (" << edges[0].dest << ", "
                 << actions.lookup(edges[0].action) << ") with antecedent "
                 << antecedent << endl;*/
            return this->operator()(edges[0].dest, mnf, reason, nest_lev+1);
        } break;
        case ZERO_DIMENSION:{
            reason = end_or_fail_state_detected;
            return false;
        } break;
        default: assert(0);
    }
    return false;
}













/*codegen::NodeType CodeGenerator::node_type(const vector<uint32_t>& interactions,
                              const vector<Edge>& edges, string nest) const
{
    unsigned int interactions_count = 0;
    unsigned int edges_number = edges.size();
    if(edges_number == 0){//If the visited state is a terminal state
        return ZERO_DIMENSION;
    }
    for(unsigned int i = 0; i < edges_number; i++){
        if(binary_search(interactions.begin(), interactions.end(),
                         edges[i].action)){
            interactions_count++;
            cout << nest << " interaction found: "
                 << actions.lookup(edges[i].action) << endl;
        }
    }
    if(interactions_count == edges_number) return SYNCHRONIZATION_POINT;
    if(interactions_count == 0 && edges_number == 1) return TRANSITORY;
    return HYBRID;
}

bool CodeGenerator::generate_mnf(const Lts& lts,
                  const vector<uint32_t>& interactions,//must be ordered
                  const int state, //first time: 0
                  vector<bool>& seen, //first time: 0 0 0 0 0 0 ...
                  bool first_time,
                  MNFAntecedent& antecedent,
                  MNF& mnf, int nesting_level) const{
    //Node analysis
    string nest(nesting_level, ' ');
    cout << nest << "state " << state << endl;
    const vector<Edge>& edges = lts.get_children(state);
    const NodeType node_nature = node_type(interactions, edges, nest);
    switch(node_nature)
    {
        case HYBRID:
            return false;
        case SYNCHRONIZATION_POINT: {
            cout << nest << " synchronization point" << endl;
            if(!first_time){
                mnf[antecedent].next_state = state;
                cout << nest << " antecedent: " << antecedent << endl;
            } else {
                cout << nest << " first node, no antecedent" << endl;
                first_time = false;
            }
            if(seen[state]){
                cout << nest << " already visited" << endl;
                return true;
            }
            seen[state] = true;
            for(unsigned int i = 0; i < edges.size(); i++){
                antecedent.state = state;
                antecedent.interaction = edges[i].action;
                cout << nest << " recursive call on ("
                     << edges[i].dest << ", " << actions.lookup(edges[i].action)
                     << ") with antecedent " << antecedent << endl;
                if(!generate_mnf(lts, interactions, edges[i].dest, seen, false,
                                 antecedent, mnf, nesting_level+1)){
                    return false;
                }
            }
            return true;
        } break;
        case TRANSITORY:{
            if(first_time){
                cout << nest << " first node deterministic? MNF impossible!"
                     << endl;
                return false;
            }
            if(seen[state]){
                cout << nest
                     << " transitory state already visited, completing path..."
                     << endl;
                int lookahead_state = state;
                vector<Edge> lookahead_children = edges;
                while(node_type(interactions, lookahead_children, nest) == TRANSITORY){
                    //attenzione!  SE CI SONO CICLI DI TAU VA IN LOOP INFINITO
                    mnf[antecedent]
                        .internal_actions
                        .push_back(lookahead_children[0].action);
                    lookahead_state = lookahead_children[0].dest;
                    lookahead_children = lts.get_children(lookahead_state);
                }
                mnf[antecedent].next_state = lookahead_state;
                return true;
            }
            seen[state] = true;
            cout << nest << " appending internal action "
                 << actions.lookup(edges[0].action) << " to antecedent "
                 << antecedent << endl;
            mnf[antecedent].internal_actions.push_back(edges[0].action);
            cout << nest << " recursive call on (" << edges[0].dest << ", "
                 << actions.lookup(edges[0].action) << ") with antecedent "
                 << antecedent << endl;
            return generate_mnf(lts, interactions, edges[0].dest, seen, false,
                                antecedent, mnf, nesting_level+1);
        } break;
        default: assert(0);
    }
    return false;
}*/

bool CodeGenerator::monitor(const Lts& lts,
                            const list<string>& labels,
                            MNF& monitorNormalForm, int& reason){
    /*<stampa>
    cout << "Interazioni indicate: ";
    for(list<string>::const_iterator i = labels.begin(); i != labels.end(); i++){
        cout << *i << ", ";
    }
    cout << endl;
    </stampa>*/

    vector<uint32_t> interactions(labels.size());
    list<string>::const_iterator l = labels.begin();
    for(unsigned int i = 0; i < labels.size(); i++){
        interactions[i] = actions.lookup(*l);
        l++;
    }
    MNFExtractor extractor(lts, interactions);
    if(extractor(0, monitorNormalForm, reason, 0)){
        remap_states(monitorNormalForm);
        return true;
    }
    return false;
}

inline string to_string(const int& a){
    std::stringstream s;
    s << a;
    return s.str();
}

void CodeGenerator::remap_states(MNF& mnf){
    mnf.erase(-1);
    State state_generator = 0;
    map<State, State> state_map;
    MNF::iterator i = mnf.begin();
    for(; i != mnf.end(); i++){
        state_map[i->first] = state_generator++;
    }
    map<State, State>::iterator k = state_map.begin();
    for(; k != state_map.end(); k++){
        if(k->first != k->second) mnf[k->second] = mnf[k->first];
        map<Interaction, MNFConsequent>::iterator j = (mnf[k->second]).begin();
        for(; j != (mnf[k->second]).end(); j++){
            (j->second).next_state = state_map[(j->second).next_state];
        }
        if(k->first != k->second) mnf.erase(k->first);
    }
    return;
}

void CodeGenerator::indent(string& s){
    int level = 0;
    string::size_type size = s.size();
    for(string::size_type i = 0; i < size; i++){
        switch(s[i]){
            case '{':{
                level++;
                if(i < size-1 && s[i+1] == '}') level--;
            } break;
            case '\n':{
                if(i < size-1 && s[i+1] == '}') level--;
                s.insert(i+1, string(level, '\t'));
                size += level;
                i += level;
            } break;
            default: if(i < size-1 && s[i+1] == '}') level--;
        }
    }
    return;
}

bool CodeGenerator::get_monitor_representation(const fsp::Lts& lts,
                                            const list<string>& interactions,
                                            string& representation)
{
    MNF monitorNormalForm;
    int reason = -1;
    if(monitor(lts, interactions, monitorNormalForm, reason)){
        string body("");
        MNF::iterator i = monitorNormalForm.begin();
        for(;i != monitorNormalForm.end(); i++){
            body = body + "S" + to_string(i->first) + " = (";
            map<Interaction, MNFConsequent>::iterator j = (i->second).begin();
            for(; j != (i->second).end(); j++){
                body = body + ((j != (i->second).begin())? "\n  | " : "")
                            + actions.lookup(j->first) + "->";
                list<int>::iterator k = (j->second).internal_actions.begin();
                for(; k != (j->second).internal_actions.end(); k++){
                    body = body + actions.lookup(*k) + "->";
                }
                body = body + "S" + to_string((j->second).next_state);
            }
            MNF::iterator current = i;
            i++;
            body = body + ")" + ((i != monitorNormalForm.end())? ",": ".") +
                          "\n";
            i = current;
        }
        representation = string(lts.name + " = S0,\n" + body);
        return true;
    }
    representation = string(MFN_fail_reasons[reason]);
    return false;
}

bool CodeGenerator::instantiate_monitor_template(const fsp::Lts& lts,
                                                   const list<string>& synchpoints,
                                                   const string monitor_identifier,
                                                   string& monitor_code)
{
    MNF model;
    int reason = -1;
    if(!monitor(lts, synchpoints, model, reason)){
        monitor_code = string(MFN_fail_reasons[reason]);
        return false;
    }
    map<Interaction, set<State> > activation_states;
    set<Interaction> interactions;
    set<InternalAction> internal_actions;
    for(MNF::const_iterator i = model.begin(); i != model.end(); i++){
        map<Interaction, MNFConsequent>::const_iterator j = (i->second).begin();
        for(; j != (i->second).end(); j++){
            interactions.insert(j->first);
            activation_states[j->first].insert(i->first);
            list<InternalAction>::const_iterator k = (j->second).internal_actions.begin();
            for(; k != (j->second).internal_actions.end(); k++){
                internal_actions.insert(*k);
            }
        }
    }
    monitor_code = string(monitor_template);
    instantiate_template_string(monitor_code, "$monitor_identifier",
                               monitor_identifier);
    string condition_variable_declarations("");
    set<Interaction>::iterator i = interactions.begin();
    for(; i != interactions.end(); i++){
        string new_declaration(condition_variable_declaration_template);
        instantiate_template_string(new_declaration,
                                    "$interaction", actions.lookup(*i));
        condition_variable_declarations += new_declaration;
    }
    instantiate_template_string(monitor_code, "$condition_variable_declarations",
                               condition_variable_declarations);
    string internal_action_definitions("");
    set<InternalAction>::iterator l = internal_actions.begin();
    for(; l != internal_actions.end(); l++){
        string new_definition(internal_action_declaration_template);
        instantiate_template_string(new_definition,
                                    "$internal_action", actions.lookup(*l));
        internal_action_definitions += new_definition;
    }
    instantiate_template_string(monitor_code, "$internal_action_definitions",
                               internal_action_definitions);
    string interaction_definitions("");
    set<Interaction>::iterator m = interactions.begin();
    for(; m != interactions.end(); m++){
        string interaction_definition(interaction_template);
        instantiate_template_string(interaction_definition, string("$interaction"), actions.lookup(*m));
        string wait_condition("");
        set<State>::iterator n = activation_states[*m].begin();
        set<State>::size_type minterm_counter = 1;
        for(; n != activation_states[*m].end(); n++){
            string minterm(wait_condition_minterm_template);
            instantiate_template_string(minterm, "$state", to_string(*n));
            wait_condition += minterm;
            if(minterm_counter < activation_states[*m].size()){
                wait_condition += " && ";
                minterm_counter++;
            }
        }
        instantiate_template_string(interaction_definition, "$wait_condition", wait_condition);
        string state_consequences("");
        set<State>::iterator c = activation_states[*m].begin();
        for(; c != activation_states[*m].end(); c++){
            string case_definition(state_consequence_template);
            instantiate_template_string(case_definition, "$state", to_string(*c));
            string internal_actions_sequence("");
            list<InternalAction>::iterator ia = model[*c][*m].internal_actions.begin();
            for(; ia != model[*c][*m].internal_actions.end(); ia++){
                string invocation(internal_action_invocation);
                instantiate_template_string(invocation, "$internal_action", actions.lookup(*ia));
                internal_actions_sequence += invocation;
            }
            instantiate_template_string(case_definition, "$internal_actions_sequence", internal_actions_sequence);
            instantiate_template_string(case_definition, "$next_state", to_string(model[*c][*m].next_state));
            state_consequences += case_definition;
        }
        instantiate_template_string(interaction_definition, "$state_consequences", state_consequences);
        interaction_definitions += interaction_definition;
    }
    instantiate_template_string(monitor_code, "$interactions", interaction_definitions);
    indent(monitor_code);
    return true;
}
