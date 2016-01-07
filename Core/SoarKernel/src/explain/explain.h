/*
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_EXPLAIN_H_
#define EBC_EXPLAIN_H_

#include "kernel.h"
#include <list>
#include <set>
#include <unordered_map>
#include <list>
#include <cstdlib>
#include <string>

typedef char* rhs_value;
typedef signed short goal_stack_level;
typedef struct action_struct action;
typedef struct agent_struct agent;
typedef struct chunk_cond_struct chunk_cond;
typedef struct condition_struct condition;
typedef struct instantiation_struct instantiation;
typedef struct preference_struct preference;
typedef struct symbol_struct Symbol;
typedef struct test_struct test_info;
typedef test_info* test;
typedef struct wme_struct wme;
typedef struct rete_node_struct rete_node;
typedef struct node_varnames_struct node_varnames;
typedef unsigned short rete_node_level;
typedef uint64_t tc_number;
typedef struct cons_struct cons;
typedef cons list;
namespace soar_module
{
    typedef struct symbol_triple_struct symbol_triple;
    typedef struct identity_triple_struct identity_triple;
    typedef struct rhs_triple_struct rhs_triple;
}

class Output_Manager;

class action_record {

        friend class Explanation_Logger;

    public:
        action_record(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID);
        ~action_record();
        preference*     original_pref;          // Only used when building explain records
        uint64_t get_actionID() { return actionID; };

    private:
        agent* thisAgent;
        preference*     instantiated_pref;
        action*         variablized_action;
        uint64_t       actionID;
} ;

class condition_record{

        friend class Explanation_Logger;

    public:
        condition_record(agent* myAgent, condition* pCond, uint64_t pCondID);
        ~condition_record();
        uint64_t get_conditionID() { return conditionID; };

    private:
        agent* thisAgent;
        condition*                  variablized_cond;
        condition*                  instantiated_cond;
        soar_module::symbol_triple* matched_wme;
        action_record*              parent_action;
        uint64_t                   conditionID;
} ;

//#ifdef USE_MEM_POOL_ALLOCATORS
//#include "soar_module.h"
//typedef std::list< condition_record*, soar_module::soar_memory_pool_allocator< condition_record* > > condition_record_list;
//typedef std::list< action_record*, soar_module::soar_memory_pool_allocator< action_record* > > action_record_list;
//typedef std::list< uint64_t, soar_module::soar_memory_pool_allocator< uint64_t > > id_list;
//#else
//typedef std::list< condition_record* > condition_record_list;
//typedef std::list< action_record* > action_record_list;
//typedef std::list< uint64_t > id_list;
//#endif
typedef std::list< condition_record* > condition_record_list;
typedef std::list< action_record* > action_record_list;
typedef std::list< uint64_t > id_list;

class instantiation_record {

        friend class Explanation_Logger;

    public:
        instantiation_record(agent* myAgent, instantiation* pInst);
        ~instantiation_record();

        uint64_t get_instantiationID() { return instantiationID; };
        action_record* find_rhs_action(preference* pPref);

    private:
        agent* thisAgent;
        Symbol*                 production_name;
        condition_record_list*  conditions;
        action_record_list*     actions;
        uint64_t                instantiationID;
} ;

class chunk_record {

        friend class Explanation_Logger;

    public:
        chunk_record(agent* myAgent, instantiation* pBaseInstantiation, uint64_t pChunkID);
        ~chunk_record();

        void                    record_chunk_contents(Symbol* pName, condition* lhs, action* rhs, preference* results);

    private:
        agent*                  thisAgent;
        Symbol*                 name;
        condition_record_list*  conditions;
        action_record_list*     actions;
        std::string             identity_set_mapping_explanation;
        instantiation_record*   baseInstantiation;
        uint64_t                chunkID;
} ;

typedef struct tr_stats_struct {
        uint64_t            chunks;
        uint64_t            instantiations;
        uint64_t            instantiations_referenced;
        uint64_t            instantiations_skipped;
        uint64_t            conditions;
        uint64_t            actions;
} tr_stats;

typedef struct chunking_stats_struct {
        uint64_t            duplicates;
        uint64_t            unorderable;
        uint64_t            justification_did_not_match;
        uint64_t            chunk_did_not_match;
        uint64_t            no_grounds;
        uint64_t            max_chunks;
        uint64_t            succeeded;
        uint64_t            tested_local_negation;
        uint64_t            merged_conditions;
        uint64_t            chunks_attempted;
        uint64_t            justifications_attempted;
        uint64_t            justifications;
        uint64_t            instantations_backtraced;
        uint64_t            seen_instantations_backtraced;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;
} chunking_stats;

class Explanation_Logger
{
        friend class instantiation_record;
        friend class chunk_record;
        friend class condition_record;

    public:
        bool                    get_enabled() { return enabled; }
        void                    set_enabled(bool pEnabled) { enabled = pEnabled; }

        void                    re_init();
        void                    clear_explanations();

        void                    set_backtrace_number(uint64_t pBT_num) { backtrace_number = pBT_num; };

        chunk_record*           add_chunk_record(instantiation* pbaseInstantiation);
        chunk_record*           get_chunk_record(Symbol* pChunkName);
        void                    cancel_chunk_record();
        void                    record_chunk_contents(Symbol* pName, condition* lhs, action* rhs, preference* results);
        instantiation_record*   add_instantiation(instantiation* pInst);

        void increment_stat_duplicates() { stats.duplicates++; };
        void increment_stat_unorderable() { stats.unorderable++; };
        void increment_stat_justification_did_not_match() { stats.justification_did_not_match++; };
        void increment_stat_chunk_did_not_match() { stats.chunk_did_not_match++; };
        void increment_stat_no_grounds() { stats.no_grounds++; };
        void increment_stat_max_chunks() { stats.max_chunks++; };
        void increment_stat_succeeded() { stats.succeeded++; };
        void increment_stat_tested_local_negation() { stats.tested_local_negation++; };
        void increment_stat_merged_conditions(int pCount = 1) { stats.merged_conditions += pCount; };
        void increment_stat_chunks_attempted() { stats.chunks_attempted++; };
        void increment_stat_justifications_attempted() { stats.justifications_attempted++; };
        void increment_stat_justifications() { stats.justifications++; };
        void increment_stat_instantations_backtraced() { stats.instantations_backtraced++; };
        void increment_stat_seen_instantations_backtraced() { stats.seen_instantations_backtraced++; };
        void increment_stat_constraints_attached() { stats.constraints_attached++; };
        void increment_stat_constraints_collected() { stats.constraints_collected++; };

        bool current_discussed_chunk_exists();
        bool watch_rule(const std::string* pStringParameter);
        bool explain_chunk(const std::string* pStringParameter);
        bool explain_item(const std::string* pStringParameter, const std::string* pStringParameter2);
        void explain_summary();
        void explain_stats();
        void explain_chunk_stats();
        void print_all_watched_rules();
        void print_all_chunks();
        void print_dependency_analysis();
        void print_identity_set_explanation();
        void print_constraints_enforced();

        void should_explain_rule(bool pEnable);
        void should_explain_all(bool pEnable);

        Explanation_Logger(agent* myAgent);
        ~Explanation_Logger();

    private:

        agent*                  thisAgent;
        Output_Manager*         outputManager;

        bool                    enabled;
        tc_number               backtrace_number;
        chunk_record*           current_discussed_chunk;
        chunk_record*           current_recording_chunk;

        void                    initialize_counters();
        instantiation_record*   get_instantiation(instantiation* pInst);
        action_record*          get_action_for_instantiation(preference* pPref, instantiation* pInst);
        condition_record*       add_condition(condition* pCond);
        action_record*          add_result(preference* pPref, action* pAction = NULL);

        void                    print_chunk_explanation();
        void                    print_condition_explanation(uint64_t pCondID);
        void                    print_instantiation_explanation(uint64_t pInstID);

        void                    print_chunk_list(short pNumToPrint = 0);
        void                    print_rules_watched(short pNumToPrint = 0);
        bool                    print_watched_rules_of_type(agent* thisAgent, unsigned int productionType, short &pNumToPrint);
        bool                    toggle_production_watch(production* pProduction);

        /* ID Counters */
        uint64_t            condition_id_count;
        uint64_t            chunk_id_count;
        uint64_t            action_id_count;

        /* Statistics on learning performed so far */
        tr_stats            total_recorded;
        chunking_stats      stats;

        /* These maps store all of the records the logger keeps */
        std::unordered_map< Symbol*, chunk_record* >*           chunks;
        std::unordered_map< uint64_t, instantiation_record* >*  instantiations;
        std::unordered_map< uint64_t, condition_record* >*      conditions;
        std::unordered_map< uint64_t, action_record* >*         actions;
};

#endif /* EBC_EXPLAIN_H_ */
