/*
 * Original_Variable_Manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
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

Variablization_Manager::Variablization_Manager(agent *myAgent)
{
  thisAgent = myAgent;
  create_table();
  id_symbol_map = new std::map< Symbol *, Symbol * >();
  current_unique_vars = new std::set< Symbol *>();
}

Variablization_Manager::~Variablization_Manager()
{
  clear_original_var_table();
  delete id_symbol_map;
}

void Variablization_Manager::create_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager crating hash table.\n");
  ht = make_hash_table (thisAgent, 0, hash_unique_string);
  init_memory_pool (thisAgent, &mp, sizeof(original_varname), "unique_string");

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

void Variablization_Manager::clear_original_var_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing hash table of original_vars...\n");
  do_for_all_items_in_hash_table( thisAgent, ht, free_original_var, 0);

  free_memory(thisAgent, ht->buckets, HASH_TABLE_MEM_USAGE);
  free_memory(thisAgent, ht, HASH_TABLE_MEM_USAGE);
}

void Variablization_Manager::clear_variablization_table() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing symbol map...\n");
  for (std::map< Symbol *, Symbol * >::iterator it=(*id_symbol_map).begin(); it!=(*id_symbol_map).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Clearing %s -> %s\n", it->first->to_string(thisAgent), it->second->to_string(thisAgent));
    clear_current_unique_var(it->first);
  }
  id_symbol_map->clear();
  print_table();
}

void Variablization_Manager::reinit_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager reinitializing hash table and clearing variablization map.\n");
  clear_variablization_table();
  if (ht)
    clear_original_var_table();
  create_table();
}

void Variablization_Manager::store_variablization(Symbol *index_sym, Symbol *instantiated_sym, Symbol *variable)
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager storing symbol %s -=> %s, %s\n",
          index_sym->to_string(thisAgent),
          instantiated_sym->to_string(thisAgent),
          variable->to_string(thisAgent));
  (*id_symbol_map)[index_sym] = variable;
}

void Variablization_Manager::clear_variablization(Symbol *index_sym)
{
  /* -- Clear this variable from symbol_map so that any RHS items that use this original var wont' be variablized -- */
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager clearing symbol %s from symbol_map.\n", index_sym->to_string(thisAgent));
  (*id_symbol_map).erase(index_sym);
  /* -- Need to clear this variable from hash table too b/c it won't be in the symbol map later when it is cleaned up -- */
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager clearing symbol %s from hash table.\n", index_sym->to_string(thisAgent));
  clear_current_unique_var(index_sym);

}
void Variablization_Manager::variablize_symbol (Symbol **sym, Symbol *original_symbol)
{
  char prefix[2];
  Symbol *var, *index_var;

  if (original_symbol)
    index_var = original_symbol;
  else
    index_var = *sym;

  var = get_variablization(index_var);
  if (var)
  {
//  if ((*sym)->tc_num == thisAgent->variablization_tc) {
    /* --- it's already been variablized, so use the existing variable --- */
    dprint(DT_CHUNK_VARIABLIZATION, "Found existing variablization %s.\n", var->to_string(thisAgent));
//    var = (*sym)->variablized_symbol;
//    var->unvariablized_symbol = *sym;
    store_variablization(index_var, (*sym), var);
    /* -- Symbol being passed in is being replaced, so decrease refcount -- */
    /* -- Increase refcount for new variable symbol being returned -- */
    symbol_remove_ref (thisAgent, (*sym));
    *sym = var;
    symbol_add_ref(thisAgent, var);
    return;
  }

  /* --- need to create a new variable.  If constant is being variablized
   *     just used 'c' instead of first letter of id name --- */
//  (*sym)->tc_num = thisAgent->variablization_tc;
  if((*sym)->is_identifier())
    prefix[0] = static_cast<char>(tolower((*sym)->id->name_letter));
  else
    prefix[0] = 'c';
  prefix[1] = 0;
  var = generate_new_variable (thisAgent, prefix);
//  if ((*sym)->variablized_symbol)
//  {
//    /* -- This symbol was variablized in a previous chunk or template, so decrease
//     *    the refcount to that cached link -- */
//    symbol_remove_ref (thisAgent, (*sym)->variablized_symbol);
//  }
//  (*sym)->variablized_symbol = var;
  store_variablization(index_var, (*sym), var);

  /* -- Though generate_new_variable() adds a refcount, we also add them for our cached links.
   *    These are cleaned up after RHS variablization. -- */
  symbol_add_ref(thisAgent, *sym);
  if (original_symbol)
    symbol_add_ref(thisAgent, original_symbol);
  symbol_add_ref(thisAgent, var);

//  var->unvariablized_symbol = *sym;
  /* -- Increase refcount for link to constant sym being replaced -- */
//  symbol_add_ref(thisAgent, var->unvariablized_symbol);
  dprint(DT_CHUNK_VARIABLIZATION, "Created new variablization %s.\n", (*sym)->variablized_symbol->to_string(thisAgent));

  *sym = var;
}

Symbol * Variablization_Manager::get_variablization(Symbol *index_sym)
{
  std::map< Symbol *, Symbol * >::iterator iter = (*id_symbol_map).find(index_sym);
  if (iter != (*id_symbol_map).end())
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map: %s\n", index_sym->to_string(thisAgent), iter->second->to_string(thisAgent));
      return iter->second;
  }
  else
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map but not found.\n");
    return NULL;
  }
}

Symbol * Variablization_Manager::get_instantiated_symbol(Symbol *index_sym)
{
  std::map< Symbol *, Symbol * >::iterator iter = (*id_symbol_map).find(index_sym);
  if (iter != (*id_symbol_map).end())
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map: %s\n", index_sym->to_string(thisAgent), iter->second->to_string(thisAgent));
      return iter->second;
  }
  else
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map but not found.\n");
    return NULL;
  }
}

/* -- The following code is used when creating instantiations.  When the rete re-creates
 *    the production from a p-node, this function is used to make sure that the original variable
 *    names (the one that are in the original production), which are stored in the tests and RHS
 *    symbols are unique across instantiations but consistent within a particular
 *    instantiation, a property needed by the chunker to avoid conflation and ungrounded constants
 *    when variablizing LHS symbols and to match rhs bindings to the proper lhs bindings.
 * -- */

bool Variablization_Manager::already_unique(Symbol *original_var) {

  dprint(DT_ORIGINAL_VAR_MANAGER, "...checking if %s is already unique...", original_var->to_string(thisAgent));

  std::set< Symbol * >::iterator it = current_unique_vars->find(original_var);
  if (it != current_unique_vars->end()) {
    dprint_noprefix(DT_ORIGINAL_VAR_MANAGER, " = TRUE\n");
    return true;
  }
  dprint_noprefix(DT_ORIGINAL_VAR_MANAGER, " = FALSE\n");
  return false;
}


void Variablization_Manager::clear_current_unique_var(Symbol *var)
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
void Variablization_Manager::clear_current_unique_vars() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing unique var cache...\n");
  for (std::set< Symbol *>::iterator it=(*current_unique_vars).begin(); it!=(*current_unique_vars).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Erasing current_unique_var %s\n", (*it)->to_string(thisAgent));
  }
  current_unique_vars->clear();
}

/* -- make_name_unique takes a symbol and replaces it with a unique version if it hasn't already
 *    been made unique for the current instantiation (thisAgent->newly_created_instantiations) -- */

void Variablization_Manager::make_name_unique(Symbol **sym)
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

/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

bool print_original_var (agent* thisAgent, void *item, void*) {

  original_varname *varname = static_cast<original_varname *>(item);

  dprint(DT_ORIGINAL_VAR_MANAGER, "%s, CurrUnqVarSym: %s(%lld) CurrInst: %d\n",
        varname->name,
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->to_string(thisAgent) : "None"),
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->reference_count : 0),
        (varname->current_instantiation ? varname->current_instantiation : NULL));
  return false;
}

void Variablization_Manager::print_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "Variablization Manager Hash Table\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  do_for_all_items_in_hash_table( thisAgent, ht, print_original_var, 0);
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "Variablization Manager Symbol Map\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  for (std::map< Symbol *, Symbol * >::iterator it=(*id_symbol_map).begin(); it!=(*id_symbol_map).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "%s -> ", it->first->to_string(thisAgent));
    dprint_noprefix(DT_ORIGINAL_VAR_MANAGER, "%s\n", it->second->to_string(thisAgent));
  }
}

