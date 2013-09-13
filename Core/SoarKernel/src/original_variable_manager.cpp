/*
 * Original_Variable_Manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "original_variable_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"

extern uint32_t hash_string (const char *s);
uint32_t hash_variable_raw_info (const char *name, short num_bits);
uint32_t compress (uint32_t h, short num_bits);

uint32_t hash_unique_string (void *item, short num_bits) {
  original_varname *var;
  var = static_cast<original_varname *>(item);
  return compress (hash_string(var->name),num_bits);
}

Original_Variable_Manager::Original_Variable_Manager(agent *myAgent)
{
  thisAgent = myAgent;
  create_table();
  id_symbol_map = new std::map< Symbol *, Symbol * >();
  current_unique_vars = new std::set< Symbol *>();
}

Original_Variable_Manager::~Original_Variable_Manager()
{
  clear_table();
  delete id_symbol_map;
}

void Original_Variable_Manager::create_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager crating hash table.\n");
  ht = make_hash_table (thisAgent, 0, hash_unique_string);
  init_memory_pool (thisAgent, &mp, sizeof(original_varname), "unique_string");

}

bool print_original_var (agent* thisAgent, void *item, void*) {

  original_varname *varname = static_cast<original_varname *>(item);

  dprint(DT_ORIGINAL_VAR_MANAGER, "%s, CurrUnqVarSym: %s(%lld) CurrInst: %d\n",
        varname->name,
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->to_string(thisAgent) : "None"),
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->reference_count : 0),
        (varname->current_instantiation ? varname->current_instantiation : NULL));
  return false;
}

void Original_Variable_Manager::print_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager Hash Table\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  do_for_all_items_in_hash_table( thisAgent, ht, print_original_var, 0);
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager Symbol Map\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  for (std::map< Symbol *, Symbol * >::iterator it=(*id_symbol_map).begin(); it!=(*id_symbol_map).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "%s -> ", it->first->to_string(thisAgent));
    dprint_noprefix(DT_ORIGINAL_VAR_MANAGER, "%s\n", it->second->to_string(thisAgent));
  }
}

bool free_original_var (agent* thisAgent, void *item, void*) {

  original_varname *varname = static_cast<original_varname *>(item);
  if (varname->current_unique_var_symbol)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "...decreasing refcount on symbol %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
    symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
    varname->current_unique_var_symbol = NULL;
  }
  varname->current_instantiation = NULL;
  dprint(DT_ORIGINAL_VAR_MANAGER, "...freeing memory for string %s\n", varname->name);
  free_memory_block_for_string(thisAgent, varname->name);
  return false;
}

void Original_Variable_Manager::clear_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing hash table of original_vars...\n");
  do_for_all_items_in_hash_table( thisAgent, ht, free_original_var, 0);

  free_memory(thisAgent, ht->buckets, HASH_TABLE_MEM_USAGE);
  free_memory(thisAgent, ht, HASH_TABLE_MEM_USAGE);
}

void Original_Variable_Manager::clear_current_unique_var(Symbol *var)
{
  uint32_t hash_value;
  original_varname *varname;

  hash_value = hash_variable_raw_info (var->var->name,ht->log2size);
  varname = reinterpret_cast<original_varname *>(*(ht->buckets + hash_value));
  for ( ; varname != NIL; varname = varname->next_in_hash_table)
  {
    if (!strcmp(varname->name,var->var->name))
    {
      if (varname->current_unique_var_symbol)
      {
        dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager decreasing refcount on symbol %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
        symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
        varname->current_unique_var_symbol = NULL;
      }
      varname->current_instantiation = NULL;
    }
  }
}

void Original_Variable_Manager::store_variablization(Symbol *original_var, Symbol *current_variable)
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager storing symbol %s -=> %s\n", original_var->to_string(thisAgent), current_variable->to_string(thisAgent));
  (*id_symbol_map)[original_var] = current_variable;
}

void Original_Variable_Manager::clear_variablization(Symbol *reversed_var)
{
  /* -- Clear this variable from symbol_map so that any RHS items that use this original var wont' be variablized -- */
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager clearing symbol %s from symbol_map b/c it's being reversed.\n", reversed_var->to_string(thisAgent));
  (*id_symbol_map).erase(reversed_var);
  /* -- Need to clear this variable from hash table too b/c it won't be in the symbol map later when it is cleaned up -- */
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager clearing symbol %s from hash table b/c it's being reversed.\n", reversed_var->to_string(thisAgent));
  clear_current_unique_var(reversed_var);

}

bool Original_Variable_Manager::already_unique(Symbol *original_var) {

  dprint(DT_ORIGINAL_VAR_MANAGER, "...checking if %s is already unique...", original_var->to_string(thisAgent));

  std::set< Symbol * >::iterator it = current_unique_vars->find(original_var);
  if (it != current_unique_vars->end()) {
    dprint_noprefix(DT_ORIGINAL_VAR_MANAGER, " = TRUE\n");
    return true;
  }
  dprint_noprefix(DT_ORIGINAL_VAR_MANAGER, " = FALSE\n");
  return false;
}

void Original_Variable_Manager::clear_symbol_map() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing symbol map...\n");
  for (std::map< Symbol *, Symbol * >::iterator it=(*id_symbol_map).begin(); it!=(*id_symbol_map).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Clearing %s -> %s\n", it->first->to_string(thisAgent), it->second->to_string(thisAgent));
    clear_current_unique_var(it->first);
  }
  id_symbol_map->clear();
  print_table();
}

void Original_Variable_Manager::clear_current_unique_vars() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing unique var cache...\n");
  for (std::set< Symbol *>::iterator it=(*current_unique_vars).begin(); it!=(*current_unique_vars).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Erasing current_unique_var %s\n", (*it)->to_string(thisAgent));
  }
  current_unique_vars->clear();
}

void Original_Variable_Manager::reinit_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager reinitializing hash table and clearing symbol map.\n");
  clear_symbol_map();
  if (ht)
    clear_table();
  create_table();
}

Symbol * Original_Variable_Manager::find_original_variable(Symbol *original_var)
{
  std::map< Symbol *, Symbol * >::iterator iter = (*id_symbol_map).find(original_var);
  if (iter != (*id_symbol_map).end())
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map: %s\n", original_var->to_string(thisAgent), iter->second->to_string(thisAgent));
      return iter->second;
  }
  else
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map but not found.\n");
    return NULL;
  }
}

/* -- make_name_unique is used when recreating conditions by the rete code.
 *    It makes sure that original variable names (the one that are in the original
 *    production) are unique across instantiations but consistent within a particular
 *    instantiation, a property needed by the chunker to match rhs bindings to lhs bindings.
 *    --- */

void Original_Variable_Manager::make_name_unique(Symbol **sym)
{
  uint32_t hash_value;
  original_varname *varname, *new_varname;

  dprint(DT_ORIGINAL_VAR_MANAGER, "...uniqueifying %s for instantiation %s...\n",
                                           (*sym)->var->name,
                                           thisAgent->newly_created_instantiations->prod->name->sc->name );

  if (already_unique(*sym))
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "...already unique, so using existing original variable %s\n",
                                      (*sym)->var->name);
    return;
  }

  hash_value = hash_variable_raw_info ((*sym)->var->name,ht->log2size);
  varname = reinterpret_cast<original_varname *>(*(ht->buckets + hash_value));
  for ( ; varname != NIL; varname = varname->next_in_hash_table)
  {
    if (!strcmp(varname->name,(*sym)->var->name))
    {
      /* -- Found unique string record that matches original var name -- */

      if (varname->current_instantiation == thisAgent->newly_created_instantiations)
      {

        /* -- We've already created and cached a unique version of this variable name for this
         *    instantiation. Note that we do not need to increase refcount, since caller will
         *    increase the refcount again when it uses the symbol in a test. -- */

        dprint(DT_ORIGINAL_VAR_MANAGER, "...found existing mapping %s -> %s for this instantiation.\n",
                (*sym)->var->name, varname->current_unique_var_symbol->var->name);
        *sym = varname->current_unique_var_symbol;

        return;
      }
      else
      {
        /* -- We need to create and cache a new unique version of this string
         *    for this instantiation -- */

        std::string suffix, new_name = (*sym)->var->name;

        /* -- Create a unique name by appending a numbered suffix to original var name -- */

        to_string(varname->next_unique_suffix_number, suffix);
        new_name.erase(new_name.end()-1);
        new_name += "+" + suffix + ">";

        /* -- Update the original_varname struct with a new variable and the current instantiation == */

        varname->next_unique_suffix_number++;
        varname->current_instantiation = thisAgent->newly_created_instantiations;

        /* MToDoRefCnt | After we clean up current_unique_vars in p_node, the following should never be necessary */
        if (varname->current_unique_var_symbol)
        {
          symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
        }
        varname->current_unique_var_symbol = make_variable(thisAgent, new_name.c_str());

        dprint(DT_ORIGINAL_VAR_MANAGER, "...creating new unique version of %s: %s\n",
                                          (*sym)->var->name, new_name.c_str());

        *sym = varname->current_unique_var_symbol;
        current_unique_vars->insert(*sym);

        return;
      }
    }
  }

  /* -- var name was not found in the hash table, so add to hash table and leave original_varsym untouched -- */

  allocate_with_pool (thisAgent, &mp, &new_varname);
  new_varname->current_instantiation = thisAgent->newly_created_instantiations;
  new_varname->current_unique_var_symbol = (*sym);
  new_varname->name = make_memory_block_for_string (thisAgent, (*sym)->var->name);
  new_varname->next_unique_suffix_number = 1;
  add_to_hash_table (thisAgent, ht, new_varname);
  /* -- Increase refcount for cached current_unique_var_symbol -- */
  symbol_add_ref(thisAgent, (*sym));
  current_unique_vars->insert(*sym);
  dprint(DT_ORIGINAL_VAR_MANAGER, "...first use, so using original variable %s\n",
                                    (*sym)->var->name);

}
