/*
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_MANAGER_H_
#define EBC_MANAGER_H_

#define EBC_SANITY_CHECK_RULES

#include "kernel.h"

#include "ebc_structs.h"
#include "ebc_identity_set.h"
#include "stl_typedefs.h"

#include <list>
#include <set>
#include <unordered_map>
#include <cstdlib>

tc_number       get_new_tc_number(agent* thisAgent);
uint64_t        get_joined_identity_id(IdentitySet* pID_Set);
IdentitySet*    get_joined_identity_set(IdentitySet* pID_Set);
uint64_t        get_joined_identity_clone_id(IdentitySet* pID_Set);
void            IdentitySet_remove_ref(agent* thisAgent, IdentitySet* &pID_Set);
void            set_test_identity_set(agent* thisAgent, test pTest, IdentitySet* pID_Set);
void            clear_test_identity_set(agent* thisAgent, test pTest);

class Explanation_Based_Chunker
{
        friend class Repair_Manager;
        friend class IdentitySet;

    public:

        Explanation_Based_Chunker(agent* myAgent);
        ~Explanation_Based_Chunker();

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
        void learn_rule_from_instance(instantiation* inst, instantiation** new_inst_list);

        /* Methods used during instantiation creation to generate identities used by the
         * explanation trace. */
        void        add_explanation_to_condition(rete_node* node, condition* cond, node_varnames* nvn, ExplainTraceType additional_tests);
        uint64_t    get_new_inst_id()               { increment_counter(inst_id_counter); return inst_id_counter; };
        uint64_t    get_new_prod_id()               { increment_counter(prod_id_counter); return prod_id_counter; };
        uint64_t    get_instantiation_count()       { return inst_id_counter; };
        uint64_t    get_new_var_identity_id()       { increment_counter(variablization_identity_counter); return variablization_identity_counter; };
        uint64_t    get_new_identity_set_id()       { increment_counter(identity_set_counter); return identity_set_counter; };
        void        reset_identity_set_id_counter() {identity_set_counter = 0; };

        /* identity generation functions */
        uint64_t get_or_create_identity_for_sym(Symbol* orig_var);
        void     add_identity_to_test(test pTest);
        void     force_add_identity(Symbol* pSym, uint64_t pID);

        /* Identity set mapping functions */
//        IdentitySet* NULL_IDENTITY_SET;

        void update_identity_sets_in_test(test pTest, instantiation* pInst);
        void update_identity_sets_in_cond(condition* pCond, instantiation* pInst);
        void update_identity_sets_in_condlist(condition* pCondTop, instantiation* pInst);
        void update_identity_sets_in_preferences(preference* lPref);

        /* Methods for operator selection knowledge tracking. */
        void    add_to_OSK(slot* s, preference* pref, bool unique_value = true);
        void    copy_OSK(instantiation* inst);
        void    copy_proposal_OSK(instantiation* inst, cons* newOSK);
        void    update_proposal_OSK(slot* s, preference* winner);

        /* Methods for identity set propagation and analysis */
        IdentitySet*   make_identity_set(uint64_t pIDSet);
        IdentitySet*   get_id_set_for_identity(uint64_t pID);
        IdentitySet*   get_or_add_id_set(uint64_t pID, IdentitySet* pIDSet);
        IdentitySet*   get_floating_identity_set();
        void           force_identity_to_id_set_mapping(uint64_t pID, IdentitySet* pIDSet)    { (*identities_to_id_sets)[pID] = pIDSet; };

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


        /* Some methods used for old school soar identifier variablization that
         * are used in a few places where identity-based variablization doesn't
         * make sense:
         * 1 - Used by repair manager when creating grounding conditions
         * 2 - Used by reinforcement learning when building template instances. */

        void        add_sti_variablization(Symbol* pSym, Symbol* pVar, uint64_t pIdentity);
        void        sti_variablize_test(test pTest, bool generate_identity = true);
        void        sti_variablize_rhs_symbol(rhs_value &pRhs_val, bool generate_identity = true);
        void        clear_sti_variablization_map() { m_sym_to_var_map->clear(); };

        void        variablize_rl_condition_list   (condition* top_cond);
        action*     variablize_rl_action        (action* pRLAction, struct token_struct* tok, wme* w, double & initial_value);

        /* Methods for printing in Soar trace */
        void print_current_built_rule(const char* pHeader = NULL);

        /* Debug printing methods */
        void print_variablization_table(TraceMode mode);
        void print_tables(TraceMode mode);
        void print_identity_tables(TraceMode mode);
        void print_constraints(TraceMode mode);
        void print_merge_map(TraceMode mode);
        void print_instantiation_identities_map(TraceMode mode);
        void print_identity_to_id_set_map(TraceMode mode);
        void print_identity_set_join_map(TraceMode mode);

        void print_singleton_summary();
        const char* singletonTypeToString(singleton_element_type pType);
        void print_chunking_summary();
        void print_chunking_settings();

        /* Clean-up */
        void reinit();
        void clear_symbol_identity_map()        { instantiation_identities->clear(); }
        void clear_identity_to_id_set_map()     { identities_to_id_sets->clear(); }
        void clear_singletons();

    private:

        agent*              thisAgent;
        Output_Manager*     outputManager;

        /* Statistics on learning performed so far */
        uint64_t            chunk_naming_counter;
        uint64_t            justification_naming_counter;
        uint64_t            chunks_this_d_cycle;
        uint64_t            justifications_this_d_cycle;

        /* String that every chunk name begins with */
        char*               chunk_name_prefix;
        char*               justification_name_prefix;

        /* -- A counter for variablization and instantiation id's - */
        uint64_t inst_id_counter;
        uint64_t identity_set_counter;
        uint64_t variablization_identity_counter;
        uint64_t prod_id_counter;

        /* Variables used by dependency analysis methods */
        cons*               grounds;
        cons*               locals;
        chunk_cond_set      negated_set;
        tc_number           grounds_tc;
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
        goal_stack_level    m_goal_level;
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
        id_to_join_map*     identities_to_id_sets;

        /* A variablization map used for old school soar identifier variablization */
        sym_to_sym_id_map*  m_sym_to_var_map;

        /* Identity sets that have had their transient data changed */
        identity_set_set    identity_sets_to_clean_up;  // cleaned up after a rule is learned

        /* Set of all attribute symbols that may be singletons */
        symbol_set*         singletons;

        /* Data structures used to track and assign loose constraints */
        constraint_list*    constraints;

        /* Table of previously seen conditions.  Used to determine whether to
         * merge or eliminate positive conditions on the LHS of a chunk. */
        triple_merge_map*   cond_merge_map;

        /* List of STIs created in the sub-state that are linked to LTMs.  Used to add link-stm-to-ltm actions */
        rhs_value_list*     local_linked_STIs;

        /* Explanation/identity generation methods */
        void            add_var_test_bound_identity_to_id_test(condition* cond, byte field_num, rete_node_level levels_up);
        void            add_constraint_to_explanation(test* dest_test_address, test new_test, bool has_referent = true);
        void            add_explanation_to_RL_condition(rete_node* node, condition* cond);

        /* Chunk building methods */
        Symbol*         generate_name_for_new_rule();
        void            set_up_rule_name();
        bool            can_learn_from_instantiation();
        void            get_results_for_instantiation();
        void            add_goal_or_impasse_tests();
        void            add_pref_to_results(preference* pref, preference* pPref, WME_Field pField);
        void            add_results_if_needed(Symbol* sym, preference* pPref, WME_Field pField);

        void            init_chunk_cond_set(chunk_cond_set* set);
        void            create_initial_chunk_condition_lists();
        bool            add_to_chunk_cond_set(chunk_cond_set* set, chunk_cond* new_cc);
        chunk_cond*     make_chunk_cond_for_negated_condition(condition* cond);
        void            merge_conditions();
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
        void trace_locals();
        void backtrace_through_instantiation(preference* pPref, condition* trace_cond, uint64_t bt_depth, BTSourceType bt_type);
        void backtrace_through_OSK(cons* pOSKPref, uint64_t lExplainDepth = 0);
        void report_local_negation(condition* c);

        /* Identity analysis and unification methods */
        void join_identity_sets(IdentitySet* lFromJoinSet, IdentitySet* lToJoinSet);
        void unify_backtraced_conditions(condition* parent_cond, identity_set_quadruple &o_ids_to_replace, const rhs_quadruple rhs_funcs);
        void add_singleton_unification_if_needed(condition* pCond);
        void literalize_RHS_function_args(const rhs_value rv, uint64_t inst_id);

        /* Constraint analysis and enforcement methods */
        void cache_constraints_in_cond(condition* c);
        void add_additional_constraints();
        void cache_constraints_in_test(test t);
        void invert_relational_test(test* pEq_test, test* pRelational_test);
        void attach_relational_test(test pRelational_test, condition* pCond, WME_Field pField);

        /* Variablization methods */
        void        variablize_rl_test(test t);
        bool        variablize_test_by_lookup(test t, bool pSkipTopLevelEqualities);
        void        variablize_tests_by_lookup(test t, bool pSkipTopLevelEqualities);
        void        variablize_equality_tests(test t);
        void        variablize_condition_list   (condition* top_cond, bool pInNegativeCondition = false);

        uint64_t    variablize_rhs_symbol(rhs_value &pRhs_val, tc_number lti_link_tc = 0);
        action*     variablize_result_into_action(preference* result, tc_number lti_link_tc);
        action*     variablize_results_into_actions();

        void        add_LTM_linking_actions(action* pLastAction);

        void        reinstantiate_test(test pTest, bool pIsInstantiationCond);
        void        reinstantiate_rhs_symbol(rhs_value pRhs_val);
        condition*  reinstantiate_lhs(condition* top_cond);
        void        reinstantiate_condition_list(condition* top_cond, bool pIsInstantiationCond, bool pIsNCC = false);
        void        reinstantiate_condition(condition* cond, bool pIsInstantiationCond, bool pIsNCC = false);
        void        reinstantiate_actions(action* pActionList);
        condition*  reinstantiate_current_rule();

        void        update_identities_in_equality_tests(test t);
        void        update_identities_in_tests_by_lookup(test t, bool pSkipTopLevelEqualities);
        bool        update_identities_in_test_by_lookup(test t, bool pSkipTopLevelEqualities);
        void        update_identities_in_condition_list(condition* top_cond, bool pInNegativeCondition = false);
        action*     convert_results_into_actions();
        action*     convert_result_into_action(preference* result);

        /* Methods to make sure that we didn't create rules that are bad but the RETE would accept */
        void sanity_chunk_conditions(condition* top_cond);
        void sanity_chunk_test (test pTest);
        void sanity_justification_test (test pTest, bool pIsNCC = false);

        /* Condition polishing methods */
        void        remove_ungrounded_sti_from_test_and_cache_eq_test(test* t);
        void        merge_values_in_conds(condition* pDestCond, condition* pSrcCond);
        condition*  get_previously_seen_cond(condition* pCond);

        /* Clean-up methods */
        void clean_up_identity_sets();
        void clear_merge_map();
        void clear_cached_constraints();
        void clear_data();

};

#endif /* EBC_MANAGER_H_ */
