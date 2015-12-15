/*
 * variablization_manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#ifndef EBC_MANAGER_H_
#define EBC_MANAGER_H_

#include "portability.h"
#include "symtab.h"
#include "test.h"
#include <list>
#include <set>
#include <unordered_map>

#define BUFFER_PROD_NAME_SIZE 256
#define CHUNK_COND_HASH_TABLE_SIZE 1024
#define LOG_2_CHUNK_COND_HASH_TABLE_SIZE 10

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

tc_number get_new_tc_number(agent* thisAgent);
class Output_Manager;

namespace soar_module
{
    typedef struct symbol_triple_struct symbol_triple;
    typedef struct identity_triple_struct identity_triple;
    typedef struct rhs_triple_struct rhs_triple;
}

typedef struct constraint_struct
{
    test eq_test;
    test constraint_test;
    constraint_struct(test new_eq, test new_constraint) : eq_test(new_eq), constraint_test(new_constraint) {}
} constraint;

typedef struct attachment_struct
{
        condition* cond;
        WME_Field field;
        attachment_struct(condition* new_cond, WME_Field new_field) : cond(new_cond), field(new_field) {}

} attachment_point;

typedef struct chunk_cond_struct
{
    condition* cond;                /* points to the original condition */

    condition* instantiated_cond;   /* points to cond in chunk instantiation */
    condition* variablized_cond;    /* points to cond in the actual chunk */

    /* dll of all cond's in a set (i.e., a chunk_cond_set, or the grounds) */
    struct chunk_cond_struct* next, *prev;

    /* dll of cond's in this particular hash bucket for this set */
    struct chunk_cond_struct* next_in_bucket, *prev_in_bucket;

    uint32_t hash_value;             /* equals hash_condition(cond) */
    uint32_t compressed_hash_value;  /* above, compressed to a few bits */
} chunk_cond;

typedef struct chunk_cond_set_struct
{
    chunk_cond* all;       /* header for dll of all chunk_cond's in the set */
    chunk_cond* table[CHUNK_COND_HASH_TABLE_SIZE];  /* hash table buckets */
} chunk_cond_set;

typedef struct backtrace_struct
{
    int result;                    /* 1 when this is a result of the chunk */
    condition* trace_cond;         /* The (local) condition being traced */
    char   prod_name[BUFFER_PROD_NAME_SIZE];         /* The production's name */
    condition* grounds;            /* The list of conds for the LHS of chunk */
    condition* potentials;         /* The list of conds which aren't linked */
    condition* locals;             /* Conds in the subgoal -- need to BT */
    condition* negated;            /* Negated conditions (sub/super) */
    struct backtrace_struct* next_backtrace; /* Pointer to next in this list */
} backtrace_str;



/* -- Variablization_Manager
 *
 * variablization_table
 *
 *    The variablization_table is used during chunking.  It stores a mapping from either a
 *    symbol's original variable, if available, or the actual symbol to the variable
 *    that was created for variablization.  It is keep track of the current variablized
 *    symbols in a chunk that is being built. This mapping is temporary and cleared after
 *    the chunk is built. This replaces the variablized pointer in versions of Soar
 *    prior to 9.4
 *
 * original_symbol_ht and original_symbol_mp
 *
 *     A hash table and memory pool to store data structures that keep track of original
 *    symbols.  Used to keep track of the next unique symbol to generate
 *
 * -- */

class Explanation_Based_Chunker
{
    public:

        uint64_t get_new_inst_id() { return (++inst_id_counter); };

        bool learning_is_on_for_instantiation() { return m_learning_on_for_instantiation; };
        bool set_learning_for_instantiation(instantiation* inst);

        /* Core public chunking methods */
        void build_chunk_or_justification(instantiation* inst, instantiation** custom_inst_list);
        chunk_cond* make_chunk_cond_for_negated_condition(condition* cond);
        bool add_to_chunk_cond_set(chunk_cond_set* set, chunk_cond* new_cc);

        /* Explanation/identity generation methods */
        void add_identity_to_id_test(condition* cond, byte field_num, rete_node_level levels_up);
        void add_constraint_to_explanation(test* dest_test_address, test new_test, uint64_t pI_id, bool has_referent = true);
        void add_explanation_to_RL_condition(rete_node* node, condition* cond,
            wme* w, node_varnames* nvn, uint64_t pI_id, AddAdditionalTestsMode additional_tests);
        void add_explanation_to_condition(rete_node* node, condition* cond,
            wme* w, node_varnames* nvn, uint64_t pI_id, AddAdditionalTestsMode additional_tests);

        /* Variablization methods */
        void variablize_condition_list(condition* top_cond, bool pInNegativeCondition = false);
        void variablize_rl_condition_list(condition* top_cond, bool pInNegativeCondition = false);
        action* variablize_results_into_actions(preference* result, bool variablize);
        action* make_variablized_rl_action(Symbol* id_sym, Symbol* attr_sym, Symbol* val_sym, Symbol* ref_sym);

        /* Clean-up */
        void cleanup_for_instantiation_deallocation(uint64_t pI_id);
        void clear_variablization_maps();
        void clear_attachment_map();
        void clear_cached_constraints();
        void clear_o_id_substitution_map();
        void clear_data();
        void reinit();

        uint64_t get_existing_o_id(Symbol* orig_var, uint64_t pI_id);
        uint64_t get_or_create_o_id(Symbol* orig_var, uint64_t pI_id);
        Symbol * get_ovar_for_o_id(uint64_t o_id);

        void reset_constraint_found_tc_num() { if (!m_learning_on) return; tc_num_found = get_new_tc_number(thisAgent); };
        tc_number get_constraint_found_tc_num() { return tc_num_found; };

        /* Constraint analysis and enforcement methods */
        void cache_constraints_in_cond(condition* c);
        void add_additional_constraints(condition* cond);
        bool has_positive_condition(uint64_t pO_id);

        /* Identity analysis and unification methods */
        void add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id);
        void unify_identity(test t);
        bool unify_backtraced_dupe_conditions(condition* ground_cond, condition* new_cond);
        void unify_backtraced_conditions(condition* parent_cond,
            const soar_module::identity_triple o_ids_to_replace,
            const soar_module::rhs_triple rhs_funcs);
        void literalize_RHS_function_args(const rhs_value rv);
        bool in_null_identity_set(test t);
        void unify_identities_for_results(preference* result);
        void merge_conditions(condition* top_cond);

        void print_variablization_tables(TraceMode mode, int whichTable = 0);
        void print_tables(TraceMode mode);
        void print_o_id_tables(TraceMode mode);
        void print_attachment_points(TraceMode mode);
        void print_constraints(TraceMode mode);
        void print_merge_map(TraceMode mode);
        void print_ovar_to_o_id_map(TraceMode mode);
        void print_o_id_substitution_map(TraceMode mode);
        void print_o_id_to_ovar_debug_map(TraceMode mode);

        Explanation_Based_Chunker(agent* myAgent);
        ~Explanation_Based_Chunker();

    private:
        agent* thisAgent;
        Output_Manager* outputManager;

        /* Dependency analysis methods */
        void add_to_grounds(condition* cond);
        void add_to_potentials(condition* cond);
        void add_to_locals(condition* cond);
        void trace_locals(goal_stack_level grounds_level, bool* reliable);
        void trace_grounded_potentials();
        bool trace_ungrounded_potentials(goal_stack_level grounds_level, bool* reliable);
        void backtrace_through_instantiation(
                instantiation* inst,
                goal_stack_level grounds_level,
                condition* trace_cond,
                bool* reliable,
                int indent,
                const soar_module::identity_triple o_ids_to_replace,
                const soar_module::rhs_triple rhs_funcs);
        void report_local_negation(condition* c);

        /* Chunk building methods */
        void add_pref_to_results(preference* pref);
        void add_results_for_id(Symbol* id);
        void add_results_if_needed(Symbol* sym);
        preference* get_results_for_instantiation(instantiation* inst);
        action* copy_action_list(action* actions);
        void init_chunk_cond_set(chunk_cond_set* set);
        void remove_from_chunk_cond_set(chunk_cond_set* set, chunk_cond* cc);
        void create_instantiated_counterparts(condition* vrblz_top, condition** inst_top, condition** inst_bottom);
        void build_chunk_conds_for_grounds_and_add_negateds(condition** inst_top, condition** inst_bottom, condition** vrblz_top, tc_number tc_to_use, bool* reliable);
        void add_goal_or_impasse_tests(condition* inst_top, condition* vrblz_top);
        void reorder_instantiated_conditions(condition* top_cond, condition** dest_inst_top, condition** dest_inst_bottom);
        void make_clones_of_results(preference* results, instantiation* chunk_inst);
        Symbol* generate_chunk_name_str_constant(instantiation* inst);
        void chunk_instantiation_cleanup (Symbol** prod_name, condition** vrblz_top);

        /* Identity analysis and unification methods */
        void update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id, uint64_t pOld_o_id_2 = 0);
        void unify_identity_for_result_element(preference* result, WME_Field field);
        void create_consistent_identity_for_result_element(preference* result, uint64_t pNew_i_id, WME_Field field);

        /* Constraint analysis and enforcement methods */
        void cache_constraints_in_test(test t);
        attachment_point* get_attachment_point(uint64_t pO_id);
        void set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField);
        void find_attachment_points(condition* cond);
        void prune_redundant_constraints();
        void invert_relational_test(test* pEq_test, test* pRelational_test);
        void attach_relational_test(test pEq_test, test pRelational_test);

        /* Variablization methods */
        void store_variablization(Symbol* instantiated_sym, Symbol* variable, uint64_t pIdentity);
        Symbol* get_variablization_for_identity(uint64_t index_id);
        Symbol* get_variablization_for_sti(Symbol* index_sym);
        void variablize_lhs_symbol(Symbol** sym, uint64_t pIdentity);
        void variablize_rhs_symbol(rhs_value pRhs_val);
        void variablize_equality_tests(test t);
        void variablize_rl_test(test chunk_test);
        bool variablize_test_by_lookup(test t, bool pSkipTopLevelEqualities);
        void variablize_tests_by_lookup(test t, bool pSkipTopLevelEqualities);

        /* Condition polishing methods */
        void remove_ungrounded_sti_from_test_and_cache_eq_test(test* t);
        void merge_values_in_conds(condition* pDestCond, condition* pSrcCond);
        condition* get_previously_seen_cond(condition* pCond);

        /* Clean-up methods */
        void clear_merge_map();
        void clear_o_id_to_ovar_debug_map();
        void clear_rulesym_to_identity_map();

        /* -- The following are tables used by the variablization manager during
         *    instantiation creation, backtracing and chunk formation.  The data
         *    they store is temporary and cleared after use. -- */

        std::unordered_map< uint64_t, std::unordered_map< Symbol*, uint64_t > >*    rulesym_to_identity_map;
        std::unordered_map< uint64_t, Symbol* >*                          o_id_to_ovar_debug_map;
        std::unordered_map< uint64_t, uint64_t >*                         unification_map;

        /* -- Look-up tables for LHS variablization -- */
        std::unordered_map< uint64_t, Symbol* >*   o_id_to_var_map;
        std::unordered_map< Symbol*, Symbol* >*    sym_to_var_map;

        std::list< constraint* >* constraints;
        std::unordered_map< uint64_t, attachment_point* >* attachment_points;

        /* -- Table of previously seen conditions.  Used to determine whether to
         *    merge or eliminate positive conditions on the LHS of a chunk. -- */
        std::unordered_map< Symbol*, std::unordered_map< Symbol*, std::unordered_map< Symbol*, condition*> > >* cond_merge_map;

        /* -- A counter for variablization and instantiation id's. 0 is the default
         *    value and not considered a valid id. -- */
        uint64_t inst_id_counter;
        uint64_t ovar_id_counter;

        bool m_learning_on_for_instantiation;
        bool m_learning_on;

        tc_number tc_num_found;

};

#endif /* EBC_MANAGER_H_ */
