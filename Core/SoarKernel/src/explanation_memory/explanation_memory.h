/*
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_EXPLAIN_H_
#define EBC_EXPLAIN_H_

#include "kernel.h"

#include "chunk_record.h"
#include "explanation_settings.h"
#include "identity_record.h"
#include "stl_typedefs.h"

#include <list>
#include <set>
#include <unordered_map>
#include <list>
#include <cstdlib>
#include <string>

typedef struct chunking_stats_struct {
        /* Core */
        uint64_t            chunks_attempted;
        uint64_t            chunks_succeeded;
        uint64_t            justifications_attempted;
        uint64_t            justifications_succeeded;
        uint64_t            instantations_backtraced;
        uint64_t            seen_instantations_backtraced;
        uint64_t            duplicates;
        uint64_t            no_grounds;
        uint64_t            max_chunks;
        uint64_t            max_dupes;
        uint64_t            tested_local_negation;
        uint64_t            tested_quiescence;
        uint64_t            tested_ltm_recall;
        uint64_t            tested_local_negation_just;
        uint64_t            tested_ltm_recall_just;
        uint64_t            chunks_repaired;

        /* Detailed (only used if EBC_DETAILED_STATISTICS is #defined) */
        uint64_t            grounding_conditions_added;
        uint64_t            merged_conditions;
        uint64_t            merged_disjunctions;
        uint64_t            merged_disjunction_values;
        uint64_t            eliminated_disjunction_values;
        uint64_t            constraints_attached;
        uint64_t            constraints_collected;
        uint64_t            tested_deep_copy;
        uint64_t            tested_deep_copy_just;

        uint64_t            identities_created;
        uint64_t            identities_joined;
        uint64_t            identities_joined_variable;
        uint64_t            identities_joined_local_singleton;
        uint64_t            identities_joined_singleton;
        uint64_t            identities_joined_child_results;
        uint64_t            identities_literalized_rhs_literal;
        uint64_t            identities_literalized_lhs_literal;
        uint64_t            identities_literalized_rhs_func_arg;
        uint64_t            identities_literalized_rhs_func_compare;

        uint64_t            identities_participated;
        uint64_t            identity_propagations;
        uint64_t            identity_propagations_blocked;
        uint64_t            operational_constraints;
        uint64_t            OSK_instantiations;

        /* Debug stats (only used if EBC_DEBUG_STATISTICS is #defined) */
        uint64_t            lhs_unconnected;
        uint64_t            rhs_unconnected;
        uint64_t            chunk_did_not_match;
        uint64_t            justification_did_not_match;
        uint64_t            chunks_reverted;
        uint64_t            repair_failed;

} chunking_stats;


class Explanation_Memory
{
        friend class instantiation_record;
        friend class chunk_record;
        friend class condition_record;
        friend class action_record;
        friend class cli::CommandLineInterface;

    public:

        Explainer_Parameters*   settings;

        bool                    is_any_enabled() { return (m_all_enabled || (num_rules_watched > 0)); }
        bool                    is_all_enabled() { return m_all_enabled; }
        void                    set_all_enabled(bool pEnabled) { m_all_enabled = pEnabled; }
        bool                    isCurrentlyRecording() { return (current_recording_chunk != NULL); }

        bool                    isRecordingJustifications() { return m_justifications_enabled; };
        void                    set_justifications_enabled(bool pEnabled) { m_justifications_enabled = pEnabled; };

        void                    re_init();
        void                    clear_explanations();
        void                    clear_identity_sets();
        void                    clear_identity_sets_for_goal(Symbol* pGoal);

        void                    set_backtrace_number(uint64_t pBT_num) { backtrace_number = pBT_num; };
        void                    add_bt_instantiation(instantiation* pInst, BTSourceType bt_type) { if (current_recording_chunk) current_recording_chunk->backtraced_instantiations->insert(pInst); };

        void                    add_chunk_record(instantiation* pBaseInstantiation);
        void                    add_result_instantiations(instantiation* pBaseInst, preference* pResults);
        void                    record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_join_map* pIdentitySetMappings, instantiation* pBaseInstantiation, instantiation* pChunkInstantiation);
        void                    cancel_chunk_record();
        void                    end_chunk_record();
        void                    save_excised_production(production* pProd);
        void                    excise_production_id(uint64_t pId);

        void                    add_identity(IdentitySet* pNewIdentity, Symbol* pGoal);
        void                    add_identity_set_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType, IdentitySet* pFromJoinSet, IdentitySet* pToJoinSet);

        instantiation_record*   add_instantiation(instantiation* pInst, uint64_t pChunkID = 0);
        chunk_record*           get_current_discussed_chunk() { return current_discussed_chunk; };

        void increment_stat_duplicates(production* duplicate_rule);
        void increment_stat_justification_did_not_match() { stats.justification_did_not_match++; if (current_recording_chunk) current_recording_chunk->stats.did_not_match_wm = true;};
        void increment_stat_chunk_did_not_match() { stats.chunk_did_not_match++; if (current_recording_chunk) current_recording_chunk->stats.did_not_match_wm = true; };
        void increment_stat_no_grounds() { stats.no_grounds++; };
        void increment_stat_max_chunks() { stats.max_chunks++; };
        void increment_stat_max_dupes() { stats.max_dupes++; if (current_recording_chunk) current_recording_chunk->stats.max_dupes = true; };
        void increment_stat_tested_local_negation(ebc_rule_type pType) { if (pType == ebc_chunk) stats.tested_local_negation++; else stats.tested_local_negation_just++; if (current_recording_chunk) current_recording_chunk->stats.tested_local_negation = true; };
        void increment_stat_tested_deep_copy(ebc_rule_type pType) { if (pType == ebc_chunk) stats.tested_deep_copy++; else stats.tested_deep_copy_just ++; if (current_recording_chunk) current_recording_chunk->stats.tested_deep_copy = true; };
        void increment_stat_tested_ltm_recall(ebc_rule_type pType) { if (pType == ebc_chunk) stats.tested_ltm_recall++; else stats.tested_ltm_recall_just ++; if (current_recording_chunk) current_recording_chunk->stats.tested_ltm_recall = true; };
        void increment_stat_tested_quiescence() { stats.tested_quiescence++; if (current_recording_chunk) current_recording_chunk->stats.tested_quiescence = true; };
        void increment_stat_merged_conditions(int pCount = 1) { stats.merged_conditions += pCount; if (current_recording_chunk) current_recording_chunk->stats.merged_conditions += pCount; };
        void increment_stat_merged_disjunctions() { stats.merged_disjunctions++; if (current_recording_chunk) current_recording_chunk->stats.merged_disjunctions++; };
        void increment_stat_eliminated_disjunction_values(int pCount = 1) { stats.eliminated_disjunction_values += pCount; if (current_recording_chunk) current_recording_chunk->stats.eliminated_disjunction_values += pCount; };
        void increment_stat_merged_disjunction_values(int pCount = 1) { stats.merged_disjunction_values += pCount; if (current_recording_chunk) current_recording_chunk->stats.merged_disjunction_values += pCount; };
        void increment_stat_chunks_attempted() { stats.chunks_attempted++; };
        void increment_stat_chunks_succeeded() { stats.chunks_succeeded++; };
        void increment_stat_justifications_attempted() { stats.justifications_attempted++; };
        void increment_stat_justifications_succeeded() { stats.justifications_succeeded++; };
        void increment_stat_instantations_backtraced() { stats.instantations_backtraced++; if (current_recording_chunk) current_recording_chunk->stats.instantations_backtraced++; };
        void increment_stat_seen_instantations_backtraced() { stats.seen_instantations_backtraced++; if (current_recording_chunk) current_recording_chunk->stats.seen_instantations_backtraced++; };
        void increment_stat_constraints_attached() { stats.constraints_attached++; if (current_recording_chunk) current_recording_chunk->stats.constraints_attached++; };
        void increment_stat_constraints_collected() { stats.constraints_collected++; if (current_recording_chunk) current_recording_chunk->stats.constraints_collected++; };
        void increment_stat_grounding_conds_added(int pNumConds);
        void increment_stat_lhs_unconnected() { stats.lhs_unconnected++; if (current_recording_chunk) current_recording_chunk->stats.lhs_unconnected = true; };
        void increment_stat_rhs_unconnected() { stats.rhs_unconnected++; if (current_recording_chunk) current_recording_chunk->stats.rhs_unconnected = true; };
        void increment_stat_could_not_repair() { stats.repair_failed++;  if (current_recording_chunk) current_recording_chunk->stats.repair_failed = true; };
        void increment_stat_chunks_repaired() { stats.chunks_repaired++; };
        void increment_stat_chunks_reverted();

        void increment_stat_identities_created() { stats.identities_created++; if (current_recording_chunk) current_recording_chunk->stats.identities_created++; };
        void increment_stat_identities_joined() { stats.identities_joined++; if (current_recording_chunk) current_recording_chunk->stats.identities_joined++; };
        void increment_stat_identity_propagations() { stats.identity_propagations++; if (current_recording_chunk) current_recording_chunk->stats.identity_propagations++; };
        void increment_stat_identities_propagated_local_singleton() { stats.identities_joined_local_singleton++; if (current_recording_chunk) current_recording_chunk->stats.identities_joined_local_singleton++; };
        void increment_stat_identity_propagations_blocked() { stats.identity_propagations_blocked++; if (current_recording_chunk) current_recording_chunk->stats.identity_propagations_blocked++; };
        void increment_stat_OSK_instantiations() { stats.OSK_instantiations++; if (current_recording_chunk) current_recording_chunk->stats.OSK_instantiations++; };

        void increment_stat_operational_constraints() { stats.operational_constraints++; if (current_recording_chunk) current_recording_chunk->stats.operational_constraints++; };

        /* Only occur with the explainer on */
        void increment_stat_identities_joined_variable() { stats.identities_joined_variable++; if (current_recording_chunk) current_recording_chunk->stats.identities_joined_variable++; };
        void increment_stat_identities_joined_singleton() { stats.identities_joined_singleton++; if (current_recording_chunk) current_recording_chunk->stats.identities_joined_singleton++; };
        void increment_stat_identities_joined_child_results() { stats.identities_joined_child_results++; if (current_recording_chunk) current_recording_chunk->stats.identities_joined_child_results++; };
        void increment_stat_identities_literalized_rhs_literal() { stats.identities_literalized_rhs_literal++; if (current_recording_chunk) current_recording_chunk->stats.identities_literalized_rhs_literal++; };
        void increment_stat_identities_literalized_lhs_literal() { stats.identities_literalized_lhs_literal++; if (current_recording_chunk) current_recording_chunk->stats.identities_literalized_lhs_literal++; };
        void increment_stat_identities_literalized_rhs_func_arg() { stats.identities_literalized_rhs_func_arg++; if (current_recording_chunk) current_recording_chunk->stats.identities_literalized_rhs_func_arg++; };
        void increment_stat_identities_literalized_rhs_func_compare() { stats.identities_literalized_rhs_func_compare++; if (current_recording_chunk) current_recording_chunk->stats.identities_literalized_rhs_func_compare++; };
        void increment_stat_identities_participated(int pCount = 1) { stats.identities_participated += pCount; if (current_recording_chunk) current_recording_chunk->stats.identities_participated += pCount; };

        uint64_t get_stat_succeeded() { return stats.chunks_succeeded; };
        uint64_t get_stat_chunks_attempted() { return stats.chunks_attempted; };
        uint64_t get_stat_justifications() { return stats.justifications_succeeded; };

        bool current_discussed_chunk_exists();
        bool watch_rule(const std::string* pStringParameter);
        bool toggle_production_watch(production* pProduction);

        bool explain_chunk(const std::string* pStringParameter);
        bool explain_instantiation(const std::string* pStringParameter);
//        bool explain_item(const std::string* pObjectTypeString, const std::string* pObjectIDString);
        void print_explain_summary();
        void print_global_stats();
        void print_chunk_stats(chunk_record* pChunkRecord, bool pPrintHeader = true);
        void print_all_watched_rules();
        void print_all_chunks(bool pChunks);
        void print_formation_explanation();
        void print_identity_set_explanation();
        void print_constraints_enforced();
        void print_involved_instantiations();
        void switch_to_explanation_trace(bool pEnableExplanationTrace);

        void visualize_last_output();
        void visualize_instantiation_graph();
        void visualize_contributors();
        void visualize_identity_graph();

        void create_after_action_report();
        void after_action_report_for_init();
        void after_action_report_for_exit();

        Explanation_Memory(agent* myAgent);
        ~Explanation_Memory();

    private:

        agent*                  thisAgent;
        Output_Manager*         outputManager;

        bool                    m_all_enabled;
        bool                    m_justifications_enabled;
        bool                    print_explanation_trace;
        uint64_t                last_printed_id;

        int                     num_rules_watched;

        tc_number               backtrace_number;
        chunk_record*           current_discussed_chunk;
        chunk_record*           current_recording_chunk;
        identity_quadruple      current_explained_ids;

        std::string             after_action_report_file;

        void                    initialize_counters();
        chunk_record*           get_chunk_record(Symbol* pChunkName);
        instantiation_record*   get_instantiation(instantiation* pInst);
        condition_record*       add_condition(condition_record_list* pCondList, condition* pCond, instantiation_record* pInst = NULL, bool pMakeNegative = false);
        action_record*          add_result(preference* pPref, action* pAction = NULL);
        uint64_t                add_production_id_if_necessary(production* pProd);
        production*             get_production(uint64_t pId);

        void                    discuss_chunk(chunk_record* pChunkRecord);

        void                    clear_chunk_from_instantiations();
        void                    clear_identities_in_set(identity_set_set* lIdenty_set);

        void                    print_chunk_list(short pNumToPrint = 0, bool pChunks = true);
        void                    print_rules_watched(short pNumToPrint = 0);
        bool                    print_watched_rules_of_type(agent* thisAgent, unsigned int productionType, short &pNumToPrint);
        void                    print_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs = NULL, production_record* pExcisedRule = NULL);
        void                    print_chunk_actions(action_record_list* pActionRecords, production* pOriginalRule, production_record* pExcisedRule);
        void                    print_instantiation_actions(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs);
        bool                    print_chunk_explanation_for_id(uint64_t pChunkID);
        void                    print_chunk_explanation();
        bool                    print_instantiation_explanation_for_id(uint64_t pInstID);
        void                    print_instantiation_explanation(instantiation_record* pInstRecord, bool printFooter = true);
        void                    print_path_to_base(const inst_record_list* pPathToBase, bool pPrintFinal = true, const char* pFailedStr = NULL, const char* pHeaderStr = NULL);
        void                    print_footer(bool pPrintDiscussedChunkCommands = false);

        bool                    visualize_instantiation_explanation_for_id(uint64_t pInstID);

        bool                    is_condition_related(condition_record* pCondRecord);

        void                    delete_condition(uint64_t pCondID);
        void                    delete_action(uint64_t pActionID);
        void                    delete_instantiation(uint64_t pInstID);

        /* ID Counters */
        uint64_t            condition_id_count;
        uint64_t            chunk_id_count;
        uint64_t            action_id_count;

        /* Statistics on learning performed so far */
        chunking_stats      stats;

        /* These maps store all of the records the logger keeps */
        /* MToDo | Why aren't these using the ones stl_typedefs? */
        std::unordered_map< Symbol*, chunk_record* >*           chunks;
        std::unordered_map< uint64_t, chunk_record* >*          chunks_by_ID;
        std::unordered_map< uint64_t, instantiation_record* >*  instantiations;
        std::unordered_map< uint64_t, condition_record* >*      all_conditions;
        std::unordered_map< uint64_t, action_record* >*         all_actions;
        sym_to_identity_set_map*                                all_identities_in_goal;

        production_record_set*                                  cached_production;
        std::unordered_map< uint64_t, production* >*            production_id_map;
};

#endif /* EBC_EXPLAIN_H_ */
