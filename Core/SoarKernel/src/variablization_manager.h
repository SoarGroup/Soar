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
typedef struct variablization_struct {
    Symbol *instantiated_symbol;
    Symbol *variablized_symbol;
    uint64_t grounding_id;
    bool grounded;
} variablization;

typedef struct original_symbol_data_struct {

    /* --- pointer to next string that matches hash --- */
    struct original_symbol_data_struct *next_in_hash_table;

    /* --- original var name that should not repeat across instantiations --- */
    char *name;

    /* --- Cache the current unique version of the requested string
     *     current_unique_string for current_instantiation. --- */
    Symbol *current_unique_var_symbol;
    instantiation *current_instantiation;

    /* --- Counter to be used as a suffix for the next unique version
     *     of a requested var name --- */
    int64_t next_unique_suffix_number;

} original_symbol_data;

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
    void make_name_unique(Symbol **sym);
    bool already_unique(Symbol *original_var);

    void clear_variablization_table();
    void clear_CUV_cache();
    void reinit();

    variablization *get_variablization(Symbol *index_sym);
    variablization *get_variablization(uint64_t index_id);
    variablization *get_variablization(char *index_var);

    void variablize_lhs_symbol (Symbol **sym, Symbol *original_symbol,
                                identity_info *identity, bool is_equality_test);
    void variablize_rl_symbol (Symbol **sym, bool is_equality_test);
    uint64_t variablize_rhs_symbol (Symbol **sym, char *original_var);

    void print_OSD_table();
    void print_variablization_table();
    void print_CUV_table();
    void print_tables();

    Variablization_Manager(agent *thisAgent);
    ~Variablization_Manager();

  private:
    agent* thisAgent;

    void store_variablization(Symbol *instantiated_sym, Symbol *variable, char *orig_varname,
                              identity_info *identity, bool is_equality_test);
    void clear_CUV_for_symbol(Symbol *var);
    void clear_data();
    void clear_OS_hashtable();
    void create_OS_hashtable();


    std::map< char *, variablization * > * variablization_ovar_table;
    std::map< uint64_t, variablization * > * variablization_g_id_table;
    std::map< Symbol *, variablization * > * variablization_sym_table;
    std::set< Symbol * > * current_unique_vars;
    struct hash_table_struct *original_symbol_ht;
    memory_pool original_symbol_mp;

    uint64_t ground_id_counter;
};

#endif /* VARIABLIZATION_MANAGER_H_ */
