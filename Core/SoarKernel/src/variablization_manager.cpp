/*
 * variablization_manager.cpp
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
#include "rhs.h"

Variablization_Manager::Variablization_Manager(agent *myAgent)
{
    thisAgent = myAgent;
    sym_to_var_map = new std::map< Symbol *, variablization * >();
    g_id_to_var_map = new std::map< uint64_t, variablization * >();
    orig_var_to_g_id_map = new std::map< Symbol *, uint64_t >();
    sti_constraints = new std::map< Symbol * , ::list * >();
    constant_constraints = new std::map< uint64_t , ::list * >();

    cond_merge_map = new std::map< Symbol *, std::map< Symbol *, std::map< Symbol *, condition *> > >();
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

void Variablization_Manager::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();
    ground_id_counter = 0;
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

/* ============================================================================
 *            Variablization_Manager::variablize_lhs_symbol
 *
 * Requires: Test must not be a conjunctive test.
 * Modifies: sym, variablization maps
 * Effect:   Replaces symbol with a variable.  Creates new variable if
 *           necessary.
 * Note:     Caller is responsible for determining whether this symbol should
 *           be variablized.  For example, we check the original symbol
 *           in the production to determine whether it is a literal and should
 *           not be variablized.
 *
 * ========================================================================= */
void Variablization_Manager::variablize_lhs_symbol (Symbol **sym, identity_info *identity, bool is_equality_test)
{
    char prefix[2];
    Symbol *var;
    variablization *var_info;
    bool is_st_id = (*sym)->is_sti();

    dprint(DT_VARIABLIZATION_MANAGER, "Variablizing %s(%llu, %s) %s.\n",
           (*sym)->to_string(),
           (identity ? identity->grounding_id : 0),
           (is_equality_test ? "T" : "F"));

    if (!is_st_id)
    {
        /* MToDo | Identity currently exists for all tests.  This isn't necessary until we change that */
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
 *      The logic for variablizing the rhs is slightly different than the lhs since constants on the
 *      rhs do not yet have grounding id's.  We match variables bound to constants to lhs variables by
 *      looking up the grounding id using the original variable names instead of using the grounding id.
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
//                this->print_variablization_tables(DT_VARIABLIZATION_MANAGER, 2);
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
            /* MToDo | Why don't we need remove this symbol reference? */
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
             *
             *    Delete the symbol references for both entries in the variablization table
             *    then delete the entries themselves.  This will pass through this case and
             *    create an unbound var in next code block.*/
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
     *    Both cases return 0.  Grounding id will be generated if requested by another match. -- */

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
        /* -- RHS constant with an original variable that does not map onto a LHS condition.  Do not variablize. -- */
        /* MToDo | Remove.  Is this even possible?  Won't this be caught by not having an original var above? */
        dprint(DT_VARIABLIZATION_MANAGER, "...is a variable that did not appear in the LHS.  Not variablizing!\n");
    }
    return 0;
}

/* ============================================================================
 *            Variablization_Manager::variablize_test
 *
 * Requires: Test from positive condition.
 *           Test must not be a conjunctive test.
 * Modifies: t
 * Effect:   If referent is variablizable, replaces referent symbol with a
 *           variable by calling variablize_lhs_symbol
 *
 * ========================================================================= */
void Variablization_Manager::variablize_test(test *t, Symbol *original_referent)
{
    Symbol *instantiated_referent;

    assert(t && (*t));

    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "Variablizing test ", "\n");

    instantiated_referent = (*t)->data.referent;
    assert (instantiated_referent && original_referent);

    if (instantiated_referent->is_variablizable(original_referent))
    {
        dprint(DT_LHS_VARIABLIZATION, "...variablizing test type %s with referent %s\n", test_type_to_string((*t)->type), instantiated_referent->to_string());
        variablize_lhs_symbol (&((*t)->data.referent), (*t)->identity, true);
    } else {
        dprint(DT_LHS_VARIABLIZATION, "...non-variablizable referent %s.  Original: %s.\n", instantiated_referent->to_string(), original_referent->to_string());
    }

    dprint(DT_LHS_VARIABLIZATION, "Result: ");
    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "", "\n");
    dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
}

/* ============================================================================
 *            Variablization_Manager::variablize_equality_test
 *
 * Requires: Test from positive condition.
 *           Test must not be a conjunctive test.
 * Modifies: t
 * Effect:   Variablizes all equality tests in a test.  Does not require all
 *           tests to be equality tests.  While it does not require that one
 *           test is an equality test, we have a bug somewhere else if this
 *           funtion gets passed a test that contains at least one equality
 *           test.
 *
 * ========================================================================= */
void Variablization_Manager::variablize_equality_test(test *t)
{
    test original_test, original_eq_test;
    Symbol *original_referent;

    assert(t && (*t));
    original_test = (*t)->original_test;

    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "Variablizing equality test ", "\n");

    // Sanity check on originals
    assert(original_test->type && (original_test->type < NUM_TEST_TYPES));

    if (original_test->type == EQUALITY_TEST)
    {
        original_referent = original_test->data.referent;
    }
    else if (original_test->type == CONJUNCTIVE_TEST)
    {
        /* --  A non-conjunctive test with a conjunctive original test must be an equality tests,
                   since it is the only type that can sensically have multiple originals and be a conjunction.  -- */

        /* MToDo | It may be legal, but are multiple original equality tests really sensical?  Does it
                 *         ever make sense to put that in a rule?  Can they ever become conjunctive at runtime. */

        dprint(DT_LHS_VARIABLIZATION, "Equality test with conjunctive set of original equalities.\n");
        assert((*t)->type==EQUALITY_TEST);
        original_eq_test = find_original_equality_test_preferring_vars(thisAgent, original_test, false);
        if (!original_eq_test) return;
        original_referent = original_eq_test->data.referent;
    }
    else /* -- Not an equality test -- */
    {
        return;
    }
    variablize_test(t, original_referent);
}

/* ============================================================================
 *            Variablization_Manager::variablize_equality_tests
 *
 * Requires: Test from positive condition.
 * Modifies: t
 * Effect:   Variablizes all equality tests in t, even if t is a conjunctive
 *           test.
 *
 * ========================================================================= */

void Variablization_Manager::variablize_equality_tests (test *t)
{
    cons *c;
    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "Variablizing equality tests in: ", "\n");
    assert(*t);

    if ((*t)->type == CONJUNCTIVE_TEST)
    {
        // Original test should always be null for a conjunctive tests.
        // MToDo | Previous logic wouldn't assert false but print an error out instead.  Remove.

        assert ((*t)->original_test == NULL);
        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
        for (c=(*t)->data.conjunct_list; c!=NIL; c=c->rest)
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing conjunctive test: ");
            variablize_equality_test(reinterpret_cast<test *>(&(c->first)));
        }

        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        variablize_equality_test(t);
    }
}

/* ============================================================================
 *            Variablization_Manager::variablize_test_by_lookup
 *
 * Requires: Nothing
 * Modifies: t
 * Effect:   Variablizes any symbols in a test that were previously variablized
 *          when variablizing the equality test.  Returns false if the the
 *          tests contains an ungrounded STI.  This is used to throw out
 *          ungrounded STI test that may have been collected during backtracing.
 *
 * ========================================================================= */
bool Variablization_Manager::variablize_test_by_lookup(test *t, bool pSkipTopLevelEqualities)
{
    variablization *found_variablization = NULL;

    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "Variablizing by lookup ", "\n");

    // Sanity check
    assert((*t)->original_test->type && ((*t)->original_test->type < NUM_TEST_TYPES));

    if (pSkipTopLevelEqualities && ((*t)->type == EQUALITY_TEST))
    {
        /* -- Wrong test type for this variablization pass -- */
        dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c equality test in second variablization pass.\n");
        return true;
    }
    if ((*t)->original_test->type == DISJUNCTION_TEST) return true;

    found_variablization = get_variablization(*t);
    if (found_variablization)
    {
        // It has been variablized before, so just variablize
        symbol_remove_ref (thisAgent, (*t)->data.referent);
        (*t)->data.referent = found_variablization->variablized_symbol;
        symbol_add_ref(thisAgent, found_variablization->variablized_symbol);
    }
    else
    {
        if ((*t)->data.referent->is_sti())
        {
            /* -- STI identifier that is ungrounded.  Error.  -- */
            dprint(DT_DEBUG, "Ungrounded STI in in chunk.  Will delete during merge.\n");
            return false;
        }
        else
        {
            /* -- Constant referent that is ungrounded.  Ignore. -- */
            dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c referent not grounded in chunk.\n");
        }
    }

    dprint(DT_LHS_VARIABLIZATION, "Result: ");
    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "", "\n");
    dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    return true;

}

void Variablization_Manager::variablize_tests_by_lookup(test *t, bool pSkipTopLevelEqualities)
{

    cons *c;
    bool isGrounded;
    dprint_test(DT_LHS_VARIABLIZATION, *t, true, false, true, "Variablizing by lookup tests in: ", "\n");

    assert(*t);

    if ((*t)->type == CONJUNCTIVE_TEST)
    {
        // previous logic wouldn't assert false but print an error out instead.`
        assert ((*t)->original_test == NULL);
        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
        for (c=(*t)->data.conjunct_list; c!=NIL; c=c->rest)
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing conjunctive test: \n");
            /* -- Note that we ignore what variablize_test_by_lookup returns b/c merge will later delete
             *    any ungrounded tests on STI's.  Any ungrounded tests on non-STIs do not need to be
             *    deleted.  We just leave them as a literal. We only use the return value of
             *    variablize_test_by_lookup when variablizing constraints collected during
             *    backtracing, since we can just avoid adding them to the condition list. -- */
            variablize_test_by_lookup (reinterpret_cast<test *>(&(c->first)), pSkipTopLevelEqualities);
        }

        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        variablize_test_by_lookup(t, pSkipTopLevelEqualities);
    }
}

/* MToDo | Check what was meant by...  This gets passed in a copy of the chunk instantiation's condition lists, which
 * will get thrown away */

void Variablization_Manager::variablize_condition_list (condition *top_cond, bool pInNegativeCondition)
{
    dprint(DT_LHS_VARIABLIZATION, "==========================================\n");
    dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS condition list:\n");
    dprint(DT_LHS_VARIABLIZATION, "==========================================\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition *cond = top_cond; cond!=NIL; cond=cond->next)
        {
            if (cond->type == POSITIVE_CONDITION)
            {
                dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
                dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS positive condition equality tests: ");
                dprint_condition(DT_LHS_VARIABLIZATION, cond, "", true, false, true);
                dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
                dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
                variablize_equality_tests (&(cond->data.tests.id_test));
                dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
                variablize_equality_tests (&(cond->data.tests.attr_test));
                dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
                variablize_equality_tests (&(cond->data.tests.value_test));
            }
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition *cond = top_cond; cond!=NIL; cond=cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS positive non-equality tests: ");
            dprint_condition(DT_LHS_VARIABLIZATION, cond, "", true, false, true);
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
            variablize_tests_by_lookup (&(cond->data.tests.id_test), !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
            variablize_tests_by_lookup (&(cond->data.tests.attr_test), !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
            variablize_tests_by_lookup (&(cond->data.tests.value_test), !pInNegativeCondition);
        } else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS negative condition: ");
            dprint_condition(DT_LHS_VARIABLIZATION, cond, "", true, false, true);
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
            variablize_tests_by_lookup (&(cond->data.tests.id_test), false);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
            variablize_tests_by_lookup (&(cond->data.tests.attr_test), false);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
            variablize_tests_by_lookup (&(cond->data.tests.value_test), false);
        } else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint(DT_LHS_VARIABLIZATION, "-------------======-----------\n");
            dprint(DT_NCC_VARIABLIZATION, "Variablizing LHS negative conjunctive condition:\n");
            dprint_condition_list(DT_NCC_VARIABLIZATION, cond->data.ncc.top);
            variablize_condition_list (cond->data.ncc.top, false);
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Done variablizing LHS condition list.\n");
    dprint(DT_LHS_VARIABLIZATION, "==========================================\n");
}


/* =====================================================================

          variablize_rl_symbol and variablize_rl_condition_list

   Variablizing of conditions is done by walking over a condition list
   and destructively modifying it, replacing tests of identifiers with
   tests of tests of variables.

   These functions are analogous to the ones in chunk.cpp but without
   all of the generalized variablization logic introduced in Soar 9.4
===================================================================== */

void Variablization_Manager::variablize_rl_test (agent* thisAgent, test *chunk_test) {
  cons *c;
  test ct;
  TestType test_type;
  Symbol *instantiated_referent;

  dprint(DT_RL_VARIABLIZATION, "Variablizing rl test: ");
  dprint_test(DT_RL_VARIABLIZATION, *chunk_test, true, false, true, "", "\n");

  assert(*chunk_test);
  test_type = (*chunk_test)->type;

  switch (test_type) {
    case EQUALITY_TEST:
    case NOT_EQUAL_TEST:
      ct = *chunk_test;
      instantiated_referent = (*chunk_test)->data.referent;
      assert (instantiated_referent);

      if (instantiated_referent->is_sti())
      {
        dprint(DT_RL_VARIABLIZATION, "Variablizing test type %s with referent %s\n", test_type_to_string(test_type), instantiated_referent->to_string());
        thisAgent->variablizationManager->variablize_rl_symbol (&(ct->data.referent), (test_type == EQUALITY_TEST));
      } else
      {
        dprint(DT_RL_VARIABLIZATION, "Non-variablizable original referent.\n");
      }
      break;
    case CONJUNCTIVE_TEST:
      dprint(DT_RL_VARIABLIZATION, "Iterating through conjunction list.\n");
      ct = *chunk_test;
      for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      {
        variablize_rl_test (thisAgent, reinterpret_cast<test *>(&(c->first)));
      }

      dprint(DT_RL_VARIABLIZATION, "Done iterating through conjunction list.\n");
      dprint(DT_RL_VARIABLIZATION, "---------------------------------------\n");
      break;
    case GOAL_ID_TEST:
    case IMPASSE_ID_TEST:
      break;
    default:
      dprint(DT_RL_VARIABLIZATION, "Illegal test type %s with referent %s\n",
          test_type_to_string(test_type), (*chunk_test)->data.referent->to_string());
      assert(false);
      break;
  }

  dprint(DT_RL_VARIABLIZATION, "Resulting in ");
  dprint_test(DT_RL_VARIABLIZATION, *chunk_test, true, true, false, "", "\n");
  dprint(DT_RL_VARIABLIZATION, "---------------------------------------\n");
}


// creates an action for a template instantiation
action * Variablization_Manager::make_variablized_rl_action( agent *thisAgent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym )
{
  action *rhs;
  Symbol *temp;

  rhs = make_action(thisAgent);
  rhs->type = MAKE_ACTION;

  // id
  temp = id_sym;
  /* MToDoRefCnt | May not need these 3 refcount adds b/c rhs_to_symbol did not increase refcount, but make_rhs_value_symbol does */
  /* MToDo | Might need to also add symbol as original var if that's what it is at this point */
  // symbol_add_ref(thisAgent, temp );
  variablize_rl_symbol (&temp, false);
  rhs->id = allocate_rhs_value_for_symbol(thisAgent, temp );

  // attribute
  temp = attr_sym;
  // symbol_add_ref(thisAgent, temp );
  variablize_rl_symbol (&temp, false);
  rhs->attr = allocate_rhs_value_for_symbol(thisAgent, temp );

  // value
  temp = val_sym;
  // symbol_add_ref(thisAgent, temp );
  variablize_rl_symbol (&temp, false);
  rhs->value = allocate_rhs_value_for_symbol(thisAgent, temp );

  // referent
  temp = ref_sym;
  // symbol_add_ref(thisAgent, temp );
  variablize_rl_symbol (&temp, false);
  rhs->referent = allocate_rhs_value_for_symbol(thisAgent, temp );

  return rhs;
}

void Variablization_Manager::variablize_rl_condition_list (agent* thisAgent, condition *cond) {

  dprint(DT_RL_VARIABLIZATION, "=============================================\n");
  dprint(DT_RL_VARIABLIZATION, "Variablizing LHS condition list for template:\n");
  dprint(DT_RL_VARIABLIZATION, "=============================================\n");

  //thisAgent->varname_table->clear_symbol_map();

  for (; cond!=NIL; cond=cond->next)
  {
    switch (cond->type) {
    case POSITIVE_CONDITION:
    case NEGATIVE_CONDITION:
      variablize_rl_test (thisAgent, &(cond->data.tests.id_test));
      variablize_rl_test (thisAgent, &(cond->data.tests.attr_test));
      variablize_rl_test (thisAgent, &(cond->data.tests.value_test));
      break;
    case CONJUNCTIVE_NEGATION_CONDITION:
        variablize_rl_condition_list (thisAgent, cond->data.ncc.top);
      break;
    }
  }
  dprint(DT_RL_VARIABLIZATION, "Done variablizing LHS condition list for template.\n");
  dprint(DT_RL_VARIABLIZATION, "==================================================\n");
}
