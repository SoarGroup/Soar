/*
 * variablization_manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#ifndef VARIABLIZATION_MANAGER_H_
#define VARIABLIZATION_MANAGER_H_

#include "portability.h"
#include <set>
#include "symtab.h"
#include "test.h"

typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct preference_struct preference;
typedef char* rhs_value;

typedef struct variablization_struct
{
    Symbol* instantiated_symbol;
    Symbol* variablized_symbol;
    uint64_t grounding_id;
} variablization;

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

/* MToDo | Check if all of these functions really still need to be public */

class Variablization_Manager
{
    public:

        uint64_t get_new_ground_id()
        {
            return (++ground_id_counter);
        };

        void clear_ovar_gid_table();
        void clear_cached_constraints();
        void clear_variablization_tables();
        void clear_merge_map();
        void clear_substitution_map();
        void clear_data();
        void reinit();

        uint64_t add_orig_var_to_gid_mapping(Symbol* index_sym, uint64_t index_g_id);
        uint64_t get_gid_for_orig_var(Symbol* index_sym);
        void update_g_ids(instantiation* inst);
        void cache_constraints_in_cond(condition* c);
        void install_cached_constraints(condition* cond);

        void fix_conditions(condition* top_cond);
        void consolidate_variables(condition* top_cond, tc_number tc_num);
        void merge_conditions(condition* top_cond);

        void      variablize_relational_constraints();

        void      variablize_condition_list(condition* top_cond, bool pInNegativeCondition = false);
        void      variablize_rl_condition_list(condition* top_cond, bool pInNegativeCondition = false);

        action* variablize_results(preference* result, bool variablize);
        action* make_variablized_rl_action(Symbol* id_sym, Symbol* attr_sym, Symbol* val_sym, Symbol* ref_sym);

        void print_OSD_table();
        void print_variablization_tables(TraceMode mode, int whichTable = 0);
        void print_tables();
        void print_cached_constraints(TraceMode mode);
        void print_merge_map(TraceMode mode);
        void print_ovar_gid_propogation_table(TraceMode mode, bool printHeader = false);

        Variablization_Manager(agent* myAgent);
        ~Variablization_Manager();

    private:
        agent* thisAgent;

        void store_variablization(Symbol* instantiated_sym, Symbol* variable, identity_info* identity);

        variablization* get_variablization_for_symbol(std::map< Symbol*, variablization* >* pMap, Symbol* index_sym);
        variablization* get_variablization(uint64_t index_id);
        variablization* get_variablization(test equality_test);
        variablization* get_variablization(Symbol* index_sym);

        void variablize_lhs_symbol(Symbol** sym, identity_info* identity);
        void variablize_rhs_symbol(rhs_value pRhs_val, Symbol* original_var);

        void variablize_test(test* t, Symbol* original_referent);
        void variablize_equality_test(test* t);
        void variablize_equality_tests(test* t);

        void variablize_rl_test(test* chunk_test);

        bool variablize_test_by_lookup(test* t, bool pSkipTopLevelEqualities);
        void variablize_tests_by_lookup(test* t, bool pSkipTopLevelEqualities);

        test      get_substitution(Symbol* sym);
        void      set_substitution(test sacrificeSymTest, test survivorSymTest, tc_number tc_num);
        void      update_ovar_table_for_sub(test sacrificeSymTest, test survivorSymTest);
        void      consolidate_variables_in_test(test t, tc_number tc_num);
        void      remove_redundancies_and_ungroundeds(test* t, tc_number tc_num);

        void      merge_values_in_conds(condition* pDestCond, condition* pSrcCond);
        condition* get_previously_seen_cond(condition* pCond);

        void cache_constraint(test equality_test, test relational_test);
        void cache_constraints_in_test(test t);
        void variablize_cached_constraints_for_symbol(::list** constraint_list);
        void install_cached_constraints_for_test(test* t);

        /* -- The following are tables used by the variablization manager during
         *    instantiation creation, backtracing and chunk formation.  The data
         *    they store is temporary and cleared after use. -- */

        /* -- Look-up tables for LHS variablization -- */
        std::map< Symbol*, uint64_t >*           orig_var_to_g_id_map;
        std::map< uint64_t, variablization* >*   g_id_to_var_map;
        std::map< Symbol*, variablization* >*   sym_to_var_map;

        /* -- Cache of constraint tests collected during backtracing -- */
        std::map< Symbol*, ::list* >*           sti_constraints;
        std::map< uint64_t, ::list* >*           constant_constraints;

        /* -- Table of previously seen conditions.  Used to determine whether to
         *    merge or eliminate positive conditions on the LHS of a chunk. -- */
        std::map< Symbol*, std::map< Symbol*, std::map< Symbol*, condition*> > >* cond_merge_map;
        std::map< Symbol*, test >* substitution_map;

        /* -- A counter for the next grounding id to assign. 0 is the default
         *    value and not considered a valid grounding id. -- */
        uint64_t ground_id_counter;
};

#endif /* VARIABLIZATION_MANAGER_H_ */
