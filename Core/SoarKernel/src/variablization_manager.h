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

typedef struct identity_struct identity_info;
typedef struct test_struct test_info;
typedef test_info * test;
typedef struct condition_struct condition;

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

class Variablization_Manager
{
  public:

    uint64_t get_new_ground_id() {return (++ground_id_counter);};

    void clear_variablization_table();
    void reinit();

    void build_orig_var_mappings(cons *grounds);
    void add_orig_var_mappings_for_cond_list(condition *cond);

    variablization *get_variablization(Symbol *index_sym);
    variablization *get_variablization(uint64_t index_id);
    uint64_t get_gid_for_orig_var(Symbol *index_sym);

    void clear_relational_constraints ();
    void add_relational_constraint (test equality_test, test relational_test);

    void variablize_lhs_symbol (Symbol **sym, Symbol *original_symbol,
                                identity_info *identity, bool is_equality_test);
    void variablize_rl_symbol (Symbol **sym, bool is_equality_test);
    uint64_t variablize_rhs_symbol (Symbol **sym, Symbol *original_var);

    void print_OSD_table();
    void print_variablization_tables(TraceMode mode, int whichTable=0);
    void print_tables();
    void print_relational_constraints (TraceMode mode);

    Variablization_Manager(agent *thisAgent);
    ~Variablization_Manager();

  private:
    agent* thisAgent;

    void store_variablization(Symbol *instantiated_sym, Symbol *variable,
                              identity_info *identity, bool is_equality_test);

    void add_orig_var_mappings_for_test(test t);
    void add_orig_var_mappings_for_cond(condition *cond);

    void clear_data();


    std::map< Symbol *, uint64_t >          * ovar_to_g_id_map;
    std::map< uint64_t, variablization * >  * g_id_to_var_map;
    std::map< Symbol *, variablization * >  * sym_to_var_map;
    std::map< test, ::list * >              * constraints;

    uint64_t ground_id_counter;
};

#endif /* VARIABLIZATION_MANAGER_H_ */
