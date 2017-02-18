/*
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_MANAGER_H_
#define EBC_MANAGER_H_

//#define EBC_SANITY_CHECK_RULES

#include "kernel.h"

#include "ebc_structs.h"
#include "stl_typedefs.h"
#include "condition.h"
#include "instantiation.h"
#include "symbol.h"
#include "test.h"

#include <list>
#include <set>
#include <unordered_map>
#include <cstdlib>

tc_number get_new_tc_number(agent* thisAgent);

class Explanation_Based_Chunker
{
        friend class Repair_Manager;

    public:

        Explanation_Based_Chunker(agent* myAgent);
        ~Explanation_Based_Chunker();

        instantiation*          instantiation_being_built;
        ebc_timer_container*    ebc_timers;

        /* Settings and cli command related functions */
        ebc_param_container*    ebc_params;
        bool                    ebc_settings[num_ebc_settings];
        uint64_t                max_chunks, max_dupes;

        /* Cached pointer to lti link rhs function since it may be used often */
        rhs_function*           lti_link_function;
        Symbol*                 deep_copy_sym_expanded;

        /* --- lists of symbols (PS names) declared chunk-free and chunky --- */
        cons*     chunk_free_problem_spaces;
        cons*     chunky_problem_spaces;   /* AGR MVL1 */

        /* Builds a chunk or justification based on a submitted instantiation
         * and adds it to the rete.  Called by create_instantiation, smem and epmem */
        void learn_EBC_rule(instantiation* inst, instantiation** new_inst_list);

        /* Methods used during instantiation creation to generate identities used by the
         * explanation trace. */
        void add_explanation_to_condition(rete_node* node, condition* cond,
                                          node_varnames* nvn, uint64_t pI_id,
                                          AddAdditionalTestsMode additional_tests);
        uint64_t get_new_inst_id() { increment_counter(inst_id_counter); return inst_id_counter; };
        uint64_t get_new_prod_id() { increment_counter(prod_id_counter); return prod_id_counter; };
        uint64_t get_instantiation_count() { return inst_id_counter; };

        /* identity generation functions */
        uint64_t get_or_create_identity(Symbol* orig_var);
        void     add_identity_to_test(test pTest);
        void     force_add_identity(Symbol* pSym, uint64_t pID);

        /* Identity set mapping functions */
        void update_remaining_identity_sets_in_test(test pTest, instantiation* pInst);
        void update_remaining_identity_sets_in_condlist(condition* pCondTop, instantiation* pInst);

        /* Methods for operator selection knowledge tracking. */
        void    add_to_OSK(slot* s, preference* pref, bool unique_value = true);
        void    copy_OSK(instantiation* inst);
        void    copy_proposal_OSK(instantiation* inst, cons* newOSK);
        void    update_proposal_OSK(slot* s, preference* winner);

        /* Methods used during condition copying to make unification and constraint
         * attachment more effecient */
        void        unify_identity(test t) { t->identity = get_identity(t->identity); }
        void        unify_preference_identities(preference* lPref);                         /* Not used currently */
        uint64_t    get_identity(uint64_t pID);
        bool        in_null_identity_set(test t);
        tc_number   get_constraint_found_tc_num() { return tc_num_found; };

        /* Methods to handle identity unification of conditions that test singletons */
        void                add_to_singletons(wme* pWME);
        bool                wme_is_a_singleton(wme* pWME);
        const std::string   add_new_singleton(singleton_element_type id_type, Symbol* attrSym, singleton_element_type value_type);
        const std::string   remove_singleton(singleton_element_type id_type, Symbol* attrSym, singleton_element_type value_type);

        /* Determines whether learning is on for a particular instantiation
         * based on the global learning settings and whether the state chunky */
        bool set_learning_for_instantiation(instantiation* inst);
        void set_failure_type(EBCFailureType pFailure_type) {m_failure_type = pFailure_type; };
        void set_rule_type(ebc_rule_type pRuleType) {m_rule_type = pRuleType; };
        void reset_chunks_this_d_cycle() { chunks_this_d_cycle = 0; justifications_this_d_cycle = 0;};

        /* RL templates utilize the EBChunker variablization code when building
         * template instances.  We make these two methods public to support that. */
        void        variablize_condition_list   (condition* top_cond, bool pInNegativeCondition = false);
        action*     variablize_rl_action        (action* pRLAction, struct token_struct* tok, wme* w, double & initial_value);

        /* Methods for printing in Soar trace */
        void print_current_built_rule(const char* pHeader = NULL);

        /* Debug printing methods */
        void print_variablization_table(TraceMode mode);
        void print_tables(TraceMode mode);
        void print_identity_tables(TraceMode mode);
        void print_attachment_points(TraceMode mode);
        void print_constraints(TraceMode mode);
        void print_merge_map(TraceMode mode);
        void print_instantiation_identities_map(TraceMode mode);
        void print_unification_map(TraceMode mode);

        void print_singleton_summary();
        const char* singletonTypeToString(singleton_element_type pType);
        void print_chunking_summary();
        void print_chunking_settings();

        /* Clean-up */
        void reinit();
        void clear_symbol_identity_map() { instantiation_identities->clear(); }
        void clear_variablization_maps();
        void clear_singletons();

    private:

        agent*              thisAgent;
        Output_Manager*     outputManager;
        Identity_Sets*      identitySets;

        /* Statistics on learning performed so far */
        uint64_t            chunk_naming_counter;
        uint64_t            justification_naming_counter;
        uint64_t            chunks_this_d_cycle;
        uint64_t            justifications_this_d_cycle;

        std::string*        chunk_history;

        /* String that every chunk name begins with */
        char*               chunk_name_prefix;
        char*               justification_name_prefix;

        /* -- A counter for variablization and instantiation id's - */
        uint64_t inst_id_counter;
        uint64_t ovar_id_counter;
        uint64_t prod_id_counter;

        tc_number tc_num_found;

        /* Variables used by dependency analysis methods */
        cons*               grounds;
        cons*               locals;
        chunk_cond_set      negated_set;
        tc_number           grounds_tc;
        tc_number           id_set_pass1_tc;
        tc_number           backtrace_number;
        uint64_t            m_current_bt_inst_id;

        /* Flags for potential issues encountered during dependency analysis */
        bool                m_correctness_issue_possible;
        bool                m_tested_quiescence;
        bool                m_tested_local_negation;
        bool                m_tested_deep_copy;
        bool                m_tested_ltm_recall;

        /* Variables used by result building methods */
        goal_stack_level    m_results_match_goal_level;
        tc_number           m_results_tc;
        preference*         m_extra_results;

        /* Variables to indicate current type of rule learning */
        bool                m_learning_on_for_instantiation;
        ebc_rule_type       m_rule_type;

        /* Intermediate rule structures */
        instantiation*      m_inst;
        preference*         m_results;
        condition*          m_lhs;
        action*             m_rhs;
        production*         m_prod;
        instantiation*      m_chunk_inst;

        /* Temporary structures */
        Symbol*             m_prod_name;
        ProductionType      m_prod_type;
        bool                m_should_print_name, m_should_print_prod;
        EBCFailureType      m_failure_type;

        /* Core tables used by EBC during identity assignment during instantiation
         * creation. The data stored within them is temporary and cleared after use. */

        sym_to_id_map*      instantiation_identities;
        id_to_sym_id_map*   identity_to_var_map;

        /* Map to unify variable identities into identity sets */
        id_to_id_map*       unification_map;
        identity_quadruple  local_singleton_superstate_identity;
        symbol_set*         local_singletons;
        symbol_set*         singletons;

        /* Data structures used to track and assign loose constraints */
        constraint_list*           constraints;
        attachment_points_map*     attachment_points;

        /* Table of previously seen conditions.  Used to determine whether to
         * merge or eliminate positive conditions on the LHS of a chunk. */
        triple_merge_map*               cond_merge_map;

        /* List of STIs created in the sub-state that are linked to LTMs.  Used to add link-stm-to-ltm actions */
        rhs_value_list*               local_linked_STIs;

        /* Explanation/identity generation methods */
        void            add_identity_to_id_test(condition* cond, byte field_num, rete_node_level levels_up);
        void            add_constraint_to_explanation(test* dest_test_address, test new_test, uint64_t pI_id, bool has_referent = true);
        void            add_explanation_to_RL_condition(rete_node* node, condition* cond, node_varnames* nvn,
                                                        uint64_t pI_id, AddAdditionalTestsMode additional_tests);
        /* Chunk building methods */
        Symbol*         generate_name_for_new_rule();
        void            set_up_rule_name();
        bool            can_learn_from_instantiation();
        void            get_results_for_instantiation();
        void            add_goal_or_impasse_tests();
        void            add_pref_to_results(preference* pref, uint64_t linked_id);
        void            add_results_for_id(Symbol* id, uint64_t linked_id);
        void            add_results_if_needed(Symbol* sym, uint64_t linked_id);
        action*         copy_action_list(action* actions);
        void            init_chunk_cond_set(chunk_cond_set* set);
        void            create_initial_chunk_condition_lists();
        bool            add_to_chunk_cond_set(chunk_cond_set* set, chunk_cond* new_cc);
        chunk_cond*     make_chunk_cond_for_negated_condition(condition* cond);
        void            make_clones_of_results();
        void            remove_chunk_instantiation();
        void            remove_from_chunk_cond_set(chunk_cond_set* set, chunk_cond* cc);
        bool            reorder_and_validate_chunk();
        void            deallocate_failed_chunk();
        void            clean_up(uint64_t pClean_up_id, bool clean_up_inst_inventory = true);
        bool            add_chunk_to_rete();

        /* Dependency analysis methods */
        void perform_dependency_analysis();
        void add_to_grounds(condition* cond);
        void add_to_locals(condition* cond);
        bool condition_is_operational(condition* cond, goal_stack_level grounds_level) { return  (cond->data.tests.id_test->eq_test->data.referent->id->level <= grounds_level); }
        void trace_locals(goal_stack_level grounds_level);
        void backtrace_through_instantiation(
                instantiation* inst,
                goal_stack_level grounds_level,
                condition* trace_cond,
                const identity_quadruple o_ids_to_replace,
                const rhs_quadruple rhs_funcs,
                uint64_t bt_depth,
                BTSourceType bt_type);
        void backtrace_through_OSK(cons* pOSKPref, goal_stack_level grounds_level, uint64_t lExplainDepth = 0);
        void report_local_negation(condition* c);

        void add_starting_identity_sets_to_cond(condition* trace_cond);
        void propagate_identity_sets(id_to_id_map* identity_set_mappings, condition* trace_cond, const identity_quadruple o_ids_to_replace);
        void propagate_and_unify_identity_sets(id_to_id_map* identity_set_mappings, condition* trace_cond, const identity_quadruple o_ids_to_replace, const rhs_quadruple rhs_funcs);
        void propagate_and_unify_element(id_to_id_map* identity_set_mappings, test parent_test, int64_t replaceID, rhs_value pRhs_func, uint64_t parent_inst_i_id);

        void btpass1_backtrace_through_instantiation(
            instantiation* inst,
            goal_stack_level grounds_level,
            condition* trace_cond,
            const identity_quadruple o_ids_to_replace,
            const rhs_quadruple rhs_funcs,
            uint64_t bt_depth,
            BTSourceType bt_type);
        void btpass1_backtrace_through_OSK(cons* pOSKPref, goal_stack_level grounds_level, uint64_t lExplainDepth = 0);
        void btpass1_trace_locals(goal_stack_level grounds_level);

        /* Identity analysis and unification methods */
        void add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id);
        void update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id, uint64_t pOld_o_id_2 = 0);
        void create_consistent_identity_for_result_element(preference* result, uint64_t pNew_i_id, WME_Field field);
        void unify_backtraced_conditions(condition* parent_cond, const identity_quadruple o_ids_to_replace, const rhs_quadruple rhs_funcs);
        void add_singleton_unification_if_needed(condition* pCond);
        void add_local_singleton_unification_if_needed(condition* pCond);
        void literalize_RHS_function_args(const rhs_value rv, uint64_t inst_id);
        void merge_conditions();

        /* Constraint analysis and enforcement methods */
        void cache_constraints_in_cond(condition* c);
        void add_additional_constraints();
        bool has_positive_condition(uint64_t pO_id);
        void cache_constraints_in_test(test t);
        void reset_constraint_found_tc_num() { if (!ebc_settings[SETTING_EBC_LEARNING_ON]) return; tc_num_found = get_new_tc_number(thisAgent); };
        attachment_point* get_attachment_point(uint64_t pO_id);
        void set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField);
        void find_attachment_points(condition* cond);
        void prune_redundant_constraints();
        void invert_relational_test(test* pEq_test, test* pRelational_test);
        void attach_relational_test(test pEq_test, test pRelational_test);

        /* Variablization methods */
        action* variablize_results_into_actions();
        action* variablize_result_into_action(preference* result, tc_number lti_link_tc);
        uint64_t variablize_rhs_symbol(rhs_value &pRhs_val, tc_number lti_link_tc = 0);
        void add_LTM_linking_actions(action* pLastAction);
        void variablize_equality_tests(test t);
        bool variablize_test_by_lookup(test t, bool pSkipTopLevelEqualities);
        void variablize_tests_by_lookup(test t, bool pSkipTopLevelEqualities);
        sym_identity_info* store_variablization(uint64_t pIdentity, Symbol* variable, Symbol* pMatched_sym);
        sym_identity_info* get_variablization(uint64_t index_id);

        void reinstantiate_test(test pTest);
        void reinstantiate_rhs_symbol(rhs_value pRhs_val);
        condition* reinstantiate_lhs(condition* top_cond);
        void reinstantiate_condition_list(condition* top_cond, bool pIsNCC = false);
        void reinstantiate_condition(condition* cond, bool pIsNCC = false);
        void reinstantiate_actions(action* pActionList);
        condition* reinstantiate_current_rule();
        action* convert_results_into_actions();
        action* convert_result_into_action(preference* result);

        /* Methods to make sure that we didn't create rules that are bad but the RETE would accept */
        void sanity_chunk_conditions(condition* top_cond);
        void sanity_chunk_test (test pTest);
        void sanity_justification_test (test pTest, bool pIsNCC = false);

        /* Condition polishing methods */
        void        remove_ungrounded_sti_from_test_and_cache_eq_test(test* t);
        void        merge_values_in_conds(condition* pDestCond, condition* pSrcCond);
        condition*  get_previously_seen_cond(condition* pCond);

        /* Clean-up methods */
        void clear_merge_map();
        void clear_attachment_map();
        void clear_cached_constraints();
        void clear_o_id_substitution_map()      { unification_map->clear(); }
        void clear_rulesym_to_identity_map()    { instantiation_identities->clear(); }
        void clear_local_arch_singletons()      { local_singleton_superstate_identity = { 0, 0, 0, 0}; }
        void clear_data();

};
#endif /* EBC_MANAGER_H_ */
