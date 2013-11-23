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
  create_OSD_table();
  variablization_table = new std::map< Symbol *, variablization * >();
  current_unique_vars = new std::set< Symbol *>();
}

Variablization_Manager::~Variablization_Manager()
{
  clear_OSD_table();
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

  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing symbol map...\n");
  print_variablization_table();
  for (std::map< Symbol *, variablization * >::iterator it=(*variablization_table).begin(); it!=(*variablization_table).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing %s(%lld) -> %s(%lld)/%s(%lld)\n",
        it->first->to_string(thisAgent), it->first->reference_count,
        it->second->instantiated_symbol->to_string(thisAgent), it->second->instantiated_symbol->reference_count,
        it->second->variablized_symbol->to_string(thisAgent),  it->second->variablized_symbol->reference_count);
    symbol_remove_ref(thisAgent, it->first);
    symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
    symbol_remove_ref(thisAgent, it->second->variablized_symbol);
    delete it->second;
  }
  variablization_table->clear();
}

void Variablization_Manager::reinit()
{
  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing hash table and clearing variablization map.\n");
  clear_variablization_table();
  if (original_symbol_ht)
    clear_OSD_table();
  create_OSD_table();
}

variablization * Variablization_Manager::get_variablization(Symbol *index_sym)
{
  std::map< Symbol *, variablization * >::iterator iter = (*variablization_table).find(index_sym);
  if (iter != (*variablization_table).end())
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...found %s in variablization table: %s/%s\n", index_sym->to_string(thisAgent),
       iter->second->variablized_symbol->to_string(thisAgent), iter->second->instantiated_symbol->to_string(thisAgent));
      return iter->second;
  }
  else
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...did not find %s in variablization table.\n", index_sym->to_string(thisAgent));
    return NULL;
  }
}

void Variablization_Manager::store_variablization(Symbol *index_sym, Symbol *instantiated_sym, Symbol *variable, bool is_equality_test)
{
  variablization *new_variablization;
  assert(index_sym && instantiated_sym && variable);
  dprint(DT_VARIABLIZATION_MANAGER, "Variablization_Manager storing symbol %s -=> %s, %s (grounded %d)\n",
          index_sym->to_string(thisAgent),
          instantiated_sym->to_string(thisAgent),
          variable->to_string(thisAgent),
          is_equality_test);

  new_variablization = new variablization;
  new_variablization->instantiated_symbol = instantiated_sym;
  new_variablization->variablized_symbol = variable;
  new_variablization->grounded = is_equality_test;

  (*variablization_table)[index_sym] = new_variablization;

  /* -- An identifier may have more than one original symbol (mostly due to the fact
   *    that placeholder variables still exist to handle dot notation, and it wasn't
   *    clear whether we should alter that mechanism).  So, we store two entries in
   *    the variablization table.
   *    (1) When variablizing identifiers, the identifier symbol will be used to look
   *        up variablization info.
   *    (2) Later, when looking for ungrounded variables in relational tests, the
   *        identifier symbol will have already been replaced with a variable,
   *        so we must use the variable instead to look up variablization info. -- */

  if (index_sym->is_sti())
  {
    dprint(DT_VARIABLIZATION_MANAGER, "Variablization_Manager also storing secondary index for identifier.\n");
    store_variablization(variable, instantiated_sym, variable, is_equality_test);
    symbol_add_ref(thisAgent, instantiated_sym);
    symbol_add_ref(thisAgent, variable);
    symbol_add_ref(thisAgent, variable);
  }
}

/* -- variablize_rl_symbol is a very limited version of variablization for templates
 *
 *    Templates
 */
void Variablization_Manager::variablize_rl_symbol (Symbol **sym, bool is_equality_test)
{
  char prefix[2];
  Symbol *var;
  variablization *var_info;

  if (!(*sym)->is_sti()) return;

  dprint(DT_VARIABLIZATION_MANAGER, "Variablization_Manager variablizing rl symbol %s.\n", (*sym)->to_string(thisAgent));

  var_info = get_variablization((*sym));
  if (var_info)
  {
    if (is_equality_test && !var_info->grounded)
    {
      var_info->grounded = true;
      /* -- Update secondary index for identifiers -- */
      variablization *var_info2;
      dprint(DT_VARIABLIZATION_MANAGER, "...updating grounded info for %s %s %d.\n", (*sym)->to_string(thisAgent),
          var_info->variablized_symbol->to_string(thisAgent), is_equality_test);
      var_info2 = get_variablization(var_info->variablized_symbol);
      var_info2->grounded = true;
    }
    /* -- Symbol being passed in is being replaced, so decrease -- */
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
  var->var->was_identifier = (*sym)->is_identifier();

  store_variablization((*sym), (*sym), var, is_equality_test);

  /* -- Though generate_new_variable() adds a refcount, we also add them for our stored pointers
   *    in the variablization table.  These are cleaned up after RHS variablization. -- */
  symbol_add_ref(thisAgent, *sym);
  symbol_add_ref(thisAgent, var);
  symbol_add_ref(thisAgent, (*sym));
  dprint(DT_VARIABLIZATION_MANAGER, "...created new variablization %s.\n", var->to_string(thisAgent));

  /* MToDoRefCnt | This remove ref was removed before, but it seems like we should have it, no? */
  symbol_remove_ref (thisAgent, *sym);
  *sym = var;
}

void Variablization_Manager::variablize_lhs_symbol (Symbol **sym, Symbol *original_symbol, bool is_equality_test)
{
  char prefix[2];
  Symbol *index_var, *var;
  variablization *var_info;
  bool is_st_id = (*sym)->is_sti();

  dprint(DT_VARIABLIZATION_MANAGER, "Variablization_Manager variablizing %s %s %d.\n", (*sym)->to_string(thisAgent)
      , (original_symbol ? original_symbol->to_string(thisAgent) : "NULL"), is_equality_test);

  if (!is_st_id && original_symbol)
    index_var = original_symbol;
  else
    index_var = *sym;

  var_info = get_variablization(index_var);
  if (var_info)
  {
    if (is_equality_test && !var_info->grounded)
    {
      var_info->grounded = true;
      /* -- Update secondary index for identifiers -- */
      if (is_st_id)
      {
        variablization *var_info2;
        dprint(DT_VARIABLIZATION_MANAGER, "...updating grounded info for %s %s %d.\n", index_var->to_string(thisAgent),
               var_info->variablized_symbol->to_string(thisAgent), is_equality_test);
        var_info2 = get_variablization(var_info->variablized_symbol);
        var_info2->grounded = true;
      }
    }
    /* -- Symbol being passed in is being replaced, so decrease -- */
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
  var->var->was_identifier = is_st_id;

  store_variablization(index_var, (*sym), var, is_equality_test);

  /* -- Though generate_new_variable() adds a refcount, we also add them for our stored pointers
   *    in the variablization table.  These are cleaned up after RHS variablization. -- */
  symbol_add_ref(thisAgent, *sym);
  symbol_add_ref(thisAgent, var);
  symbol_add_ref(thisAgent, index_var);
  dprint(DT_VARIABLIZATION_MANAGER, "...created new variablization %s.\n", var->to_string(thisAgent));

  /* MToDoRefCnt | This remove ref was removed before, but it seems like we should have it, no? */
  symbol_remove_ref (thisAgent, *sym);
  *sym = var;
}
/* ======================================================================================================
 *
 *                                          variablize_rhs_symbol
 *
 *      The logic for variablizing the rhs is slightly different than the lhs since we need to
 *      match constants on the rhs with the conditions they match up with on the lhs. We can't
 *      just look up the variablization info from the symbol, like we do on the lhs.  So, we match
 *      them up using the original variable names, which are cached by the rete for both rhs actions
 *      and lhs tests when creating the instantiation.
 *
 *      We need to do this for 2 reasons.
 *
 *        (1) If an instantiation has two different conditions that each have an item that binds to
 *            the same constant but, in their original productions,bind to different variables, we
 *            will need a way to differentiate between the two to determine whether to variablize
 *            this rhs action. But, since variablization info is stored in a symbol, we can't store
 *            more than one mapping to a condition or variable without doing something convoluted.
 *
 *        (2) Second, the condition grounding a rhs action may have not made it into the conditions
 *            of the chunk if it was produced without testing anything in the superstate.  (This is
 *            further complicated by the fact that another variable might have been bound
 *            coincidentally to the same constant, so variablization info will be stored in the
 *            constant's symbol.)
 *
 *      So, we use the original variable names that we've stored in the both the symbol and the rhs
 *      action to  match up with the specific binding on the lhs that it originated from.  To deal
 *      with both the fact that a production can fire multiple times and have multiple versions of
 *      the same conditions appear in a chunk and the fact that different productions may use the
 *      same variable names, make_instantiation renames the original variable names when it creates
 *      each instantiation to make them unique between instantiations.  So, we are guaranteed to
 *      match up each element of the rhs action to the correct lhs binding.
 *
 *      The function does the following:
 *
 *      Check if original var names of rhs action matches with the original var name of what's in
 *      the symbol that is bound there.  (not necessary since we look up in hash table directly in
 *      the next step, but comes cheap since it's already cached there for lhs variablization
 *      and will work in most cases)  If it matches, variablize using stored variablization sym.
 *      If it doesn't match, search unique string table for the original variable name stringleave rhs item as constant if no variablization sym)
 *
 *        (No)  Look up the the last variablization symbol associated with that original
 *              variable name in the unique var name hash table.
 *
 *              (Found)     Use the variablization info cached within it.
 *
 *              (Not found) Keep as a constant.
 *
 *      Note: the original id is in the sym's variablized symbol's original_var pointer
 *
  ====================================================================================================== */

void Variablization_Manager::variablize_rhs_symbol (Symbol **sym, Symbol *original_symbol) {
  char prefix[2];
  Symbol *var, *index_sym;
  variablization *found_variablization;
  bool is_st_id;

  dprint(DT_VARIABLIZATION_MANAGER, "variablize_rhs_symbol called for %s(%s).\n",
      (*sym)->to_string(thisAgent),
      (original_symbol ? original_symbol->to_string(thisAgent) : "NULL"));

  /* -- identifiers and unbound vars (which are instantiated as identifiers) are indexed by their symbol
   *    instead of their original variable. --  */
  is_st_id = (*sym)->is_sti();

  if (is_st_id)
    index_sym = *sym;
  else
  {
    if (original_symbol)
      index_sym = original_symbol;
    else
    {
      dprint(DT_VARIABLIZATION_MANAGER, "...is a literal constant.  Not variablizing!\n");
      return;
    }
  }

//  if (!((*sym)->is_non_lti_identifier()) && original_symbol)
//    index_var = original_symbol;
//  else
//    index_var = *sym;

  dprint(DT_VARIABLIZATION_MANAGER, "...searching for varname %s in unique varname table...\n", index_sym->to_string(thisAgent));
  found_variablization = thisAgent->variablizationManager->get_variablization(index_sym);

  if (found_variablization)
  {
    if (found_variablization->grounded)
    {
      /* --- Grounded symbol that has been variablized before--- */

      dprint(DT_VARIABLIZATION_MANAGER, "... found existing grounded variablization %s.\n", found_variablization->variablized_symbol->to_string(thisAgent));

      symbol_add_ref(thisAgent, found_variablization->variablized_symbol);
      //symbol_remove_ref (thisAgent, (*sym));
      *sym = found_variablization->variablized_symbol;
      return;
    }
    else if (!is_st_id)
    {
      dprint(DT_VARIABLIZATION_MANAGER, "...is ungrounded constant.  Not variablizing!\n");
      return;
    }
    else
    {
      /* -- Ungrounded identifiers fall into this case.  This will pass through this case
       *    and create an unbound var in next code block. -- */

      /* -- Delete the symbol references for both entries in the variablization table
       *    then delete the entries themselves. */
      dprint(DT_VARIABLIZATION_MANAGER, "...is ungrounded identifier.  Clearing variablization entry and generating unbound var.\n");
      print_variablization_table();
      symbol_remove_ref(thisAgent, index_sym);
      symbol_remove_ref(thisAgent, found_variablization->variablized_symbol);
      symbol_remove_ref(thisAgent, found_variablization->instantiated_symbol);
      variablization_table->erase(index_sym);

      dprint(DT_VARIABLIZATION_MANAGER, "...searching for varname %s in unique varname table...\n", found_variablization->variablized_symbol->to_string(thisAgent));
      delete found_variablization;
      found_variablization = thisAgent->variablizationManager->get_variablization(found_variablization->variablized_symbol);
      dprint(DT_VARIABLIZATION_MANAGER, "...searching for varname %s in unique varname table...\n", found_variablization->variablized_symbol->to_string(thisAgent));

      /* MToDoRefCnt | This was disabled before.  Might have just been for initial testing. */
      symbol_remove_ref(thisAgent, found_variablization->instantiated_symbol);
      symbol_remove_ref(thisAgent, found_variablization->variablized_symbol);
      symbol_remove_ref(thisAgent, found_variablization->variablized_symbol);
      variablization_table->erase(found_variablization->variablized_symbol);
//      print_variablization_table();
    }
  }

  /* -- Variablization manager has never seen this symbol.  Unbound RHS var or constant. -- */

  if((*sym)->is_sti())
  {
    /* -- First instance of an unbound rhs var -- */
    dprint(DT_VARIABLIZATION_MANAGER, "...is unbound variable.\n");
    prefix[0] = static_cast<char>(tolower((*sym)->id->name_letter));
    prefix[1] = 0;
    var = generate_new_variable (thisAgent, prefix);

    dprint(DT_VARIABLIZATION_MANAGER, "...created new variable for unbound rhs %s.\n", var->to_string(thisAgent));
    thisAgent->variablizationManager->store_variablization((*sym), (*sym), var, true);

    /* -- Though generate_new_variable() adds a refcount, we also add them for our stored pointers
     *    in the variablization table.  These are cleaned up after RHS variablization. -- */
    symbol_add_ref(thisAgent, *sym);
    symbol_add_ref(thisAgent, *sym);
    symbol_add_ref(thisAgent, var);
    //symbol_remove_ref (thisAgent, (*sym));
    *sym = var;
  }
  else
  {
    /* -- RHS constant that was not in LHS condition -- */
    dprint(DT_VARIABLIZATION_MANAGER, "...is a variable that did not appear in the LHS.  Not variablizing!\n");
  }
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

void Variablization_Manager::create_OSD_table()
{
  dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager creating hash table.\n");
  original_symbol_ht = make_hash_table (thisAgent, 0, hash_unique_string);
  init_memory_pool (thisAgent, &original_symbol_mp, sizeof(original_symbol_data), "unique_string");

}

bool free_original_symbol_data (agent* thisAgent, void *item, void*) {

  original_symbol_data *varname = static_cast<original_symbol_data *>(item);
  if (varname->current_unique_var_symbol)
  {
    dprint(DT_UNIQUE_VARIABLIZATION, "...decreasing refcount on symbol %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
    symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
    varname->current_unique_var_symbol = NULL;
  }
  varname->current_instantiation = NULL;
  dprint(DT_UNIQUE_VARIABLIZATION, "...freeing memory for string %s\n", varname->name);
  free_memory_block_for_string(thisAgent, varname->name);
  return false;
}

void Variablization_Manager::clear_OSD_table()
{
  dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager clearing hash table of original_vars...\n");
  do_for_all_items_in_hash_table( thisAgent, original_symbol_ht, free_original_symbol_data, 0);

  free_memory(thisAgent, original_symbol_ht->buckets, HASH_TABLE_MEM_USAGE);
  free_memory(thisAgent, original_symbol_ht, HASH_TABLE_MEM_USAGE);
}

void Variablization_Manager::clear_CUV_for_symbol(Symbol *var)
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
        dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager decreasing refcount on symbol %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
        symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
        varname->current_unique_var_symbol = NULL;
      }
      varname->current_instantiation = NULL;
    }
  }
}
void Variablization_Manager::clear_CUV_cache() {

  dprint(DT_UNIQUE_VARIABLIZATION, "Original_Variable_Manager clearing unique var cache...\n");
  for (std::set< Symbol *>::iterator it=(*current_unique_vars).begin(); it!=(*current_unique_vars).end(); ++it)
  {
    dprint(DT_UNIQUE_VARIABLIZATION, "Erasing current_unique_var %s\n", (*it)->to_string(thisAgent));
  }
  current_unique_vars->clear();
}

bool Variablization_Manager::already_unique(Symbol *original_var) {

  dprint(DT_UNIQUE_VARIABLIZATION, "...checking if %s is already unique...", original_var->to_string(thisAgent));

  std::set< Symbol * >::iterator it = current_unique_vars->find(original_var);
  if (it != current_unique_vars->end()) {
    dprint_noprefix(DT_UNIQUE_VARIABLIZATION, " = TRUE\n");
    return true;
  }
  dprint_noprefix(DT_UNIQUE_VARIABLIZATION, " = FALSE\n");
  return false;
}

/* -- make_name_unique takes a symbol and replaces it with a unique version if it hasn't already
 *    been made unique for the current instantiation (thisAgent->newly_created_instantiations) -- */

void Variablization_Manager::make_name_unique(Symbol **sym)
{
  uint32_t hash_value;
  original_symbol_data *varname, *new_varname;

  dprint(DT_UNIQUE_VARIABLIZATION, "...uniqueifying %s for instantiation %s...\n",
                                           (*sym)->var->name,
                                           thisAgent->newly_created_instantiations->prod->name->sc->name );

  if (already_unique(*sym))
  {
    dprint(DT_UNIQUE_VARIABLIZATION, "...already unique, so using existing original variable %s\n",
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

        dprint(DT_UNIQUE_VARIABLIZATION, "...found existing mapping %s -> %s for this instantiation.\n",
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
          dprint(DT_UNIQUE_VARIABLIZATION, "...cleaning up current unique var still in variablization manager OSD table: %s\n", varname->current_unique_var_symbol->to_string(thisAgent));
          symbol_remove_ref(thisAgent, varname->current_unique_var_symbol);
        }
        varname->current_unique_var_symbol = make_variable(thisAgent, new_name.c_str());

        dprint(DT_UNIQUE_VARIABLIZATION, "...creating new unique version of %s: %s\n",
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
  dprint(DT_UNIQUE_VARIABLIZATION, "...first use, so using original variable %s\n",
                                    (*sym)->var->name);
}

/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

bool print_original_symbol_data (agent* thisAgent, void *item, void*) {

  original_symbol_data *varname = static_cast<original_symbol_data *>(item);

  dprint(DT_VARIABLIZATION_MANAGER, "%s, CurrUnqVarSym: %s(%lld) CurrInst: %d\n",
        varname->name,
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->to_string(thisAgent) : "None"),
        (varname->current_unique_var_symbol ? varname->current_unique_var_symbol->reference_count : 0),
        (varname->current_instantiation ? varname->current_instantiation : NULL));
  return false;
}

void Variablization_Manager::print_OSD_table()
{
  dprint(DT_VARIABLIZATION_MANAGER, "------------------------------------\n");
  dprint(DT_VARIABLIZATION_MANAGER, "   Variablization OSD Hash Table\n");
  dprint(DT_VARIABLIZATION_MANAGER, "------------------------------------\n");
  do_for_all_items_in_hash_table( thisAgent, original_symbol_ht, print_original_symbol_data, 0);
}

void Variablization_Manager::print_variablization_table()
{
  dprint(DT_VARIABLIZATION_MANAGER, "------------------------------------\n");
  dprint(DT_VARIABLIZATION_MANAGER, "       Variablization Table\n");
  dprint(DT_VARIABLIZATION_MANAGER, "------------------------------------\n");
  for (std::map< Symbol *, variablization * >::iterator it=(*variablization_table).begin(); it!=(*variablization_table).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "%s -> %s/%s (grounded %d)\n", it->first->to_string(thisAgent),
        it->second->variablized_symbol->to_string(thisAgent), it->second->instantiated_symbol->to_string(thisAgent), it->second->grounded);
  }
}
void Variablization_Manager::print_CUV_table() {

  dprint(DT_VARIABLIZATION_MANAGER, "------------------------------------\n");
  dprint(DT_VARIABLIZATION_MANAGER, "   Current Unique Variable Table\n");
  dprint(DT_VARIABLIZATION_MANAGER, "------------------------------------\n");
  for (std::set< Symbol *>::iterator it=(*current_unique_vars).begin(); it!=(*current_unique_vars).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "%s\n", (*it)->to_string(thisAgent));
  }
}
void Variablization_Manager::print_tables()
{
  print_OSD_table();
  print_variablization_table();
  print_CUV_table();
}
