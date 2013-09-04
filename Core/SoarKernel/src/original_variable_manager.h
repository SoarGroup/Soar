/*
 * original_variable_manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#ifndef ORIGINAL_VARIABLE_MANAGER_H_
#define ORIGINAL_VARIABLE_MANAGER_H_

#include <portability.h>
#include <set>
#include "symtab.h"

typedef struct original_varname_struct {

    /* --- pointer to next string that matches hash --- */
    struct original_varname_struct *next_in_hash_table;

    /* --- original var name that should not repeat across instantiations --- */
    char *name;

    /* --- Cache the current unique version of the requested string
     *     current_unique_string for current_instantiation. --- */
    Symbol *current_unique_var_symbol;
    instantiation *current_instantiation;

    /* --- Counter to be used as a suffix for the next unique version
     *     of a requested var name --- */
    int64_t next_unique_suffix_number;

} original_varname;

class Original_Variable_Manager
{
  public:
    /* -- This is the main function that takes the name of a variable and potentially
     *    modifies it so that is is unique to the current instantiation being built -- */
    void make_name_unique(Symbol **sym);
    bool already_unique(Symbol *original_var);

    void clear_symbol_map();
    void clear_current_unique_vars();
    void store_variablization(Symbol *original_var, Symbol *current_variable);
    Symbol *find_original_variable(Symbol *original_var);

    void reinit_table();
    void print_table();
    Original_Variable_Manager(agent *thisAgent);
    ~Original_Variable_Manager();

  private:
    void clear_current_unique_var(Symbol *var);
    void clear_table();
    void create_table();

    /* -- id_symbol_map is used to store the mapping from an original variable
     *    on the LHS to the variable that was created for variablization.  It is used
     *    to variablize the rhs actions. This mapping is temporary and cleared after
     *    the chunk is built. -- */

    std::map< Symbol *, Symbol * > * id_symbol_map;

    /* -- current_unique_vars keeps a list of all original vars in the current instantiation
     *    that have been made unique.  This is needed so we don't try to make an original variable
     *    unique twice because it appears in two different conditions -- */

    std::set< Symbol * > * current_unique_vars;

    struct hash_table_struct *ht;
    memory_pool mp;
    agent* thisAgent;
};

#endif /* ORIGINAL_VARIABLE_MANAGER_H_ */
