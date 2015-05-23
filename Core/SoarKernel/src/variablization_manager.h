/*
 * variablization_manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#ifndef VARIABLIZATION_MANAGER_H_
#define VARIABLIZATION_MANAGER_H_

#include "portability.h"
#include "symtab.h"
#include "test.h"
#include <list>

typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct preference_struct preference;
typedef char* rhs_value;
typedef struct chunk_cond_struct chunk_cond;
tc_number get_new_tc_number(agent* thisAgent);

typedef struct variablization_struct
{
    Symbol* instantiated_symbol;
    Symbol* variablized_symbol;
    variablization_struct() : instantiated_symbol(NULL), variablized_symbol(NULL) {}
} variablization;

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

        uint64_t get_new_inst_id() { return (++inst_id_counter); };
        uint64_t get_new_ovar_id() { return (++ovar_id_counter); };

        void clear_variablization_maps();
        void clear_o_id_update_map();
        void clear_attachment_map();
        void clear_cached_constraints();
        void clear_ovar_to_o_id_map();
        void clear_merge_map();
        void clear_o_id_to_ovar_debug_map();
        void clear_o_id_substitution_map();
        void clear_data();
        void reinit();

        uint64_t get_existing_o_id(Symbol* orig_var, uint64_t pI_id);
        uint64_t get_or_create_o_id(Symbol* orig_var, uint64_t pI_id);
        Symbol * get_ovar_for_o_id(uint64_t o_id);

        void reset_constraint_found_tc_num() { tc_num_found = get_new_tc_number(thisAgent); };
        tc_number get_constraint_found_tc_num() { return tc_num_found; };

        void cache_constraints_in_cond(condition* c);
        void add_additional_constraints(condition* cond, uint64_t pI_id);
        bool has_positive_condition(uint64_t pO_id);

        void add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id);
        void unify_identity(agent* thisAgent, test t);

        void create_consistent_identity_for_chunk(uint64_t* pO_id, uint64_t pNew_i_id, bool pIsResult = false);

        void fix_conditions(condition* top_cond, uint64_t pI_id, bool ignore_ungroundeds = false);
        void fix_results(preference* result, uint64_t pI_id);
        void merge_conditions(condition* top_cond);

        void      variablize_relational_constraints();
        void      variablize_condition_list(condition* top_cond, bool pInNegativeCondition = false);
        void      variablize_rl_condition_list(condition* top_cond, bool pInNegativeCondition = false);

        action* variablize_results(preference* result, bool variablize);
        action* make_variablized_rl_action(Symbol* id_sym, Symbol* attr_sym, Symbol* val_sym, Symbol* ref_sym);

        void print_OSD_table(TraceMode mode);
        void print_variablization_tables(TraceMode mode, int whichTable = 0);
        void print_tables(TraceMode mode);
        void print_o_id_update_map(TraceMode mode, bool printHeader = true);
        void print_o_id_tables(TraceMode mode);
        void print_attachment_points(TraceMode mode);
        void print_constraints(TraceMode mode);
        void print_merge_map(TraceMode mode);
        void print_ovar_to_o_id_map(TraceMode mode);
        void print_o_id_substitution_map(TraceMode mode);
        void print_o_id_to_ovar_debug_map(TraceMode mode);

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
        void variablize_rhs_symbol(rhs_value pRhs_val);

        void variablize_equality_tests(test* t);

        void variablize_rl_test(test* chunk_test);

        void variablize_test_by_lookup(test* t, bool pSkipTopLevelEqualities);
        void variablize_tests_by_lookup(test* t, bool pSkipTopLevelEqualities);

        void remove_ungrounded_sti_tests(test* t, bool ignore_ungroundeds);
        void merge_values_in_conds(condition* pDestCond, condition* pSrcCond);
        condition* get_previously_seen_cond(condition* pCond);

        uint64_t get_updated_o_id_info(uint64_t old_o_id);
        void add_updated_o_id_info(uint64_t old_o_id, uint64_t new_o_id);
        void update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id, uint64_t pOld_o_id_2 = 0);
        void unify_identity_for_result_element(agent* thisAgent, preference* result, WME_Field field);
        void create_consistent_identity_for_result_element(preference* result, uint64_t pNew_i_id, WME_Field field);

        void cache_constraints_in_test(test t);

        attachment_point* get_attachment_point(uint64_t pO_id);
        void set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField);
        void find_attachment_points(condition* cond);
        void prune_redundant_constraints();
        void invert_relational_test(test* pEq_test, test* pRelational_test);
        void attach_relational_test(test pEq_test, test pRelational_test, uint64_t pI_id);

        /* -- The following are tables used by the variablization manager during
         *    instantiation creation, backtracing and chunk formation.  The data
         *    they store is temporary and cleared after use. -- */

        std::map< Symbol*, std::map< uint64_t, uint64_t > >*    ovar_to_o_id_map;
        std::map< uint64_t, Symbol* >*                          o_id_to_ovar_debug_map;

        /* This is a map of original variable symbols to its map of instantiations to o_ids */
        std::map< uint64_t, uint64_t >*                         unification_map;
        std::map< uint64_t, uint64_t >*                         o_id_update_map;

        /* -- Look-up tables for LHS variablization -- */
        std::map< uint64_t, variablization* >*   o_id_to_var_map;
        std::map< Symbol*, variablization* >*    sym_to_var_map;

        std::list< constraint* >* constraints;
        std::map< uint64_t, attachment_point* >* attachment_points;

        /* -- Table of previously seen conditions.  Used to determine whether to
         *    merge or eliminate positive conditions on the LHS of a chunk. -- */
        std::map< Symbol*, std::map< Symbol*, std::map< Symbol*, condition*> > >* cond_merge_map;

        /* -- A counter for variablization and instantiation id's. 0 is the default
         *    value and not considered a valid id. -- */
        uint64_t inst_id_counter;
        uint64_t ovar_id_counter;

        tc_number tc_num_found;

};

#endif /* VARIABLIZATION_MANAGER_H_ */
