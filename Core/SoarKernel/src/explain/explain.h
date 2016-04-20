/*
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_EXPLAIN_H_
#define EBC_EXPLAIN_H_

#include "kernel.h"
#include "stl_typedefs.h"

#include <list>
#include <set>
#include <unordered_map>
#include <list>
#include <cstdlib>
#include <string>

typedef struct chunking_stats_struct {
        uint64_t            duplicates;
        uint64_t            unorderable;
        uint64_t            justification_did_not_match;
        uint64_t            chunk_did_not_match;
        uint64_t            no_grounds;
        uint64_t            max_chunks;
        uint64_t            tested_local_negation;
        uint64_t            merged_conditions;
        uint64_t            chunks_attempted;
        uint64_t            chunks_reverted;
        uint64_t            chunks_succeeded;
        uint64_t            justifications_attempted;
        uint64_t            justifications_succeeded;
        uint64_t            instantations_backtraced;
        uint64_t            seen_instantations_backtraced;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;
        uint64_t            grounding_conditions_added;
} chunking_stats;

typedef struct chunk_stats_struct {
        uint64_t            duplicates;
        bool                tested_local_negation;
        bool                reverted;
        uint64_t            num_grounding_conditions_added;
        uint64_t            merged_conditions;
        uint64_t            instantations_backtraced;
        uint64_t            seen_instantations_backtraced;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;
} chunk_stats;

class instantiation_record;
class chunk_record;

class action_record
{
        friend class Explanation_Logger;

    public:
        action_record(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID);
        ~action_record();

        uint64_t                get_actionID()   { return actionID; };
        id_set*                 get_identities();

        preference*             original_pref;          // Only used when building explain records

    private:
        agent* thisAgent;
        preference*             instantiated_pref;
        action*                 variablized_action;
        id_set*                 identities_used;
        uint64_t                actionID;
};

class condition_record
{
        friend class Explanation_Logger;

    public:
        condition_record(agent* myAgent, condition* pCond, uint64_t pCondID);
        ~condition_record();

        void        connect_to_action();
        void        create_identity_paths(const inst_record_list* pInstPath);
        bool        contains_identity_from_set(const id_set* pIDs);
        void        set_path_to_base(inst_record_list* pPath)
                    {   assert(pPath);
                        if (!path_to_base) path_to_base = new inst_record_list();
                        else path_to_base->clear();
                        (*path_to_base) = (*pPath); }
        void        set_instantiation(instantiation_record* pInst) { my_instantiation = pInst; };
        void        set_matched_wme_for_cond(condition* pCond);
        void        update_condition(condition* pCond, instantiation_record* pInst);

        uint64_t                        get_conditionID()   { return conditionID; };
        goal_stack_level                get_level()         { return wme_level_at_firing; };
        instantiation_record*           get_parent_inst()   { return parent_instantiation; };
        instantiation_record*           get_instantiation() { return my_instantiation; };
        inst_record_list*               get_path_to_base()  { return path_to_base; };
        preference*                     get_cached_pref()   { return cached_pref; };
        wme*                            get_cached_wme()    { return cached_wme; };

    private:
        agent* thisAgent;

        uint64_t                        conditionID;
        preference*                     cached_pref;
        wme*                            cached_wme;
        instantiation_record*           my_instantiation;
        instantiation_record*           parent_instantiation;
        action_record*                  parent_action;

        byte                            type;
        goal_stack_level                wme_level_at_firing;
        inst_record_list*               path_to_base;

        test_triple                     condition_tests;
        symbol_triple*                  matched_wme;
};

class production_record
{
        friend class Explanation_Logger;

    public:

        production_record(agent* pAgent, production* pProd);
        ~production_record();

        condition*  get_lhs() { return lhs_conds; }
        action*     get_rhs() { return rhs_actions; }

    private:
        agent*      thisAgent;
        condition*  lhs_conds;
        action*     rhs_actions;
};

class instantiation_record
{
        friend class Explanation_Logger;

    public:
        instantiation_record(agent* myAgent, instantiation* pInst);
        ~instantiation_record();

        uint64_t                get_instantiationID() { return instantiationID; };
        goal_stack_level        get_match_level() { return match_level; };
        inst_record_list*       get_path_to_base() { return path_to_base; };
        uint64_t                get_chunk_creator() { return creating_chunk; }
        id_set*                 get_lhs_identities();
        void                    record_instantiation_contents();
        void                    update_instantiation_contents();
        void                    create_identity_paths(const inst_record_list* pInstPath);
        condition_record*       find_condition_for_chunk(preference* pPref, wme* pWME);
        action_record*          find_rhs_action(preference* pPref);
        void                    delete_instantiation();

        instantiation*          cached_inst;

    private:
        agent* thisAgent;
        uint64_t                instantiationID;
        Symbol*                 production_name;
        production*             original_production;
        production_record*      excised_production;
        uint64_t                creating_chunk;

        goal_stack_level        match_level;
        bool                    terminal;
        inst_record_list*       path_to_base;
        id_set*                 lhs_identities;

        condition_record_list*  conditions;
        action_record_list*     actions;
};

typedef struct identity_set_struct {
        uint64_t    identity_set_ID;
        Symbol*     rule_variable;
} identity_set_info;

class identity_record
{
        friend class Explanation_Logger;

    public:
        identity_record(agent* myAgent, chunk_record* pChunkRecord, id_to_id_map_type* pIdentitySetMappings);
        ~identity_record();

        void    generate_identity_sets(condition* lhs);
        void    print_identity_mappings_for_instantiation(instantiation_record* pInstRecord);
        void    print_identity_explanation(chunk_record* pChunkRecord);
    private:

        agent*                  thisAgent;
        id_set*                 identities_in_chunk;
        id_to_id_map_type*      original_ebc_mappings;
        id_to_idset_map_type*   id_to_id_set_mappings;

        void    print_identities_in_chunk();
        void    print_identity_mappings();
        void    print_original_ebc_mappings();
};

class chunk_record
{
        friend class Explanation_Logger;

    public:
        chunk_record(agent* myAgent, uint64_t pChunkID);
        ~chunk_record();

        void                    record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation);
        void                    generate_dependency_paths();
        void                    end_chunk_record();
        void                    excise_chunk_record();

    private:
        agent*                  thisAgent;

        Symbol*                 name;
        uint64_t                chunkID;
        instantiation_record*   chunkInstantiation;
        production*             original_production;
        production_record*      excised_production;
        uint64_t                time_formed;
        goal_stack_level        match_level;


        condition_record_list*  conditions;
        action_record_list*     actions;

        instantiation_record*   baseInstantiation;
        inst_set*               result_instantiations;
        inst_record_set*        result_inst_records;

        inst_set*               backtraced_instantiations;
        inst_record_list*       backtraced_inst_records;

        identity_record*        identity_analysis;
        chunk_stats             stats;
};


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
        void                    add_bt_instantiation(instantiation* pInst, BTSourceType bt_type) { if (current_recording_chunk) current_recording_chunk->backtraced_instantiations->insert(pInst); };

        void                    add_chunk_record(instantiation* pBaseInstantiation);
        void                    add_result_instantiations(instantiation* pBaseInst, preference* pResults);
        void                    record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, instantiation* pChunkInstantiation);
        void                    cancel_chunk_record();
        void                    end_chunk_record();
        void                    save_excised_production(production* pProd);

        void                    reset_identity_set_counter() { id_set_counter = 0; };
        uint64_t                get_identity_set_counter() { return ++id_set_counter; };

        instantiation_record*   add_instantiation(instantiation* pInst, uint64_t pChunkID = 0);

        void increment_stat_duplicates(production* duplicate_rule);
        void increment_stat_grounded(int pNumConds);
        void increment_stat_reverted();
        void increment_stat_unorderable() { stats.unorderable++; };
        void increment_stat_justification_did_not_match() { stats.justification_did_not_match++; };
        void increment_stat_chunk_did_not_match() { stats.chunk_did_not_match++; };
        void increment_stat_no_grounds() { stats.no_grounds++; };
        void increment_stat_max_chunks() { stats.max_chunks++; };
        void increment_stat_succeeded() { stats.chunks_succeeded++; };
        void increment_stat_tested_local_negation() { stats.tested_local_negation++; if (current_recording_chunk) current_recording_chunk->stats.tested_local_negation = true; };
        void increment_stat_merged_conditions(int pCount = 1) { stats.merged_conditions += pCount; if (current_recording_chunk) current_recording_chunk->stats.merged_conditions++; };
        void increment_stat_chunks_attempted() { stats.chunks_attempted++; };
        void increment_stat_justifications_attempted() { stats.justifications_attempted++; };
        void increment_stat_justifications() { stats.justifications_succeeded++; };
        void increment_stat_instantations_backtraced() { stats.instantations_backtraced++; if (current_recording_chunk) current_recording_chunk->stats.instantations_backtraced++; };
        void increment_stat_seen_instantations_backtraced() { stats.seen_instantations_backtraced++; if (current_recording_chunk) current_recording_chunk->stats.seen_instantations_backtraced++; };
        void increment_stat_constraints_attached() { stats.constraints_attached++; if (current_recording_chunk) current_recording_chunk->stats.constraints_attached++; };
        void increment_stat_constraints_collected() { stats.constraints_collected++; if (current_recording_chunk) current_recording_chunk->stats.constraints_collected++; };

        bool current_discussed_chunk_exists();
        bool watch_rule(const std::string* pStringParameter);
        bool toggle_production_watch(production* pProduction);

        bool explain_chunk(const std::string* pStringParameter);
        bool explain_item(const std::string* pObjectTypeString, const std::string* pObjectIDString);
        void print_explain_summary();
        void print_explainer_stats();
        void print_chunk_stats();
        void print_all_watched_rules();
        void print_all_chunks();
        void print_formation_explanation();
        void print_identity_set_explanation();
        void print_constraints_enforced();
        void print_involved_instantiations();
        void switch_to_explanation_trace(bool pEnableExplanationTrace);
        void visualize_last_output();

        Explanation_Logger(agent* myAgent);
        ~Explanation_Logger();

    private:

        agent*                  thisAgent;
        Output_Manager*         outputManager;

        bool                    enabled;
        bool                    print_explanation_trace;
        uint64_t                last_printed_id;

        int                     num_rules_watched;
        bool                    shouldRecord() { return (enabled || current_recording_chunk); }

        tc_number               backtrace_number;
        chunk_record*           current_discussed_chunk;
        chunk_record*           current_recording_chunk;
        identity_triple         current_explained_ids;

        void                    initialize_counters();
        chunk_record*           get_chunk_record(Symbol* pChunkName);
        instantiation_record*   get_instantiation(instantiation* pInst);
        condition_record*       add_condition(condition_record_list* pCondList, condition* pCond, instantiation_record* pInst = NULL, bool pMakeNegative = false);
        action_record*          add_result(preference* pPref, action* pAction = NULL);

        void                    discuss_chunk(chunk_record* pChunkRecord);
        void                    clear_chunk_from_instantiations();
        void                    print_chunk_list(short pNumToPrint = 0);
        void                    print_rules_watched(short pNumToPrint = 0);
        bool                    print_watched_rules_of_type(agent* thisAgent, unsigned int productionType, short &pNumToPrint);

        void                    print_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs = NULL, production_record* pExcisedRule = NULL);
        void                    print_chunk_explanation();
        bool                    print_chunk_explanation_for_id(uint64_t pChunkID);
        void                    print_instantiation_explanation(instantiation_record* pInstRecord, bool printFooter = true);
        bool                    print_instantiation_explanation_for_id(uint64_t pInstID);
        bool                    print_condition_explanation_for_id(uint64_t pInstID);
        void                    print_condition_explanation(uint64_t pCondID);
        void                    print_path_to_base(const inst_record_list* pPathToBase, bool pPrintFinal = true, const char* pFailedStr = NULL, const char* pHeaderStr = NULL);
        void                    print_footer(bool pPrintDiscussedChunkCommands = false);

        void                    visualize_chunk_explanation();
        void                    visualize_instantiation_explanation(instantiation_record* pInstRecord);
        bool                    visualize_instantiation_explanation_for_id(uint64_t pInstID);
        void                    visualize_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs = NULL);
        void 					viz_action(action* pAction, uint64_t pNodeID);
        void 					viz_preference(preference* pPref, uint64_t pNodeID);
        void 					viz_graph_start();
        void 					viz_graph_end();
        void 					viz_rule_start(Symbol* pName, uint64_t node_id);
        void 					viz_rule_end();
        void 					viz_NCC_start();
        void 					viz_NCC_end();
        void 					viz_seperator();
        void 					viz_text_record(const char* pMsg);
        void 					viz_condition_record(condition_record* pCond);
        void 					viz_condition(condition_record* pCondRecord, condition* pCond);
        bool                    is_condition_related(condition_record* pCondRecord);

        void                    delete_condition(uint64_t pCondID);
        void                    delete_action(uint64_t pActionID);
        void                    delete_instantiation(uint64_t pInstID);

        /* ID Counters */
        uint64_t            condition_id_count;
        uint64_t            chunk_id_count;
        uint64_t            action_id_count;
        uint64_t            id_set_counter;

        /* Statistics on learning performed so far */
        chunking_stats      stats;

        /* A string buffer for the visualization command */
        std::string			graphviz_output;

        /* These maps store all of the records the logger keeps */
        std::unordered_map< Symbol*, chunk_record* >*           chunks;
        std::unordered_map< uint64_t, chunk_record* >*          chunks_by_ID;
        std::unordered_map< uint64_t, instantiation_record* >*  instantiations;
        std::unordered_map< uint64_t, condition_record* >*      all_conditions;
        std::unordered_map< uint64_t, action_record* >*         all_actions;
        std::set< production_record* >*                         all_excised_productions;

};

#endif /* EBC_EXPLAIN_H_ */
