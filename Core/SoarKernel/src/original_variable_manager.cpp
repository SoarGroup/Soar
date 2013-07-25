/*
 * Original_Variable_Manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "original_variable_manager.h"
#include "agent.h"
#include "instantiations.h"

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
}

Original_Variable_Manager::~Original_Variable_Manager()
{
  clear_table();
  delete id_symbol_map;
}

bool remove_cached_var_refcounts (agent* thisAgent, void *item, void*) {

  original_varname *varname = static_cast<original_varname *>(item);
  if (varname->current_unique_var_symbol)
  {
    dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager decreasing refcount on symbol %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
    symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
    varname->current_unique_var_symbol = NULL;
  }
  varname->current_instantiation = NULL;
  dprint(DT_UNIQUE_VARIABLIZATION, "Freeing memory for string %s\n", varname->name);
  free_memory_block_for_string(thisAgent, varname->name);
  return false;
}

void Original_Variable_Manager::clear_table()
{
  dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager clearing table...\n");
  do_for_all_items_in_hash_table( thisAgent, ht, remove_cached_var_refcounts, 0);

  free_memory(thisAgent, ht->buckets, HASH_TABLE_MEM_USAGE);
  free_memory(thisAgent, ht, HASH_TABLE_MEM_USAGE);
}

void Original_Variable_Manager::clear_symbol_map() {
  uint32_t hash_value;
  original_varname *varname;

  dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager clearing symbol map...\n");
  for (std::map< Symbol *, Symbol * >::iterator it=(*id_symbol_map).begin(); it!=(*id_symbol_map).end(); ++it)
  {
    hash_value = hash_variable_raw_info (it->first->var->name,ht->log2size);
    varname = reinterpret_cast<original_varname *>(*(ht->buckets + hash_value));
    for ( ; varname != NIL; varname = varname->next_in_hash_table)
    {
      if (!strcmp(varname->name,(it->first)->var->name))
      {
        if (varname->current_unique_var_symbol)
        {
          dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager decreasing refcount on symbol %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
          symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
          varname->current_unique_var_symbol = NULL;
        }
        varname->current_instantiation = NULL;
      }
    }
  }
  id_symbol_map->clear();
}

void Original_Variable_Manager::create_table()
{
  ht = make_hash_table (thisAgent, 0, hash_unique_string);
  init_memory_pool (thisAgent, &mp, sizeof(original_varname), "unique_string");

}

void Original_Variable_Manager::reinit_table()
{
  if (ht)
    clear_table();
  clear_symbol_map();
  create_table();
}

/* -- make_name_unique is used when recreating conditions by the rete code.
 *    It makes sures that original variable names (the one that are in the original
 *    production) are unique across instantiations, a property needed by the chunker
 *    to match rhs bindings to lhs bindings.
 *
 *    next_unique_suffix_number is used to quickly generate a new name for
 *    conflicting variable name. --- */

void Original_Variable_Manager::make_name_unique(Symbol **sym)
{
  uint32_t hash_value;
  original_varname *varname, *new_varname;

  assert(thisAgent->newly_created_instantiations != NIL);
  dprint(DT_UNIQUE_VARIABLIZATION, "make_varsym_unique called with original sym %s for instantiation %s!\n",
                                           (*sym)->var->name,
                                           thisAgent->newly_created_instantiations->prod->name->sc->name );

  hash_value = hash_variable_raw_info ((*sym)->var->name,ht->log2size);
  varname = reinterpret_cast<original_varname *>(*(ht->buckets + hash_value));
  for ( ; varname != NIL; varname = varname->next_in_hash_table)
  {
    if (!strcmp(varname->name,(*sym)->var->name))
    {
      /* -- Found unique string record that matches original var name -- */

      if (varname->current_instantiation == thisAgent->newly_created_instantiations)
      {

        /* -- We've already created and cached a unique version of this variable name
         *    for this instantiation -- */
        dprint(DT_UNIQUE_VARIABLIZATION, "make_varsym_unique found existing unique sym %s (%s) for this instantiation.\n",
                                          varname->current_unique_var_symbol->var->name, (*sym)->var->name);
        *sym = varname->current_unique_var_symbol;
        /* -- MToDoRefCnt | May not need to add this refcount.  Caller will increase the refcount again when it uses the symbol in a test. -- */
        //symbol_add_ref(thisAgent, varname->current_unique_var_symbol);
        return;
      }
      else
      {
        /* -- We've need to create and cache a new unique version of this string
         *    for this instantiation -- */

        std::string suffix, new_name = (*sym)->var->name;

        /* -- Create a unique name by appending a numbered suffix to original var name -- */

        varname->next_unique_suffix_number++;
        to_string(varname->next_unique_suffix_number, suffix);
        new_name.erase(new_name.end()-1);
        new_name += "+" + suffix + ">";

        /* -- Create a new unique string entry in the hash table == */

        /* -- Debug | Why do we need to create a new u_string here?  Can't we just
         *            create the var and put it in the old u-string?  Try it. */

        allocate_with_pool (thisAgent, &mp, &new_varname);
        new_varname->current_instantiation = thisAgent->newly_created_instantiations;
        new_varname->name = make_memory_block_for_string (thisAgent, new_name.c_str());
        new_varname->next_unique_suffix_number = 1;
        new_varname->current_unique_var_symbol = make_variable(thisAgent, new_name.c_str());
        if (varname->current_unique_var_symbol)
          symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
        varname->current_unique_var_symbol = new_varname->current_unique_var_symbol;
        varname->current_instantiation = thisAgent->newly_created_instantiations;

        dprint(DT_UNIQUE_VARIABLIZATION, "make_varsym_unique creating new unique version of %s: %s\n",
                                          (*sym)->var->name, new_name.c_str());

        *sym = new_varname->current_unique_var_symbol;

        /* -- MToDoRefCnt | May not need to add this refcount.  Make variable will already increase which takes care of the
         *                  reference for our cached copy.  Caller will increase the refcount again when it uses the symbol in a test. -- */
        //symbol_add_ref(thisAgent, (*sym));
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
  /* -- Increase refcount for our cached copy -- */
  symbol_add_ref(thisAgent, (*sym));

  dprint(DT_UNIQUE_VARIABLIZATION, "make_varsym_unique generated a var for the first time: %s\n",
                                    (*sym)->var->name);
}
