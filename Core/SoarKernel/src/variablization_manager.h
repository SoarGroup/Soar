/*
 * variablization_manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#ifndef VARIABLIZATION_MANAGER_H_
#define VARIABLIZATION_MANAGER_H_

#include <portability.h>
#include <set>
#include "symtab.h"
#include "test.h"

typedef struct condition_struct condition;

/* -- Following requires that both tests exists and are equality tests -- */
//struct cmp_constraint_eq_test {
//    bool operator()(test a,  test b) const {
//      assert (a && b);
//      return (a->data.referent == b->data.referent);
//    }
//};


typedef struct variablization_struct {
    Symbol *instantiated_symbol;
    Symbol *variablized_symbol;
    uint64_t grounding_id;
    bool grounded;
} variablization;

/* -- Variablization_Manager
 *
 * make_name_unique
 *
 *    Takes the name of a variable and potentially modifies it so
 *    that is is unique to the current instantiation being built
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
 * current_unique_vars
 *
 *    The current_unique_vars set is used during instantiation creation.  It keeps a list
 *    of all original vars in the current instantiation that have been made unique.
 *    This is needed so we don't try to make an original variable unique twice
 *    because it appears in two different conditions
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

    uint64_t get_new_ground_id() {return (++ground_id_counter);};

    void clear_variablization_table();
    void reinit();

    void add_orig_var_mappings(condition *cond);

    void add_relational_constraints(condition *cond);
    void clear_relational_constraints ();
    void cache_relational_constraint (test equality_test, test relational_test);

    void merge_conditions(condition **top_cond);
    void clear_merge_map();

    void      variablize_lhs_symbol (Symbol **sym, Symbol *original_symbol,
                                     identity_info *identity, bool is_equality_test);
    void      variablize_rl_symbol (Symbol **sym, bool is_equality_test);
    uint64_t  variablize_rhs_symbol (Symbol **sym, Symbol *original_var);
    void      variablize_condition_list (condition *cond);
    void      variablize_relational_constraints();

    void print_OSD_table();
    void print_variablization_tables(TraceMode mode, int whichTable=0);
    void print_tables();
    void print_relational_constraints (TraceMode mode);
    void print_merge_map (TraceMode mode);

    Variablization_Manager(agent *thisAgent);
    ~Variablization_Manager();

  private:
    agent* thisAgent;

    void store_variablization(Symbol *instantiated_sym, Symbol *variable,
                              identity_info *identity, bool is_equality_test);
    variablization *get_variablization(Symbol *index_sym);
    variablization *get_variablization(uint64_t index_id);
    variablization *get_variablization(test equality_test);
    uint64_t        get_gid_for_orig_var(Symbol *index_sym);

    void      merge_values_in_conds(condition *pDestCond, condition *pSrcCond);
    void      set_cond_for_id_attr_tests(condition *pCond);
    condition *get_previously_seen_cond(condition *pCond);

    void add_orig_var_mappings_for_test(test t);
    void add_orig_var_mappings_for_cond(condition *cond);

    void add_relational_constraints_for_test(test *t);
    void add_relational_constraints_for_cond(condition *cond);
    void variablize_relational_constraints_for_symbol(::list **constraint_list);

    bool symbol_in_chunk(Symbol *sym1, Symbol *sym2 = NULL);

    void clear_data();

    void variablize_test(test *chunk_test);

    std::map< Symbol *, uint64_t >          * ovar_to_g_id_map;
    std::map< uint64_t, variablization * >  * g_id_to_var_map;
    std::map< Symbol *, variablization * >  * sym_to_var_map;
    std::map< Symbol *, ::list * >          * sti_constraints;
    std::map< uint64_t, ::list * >          * constant_constraints;

    std::map< Symbol *, std::map< Symbol *, ::list *> > *cond_merge_map;
    uint64_t ground_id_counter;
};

#endif /* VARIABLIZATION_MANAGER_H_ */
