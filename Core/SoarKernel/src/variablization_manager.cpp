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
  original_symbol_data *var;
  var = static_cast<original_symbol_data *>(item);
  return compress (hash_string(var->name),num_bits);
}

Variablization_Manager::Variablization_Manager(agent *myAgent)
{
  thisAgent = myAgent;
  create_original_symbol_data_table();
  variablization_table = new std::map< Symbol *, variablization * >();
  current_unique_vars = new std::set< Symbol *>();
}

Variablization_Manager::~Variablization_Manager()
{
  clear_original_symbol_data_table();
  delete variablization_table;
}
/* -- ----------------------------------
 *    Variablization functions
 *    ----------------------------------
 *    The following functions handle variablization of LHS items.  It replaces
 *    variablize_symbol.
 *
 *    -- */

void Variablization_Manager::clear_variablization_table() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing symbol map...\n");
  for (std::map< Symbol *, variablization * >::iterator it=(*variablization_table).begin(); it!=(*variablization_table).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Clearing %s -> %s/%s\n", it->first->to_string(thisAgent),
        it->second->instantiated_symbol->to_string(thisAgent), it->second->variablized_symbol->to_string(thisAgent));
    symbol_remove_ref(thisAgent, it->first);
    symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
    symbol_remove_ref(thisAgent, it->second->variablized_symbol);
    delete it->second;
  }
  variablization_table->clear();
  print_tables();
}

void Variablization_Manager::reinit_original_symbol_data()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager reinitializing hash table and clearing variablization map.\n");
  clear_variablization_table();
  if (original_symbol_ht)
    clear_original_symbol_data_table();
  create_original_symbol_data_table();
}

variablization * Variablization_Manager::get_variablization(Symbol *index_sym)
{
  std::map< Symbol *, variablization * >::iterator iter = (*variablization_table).find(index_sym);
  if (iter != (*variablization_table).end())
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map: %s/%s\n", index_sym->to_string(thisAgent),
       iter->second->variablized_symbol->to_string(thisAgent), iter->second->instantiated_symbol->to_string(thisAgent));
      return iter->second;
  }
  else
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager looking up %s in id_symbol_map but not found.\n");
    return NULL;
  }
}

void Variablization_Manager::store_variablization(Symbol *index_sym, Symbol *instantiated_sym, Symbol *variable, bool is_equality_test)
{
  variablization *new_variablization;

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original Variable Manager storing symbol %s -=> %s, %s\n",
          index_sym->to_string(thisAgent),
          instantiated_sym->to_string(thisAgent),
          variable->to_string(thisAgent));

  new_variablization = new variablization;
  new_variablization->instantiated_symbol = instantiated_sym;
  new_variablization->variablized_symbol = variable;
  new_variablization->grounded = is_equality_test;

  (*variablization_table)[index_sym] = new_variablization;
}

void Variablization_Manager::variablize_symbol (Symbol **sym, Symbol *original_symbol, bool is_equality_test)
{
  char prefix[2];
  Symbol *index_var, *var;
  variablization *var_info;

  if (original_symbol)
    index_var = original_symbol;
  else
    index_var = *sym;

  var_info = get_variablization(index_var);
  if (var_info)
  {
    dprint(DT_CHUNK_VARIABLIZATION, "Found existing variablization %s.\n", var_info->variablized_symbol->to_string(thisAgent));

    if (is_equality_test)
      var_info->grounded = true;

    /* -- Symbol being passed in is being replaced, so decrease refcount -- */
    /* -- and increase refcount for new variable symbol being returned -- */
    symbol_remove_ref (thisAgent, (*sym));
    *sym = var_info->variablized_symbol;
    symbol_add_ref(thisAgent, var_info->variablized_symbol);
    return;
  }

  /* --- need to create a new variable.  If constant is being variablized
   *     just used 'c' instead of first letter of id name --- */
  if((*sym)->is_identifier())
    prefix[0] = static_cast<char>(tolower((*sym)->id->name_letter));
  else
    prefix[0] = 'c';
  prefix[1] = 0;
  var = generate_new_variable (thisAgent, prefix);

  store_variablization(index_var, (*sym), var, is_equality_test);

  /* -- Though generate_new_variable() adds a refcount, we also add them for our stored pointers
   *    in the variablization table.  These are cleaned up after RHS variablization. -- */
  symbol_add_ref(thisAgent, *sym);
  symbol_add_ref(thisAgent, var);
  /* -- And one for the symbol we're indexing by -- */
  if (original_symbol)
    symbol_add_ref(thisAgent, original_symbol);
  else
    symbol_add_ref(thisAgent, *sym);

  dprint(DT_CHUNK_VARIABLIZATION, "Created new variablization %s.\n", (*sym)->variablized_symbol->to_string(thisAgent));

  *sym = var;
}

/* -- ----------------------------------
 *    Unique original variable functions
 *    ----------------------------------
 *    The following code is used when creating instantiations.  When the rete re-creates
 *    the production from a p-node, this function is used to make sure that the original variable
 *    names (the one that are in the original production), which are stored in the tests and RHS
 *    symbols are unique across instantiations but consistent within a particular
 *    instantiation, a property needed by the chunker to avoid conflation and ungrounded constants
 *    when variablizing LHS symbols and to match rhs bindings to the proper lhs bindings.
 * -- */

void Variablization_Manager::create_original_symbol_data_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager creating hash table.\n");
  original_symbol_ht = make_hash_table (thisAgent, 0, hash_unique_string);
  init_memory_pool (thisAgent, &original_symbol_mp, sizeof(original_symbol_data), "unique_string");

}

bool free_original_symbol_data (agent* thisAgent, void *item, void*) {

  original_symbol_data *varname = static_cast<original_symbol_data *>(item);
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

void Variablization_Manager::clear_original_symbol_data_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing hash table of original_vars...\n");
  do_for_all_items_in_hash_table( thisAgent, original_symbol_ht, free_original_symbol_data, 0);

  free_memory(thisAgent, original_symbol_ht->buckets, HASH_TABLE_MEM_USAGE);
  free_memory(thisAgent, original_symbol_ht, HASH_TABLE_MEM_USAGE);
}

void Variablization_Manager::clear_current_unique_var(Symbol *var)
{
  uint32_t hash_value;
  original_symbol_data *varname;

  hash_value = hash_variable_raw_info (var->var->name,original_symbol_ht->log2size);
  varname = reinterpret_cast<original_symbol_data *>(*(original_symbol_ht->buckets + hash_value));
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
void Variablization_Manager::clear_current_unique_var_set() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "Original_Variable_Manager clearing unique var cache...\n");
  for (std::set< Symbol *>::iterator it=(*current_unique_vars).begin(); it!=(*current_unique_vars).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "Erasing current_unique_var %s\n", (*it)->to_string(thisAgent));
  }
  current_unique_vars->clear();
}

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

/* -- make_name_unique takes a symbol and replaces it with a unique version if it hasn't already
 *    been made unique for the current instantiation (thisAgent->newly_created_instantiations) -- */

void Variablization_Manager::make_name_unique(Symbol **sym)
{
  uint32_t hash_value;
  original_symbol_data *varname, *new_varname;

  dprint(DT_ORIGINAL_VAR_MANAGER, "...uniqueifying %s for instantiation %s...\n",
                                           (*sym)->var->name,
                                           thisAgent->newly_created_instantiations->prod->name->sc->name );

  if (already_unique(*sym))
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "...already unique, so using existing original variable %s\n",
                                      (*sym)->var->name);
    return;
  }

  hash_value = hash_variable_raw_info ((*sym)->var->name,original_symbol_ht->log2size);
  varname = reinterpret_cast<original_symbol_data *>(*(original_symbol_ht->buckets + hash_value));
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

  allocate_with_pool (thisAgent, &original_symbol_mp, &new_varname);
  new_varname->current_instantiation = thisAgent->newly_created_instantiations;
  new_varname->current_unique_var_symbol = (*sym);
  new_varname->name = make_memory_block_for_string (thisAgent, (*sym)->var->name);
  new_varname->next_unique_suffix_number = 1;
  add_to_hash_table (thisAgent, original_symbol_ht, new_varname);
  /* -- Increase refcount for cached current_unique_var_symbol -- */
  symbol_add_ref(thisAgent, (*sym));
  current_unique_vars->insert(*sym);
  dprint(DT_ORIGINAL_VAR_MANAGER, "...first use, so using original variable %s\n",
                                    (*sym)->var->name);
}

/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

bool print_original_symbol_data (agent* thisAgent, void *item, void*) {

  original_symbol_data *varname = static_cast<original_symbol_data *>(item);

  dprint(DT_ORIGINAL_VAR_MANAGER, "%s, CurrUnqVarSym: %s(%lld) CurrInst: %d\n",
        varname->name,
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->to_string(thisAgent) : "None"),
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->reference_count : 0),
        (varname->current_instantiation ? varname->current_instantiation : NULL));
  return false;
}

void Variablization_Manager::print_OSD_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "   Variablization OSD Hash Table\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  do_for_all_items_in_hash_table( thisAgent, original_symbol_ht, print_original_symbol_data, 0);
}

void Variablization_Manager::print_variablization_table()
{
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "       Variablization Table\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  for (std::map< Symbol *, variablization * >::iterator it=(*variablization_table).begin(); it!=(*variablization_table).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "%s -> %s/%s\n", it->first->to_string(thisAgent),
        it->second->variablized_symbol->to_string(thisAgent), it->second->instantiated_symbol->to_string(thisAgent));
  }
}
void Variablization_Manager::print_CUV_table() {

  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "   Current Unique Variable Table\n");
  dprint(DT_ORIGINAL_VAR_MANAGER, "------------------------------------\n");
  for (std::set< Symbol *>::iterator it=(*current_unique_vars).begin(); it!=(*current_unique_vars).end(); ++it)
  {
    dprint(DT_ORIGINAL_VAR_MANAGER, "%s\n", (*it)->to_string(thisAgent));
  }
}
void Variablization_Manager::print_tables()
{
  print_OSD_table();
  print_variablization_table();
  print_CUV_table();
}
