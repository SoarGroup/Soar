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

#include "chunk_record.h"

#include <list>
#include <set>
#include <unordered_map>
#include <list>
#include <cstdlib>
#include <string>

class chunk_record;
class instantiation_record;
class chunk_record;
class condition_record;
class action_record;
class production_record;
class identity_record;

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


class Explanation_Logger
{
        friend class instantiation_record;
        friend class chunk_record;
        friend class condition_record;
        friend class CommandLineInterface;

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
        void escape_graphviz_chars();


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
        bool                    print_chunk_explanation_for_id(uint64_t pChunkID);
        void                    print_chunk_explanation();
        bool                    print_instantiation_explanation_for_id(uint64_t pInstID);
        void                    print_instantiation_explanation(instantiation_record* pInstRecord, bool printFooter = true);
        bool                    print_condition_explanation_for_id(uint64_t pInstID);
        void                    print_condition_explanation(uint64_t pCondID);
        void                    print_path_to_base(const inst_record_list* pPathToBase, bool pPrintFinal = true, const char* pFailedStr = NULL, const char* pHeaderStr = NULL);
        void                    print_footer(bool pPrintDiscussedChunkCommands = false);

        void					visualize_explanation_trace();
        void                    visualize_chunk_explanation();
        void                    viz_instantiation(instantiation_record* pInstRecord);
        bool                    visualize_instantiation_explanation_for_id(uint64_t pInstID);
        void 					clear_visualization();
        void                    viz_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs = NULL);
        void 					viz_et_action(action* pAction, action* pVariablizedAction, preference* pPref, uint64_t pNodeID);
        void 					viz_wt_preference(preference* pPref, uint64_t pNodeID);
        void 					viz_graph_start();
        void 					viz_graph_end();
        void 					viz_rule_start(Symbol* pName, uint64_t node_id);
        void 					viz_rule_end();
        void 					viz_NCC_start();
        void 					viz_NCC_end();
        void 					viz_seperator();
        void 					viz_record_seperator(bool pLeftJustify = true);
        void 					viz_record_start();
        void 					viz_record_end(bool pLeftJustify = true);
        void 					viz_text_record(const char* pMsg);
        void					viz_et_test(test pTest, test pTestIdentity, uint64_t pNode_id, bool printInitialPort, bool printFinalPort);
        void 					viz_wt_condition(condition_record* pCond);
        void 					viz_et_condition(condition_record* pCondRecord, condition* pCond);
        void 					viz_rhs_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, uint64_t pID);
        void					viz_add_port(char pTypeChar, uint64_t pNodeID, bool pIsLeftPort);
        void 					viz_port(uint64_t pSrcRuleID, uint64_t pSrcActionID, uint64_t pTargetRuleID, uint64_t pTargetCondID);
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
