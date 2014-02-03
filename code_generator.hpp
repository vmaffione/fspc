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

#ifndef __CODE_GENERATOR__HH
#define __CODE_GENERATOR__HH

#include <map>
#include <string>
#include <list>
#include <assert.h>
#include <iostream>

using namespace std;

#include "symbols_table.hpp"
#include "lts.hpp"

namespace codegen{

/*MNF stands for Monitor Normal Form.*/
typedef int State;
typedef int Interaction;
typedef int InternalAction;
struct MNFAntecedent{
    State state;
    Interaction interaction;
    MNFAntecedent() : state(-1), interaction(-1){}
    MNFAntecedent(int s, int i) : state(s), interaction(i){}
};

bool operator<(const MNFAntecedent& a, const MNFAntecedent& b);
ostream& operator<<(ostream& os, const MNFAntecedent& mnf_antecedent);

struct MNFConsequent{
    list<InternalAction> internal_actions;
    int next_state;
    MNFConsequent(){
        next_state = -1;
    }
};

ostream& operator<<(ostream& os, const MNFConsequent& mnf_consequent);

typedef map<State , map<Interaction, MNFConsequent> > MNF;

ostream& operator<<(ostream& os, const MNF& mnf);

enum NodeType {
    SYNCHRONIZATION_POINT,
    TRANSITORY,
    STOCHASTIC,
    HYBRID,
    ZERO_DIMENSION
};

typedef map<int, NodeType> VisitMemoization;

class MNFExtractor{
    private:
        fsp::ActionsTable& actions;
        const fsp::Lts& lts;
        vector<uint32_t> interactions;
        int state;
        vector<bool> seen; //first time: 0 0 0 0 0 0 ...
        bool firts_time;
        VisitMemoization cache;
        MNFAntecedent antecedent;
        int nest_lev;
        bool internal_actions_lookahead(int state, MNF& mnf, int& reason);
        NodeType node_type(int node, const vector<Edge>& edges);
    public:
        MNFExtractor(const fsp::Lts& _lts,
                     const vector<uint32_t>& _interactions);
        bool operator()(int node, MNF& mnf, int& reason, int nest_lev);
};

class CodeGenerator{
    private:
        fsp::ActionsTable& actions;
        /*NodeType node_type(const vector<uint32_t>& interactions,
                           const vector<Edge>& edges, string nest) const;
        bool generate_mnf(const fsp::Lts& lts,
                          const vector<uint32_t>& interactions,//must be ordered
                          const int state, //first time: 0
                          vector<bool>& seen, //first time: 0 0 0 0 0 0 ...
                          bool firts_time,
                          MNFAntecedent& antecedent,
                          MNF& mnf, int nest_lev) const;*/
        void remap_states(MNF& mnf);
        void indent(string& s);
    public:
        CodeGenerator() : actions(fsp::ActionsTable::getref()){}
        bool monitor(const fsp::Lts& lts, const list<string>& interactions,
                     MNF& monitorNormalForm, int& reason);
        bool get_monitor_representation(const fsp::Lts& lts,
                                        const list<string>& interactions,
                                        string& representation);
        bool instantiate_monitor_template(const fsp::Lts& lts,
                                          const list<string>& synchpoints,
                                          const string monitor_identifier,
                                          string& monitor_code);
        //bool thread(const Lts& lts, const list<string>& interactions);
};

}
#endif
