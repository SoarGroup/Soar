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
#include "test.h"
#include "debug.h"

inline variablization * copy_variablization(agent *thisAgent, variablization *v)
{
  variablization *new_variablization = new variablization;
  new_variablization->instantiated_symbol = v->instantiated_symbol;
  new_variablization->variablized_symbol = v->variablized_symbol;
  symbol_add_ref(thisAgent, new_variablization->instantiated_symbol);
  symbol_add_ref(thisAgent, new_variablization->variablized_symbol);
  new_variablization->grounded = v->grounded;
  new_variablization->grounding_id= v->grounding_id;
  return new_variablization;
}

Variablization_Manager::Variablization_Manager(agent *myAgent)
{
  thisAgent = myAgent;
  sym_to_var_map = new std::map< Symbol *, variablization * >();
  g_id_to_var_map = new std::map< uint64_t, variablization * >();
  ovar_to_g_id_map = new std::map< Symbol *, uint64_t >();
  constraints = new std::map< test , ::list * >();

  ground_id_counter = 0;
}

Variablization_Manager::~Variablization_Manager()
{
  clear_data();
  delete sym_to_var_map;
  delete g_id_to_var_map;
  delete ovar_to_g_id_map;
  delete constraints;
}

void Variablization_Manager::clear_data()
{
  dprint(DT_VARIABLIZATION_MANAGER, "Clearing variablization maps.\n");
  clear_relational_constraints ();
  clear_variablization_table();
}

void Variablization_Manager::reinit()
{
  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
  clear_data();
  ground_id_counter = 0;
}

/* -- ----------------------------------
 *    Variablization functions
 *    ----------------------------------
 *    The following functions handle variablization of LHS items.  It replaces
 *    variablize_symbol.
 *
 *    -- */


void Variablization_Manager::clear_variablization_table() {

  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing variablization data...\n");
  print_variablization_tables(DT_VARIABLIZATION_MANAGER);

  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar table...\n");
  /* -- Clear original variable map -- */
  for (std::map< Symbol *, uint64_t >::iterator it=(*ovar_to_g_id_map).begin(); it!=(*ovar_to_g_id_map).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing %s -> %llu\n", it->first->to_string(), it->second);
    symbol_remove_ref(thisAgent, it->first);
  }
  ovar_to_g_id_map->clear();

  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing symbol->variablization map...\n");
  /* -- Clear symbol->variablization map -- */
  for (std::map< Symbol *, variablization * >::iterator it=(*sym_to_var_map).begin(); it!=(*sym_to_var_map).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing %s -> %s(%lld)/%s(%lld)\n",
        it->first->to_string(),
        it->second->instantiated_symbol->to_string(), it->second->instantiated_symbol->reference_count,
        it->second->variablized_symbol->to_string(),  it->second->variablized_symbol->reference_count);
    symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
    symbol_remove_ref(thisAgent, it->second->variablized_symbol);
    delete it->second;
  }
  sym_to_var_map->clear();

  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing grounding_id->variablization map...\n");
  /* -- Clear grounding_id->variablization map -- */
  for (std::map< uint64_t, variablization * >::iterator it=(*g_id_to_var_map).begin(); it!=(*g_id_to_var_map).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing %llu -> %s(%lld)/%s(%lld)\n",
        it->first,
        it->second->instantiated_symbol->to_string(), it->second->instantiated_symbol->reference_count,
        it->second->variablized_symbol->to_string(),  it->second->variablized_symbol->reference_count);
    symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
    symbol_remove_ref(thisAgent, it->second->variablized_symbol);
    delete it->second;
  }
  g_id_to_var_map->clear();
  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing variablization data.\n");
}

variablization * Variablization_Manager::get_variablization(uint64_t index_id)
{
  std::map< uint64_t, variablization * >::iterator iter = (*g_id_to_var_map).find(index_id);
  if (iter != (*g_id_to_var_map).end())
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...found %llu in g_id variablization table: %s/%s\n", index_id,
       iter->second->variablized_symbol->to_string(), iter->second->instantiated_symbol->to_string());
      return iter->second;
  }
  else
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...did not find %llu in g_id variablization table.\n", index_id);
    print_variablization_tables(DT_VARIABLIZATION_MANAGER, 2);
    return NULL;
  }
}

variablization * Variablization_Manager::get_variablization(Symbol *index_sym)
{
  std::map< Symbol *, variablization * >::iterator iter = (*sym_to_var_map).find(index_sym);
  if (iter != (*sym_to_var_map).end())
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...found %s in variablization table: %s/%s\n", index_sym->to_string(),
       iter->second->variablized_symbol->to_string(), iter->second->instantiated_symbol->to_string());
      return iter->second;
  }
  else
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...did not find %s in variablization table.\n", index_sym->to_string());
    print_variablization_tables(DT_VARIABLIZATION_MANAGER, 1);
    return NULL;
  }
}

uint64_t Variablization_Manager::get_gid_for_orig_var(Symbol *index_sym)
{
  std::map< Symbol *, uint64_t >::iterator iter = (*ovar_to_g_id_map).find(index_sym);
  if (iter != (*ovar_to_g_id_map).end())
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...found %llu in orig_var variablization table for %s\n",
        iter->second, index_sym);

    return iter->second;
  }
  else
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...did not find %s in orig_var variablization table.\n", index_sym);
  }

  print_variablization_tables(DT_VARIABLIZATION_MANAGER, 3);

  return 0;
}

void Variablization_Manager::add_orig_var_mappings_for_test(test t)
{
  cons *c;
  test check_test;

  switch (t->type)
  {
    case DISJUNCTION_TEST:
    case GOAL_ID_TEST:
    case IMPASSE_ID_TEST:
      break;
    case CONJUNCTIVE_TEST:
      dprint(DT_VARIABLIZATION_MANAGER, "Adding original variable mappings for conjunctive test\n");
      cons *c;
      test check_test;
      for (c=t->data.conjunct_list; c!=NIL; c=c->rest)
      {
        add_orig_var_mappings_for_test(static_cast<test>(c->first));
      }
      break;
    default:
      assert(t->data.referent);
//      dprint(DT_VARIABLIZATION_MANAGER, "Adding original variable mappings for test with referent.\n");
      if (t->identity && t->identity->original_var && (t->identity->grounding_id > 0))
      {
        dprint(DT_VARIABLIZATION_MANAGER, "Adding original variable mappings entry: %s -> %llu\n", t->identity->original_var->to_string(), t->identity->grounding_id);
        (*ovar_to_g_id_map)[t->identity->original_var] = t->identity->grounding_id;
        symbol_add_ref(thisAgent, t->identity->original_var);
      } else {
//        dprint(DT_VARIABLIZATION_MANAGER, "Did not add b/c %s %s %llu.\n",
//            (t->identity ? "True" : "False"),
//            ((t->identity && t->identity->original_var) ? t->identity->original_var : "No orig var"),
//            ((t->identity && t->identity->grounding_id) ? t->identity->grounding_id : 0));
      }
  }
}

void Variablization_Manager::add_orig_var_mappings_for_cond(condition *cond)
{
  switch (cond->type) {
  case POSITIVE_CONDITION:
  case NEGATIVE_CONDITION:
    add_orig_var_mappings_for_test(cond->data.tests.id_test);
    add_orig_var_mappings_for_test(cond->data.tests.attr_test);
    add_orig_var_mappings_for_test(cond->data.tests.value_test);
    break;
  case CONJUNCTIVE_NEGATION_CONDITION:
    add_orig_var_mappings_for_cond_list (cond->data.ncc.top);
    break;
  }
}

void Variablization_Manager::add_orig_var_mappings_for_cond_list(condition *cond)
{
  dprint(DT_VARIABLIZATION_MANAGER, "=============================================\n");
  dprint(DT_VARIABLIZATION_MANAGER, "add_orig_var_mappings_for_cond_list called...\n");
  print_variablization_tables(DT_VARIABLIZATION_MANAGER, 3);
  while (cond) {
    dprint(DT_VARIABLIZATION_MANAGER, "Adding original variable mappings for cond ");
    dprint_condition(DT_VARIABLIZATION_MANAGER, cond, "", true, true, true);
    add_orig_var_mappings_for_cond(cond);
    cond = cond->next;
  }
  dprint(DT_VARIABLIZATION_MANAGER, "Done adding original var mappings.\n");
  print_variablization_tables(DT_VARIABLIZATION_MANAGER, 3);
  dprint(DT_VARIABLIZATION_MANAGER, "=============================================\n");
}

void Variablization_Manager::store_variablization(Symbol *instantiated_sym,
                                                  Symbol *variable,
                                                  identity_info *identity,
                                                  bool is_equality_test)
{
  variablization *new_variablization;
  assert(instantiated_sym && variable);
  dprint(DT_VARIABLIZATION_MANAGER, "Storing variablization for %s(%llu) -=> %s (grounded %s) in ",
          instantiated_sym->to_string(),
          identity ? identity->grounding_id : 0,
          variable->to_string(),
          is_equality_test ? "T" : "F");

  new_variablization = new variablization;
  new_variablization->instantiated_symbol = instantiated_sym;
  new_variablization->variablized_symbol = variable;
  symbol_add_ref(thisAgent, instantiated_sym);
  symbol_add_ref(thisAgent, variable);
  new_variablization->grounded = is_equality_test;
  new_variablization->grounding_id = identity ? identity->grounding_id : 0;

  if (instantiated_sym->is_sti())
  {
    /* -- STI may have more than one original symbol (mostly due to the fact
     *    that placeholder variables still exist to handle dot notation).  So, we
     *    look them up using the identifier symbol instead of the original variable.
     *
     *    Note that we also store an entry using the new variable as an index. Later,
     *    when looking for ungrounded variables in relational tests, the
     *    identifier symbol will have already been replaced with a variable,
     *    so we must use the variable instead to look up variablization info.
     *    This may not be necessary after we resurrect the old NOT code. -- */

    (*sym_to_var_map)[instantiated_sym] = new_variablization;
    (*sym_to_var_map)[variable] = copy_variablization(thisAgent, new_variablization);
    dprint_noprefix(DT_VARIABLIZATION_MANAGER, "symbol ([%s][%s] variablization table.\n",
        instantiated_sym->to_string(), variable->to_string());
  } else if (identity) {

    /* -- A constant symbol is being variablized, so store variablization info
     *    indexed by the constant's grounding id. -- */
    (*g_id_to_var_map)[identity->grounding_id] = new_variablization;

    dprint_noprefix(DT_VARIABLIZATION_MANAGER, "identity[%llu] variablization table.\n",
        identity->grounding_id);
  } else {
    assert(false);
  }
//  print_variablization_table();
}

/* -- variablize_rl_symbol is a very limited version of variablization for templates
 *    - The symbol passed in is guaranteed to be a short-term identifier.
 */
void Variablization_Manager::variablize_rl_symbol (Symbol **sym, bool is_equality_test)
{
  char prefix[2];
  Symbol *var;
  variablization *var_info;

  if (!(*sym)->is_sti()) return;

  dprint(DT_VARIABLIZATION_MANAGER, "Variablization_Manager variablizing rl symbol %s.\n", (*sym)->to_string());

  var_info = get_variablization((*sym));
  if (var_info)
  {
    if (is_equality_test && !var_info->grounded)
    {
      var_info->grounded = true;
      /* -- Update secondary index for identifiers -- */
      variablization *var_info2;
      dprint(DT_VARIABLIZATION_MANAGER, "...updating grounded info for %s %s %s.\n", (*sym)->to_string(),
          var_info->variablized_symbol->to_string(), (is_equality_test ? "T" : "F"));
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

  store_variablization((*sym), var, NULL, is_equality_test);

  dprint(DT_VARIABLIZATION_MANAGER, "...created new variablization %s.\n", var->to_string());

  /* MToDoRefCnt | This remove ref was removed before, but it seems like we should have it, no? */
  symbol_remove_ref (thisAgent, *sym);
  *sym = var;
}

void Variablization_Manager::variablize_lhs_symbol (Symbol **sym, Symbol *original_symbol, identity_info *identity, bool is_equality_test)
{
  char prefix[2];
  Symbol *var;
  variablization *var_info;
  bool is_st_id = (*sym)->is_sti();

  dprint(DT_VARIABLIZATION_MANAGER, "Variablizing %s(%llu, %s) %s.\n",
      (*sym)->to_string(),
      (identity ? identity->grounding_id : 0),
      (original_symbol ? original_symbol->to_string() : "NULL"),
      (is_equality_test ? "T" : "F"));

  if (!is_st_id)
  {
    assert(identity);
    var_info = get_variablization(identity->grounding_id);
  } else {
    var_info = get_variablization(*sym);
  }
  if (var_info)
  {
    if (is_equality_test && !var_info->grounded)
    {
      var_info->grounded = true;
      if (is_st_id)
      {
        /* -- Update secondary index for identifiers -- */
        variablization *var_info2;
        dprint(DT_VARIABLIZATION_MANAGER, "...updating grounded info for %s/%s\n",
            (*sym)->to_string(),
            var_info->variablized_symbol->to_string());
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

  store_variablization((*sym), var, identity, is_equality_test);

  dprint(DT_VARIABLIZATION_MANAGER, "...created new variablization %s.\n", var->to_string());

  /* MToDoRefCnt | This remove ref was removed before, but it seems like we should have it, no? */
  symbol_remove_ref (thisAgent, *sym);
  *sym = var;
}
/* ======================================================================================================
 *
 *                                          variablize_rhs_symbol
 *
 *      The logic for variablizing the rhs is slightly different than the lhs since we need to
 *      match constants on the rhs with the conditions they match up with on the lhs. So, we match
 *      them up using the original variable names.
 *
 * ====================================================================================================== */

uint64_t Variablization_Manager::variablize_rhs_symbol (Symbol **sym, Symbol *original_var) {
  char prefix[2];
  Symbol *var;
  variablization *found_variablization=NIL;
  bool is_st_id;
  uint64_t g_id;

  dprint(DT_VARIABLIZATION_MANAGER, "variablize_rhs_symbol called for %s(%s).\n",
      (*sym)->to_string(),
      (original_var ? original_var->to_string() : "NULL"));

  /* -- identifiers and unbound vars (which are instantiated as identifiers) are indexed by their symbol
   *    instead of their original variable. --  */
  is_st_id = (*sym)->is_sti();

  if (is_st_id)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...searching for sti %s in variablization sym table...\n", (*sym)->to_string());
    found_variablization = get_variablization(*sym);
  }
  else
  {
    if (original_var)
    {
      dprint(DT_VARIABLIZATION_MANAGER, "...searching for original var %s in variablization orig var table...\n", original_var);
      g_id = get_gid_for_orig_var(original_var);
      if (g_id > 0)
      {
        found_variablization = get_variablization(g_id);
      }
      else
      {
        dprint(DT_VARIABLIZATION_MANAGER, "...did not find entry for g_id %llu!  Not variablizing!\n", g_id);
        this->print_variablization_tables(DT_VARIABLIZATION_MANAGER, 2);
      }
    }
    else
    {
      dprint(DT_VARIABLIZATION_MANAGER, "...is a literal constant.  Not variablizing!\n");
      return 0;
    }
  }


  if (found_variablization)
  {
    if (found_variablization->grounded)
    {
      /* --- Grounded symbol that has been variablized before--- */

      dprint(DT_VARIABLIZATION_MANAGER, "... found existing grounded variablization %s.\n", found_variablization->variablized_symbol->to_string());

      symbol_add_ref(thisAgent, found_variablization->variablized_symbol);
      //symbol_remove_ref (thisAgent, (*sym));
      *sym = found_variablization->variablized_symbol;
      return found_variablization->grounding_id;
    }
    else if (!is_st_id)
    {
      dprint(DT_VARIABLIZATION_MANAGER, "...is ungrounded constant.  Not variablizing!\n");
      return 0;
    }
    else
    {
      /* -- Ungrounded short-term identifier
       *    This will pass through this case and create an unbound var in next code block. -- */

      /* -- Delete the symbol references for both entries in the variablization table
       *    then delete the entries themselves. */
      dprint(DT_VARIABLIZATION_MANAGER, "...is ungrounded identifier.  Clearing variablization entry for %s/%s and generating unbound var.\n",
          (*sym)->to_string(), found_variablization->variablized_symbol->to_string());

      print_variablization_tables(DT_VARIABLIZATION_MANAGER, 1);
      sym_to_var_map->erase(*sym);
      sym_to_var_map->erase(found_variablization->variablized_symbol);
      symbol_remove_ref(thisAgent, found_variablization->variablized_symbol);
      symbol_remove_ref(thisAgent, found_variablization->instantiated_symbol);
      delete found_variablization;
      print_variablization_tables(DT_VARIABLIZATION_MANAGER, 1);
    }
  }

  /* -- Either the variablization manager has never seen this symbol or symbol is ungrounded symbol or literal constant.
   *    Both cases return 0.  Grounding id will be generate if requested by another match. -- */

  if((*sym)->is_sti())
  {
    /* -- First instance of an unbound rhs var -- */
    dprint(DT_VARIABLIZATION_MANAGER, "...is unbound variable.\n");
    prefix[0] = static_cast<char>(tolower((*sym)->id->name_letter));
    prefix[1] = 0;
    var = generate_new_variable (thisAgent, prefix);

    dprint(DT_VARIABLIZATION_MANAGER, "...created new variable for unbound rhs %s.\n", var->to_string());
    store_variablization((*sym), var, NULL, true);

    *sym = var;
  }
  else
  {
    /* -- RHS constant that was not in LHS condition.  -- */

    /* MToDo | Is this even possible?  Won't this be caught by not having an original var above? */
    dprint(DT_VARIABLIZATION_MANAGER, "...is a variable that did not appear in the LHS.  Not variablizing!\n");
  }
  return 0;
}


void Variablization_Manager::print_relational_constraints (TraceMode mode)
{
  dprint(mode, "------------------------------------\n");
  dprint(mode, "            Constraint Map\n");
  dprint(mode, "------------------------------------\n");

  cons *c;

  for (std::map< test, ::list * >::iterator it=constraints->begin(); it!=constraints->end(); ++it)
  {
    c = it->second;
    while (c) {
      dprint_test(mode, it->first, true, true, true);
      dprint_test(mode, static_cast<test>(c->first), true, true, true, " ", "\n");
      c = c->rest;
    }
  }
}

void Variablization_Manager::clear_relational_constraints ()
{
  for (std::map< test, ::list * >::iterator it=constraints->begin(); it!=constraints->end(); ++it)
  {
    free_list (thisAgent, it->second);
  }
  constraints->clear();
}

void Variablization_Manager::add_relational_constraint (test equality_test, test relational_test)
{
  ::list * new_list=NULL;
  dprint(DT_CONSTRAINTS, "Adding relational constraint %s to %s.\n", test_to_string(relational_test), test_to_string(equality_test));
  std::map< test, ::list * >::iterator iter = (*constraints).find(equality_test);
  if (iter == constraints->end())
  {
    push(thisAgent, (relational_test), new_list);
    (*constraints)[equality_test] = new_list;
  }
  else
  {
    new_list = (*constraints)[equality_test];
    push(thisAgent, (relational_test), new_list);
    (*constraints)[equality_test] = new_list;
  }
}

/* -- A utility function to print all data stored in the variablization manager.  Used only for debugging -- */

void Variablization_Manager::print_variablization_tables(TraceMode mode, int whichTable)
{
  dprint(mode, "------------------------------------\n");
  if (whichTable == 0)
  {
    dprint(mode, "       Variablization Tables\n");
    dprint(mode, "------------------------------------\n");
  }
  if ((whichTable == 0) || (whichTable == 1))
  {
    dprint(mode, "------------ Symbol -> v_info table ----------\n");
    if (whichTable != 0)
      dprint(mode, "------------------------------------\n");
    for (std::map< Symbol *, variablization * >::iterator it=(*sym_to_var_map).begin(); it!=(*sym_to_var_map).end(); ++it)
    {
      dprint(mode, "%s -> %s/%s (grounded %d)\n", it->first->to_string(),
          it->second->variablized_symbol->to_string(), it->second->instantiated_symbol->to_string(), it->second->grounded);
    }
  }
  if ((whichTable == 0) || (whichTable == 2))
  {
    dprint(mode, "--------- G_ID -> v_info table -------\n");
    if (whichTable != 0)
      dprint(mode, "------------------------------------\n");
    for (std::map< uint64_t, variablization * >::iterator it=(*g_id_to_var_map).begin(); it!=(*g_id_to_var_map).end(); ++it)
    {
      dprint(mode, "%llu -> %s/%s (grounded %d)\n", it->first,
          it->second->variablized_symbol->to_string(), it->second->instantiated_symbol->to_string(), it->second->grounded);
    }
  }
  if ((whichTable == 0) || (whichTable == 3))
  {
    dprint(mode, "----- Original Var -> G_ID Table -----\n");
    if (whichTable != 0)
      dprint(mode, "------------------------------------\n");
    for (std::map< Symbol *, uint64_t >::iterator it=(*ovar_to_g_id_map).begin(); it!=(*ovar_to_g_id_map).end(); ++it)
    {
      dprint(mode, "%s -> %llu\n", it->first->to_string(), it->second);
    }
  }
  dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_tables()
{
  print_variablization_tables(DT_VARIABLIZATION_MANAGER);
}
