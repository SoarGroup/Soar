/*
 * original_variable_manager.h
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#ifndef ORIGINAL_VARIABLE_MANAGER_H_
#define ORIGINAL_VARIABLE_MANAGER_H_

#include <portability.h>
#include "symtab.h"

extern void dprint (TraceMode mode, const char *format, ... );
extern void print (agent* thisAgent, const char *format, ... );

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
    void make_name_unique(Symbol **sym);
    void reinit_table();

    /* -- The following functions are used to store the mapping from an original variable
     *    on the LHS to the variable that was created for variablization.  It is used
     *    to variablize the rhs actions. -- */

    void clear_symbol_map();
    void store_variablization(Symbol *original_var, Symbol *current_variable)
    {
      dprint(DT_UNIQUE_VARIABLIZATION, "Original Variable Manager storing symbol %s -=> %s\n", original_var->to_string(thisAgent), current_variable->to_string(thisAgent));
      (*id_symbol_map)[original_var] = current_variable;
    }
    void clear_variablization(Symbol *reversed_var)
    {
      dprint(DT_UNIQUE_VARIABLIZATION, "Original Variable Manager clearing symbol being reversed %s.\n", reversed_var->to_string(thisAgent));
      (*id_symbol_map).erase(reversed_var);
      /* -- MToDo | We need to clear this variable from table too b/c it won't be in the symbol map later -- */
    }

    Symbol *find_original_variable(Symbol *original_var)
    {
      dprint(DT_UNIQUE_VARIABLIZATION, "Original Variable Manager looking up %s: %s\n", original_var->to_string(thisAgent),
          (*id_symbol_map)[original_var] ? (*id_symbol_map)[original_var]->to_string(thisAgent) : "not found");
      return (*id_symbol_map)[original_var];
    }

    Original_Variable_Manager(agent *thisAgent);
    ~Original_Variable_Manager();

  private:
    void clear_table();
    void create_table();

    std::map< Symbol *, Symbol * > * id_symbol_map;

    struct hash_table_struct *ht;
    memory_pool mp;
    agent* thisAgent;
};

#endif /* ORIGINAL_VARIABLE_MANAGER_H_ */
