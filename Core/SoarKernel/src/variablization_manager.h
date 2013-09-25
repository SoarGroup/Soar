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

typedef struct variablization_struct {
    Symbol *instantiated_symbol;
    Symbol *variablized_symbol;
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

class Variablization_Manager
{
  public:
    /* -- This function takes the name of a variable and potentially modifies it so
     *    that is is unique to the current instantiation being built -- */
    void make_name_unique(Symbol **sym);
    bool already_unique(Symbol *original_var);

    void clear_variablization_table();
    void clear_current_unique_var_set();
    variablization *get_variablization(Symbol *original_var);

    void variablize_symbol (Symbol **sym, Symbol *original_symbol, bool is_equality_test);

    void reinit_original_symbol_data();

    void print_OSD_table();
    void print_variablization_table();
    void print_CUV_table();
    void print_tables();

    Variablization_Manager(agent *thisAgent);
    ~Variablization_Manager();

  private:
    agent* thisAgent;

    void store_variablization(Symbol *index_sym, Symbol *instantiated_sym, Symbol *variable, bool is_equality_test);
    void clear_current_unique_var(Symbol *var);
    void clear_original_symbol_data_table();
    void create_original_symbol_data_table();

    /* -- The variablization_table is used during chunking.  It stores a mapping from either a
     *    symbol's original variable, if available, or the actual symbol to the variable
     *    that was created for variablization.  It is keep track of the current variablized
     *    symbols in a chunk that is being built. This mapping is temporary and cleared after
     *    the chunk is built. This replaces the variablized pointer in versions of Soar
     *    prior to 9.4 -- */

    std::map< Symbol *, variablization * > * variablization_table;

    /* -- The current_unique_vars set is used during instantiation creation.  It keeps a list
     *    of all original vars in the current instantiation that have been made unique.
     *    This is needed so we don't try to make an original variable unique twice
     *    because it appears in two different conditions -- */

    std::set< Symbol * > * current_unique_vars;

    /* -- A hash table and memory pool to store data structures that keep track of original
     *    symbols.  Used to keep track of the next unique symbol to generate -- */

    struct hash_table_struct *original_symbol_ht;
    memory_pool original_symbol_mp;
};

#endif /* VARIABLIZATION_MANAGER_H_ */
