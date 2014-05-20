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

#ifndef __FSPC_CODEGEN_MONITOR_ANALYST__HH
#define __FSPC_CODEGEN_MONITOR_ANALYST__HH
/*#define MONITOR_DEBUG*/

#include "lts.hpp"
#include "fspc_experts.hpp"
#include "symbols_table.hpp"

#include <map>
#include <list>
#include <vector>
#include <string>

namespace codegen {

/*___________________________About Monitor Normal Form__________________________
 * LTS labels can be divided in:
 * - internal actions;
 * - interactions;
 * Iternal actions are not meant to be shared with other LTSs, while
 * interactions are meant to be shared between two or more LTSs which must
 * synchronize before firing the interaction (each one must reach a state
 * offering that ineraction).
 * A monitor is a construct which offers syncrhonized methods. A monitor has
 * several states. Each state admits a certain set of synchronized methods to
 * be called. When one of the permitted methods is called, the monitor traverses
 * several internal states in the which no synchronized method can be executed,
 * and execution requests are enqueued. During this traversal, the monitor
 * performs a deterministic serie of internal actions. A monitor can perform
 * internal actions only as a consequence of a synchronized method invocation.
 * In order to represent a monitor, which is a passive entity, an LTS must
 * satisfy the following requirements
 * -------------------------------[MNF Requirements]----------------------------
 * - let LTS be an LTS
 * - let S be a state of LTS
 * - let alpha(LTS) be the set of all labels of LTS
 * - let out(S) be the exit set of S (that is, the set of labels active in S)
 * - let in(S) be the entry set of S (that is, the set which lead to S)
 * - let #(out(S)) be the arity of out(S)
 * - let LTSint : alpha(LTS) -> {true, false} be a function stating wheter a
 * label is an interaction (true) or an internal action (false)
 * - let (S1)-a->(S2) denote the existence of a label "a" both in out(S1) and in
 * in(S2);
 * - let (S1)={a,b,c}=>(S2) denote (S1)-a->(X)-b->(Y)-c->(S2), that is, the
 * existence of a path between S1 and S2 which is composed by the labels in the
 * specified set;
 * - let a path be a set of labels;
 *
 * Then the following conditions must hold:
 *
 * [1] for each S in LTS, #(out(S)) > 0
 *
 *     (that is: no End, Stop or Error state is admitted);
 *
 * [2] for each S in LTS:
 *         either (LTSint(l) = true for all l in out(S))
 *         or (LTSint(l) = false for all l in out(S))
 *
 *     (that is: out(S) must be composed of all internal actions or all
 *      interactions, no hybrid composition is admitted);
 *
 * [3] LTSint(l) = false for some l in out(S) => #(out(S)) = 1
 *
 *     (that is: an exiting set can contain just one internal action);
 *
 * [4] for all possible path P:
 *         if exists S such that S=P=>S:
 *             there must exist at least one l in P such that LTSint(l) = true
 *
 *     (that is: no cycle composed exclusively by internal actions is admitted);
 * ---------------------------------(Comments)----------------------------------
 * to be filled...
 * ----------------------------[What MNF looks like]----------------------------
 * If an valid FSP description of an LTS can be rearranged in a form satisfying
 * the following EBNF syntax:
 *
 * Mnf :=
 *     Identifier '=' State StateDefinitions '.'
 * StateDefinitions :=
 *     (',' StateDefinition)+
 * StateDefinition :=
 *     State '=' '(' Alternative ('|' Alternative)* ')'
 * Alternative :=
 *     Interaction '->' (InternalAction '->')* State
 *
 * then the LTS is in monitor normal form. An example (interactions enclosed
 * by __):
 *
 * CANDIDATE_MONITOR = S0,
 * S0 = (__interaction1__->action1->action2->S2 |
 *       __interaction2__->action3->action2->action1->S1 |
 *       __interaction3__->action2->action1->S0),
 * S1 = (__interaction2__->S1 |
 *       __interaction4__->action3->action4->action5->S1),
 * S2 = (__interaction1__->S0 |
 *       __interaction4__->action5->S2 |
 *       __interaction5__->S1).
 * ------------------------------[Data Structure]-------------------------------
 * Each pair <state, interaction> (the "antecedent") identifies a pair
 * <action path, ending state> (the "consequent"). The used data structure is a
 * map which maps states to maps which map interactions to consequents:
 * .____________________________________________________________________.
 * |<Key:State> | <Value:Possibilities>                                 |
 * |            |.......................................................|
 * |            | <Key:Interaction> . <Value:ActionPath->EndingState>   |
 * |____________|___________________.___________________________________|
 * |         \    / interaction1   \/ [action1, action2]          -> S2 |
 * |          \  /_________________/\___________________________________|
 * |     S0    \/ interaction2     \/ [action3, action2, action1] -> S1 |
 * |           /\__________________/\___________________________________|
 * |          /  \ interaction3    \/ [action2, action1]          -> S0 |
 * |_________/    \________________/\___________________________________|
 * |         \    / interaction2   \/ []                          -> S1 |
 * |     S1   \__/_________________/\___________________________________|
 * |          /  \ interaction4    \/ [action3, action4, action5] -> S1 |
 * |_________/    \________________/\___________________________________|
 * |         \    /                \/                                   |
 * .          .      and so on     ..             and so on             .
 * . and so on .                   ..                                   .
 * .            .                  ..                                   .
 *
 * --------------------------[About Thread Normal Form]-------------------------
 * There are way less restrictions for threads.
 * For each state S:
 * - if #(out(S)) = 0: the thread stops;
 * - if #(out(S)) = 1: the thread engages in some action/interaction;
 * - if #(out(S)) > 1: the thread waits for some event (for example, user input)
 *                     to be used to identify the path to follow;
 */

/**
 * @class MNFAntecedent
 * @brief Struct used in the construction of the Monitor Normal Form
 *
 * The struct models an antecedent in the monitor normal form, that is, a
 * pair of state and interaction.
 **/
struct MNFAntecedent {
    State state;
    Interaction interaction;
    MNFAntecedent() : state(-1), interaction(-1) {}
    MNFAntecedent(State s, Interaction i) : state(s), interaction(i) {}
};

/**
 * @class MNFConsequent
 * @brief Struct used in the construction of the Monitor Normal Form
 *
 * The struct maintains path of internal actions which are deterministic
 * consequence of the invocation of an interaction on a particular state,
 * together to the state to the destination state.
 **/
struct MNFConsequent {
    std::list<InternalAction> internal_actions;
    State next_state;
    MNFConsequent()
    {
        next_state = -1;
    }
};

/*MNF is an alias for the type used to describe a Monitor Normal Form*/
typedef std::map<State , std::map<Interaction, MNFConsequent>> MNF;

/*TNF is an alias for the type used to describe a Thread Normal Form*/
typedef std::map<State , std::map<ActionSequence, State>> TNF;

/**
 * @class MonitorSpecification
 * @brief Complete thread/monitor specification
 *
 * The specification states:
 * - which monitor classes should be coded;
 * - for each monitor class, which are the synchronized methods;
 * - for each monitor class, its representation in terms of Monitor Normal Form;
 * - which monitor instances will be shared;
 * - for each monitor instance, which interactions it offers;
 * - which thread classes should be coded;
 * - for each thread class, its representation in terms of Thread Normal Form;
 * - for each thread class, which monitor instances it uses;
 **/
struct MonitorSpecification : Information, MakeVisitable<MonitorSpecification> {
    std::map<MonitorIdentifier, std::list<Interaction>> monitors;
    std::map<MonitorIdentifier, MNF> monitor_models;
    std::map<ThreadIdentifier, std::list<MonitorInstance>> threads;
    std::map<ThreadIdentifier, TNF> thread_models;
    std::map<Interaction, std::pair<MonitorInstance, bool>>
            interaction_instance_map;
    std::map<MonitorInstance, MonitorIdentifier> instance_type_map;
#ifdef MONITOR_DEBUG
    void print();
#endif
};

/**
 * @class MonitorAnalyst
 * @brief Analyst specialized in the monitor formalism
 *
 * A MonitorAnalyst is a FspcAnalyst, namely an Analyst having access to the
 * fspc environment. Being an Analyst, it can visit UserRequirements (which is
 * an Information) to produce a MonitorSpecification (which is an Information).
 **/
class MonitorAnalyst : public FspcAnalyst {
    private:

        /**
         * @enum NodeType
         * @brief Internal (to MonitorAnalyst) classification of nodes
         *
         * In order to produce a MonitorSpecification, a MonitorAnalyst has to
         * navigate all the specified LTSs and classify its states. This
         * enumerate offers some symbolic values, useful to perform such
         * classification.
         **/
        enum NodeType {
            SYNCHRONIZATION_POINT,
            TRANSITORY,
            STOCHASTIC,
            HYBRID,
            ZERO_DIMENSION
        };

        /**
         * @enum NodeClassifier
         * @brief Inner class for cached node classification
         *
         * NodeClassifier wraps classification criteria useful for a
         * MonitorAnalyst (thus, it has been declared as an inner class).
         * A node is classified looking at its exit set and knowing which are
         * the interactions. A NodeClassifier has a simple classification cache,
         * since the Monitor Normal Form algorithm could require the
         * classification for a single node several times. So, for each LTS,
         * a new NodeClassifier is created and initialized with the
         * classification parameters (the interactions). When a classification
         * for a node is requested, the NodeClassifier looks up the cache. If a
         * classification is found, it is immediately returned, else the
         * classification algorithm is performed and the cache filled with the
         * result.
         **/
        class NodeClassifier {
            private:
                std::map<State, NodeType> cache;
                std::list<Interaction> interactions;
            public:
                /**
                 * @param _lts The LTS is requested only to check if the it
                 * contains the specified labels
                 * @param interactions This list specifies which labels are, in
                 * fact, interactions
                 **/
                NodeClassifier(const fsp::Lts& _lts,
                               const std::list<Interaction>& interactions);
                /**
                 * @param node The node to classify
                 * @param edges The exit set of the node to classify
                 **/
                NodeType node_type(State node, const std::vector<Edge>& edges);
        };

        /**
         * @class MNFExtractor
         * @brief Monitor Normal Form extraction algorithm
         *
         * The Monitor Normal Form extraction algorithm is a recursive algorithm
         * which must be initialized with some parameters. It has been modeled
         * as a stateful function (functor).
         **/
        class MNFExtractor {
            private:
                fsp::ActionsTable& actions;
                const fsp::Lts& lts;
                std::list<Interaction> interactions;
                State state;
                std::vector<bool> seen; /*first time: 0 0 0 0 0 0 ...*/
                NodeClassifier classifier;
                MNFAntecedent antecedent;
                void internal_actions_lookahead(State state, MNF& mnf);
                void remap_states(MNF& mnf);
            public:
                MNFExtractor(const fsp::Lts& _lts,
                             const std::list<Interaction>& _interactions);
                /**
                 * This method implements the recursive Monitor Normal Form
                 * extraction algorithm. It's a recursive algorithm.
                 * @param node Current node
                 * @param recursive_call Parameter used to discern whether the
                 * current call is the first call
                 * @return The Monitor Normal Form is returned by reference
                 **/
                void operator()(State node, MNF& mnf,
                                bool recursive_call = false);
        };

        /**
         * @class TNFExtractor
         * @brief Thread Normal Form extraction algorithm
         *
         * The Thread Normal Form extraction algorithm is a recursive algorithm
         * which must be initialized with some parameters. It has been modeled
         * as a stateful function (functor).
         **/
        class TNFExtractor {
            private:
                const fsp::Lts& lts;
                std::vector<bool> seen;
                State currentRoot;
                ActionSequence currentPath;
                void complete_path(int node, TNF& tnf);
                void remap_states(TNF& tnf);
            public:
                TNFExtractor(const fsp::Lts& _lts);
                /**
                 * This method implements the recursive Thread Normal Form
                 * extraction algorithm. It's a recursive algorithm.
                 * @param node Current node
                 * @param recursive_call Parameter used to discern whether the
                 * current call is the first call
                 * @return The Thread Normal Form is returned by reference
                 **/
                void operator()(State node, TNF& tnf, bool recursive_call = false);
        };

        /**
         * This method explores the given LTS and, if possible, tries to obtain
         * its representation in terms of Monitor Normal Form.
         * @param lts The LTS to analyze
         * @param interactions The interactions exposed by the LTS
         * @return The Monitor Normal Form is returned by reference
         **/
        void monitor(const fsp::Lts& lts,
                     const std::list<Interaction>& interactions,
                     MNF& mnf);
        /**
         * This method explores the given LTS and, if possible, tries to obtain
         * its representation in terms of Thread Normal Form.
         * @param lts The LTS to analyze
         * @return The Monitor Normal Form is returned by reference
         **/
        void thread(const fsp::Lts& lts, TNF& tnf);

        /**
         * @enum ParseMode
         * @brief UserRequirements parsing states
         * UserRequirements parsing is implemented as a state machine. This
         * enumerate defines the possible parsing states. The naming convention
         * is the following:
         * - x_y_z means that, in the current state, the parser accepts an x, y
         * or z token;
         * - monitor is a symbol for "-m" flag;
         * - thread is a symbol for "-t" flag;
         * - instance is a symbol for "-i" flag;
         * - monitorType is a symbol for a FSP identifier;
         * - threadType is a symbol for a FSP identifier;
         * - instanceType is a symbol for a FSP identifier;
         * - monitorInteraction is a symbol for a FSP label;
         * - monitorInstance is a symbol for a monitor instance identifier;
         * - instanceIdentifier is a symbol for a monitor instance identifier;
         **/
        enum ParseMode {
            monitor_thread_instance,
            monitorType, threadType, instanceType,
            monitor_thread_instance_monitorInteraction,
            monitor_thread_instance_monitorInstance,
            monitor_thread_instance_instanceIdentifier
        };
    public:
        MonitorAnalyst(FspDriver& _c);
        /**
         * A MonitorAnalyst maps UserRequirements (which is an Information) into
         * a MonitorSpecification (which is an Information).
         * @param r The UserRequirements to parse and analyze
         * @return The resulting MonitorSpecification
         **/
        virtual heap<Information> visit(const UserRequirements& r);
};

}
#endif
