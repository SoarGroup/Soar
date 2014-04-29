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
#include "print.h"
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
  orig_var_to_g_id_map = new std::map< Symbol *, uint64_t >();
  sti_constraints = new std::map< Symbol * , ::list * >();
  constant_constraints = new std::map< uint64_t , ::list * >();

  cond_merge_map = new std::map< Symbol *, std::map< Symbol *, ::list *> >();
  ground_id_counter = 0;
}

Variablization_Manager::~Variablization_Manager()
{
  clear_data();
  delete sym_to_var_map;
  delete g_id_to_var_map;
  delete orig_var_to_g_id_map;
  delete sti_constraints;
  delete constant_constraints;

  delete cond_merge_map;
}

void Variablization_Manager::clear_data()
{
  dprint(DT_VARIABLIZATION_MANAGER, "Clearing variablization maps.\n");
  clear_relational_constraints ();
  clear_ovar_gid_table();
  clear_variablization_tables();
  clear_merge_map();
}

void Variablization_Manager::reinit()
{
  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
  clear_data();
  ground_id_counter = 0;
}

void Variablization_Manager::print_ovar_gid_propogation_table(TraceMode mode, bool printHeader)
{
  if (printHeader)
  {
    dprint(mode, "------------------------------------\n");
    dprint(mode, "OrigVariable to g_id Propagation Map\n");
    dprint(mode, "------------------------------------\n");
  }
  for (std::map< Symbol *, uint64_t >::iterator it=(*orig_var_to_g_id_map).begin(); it!=(*orig_var_to_g_id_map).end(); ++it)
  {
    dprint(mode, "%s -> %llu\n", it->first->to_string(), it->second);
  }

}

void Variablization_Manager::print_relational_constraints (TraceMode mode)
{
  dprint(mode, "------------------------------------\n");
  dprint(mode, "            Constraint Map\n");
  dprint(mode, "------------------------------------\n");

  cons *c;

  for (std::map< Symbol *, ::list * >::iterator it=sti_constraints->begin(); it!=sti_constraints->end(); ++it)
  {
    c = it->second;
    while (c) {
      dprint(mode, "%s: ", it->first->to_string());
      dprint_test(mode, static_cast<test>(c->first), true, false, true, " ", "\n");
      c = c->rest;
    }
  }
  dprint(mode, "------------------------------------\n");
  for (std::map< uint64_t, ::list * >::iterator it=constant_constraints->begin(); it!=constant_constraints->end(); ++it)
  {
    c = it->second;
    while (c) {
      dprint(mode, "%llu: ", it->first);
      dprint_test(mode, static_cast<test>(c->first), true, false, true, " ", "\n");
      c = c->rest;
    }
  }
  dprint(mode, "------------------------------------\n");
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
    dprint(mode, "------ Symbol -> v_info table -----\n");
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
    dprint(mode, "-------- G_ID -> v_info table ------\n");
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
    dprint(mode, "---- Original Var -> G_ID Table ----\n");
    if (whichTable != 0)
      dprint(mode, "------------------------------------\n");
    print_ovar_gid_propogation_table(mode);
  }
  dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::print_tables()
{
  print_variablization_tables(DT_VARIABLIZATION_MANAGER);
}

void Variablization_Manager::clear_ovar_gid_table()
{
  dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar g_id table...\n");
  /* -- Clear original variable map -- */
  for (std::map< Symbol *, uint64_t >::iterator it=(*orig_var_to_g_id_map).begin(); it!=(*orig_var_to_g_id_map).end(); ++it)
  {
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing %s -> %llu\n", it->first->to_string(), it->second);
    symbol_remove_ref(thisAgent, it->first);
  }
  orig_var_to_g_id_map->clear();
}

void Variablization_Manager::clear_variablization_tables()
{

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
  if (index_id == 0) return NULL;

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

variablization * Variablization_Manager::get_variablization_for_symbol(std::map< Symbol *, variablization * > *pMap, Symbol *index_sym)
{
  std::map< Symbol *, variablization * >::iterator iter = (*pMap).find(index_sym);
  if (iter != (*pMap).end())
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
variablization * Variablization_Manager::get_variablization(Symbol *index_sym)
{
  return get_variablization_for_symbol(sym_to_var_map, index_sym);
}

variablization * Variablization_Manager::get_variablization(test t)
{
  /* -- MToDo | I don't think this is used any more. Check and remove -- */
  assert(t->data.referent);
  if (t->data.referent->is_sti())
  {
    return get_variablization(t->data.referent);
  }
  else
  {
    return get_variablization(t->identity->grounding_id);
  }
}

uint64_t Variablization_Manager::get_gid_for_orig_var(Symbol *index_sym)
{
  std::map< Symbol *, uint64_t >::iterator iter = (*orig_var_to_g_id_map).find(index_sym);
  if (iter != (*orig_var_to_g_id_map).end())
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...found %llu in orig_var variablization table for %s\n",
        iter->second, index_sym);

    return iter->second;
  }
  else
  {
    dprint(DT_VARIABLIZATION_MANAGER, "...did not find %s in orig_var variablization table.\n", index_sym->to_string());
    print_ovar_gid_propogation_table(DT_VARIABLIZATION_MANAGER);
  }

  return 0;
}

uint64_t Variablization_Manager::add_orig_var_to_gid_mapping(Symbol *index_sym, uint64_t index_g_id)
{
  std::map< Symbol *, uint64_t >::iterator iter = (*orig_var_to_g_id_map).find(index_sym);
  if (iter == (*orig_var_to_g_id_map).end())
  {
    dprint(DT_OVAR_MAPPINGS, "Adding original variable mappings entry: %s -> %llu\n", index_sym->to_string(), index_g_id);
    (*orig_var_to_g_id_map)[index_sym] = index_g_id;
    symbol_add_ref(thisAgent, index_sym);
    return 0;
  } else {
    dprint(DT_OVAR_MAPPINGS,
        "...%llu already exists in orig_var variablization table for %s.  add_orig_var_to_gid_mapping returning false.\n",
        iter->second, index_sym->to_string());
  }
  return iter->second;
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
      dprint(DT_VARIABLIZATION_MANAGER, "...searching for original var %s in variablization orig var table...\n", original_var->to_string());
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

/* =====================================================================

                  Variablizing Conditions and Results

   Variablizing of conditions is done by walking over a condition list
   and destructively modifying it, replacing tests with tests of variables.
   The identifier-to-variable mapping is built as we go along:  identifiers
   that have already been assigned a variablization are marked with
   common.tc_num==variablization_tc, and id.variablization points to the
   corresponding variable.

   Variablizing of results can't be done destructively because we need
   to convert the results--preferences--into actions.  This is done
   by copy_and_variablize_result_list(), which takes the result preferences
   and returns an action list.

   Note:  The caller to this function is now responsible for making sure
   the symbol passed in should be variablized.

===================================================================== */

void Variablization_Manager::variablize_test (test *t)
{
  cons *c, *c_orig;
  test ct, ct_original, *original_test, original_eq_test;
  TestType original_test_type, test_type;
  Symbol *original_referent, *instantiated_referent;

  original_test = &((*t)->original_test);

  dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "", "\n");

  assert(*t);
  test_type = (*t)->type;

  if (test_is_blank(*original_test))
  {
    if (test_type == CONJUNCTIVE_TEST)
    {
      dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
      ct = *t;
      for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      {
        dprint(DT_LHS_VARIABLIZATION, "Variablizing conjunctive test: ");
        variablize_test (reinterpret_cast<test *>(&(c->first)));
      }

      dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
      dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    } else
    {
      dprint(DT_LHS_VARIABLIZATION, "...ignoring non-conjunctive test because original is blank (should not be possible!):\n");
      dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "          ", "\n");
      return;
    }
  } else
  {
    /* -- ORIGINAL can differ from CHUNK tests if there are goal, impasse or disjunction tests in ORIGINAL test. Goal/impasse
     *    tests added in a separate step.  Disjunctions don't need to be variablized.  Original can also differ if there
     *    are multiple equality tests in the original.  The instantiated test will only have one equality test for the symbol
     *    matched.
     *
     *    MToDo:  Is the part about disjunctions still true?  I think the logic was that original disjunctions are a list of constants and will
     *    not be needed to variablize, so no need to copy?*/

    original_test_type = (*original_test)->type;

    assert(original_test_type);
    switch (original_test_type) {
      case DISJUNCTION_TEST:
        break;
      case EQUALITY_TEST:
      case NOT_EQUAL_TEST:
      case LESS_TEST:
      case GREATER_TEST:
      case LESS_OR_EQUAL_TEST:
      case GREATER_OR_EQUAL_TEST:
      case SAME_TYPE_TEST:
      case CONJUNCTIVE_TEST:
        if (original_test_type == CONJUNCTIVE_TEST)
        {
          dprint(DT_LHS_VARIABLIZATION, "Equality test with conjunctive set of original equalities.\n");
          // Only equality tests can have multiple originals and hence a conjunction
          assert(test_type==EQUALITY_TEST);
          original_eq_test = find_original_equality_test_preferring_vars(thisAgent, (*original_test), false);
          if (!original_eq_test) return;
          original_referent = original_eq_test->data.referent;
        } else {
          original_referent = (*original_test)->data.referent;
        }
        instantiated_referent = (*t)->data.referent;
          assert (instantiated_referent && original_referent);

          if (instantiated_referent->is_variablizable(original_referent))
          {
            dprint(DT_LHS_VARIABLIZATION, "...variablizing test type %s with referent %s\n", test_type_to_string(test_type), instantiated_referent->to_string());
            thisAgent->variablizationManager->variablize_lhs_symbol (&((*t)->data.referent), original_referent,
                (*t)->identity, (original_test_type == EQUALITY_TEST));
          } else {
            dprint(DT_LHS_VARIABLIZATION, "...non-variablizable referent %s.  Original: %s.\n", instantiated_referent->to_string(), original_referent->to_string());
            //assert(false);
          }
        break;
      default:
        dprint(DT_DEBUG, "...invalid test type in variablize_test!!!\n");
        assert(false);
        break;
    }
  }
  dprint(DT_LHS_VARIABLIZATION, "Result: ");
  dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "", "\n");
  dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
}

/* This gets passed in a copy of the chunk instantiation's condition lists, which
 * will get thrown away
 */

void Variablization_Manager::variablize_condition_list (condition *top_cond)
{
  dprint(DT_LHS_VARIABLIZATION, "==========================================\n");
  dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS condition list:\n");
  dprint(DT_LHS_VARIABLIZATION, "==========================================\n");

  //thisAgent->varname_table->clear_symbol_map();

  dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing positive conditions...\n");
  for (condition *cond = top_cond; cond!=NIL; cond=cond->next)
  {
    if (cond->type == POSITIVE_CONDITION)
    {
      dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
      dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS positive condition: ");
      dprint_condition(DT_LHS_VARIABLIZATION, cond, "", true, false, true);
      dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
      dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
      variablize_test (&(cond->data.tests.id_test));
      dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
      variablize_test (&(cond->data.tests.attr_test));
      dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
      variablize_test (&(cond->data.tests.value_test));
    }
  }
  dprint(DT_LHS_VARIABLIZATION, "Pass 2: Variablizing negative conditions and negative conjunctive conditions...\n");
  for (condition *cond = top_cond; cond!=NIL; cond=cond->next)
  {
    if (cond->type == NEGATIVE_CONDITION)
    {
      dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
      dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS negative condition: ");
      dprint_condition(DT_LHS_VARIABLIZATION, cond, "", true, false, true);
      dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
      dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
      variablize_test (&(cond->data.tests.id_test));
      dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
      variablize_test (&(cond->data.tests.attr_test));
      dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
      variablize_test (&(cond->data.tests.value_test));
    } else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
      dprint(DT_LHS_VARIABLIZATION, "-------------======-----------\n");
      dprint(DT_NCC_VARIABLIZATION, "Variablizing LHS NCC condition:\n");
      dprint_condition_list(DT_NCC_VARIABLIZATION, cond->data.ncc.top);
      variablize_condition_list (cond->data.ncc.top);
    }
  }
  dprint(DT_LHS_VARIABLIZATION, "Done variablizing LHS condition list.\n");
  dprint(DT_LHS_VARIABLIZATION, "==========================================\n");
}

void Variablization_Manager::variablize_relational_constraints_for_symbol(::list **constraint_list)
{
  variablization *found_variablization = NULL;
  cons *c, *c_next, *c_last;
  test t;

  c = (*constraint_list);
  c_last = NULL;
  while (c)
  {
    c_next = c->rest;
    t = static_cast<test>(c->first);
    // Disjunctions don't have referents and aren't variablized
    if (t->type != DISJUNCTION_TEST)
    {
      found_variablization = NULL;
      found_variablization = get_variablization(t);
      /* -- Three cases for referent in relational constraint:
       *    (1) It has been variablized before, so just variablize.
       *    (2) It has not been variablized before and is a STI, so it's an ungrounded comparison.  So,
       *        delete from list.
       *    (3) It has not been variablized before and is a constant.  Ignore.  Will become a relational
       *        test against a literal constant in final chunk.
       * -- */
      if (found_variablization)
      {
        /* -- Grounded symbol.  Variablize. -- */
        variablize_test (&(t));
      } else if (t->data.referent->is_sti())
      {
        /* -- STI identifier that is ungrounded.  Delete. -- */
        dprint(DT_CONSTRAINTS, "Deleting constraint b/c STI not in in chunk.\n");
        if (c_last)
        {
          /* -- Not at the head of the list -- */
          c_last->rest = c->rest;
          free_cons(thisAgent, c);
          c = c_last;
        } else {
          /* -- At the head of the list -- */
          (*constraint_list) = c->rest;
          free_cons(thisAgent, c);
          /* -- This will cause c_last to be set to NULL, indicating we're
           *    at the head of the list -- */
          c = NULL;
        }
        deallocate_test(thisAgent, t);
      } else
      {
        /* -- Constant referent that is ungrounded.  Ignore. -- */
        dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c referent not grounded in chunk.\n");
      }
    }
    c_last = c;
    c = c_next;
  }
}

void Variablization_Manager::variablize_relational_constraints()
{
  variablization *found_variablization = NULL;
  ::list *c;
  std::map< Symbol *, ::list * > *variablized_sti_constraints = new std::map< Symbol *, ::list * >;
  std::map< uint64_t, ::list * > *variablized_constant_constraints = new std::map< uint64_t, ::list * >;

  dprint(DT_CONSTRAINTS, "=============================================\n");
  dprint(DT_CONSTRAINTS, "Variablizing relational constraints.\n");
  dprint(DT_CONSTRAINTS, "(1) Variablizing relational constraints for short-term identifiers.\n");
  dprint_wmes(DT_CONSTRAINTS, true);

  for (std::map< Symbol *, ::list * >::iterator it=sti_constraints->begin(); it!=sti_constraints->end(); ++it)
  {

    dprint(DT_CONSTRAINTS, "Looking for variablization for equality symbol %s.\n", it->first->to_string());
    found_variablization = get_variablization(it->first);

    if (found_variablization)
    {
      // Should always be grounded now that relationals are done on their own
      assert(found_variablization->grounded);

      dprint(DT_CONSTRAINTS, "...found grounding.  Variablizing constraint list.\n", it->first->to_string());
      variablize_relational_constraints_for_symbol(&(it->second));

      /* -- If at least one relational constraint remains in the list, add to variablized constraint
       *    list, using the variablized equality symbol -- */
      if (it->second)
      {
        dprint(DT_CONSTRAINTS, "...variablized constraints exist.  Copying to new constraint list.\n");
        (*variablized_sti_constraints)[found_variablization->variablized_symbol] = it->second;
      }
    } else {
      /* -- Delete entire constraint list for ungrounded identifier -- */
      dprint(DT_CONSTRAINTS, "...not variablizing constraint list b/c equality symbol not in chunk.  Deallocating tests.\n");
      c = it->second;
      while (c)
      {
        dprint(DT_CONSTRAINTS, "...deallocating test %s\n", test_to_string(static_cast<test>(c->first)));
        deallocate_test(thisAgent, static_cast<test>(c->first));
        c = c->rest;
      }
      free_list (thisAgent, it->second);
      it->second = NULL;
    }
  }

  /* -- Replace sti constraints with variablized version.
   *
   *    Note:  Symbols in key did not have their refcount increased, so they don't need to be
   *    deallocated.  Moreover, any relational constraints that were not relevant to this chunk
   *    will have already had their tests deallocated above. So, we can just clear the map.
   *
   * -- */
  sti_constraints->clear();
  sti_constraints = variablized_sti_constraints;

  dprint(DT_CONSTRAINTS, "(2) Variablizing relational constraints for constant symbols.\n");
  for (std::map< uint64_t, ::list * >::iterator it=constant_constraints->begin(); it!=constant_constraints->end(); ++it)
  {

    dprint(DT_CONSTRAINTS, "Looking for variablization for equality g_id %llu.\n", it->first);
    found_variablization = get_variablization(it->first);

    if (found_variablization)
    {
      // Should always be grounded now that relationals are done on their own
      assert(found_variablization->grounded);

      dprint(DT_CONSTRAINTS, "...found grounding for grounding id %llu.  Variablizing constraint list.\n", it->first);
      variablize_relational_constraints_for_symbol(&(it->second));

      /* -- If at least one relational constraint remains in the list, add to variablized constraint
       *    list, using the variablized equality symbol -- */
      if (it->second)
      {
        dprint(DT_CONSTRAINTS, "...variablized constraints exist.  Copying to new constraint list.\n");
        (*variablized_constant_constraints)[found_variablization->grounding_id] = it->second;
      }
    } else {
      /* -- Delete entire constraint list for ungrounded identifier -- */
      dprint(DT_CONSTRAINTS, "...not variablizing constraint list b/c equality g_id not in chunk.  Deallocating tests.\n");
      c = it->second;
      while (c)
      {
        dprint(DT_CONSTRAINTS, "...deallocating test %s\n", test_to_string(static_cast<test>(c->first)));
        deallocate_test(thisAgent, static_cast<test>(c->first));
        c = c->rest;
      }
      free_list (thisAgent, it->second);
      it->second = NULL;
    }
  }

  /* -- Replace constant constraints with variablized version. -- */
  constant_constraints->clear();
  constant_constraints = variablized_constant_constraints;

  dprint(DT_CONSTRAINTS, "Done variablizing relational constraints.\n");

}

void Variablization_Manager::clear_relational_constraints ()
{
  for (std::map< Symbol *, ::list * >::iterator it=sti_constraints->begin(); it!=sti_constraints->end(); ++it)
  {
    free_list (thisAgent, it->second);
  }
  sti_constraints->clear();

  for (std::map< uint64_t, ::list * >::iterator it=constant_constraints->begin(); it!=constant_constraints->end(); ++it)
  {
    free_list (thisAgent, it->second);
  }
  constant_constraints->clear();
}

void Variablization_Manager::cache_relational_constraint (test equality_test, test relational_test)
{
  dprint(DT_CONSTRAINTS, "Adding relational constraint %s to %s.\n", test_to_string(relational_test), test_to_string(equality_test));
  ::list * new_list=NULL;
  test copied_test = copy_test(thisAgent, relational_test);

  if (equality_test->data.referent->is_sti())
  {
    std::map< Symbol *, ::list * >::iterator iter = (*sti_constraints).find(equality_test->data.referent);
    if (iter == sti_constraints->end())
    {
      push(thisAgent, (copied_test), new_list);
      (*sti_constraints)[equality_test->data.referent] = new_list;
      dprint(DT_CONSTRAINTS, "ADDED (*sti_constraints)[%s] + %s\n", equality_test->data.referent->to_string(), test_to_string(copied_test));
    }
    else
    {
      new_list = (*sti_constraints)[equality_test->data.referent];
      push(thisAgent, (copied_test), new_list);
      (*sti_constraints)[equality_test->data.referent] = new_list;
      dprint(DT_CONSTRAINTS, "ADDED (*sti_constraints)[%s] + %s\n", equality_test->data.referent->to_string(), test_to_string(copied_test));
    }
  } else {
    std::map< uint64_t, ::list * >::iterator iter = (*constant_constraints).find(equality_test->identity->grounding_id);
    if (iter == constant_constraints->end())
    {
      push(thisAgent, (copied_test), new_list);
      (*constant_constraints)[equality_test->identity->grounding_id] = new_list;
      dprint(DT_CONSTRAINTS, "ADDED (*constant_constraints)[%llu] + %s\n", equality_test->identity->grounding_id, test_to_string(copied_test));
    }
    else
    {
      new_list = (*constant_constraints)[equality_test->identity->grounding_id];
      push(thisAgent, (copied_test), new_list);
      (*constant_constraints)[equality_test->identity->grounding_id] = new_list;
      dprint(DT_CONSTRAINTS, "ADDED (*constant_constraints)[%llu] + %s\n", equality_test->identity->grounding_id, test_to_string(copied_test));
    }
  }
}

void Variablization_Manager::cache_relational_constraints_in_test (test t)
{
  /* -- Only conjunctive tests can have relational tests here.  Otherwise,
   *    should be an equality test. -- */
  if (t->type != CONJUNCTIVE_TEST)
  {
    assert(t->type == EQUALITY_TEST);
    return;
  }

  test equality_test=NULL, referent_test, ctest;
  cons *c;
  for (c=t->data.conjunct_list; c!=NIL; c=c->rest)
  {
    if (static_cast<test>(c->first)->type == EQUALITY_TEST)
    {
      equality_test = static_cast<test>(c->first);
      break;
    }
  }
  assert(equality_test);
  for (c=t->data.conjunct_list; c!=NIL; c=c->rest)
  {
    ctest = static_cast<test>(c->first);
    switch (ctest->type) {
      case EQUALITY_TEST:
        break;
      case GREATER_TEST:
      case GREATER_OR_EQUAL_TEST:
      case LESS_TEST:
      case LESS_OR_EQUAL_TEST:
      case NOT_EQUAL_TEST:
      case SAME_TYPE_TEST:
      case DISJUNCTION_TEST:
        thisAgent->variablizationManager->cache_relational_constraint(equality_test, ctest);
        break;
      default:
        break;
    }
  }
}

void Variablization_Manager::cache_relational_constraints_in_cond (condition *c)
{
  /* Don't need to do id element.  It should always be an equality test */
//  assert(!c->data.tests.id_test || (c->data.tests.id_test->type == EQUALITY_TEST));
  dprint_condition(DT_CONSTRAINTS, c, "Caching relational constraints in condition: ", true, false, true);
  cache_relational_constraints_in_test(c->data.tests.attr_test);
  cache_relational_constraints_in_test(c->data.tests.value_test);
}

void Variablization_Manager::install_relational_constraints_for_test(test *t)
{
  if (!t) return;

  cons *c;
  test eq_test, ct;
  Symbol *eq_symbol;
  variablization *found_variablization;

  eq_test = equality_test_found_in_test(thisAgent, *t);
  assert(eq_test);
  eq_symbol = eq_test->data.referent;
  dprint(DT_CONSTRAINTS, "Calling add_relational_constraints_for_test() for symbol %s(%llu).\n", eq_symbol->to_string(), eq_test->identity ? eq_test->identity->grounding_id : 0);
  if (!eq_test->identity || (eq_test->identity->grounding_id == 0))
  {
    /* MToDo | Could also just use was_identifier, though that might not be needed now that we don't reverse */
    assert(eq_symbol->is_variable() && eq_symbol->var->was_identifier);
    dprint(DT_CONSTRAINTS, "...no identity, so must be STI.  Using symbol to look up.\n");
    found_variablization = get_variablization(eq_symbol);
    if (found_variablization)
    {
      dprint(DT_CONSTRAINTS, "...variablization found.  Variablized symbol = %s.\n", found_variablization->variablized_symbol->to_string());
      print_relational_constraints(DT_CONSTRAINTS);
      std::map< Symbol *, ::list * >::iterator iter = (*sti_constraints).find(eq_symbol);
      if (iter != (*sti_constraints).end())
      {
        dprint(DT_CONSTRAINTS, "...adding relational constraint list for symbol %s...\n", eq_symbol->to_string());
        c = iter->second;
        while (c) {
          ct = static_cast<test>(c->first);
          dprint_test(DT_CONSTRAINTS, ct, true, false, true, "...adding", "\n");
          add_test(thisAgent, t, ct);
          c = c->rest;
        }
        free_list (thisAgent, iter->second);
        (*sti_constraints).erase(iter->first);
      }
      else
      {
        dprint(DT_CONSTRAINTS, "...no relational constraints found.\n");
      }
    } else {
      dprint(DT_CONSTRAINTS, "... was never variablized. Skipping...\n");
    }
  } else {
    dprint(DT_CONSTRAINTS, "...identity, so must be constant.  Using g_id to look up.\n");
    found_variablization = get_variablization(eq_test->identity->grounding_id);
    if (found_variablization)
    {
      dprint(DT_CONSTRAINTS, "...variablization found.  Variablized symbol = %s.\n", found_variablization->variablized_symbol->to_string());
      print_relational_constraints(DT_CONSTRAINTS);
      std::map< uint64_t, ::list * >::iterator iter = (*constant_constraints).find(eq_test->identity->grounding_id);
      if (iter != (*constant_constraints).end())
      {
        dprint(DT_CONSTRAINTS, "...adding relational constraint list for symbol %s...\n", eq_symbol->to_string());
        c = iter->second;
        while (c) {
          ct = static_cast<test>(c->first);
          dprint_test(DT_CONSTRAINTS, ct, true, false, true, "...adding ", "\n");
          add_test(thisAgent, t, ct);
          c = c->rest;
        }
        free_list (thisAgent, iter->second);
        (*constant_constraints).erase(iter->first);
      }
      else
      {
        dprint(DT_CONSTRAINTS, "...no relational constraints found.\n");
      }
    } else {
      dprint(DT_CONSTRAINTS, "... was never variablized. Skipping...\n");
    }
  }
}

void Variablization_Manager::install_relational_constraints(condition *cond)
{
  dprint(DT_CONSTRAINTS, "=============================================\n");
  dprint(DT_CONSTRAINTS, "install_relational_constraints called...\n");
  print_variablization_tables(DT_CONSTRAINTS);
  print_relational_constraints(DT_CONSTRAINTS);

  while (cond && ((sti_constraints->size() > 0) || (constant_constraints->size() > 0))) {
    if (cond->type == POSITIVE_CONDITION)
    {
      dprint(DT_CONSTRAINTS, "Adding for positive condition ");
      dprint_condition(DT_CONSTRAINTS, cond, "", true, false, true);
      install_relational_constraints_for_test(&cond->data.tests.attr_test);
      install_relational_constraints_for_test(&cond->data.tests.value_test);
    } else {
      dprint(DT_CONSTRAINTS, (cond->type == NEGATIVE_CONDITION) ? "Skipping for negative condition " : "Skipping for negative conjunctive condition:\n");
      dprint_condition(DT_CONSTRAINTS, cond, "", true, false, true);
    }
    cond = cond->next;
  }
  dprint(DT_CONSTRAINTS, "install_relational_constraints done adding constraints.  Final tables:\n");
  print_variablization_tables(DT_CONSTRAINTS);
  print_relational_constraints(DT_CONSTRAINTS);
  dprint_condition_list(DT_CONSTRAINTS, cond);
  dprint(DT_CONSTRAINTS, "=============================================\n");
}

void Variablization_Manager::print_merge_map (TraceMode mode)
{
  dprint(mode, "------------------------------------\n");
  dprint(mode, "            Merge Map\n");
  dprint(mode, "------------------------------------\n");

  cons *c;

  std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
  std::map< Symbol *, ::list *>::iterator iter_attr;
  std::map< Symbol *, ::list *> *attr_values;

  for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
  {
    dprint(DT_MERGE, "%s conditions: \n", iter_id->first->to_string());
    attr_values = &(iter_id->second);
    for (iter_attr = attr_values->begin(); iter_attr != attr_values->end(); ++iter_attr)
    {
      dprint_condition_cons(DT_MERGE, iter_attr->second, true, false, true, "   ");
    }
  }

  dprint(mode, "------------------------------------\n");
}

void Variablization_Manager::clear_merge_map()
{
  std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
  std::map< Symbol *, ::list *>::iterator iter_attr;
  std::map< Symbol *, ::list *> *attr_values;

  for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
  {
    attr_values = &(iter_id->second);
    for (iter_attr = attr_values->begin(); iter_attr != attr_values->end(); ++iter_attr)
    {
      free_list (thisAgent, iter_attr->second);
    }
    attr_values->clear();
  }
  cond_merge_map->clear();
}

void Variablization_Manager::merge_values_in_conds(condition *pDestCond, condition *pSrcCond)
{
    add_non_identical_tests(thisAgent, &(pDestCond->data.tests.value_test), pSrcCond->data.tests.value_test);
}

void Variablization_Manager::set_cond_for_id_attr_tests(condition *pCond)
{
  std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
  std::map< Symbol *, ::list *>::iterator iter_attr;
  ::list * new_list=NULL;

  dprint_condition(DT_MERGE, pCond, "Savind cond in merge map: ", true, false, true);
  test id_test = equality_test_found_in_test(thisAgent, pCond->data.tests.id_test);
  test attr_test = equality_test_found_in_test(thisAgent, pCond->data.tests.attr_test);
  test val_test = equality_test_found_in_test(thisAgent, pCond->data.tests.value_test);
  dprint(DT_MERGE, "...found equality tests (%s ^%s %s)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string(), val_test->data.referent->to_string());
  iter_id = cond_merge_map->find(id_test->data.referent);
  if (iter_id == cond_merge_map->end())
  {
    dprint(DT_MERGE, "...id test not found.  Creating new entry.\n");
    /* Add new attr->value-cons-list map */
    push(thisAgent, pCond, new_list);
    std::map<Symbol *, ::list *> inner;
    inner.insert(std::make_pair(attr_test->data.referent, new_list));
    cond_merge_map->insert(std::make_pair(id_test->data.referent, inner));
    dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s] -> new_list (1 entry)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string());
  } else {
    dprint(DT_MERGE, "...id test found.  Looking for attribute test...\n");
    iter_attr = iter_id->second.find(attr_test->data.referent);
    if (iter_attr == iter_id->second.end())
    {
      dprint(DT_MERGE, "...attr test not found.  Creating new entry.\n");
      push(thisAgent, pCond, new_list);
      (*cond_merge_map)[id_test->data.referent][attr_test->data.referent] = new_list;
      dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s] -> new_list (1 entry)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string());
    } else {
      dprint(DT_MERGE, "...attr test found.  Creating new entry.\n");
      new_list = (*cond_merge_map)[id_test->data.referent][attr_test->data.referent];
      push(thisAgent, pCond, new_list);
      (*cond_merge_map)[id_test->data.referent][attr_test->data.referent] = new_list;
      dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s] -> new_list (+ new entry)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string());
    }
  }
}

condition *Variablization_Manager::get_previously_seen_cond(condition *pCond)
{
  std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
  std::map< Symbol *, ::list *>::iterator iter_attr;

//  dprint_condition(DT_MERGE, pCond, "get_previously_seen_cond() called with: ", true, false, true);
  test id_test = equality_test_found_in_test(thisAgent, pCond->data.tests.id_test);
  test attr_test = equality_test_found_in_test(thisAgent, pCond->data.tests.attr_test);
  test val_test = equality_test_found_in_test(thisAgent, pCond->data.tests.value_test);

  dprint(DT_MERGE, "...looking for id equality test %s\n", test_to_string(id_test));
  iter_id = cond_merge_map->find(id_test->data.referent);
  if (iter_id != cond_merge_map->end())
  {
    dprint(DT_MERGE, "...Found entry for %s.  Looking for attr equality test %s\n", static_cast<Symbol *>(iter_id->first)->to_string(), test_to_string(attr_test));
    iter_attr = iter_id->second.find(attr_test->data.referent);
    if (iter_attr != iter_id->second.end())
    {
      dprint(DT_MERGE, "...Found.  Looking in cons list for value equality test %s\n", test_to_string(val_test));
      /* Iterate through cons list and look for matching equality value with the same identity or identifier */
      cons *c;
      condition *lCond;
      test lEqTest;
      c = iter_attr->second;
      while (c)
      {
        lCond = static_cast<condition *>(c->first);
        lEqTest = equality_test_found_in_test(thisAgent, lCond->data.tests.value_test);
        if (lEqTest->data.referent->is_sti())
        {
          dprint(DT_MERGE, "...comparing with sti %s\n", lEqTest->data.referent);
          if (lEqTest->data.referent == val_test->data.referent)
          {
            dprint_condition(DT_MERGE, lCond, "...returning TRUE with condition: ", true, false, true);
            return lCond;
          }
        } else if (lEqTest->identity->grounding_id > 0) {
          dprint(DT_MERGE, "...comparing with constant %s\n", lEqTest->data.referent->to_string());
          /* MToDo | Only equality tests on non-literals should be here.  Need to add something to make sure that's true! */
          if (lEqTest->identity->grounding_id == val_test->identity->grounding_id)
          {
            if (lEqTest->identity->original_var == val_test->identity->original_var)
            {
              dprint_condition(DT_MERGE, lCond, "...orig vars and g_id match.  returning TRUE with condition: ", true, false, true);
              return lCond;
            } else {
              dprint(DT_MERGE, "...Not merging.  Different original vars: %s != %s\n", val_test->identity->original_var->to_string(), lEqTest->identity->original_var->to_string());
            }
          } else {
            dprint(DT_MERGE, "...Not merging.  Different g_ids: %llu != %llu\n", val_test->identity->grounding_id, lEqTest->identity->grounding_id);
          }
        } else {
          dprint(DT_MERGE, "...no grounding id for constant %s!  Should not happen.\n", lEqTest->data.referent);
        }
        c = c->rest;
      }
    }
  }

  dprint(DT_MERGE, "...returning FALSE\n");
  return NULL;
}

void Variablization_Manager::merge_conditions(condition **top_cond)
{
  /* -- This function merges redundant conditions in a condition list by
   *    combining constraints of conditions that share identical equality tests
   *    for all three elements of the condition.
   *
   *    - Iterate through conditions
   *        - Check if value exists in map
   *          - If so,
   *            - add test to original cond value if it doesn't exist (have asserts about extra info not being thrown away)
   *            - delete condition
   *          - If not,
   *            - add cond to map
   * -- */

  /* MToDo | Will probably need to do this for attributes with the same value.  Would double cost but hardly be used I would
   *         think. */

  condition *cond, *found_cond, *last_cond, *next_cond;
  cond = (*top_cond);
  last_cond = NULL;

  dprint(DT_MERGE, "======================\n");
  dprint(DT_MERGE, "= Merging Conditions =\n");
  dprint(DT_MERGE, "======================\n");
  dprint_condition_list(DT_MERGE, *top_cond, "", true, false, true);
  dprint(DT_MERGE, "======================\n");
  while (cond)
  {
    dprint_condition(DT_MERGE, cond, "Merging constraint: ", true, false, true);
    next_cond = cond->next;
    if (cond->type==POSITIVE_CONDITION) {
      /* -- Check if there already exists a condition with the same id and
       *    attribute equality tests -- */
      dprint(DT_MERGE, "...looking for previously seen similar condition...\n");
      found_cond = get_previously_seen_cond(cond);

      if (found_cond)
      {
        dprint(DT_MERGE, "...found condition to merge into.  Merging conditions...\n");
        /* -- Add tests in this condition to the already seen condition -- */
        merge_values_in_conds(found_cond, cond);

        /* -- Delete the redundant condition -- */
        if (last_cond)
        {
          /* -- Not at the head of the list -- */
          dprint(DT_MERGE, "...deleting non-head item.\n");
          last_cond->next = cond->next;
          deallocate_condition(thisAgent, cond);
          if (last_cond->next)
            last_cond->next->prev = last_cond;
          cond = last_cond;
        } else {
          /* -- At the head of the list -- */
          dprint(DT_MERGE, "...deleting head of list.\n");
          (*top_cond) = cond->next;
          deallocate_condition(thisAgent, cond);
          if ((*top_cond)->next)
            (*top_cond)->next->prev = (*top_cond);
          /* -- This will cause last_cond to be set to  NULL, indicating we're
           *    at the head of the list -- */
          cond = NULL;
        }
      } else {
        /* -- First condition seen with given id/attr tests.  So just add to
         *    map so that future similar conditions can add to it. -- */
        dprint(DT_MERGE, "...did not find condition that matched.  Creating entry in merge map.\n");
        set_cond_for_id_attr_tests(cond);
      }
    }
    last_cond = cond;
    cond = next_cond;
    dprint(DT_MERGE, "...done merging this constraint.\n");
  }
  dprint(DT_MERGE, "======================\n");
  dprint_condition_list(DT_MERGE, *top_cond, "", true, false, true);
  dprint(DT_MERGE, "===========================\n");
  dprint(DT_MERGE, "= Done Merging Conditions =\n");
  dprint(DT_MERGE, "===========================\n");
}


