/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "prefmem.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::clear_substitution_map()
{
    substitution_map->clear();
}

test Variablization_Manager::get_substitution(Symbol* sym)
{
    std::map< Symbol*, test >::iterator iter;
    iter = substitution_map->find(sym);
    if (iter != substitution_map->end())
    {
        return (iter->second);
    }
    return NULL;
}

void Variablization_Manager::update_ovar_table_for_sub(test sacrificeSymTest, test survivorSymTest)
{
    std::map< uint64_t, uint64_t >::iterator iter;

    print_o_id_to_gid_map(DT_FIX_CONDITIONS, true);
    for (iter = o_id_to_g_id_map->begin(); iter != o_id_to_g_id_map->end(); ++iter)
    {

        if (iter->second == sacrificeSymTest->identity->grounding_id)
        {
            dprint(DT_FIX_CONDITIONS, "...found ovar->g_id mapping that needs updated: o%u = g%u -> g%u.\n", iter->first, iter->second, survivorSymTest->identity->grounding_id);
            (*o_id_to_g_id_map)[iter->first] = survivorSymTest->identity->grounding_id;
        }
    }
}

// Requires:  Two equality tests
void Variablization_Manager::set_substitution(test sacrificeSymTest, test survivorSymTest, tc_number tc_num)
{
    dprint(DT_FIX_CONDITIONS, "Storing substitution %y(g%u)->%y(g%u) (tc_num %u)...\n",
                    sacrificeSymTest->data.referent, sacrificeSymTest->identity->grounding_id,
                    survivorSymTest->data.referent, survivorSymTest->identity->grounding_id,
                    tc_num);

    /* -- If we're already supposed to substitute the survivor sym for another, we need to
          scan ovar->g_id list looking for g_ids that match previous survivor and redirect
          to new survivor -- */
    test existing_survivor = get_substitution(survivorSymTest->data.referent);
    if (existing_survivor)
    {
        dprint(DT_FIX_CONDITIONS, "...found existing survivor %y.  Fixing ovar table and deleting entry.\n", existing_survivor->data.referent);
        update_ovar_table_for_sub(existing_survivor, survivorSymTest);
        existing_survivor->data.referent->tc_num = tc_num;
        substitution_map->erase(survivorSymTest->data.referent);
    }

    (*substitution_map)[sacrificeSymTest->data.referent] = survivorSymTest;

    sacrificeSymTest->data.referent->tc_num = tc_num;
    survivorSymTest->data.referent->tc_num = 0;

    dprint(DT_FIX_CONDITIONS, "...fixing ovar table...\n");
    update_ovar_table_for_sub(sacrificeSymTest, survivorSymTest);

    // Scan substitution map looking for values that match sacrifice and redirect to survivor
    dprint(DT_FIX_CONDITIONS, "...updating existing substitutions...\n");
    std::map< Symbol*, test >::iterator iter;
    for (iter = substitution_map->begin(); iter != substitution_map->end(); ++iter)
    {
        if (iter->second->data.referent == sacrificeSymTest->data.referent)
        {
            dprint(DT_FIX_CONDITIONS, "...found substitution that needs updated: %y = %y (g%u).\n", iter->first, iter->second->data.referent, survivorSymTest->identity->grounding_id);
            iter->second = survivorSymTest;
        }
    }
}

void Variablization_Manager::remove_redundancies_and_ungroundeds(test* t, tc_number tc_num, bool ignore_ungroundeds)
{
    assert(t);

    if ((*t)->type == EQUALITY_TEST)
    {
        /* Cache main equality test.  Clearly redundant for an equality test, but simplifies code. */
        (*t)->eq_test = (*t);
        return;
    }
    else if ((*t)->type == CONJUNCTIVE_TEST)
    {
        test found_eq_test = NULL, tt, survivor, sacrifice;
        ::list* c = (*t)->data.conjunct_list;
        while (c)
        {
            tt = static_cast<test>(c->first);

            // For all tests, check if referent is STI.  If so, it's ungrounded.  Delete.
            if (test_has_referent(tt) && !ignore_ungroundeds && (tt->data.referent->is_sti()))
            {
                dprint(DT_FIX_CONDITIONS, "Ungrounded STI found: %y\n", tt->data.referent);
                c = delete_test_from_conjunct(thisAgent, t, c);
                dprint(DT_FIX_CONDITIONS, "          ...after deletion: %t [%g]\n", (*t), (*t));
            }

            // Code to detect a conjunction that contain more than one equality tests.  If found, eliminate the redundancy.
            else if (tt->type == EQUALITY_TEST)
            {
                if (found_eq_test != NULL)
                {
                    dprint(DT_FIX_CONDITIONS, "Found second equality test %y.\n", tt->data.referent);
                    /* -- Choose the symbol with the lower string value as the survivor, to be consistent -- */
                    if (strcmp(found_eq_test->data.referent->to_string(), tt->data.referent->to_string()) > 0)
                    {
                        survivor = tt;
                        sacrifice = found_eq_test;
                    }
                    else
                    {
                        survivor = found_eq_test;
                        sacrifice = tt;

                    }
//                    dprint(DT_FIX_CONDITIONS, "Surviving test = %y, sacrificed test = %y.\n", survivor->data.referent, sacrifice->data.referent);
                    dprint(DT_FIX_CONDITIONS, "Surviving test = %t [%g]", survivor, survivor);
                    dprint_noprefix(DT_FIX_CONDITIONS, ", sacrificed test = %t [%g]\n", sacrifice, sacrifice);
                    // MToDo | If there's a problem, make sure we have g_id in this variablized test.
                    set_substitution(sacrifice, survivor, tc_num);
                    c = delete_test_from_conjunct(thisAgent, t, c);
                }
                else
                {
                    dprint(DT_FIX_CONDITIONS, "Found first equality test %y.\n", tt->data.referent);
                    found_eq_test = survivor = tt;
                    c = c->rest;
                }
            }
            else
            {
                c = c->rest;
            }
        }

        /* -- We also cache the main equality test for each element in each condition
         *    since it's easy to do here. This allows us to avoid searching conjunctive
         *    tests repeatedly during merging. -- */
        (*t)->eq_test = survivor;
        survivor->eq_test = survivor;
    }
}

o_id_update_info* Variablization_Manager::get_updated_o_id_info(uint64_t old_o_id)
{
    std::map< uint64_t, o_id_update_info* >::iterator iter = (*o_id_update_map).find(old_o_id);
    if (iter != (*o_id_update_map).end())
    {
        dprint(DT_VM_MAPS, "...found o%u(%y) in o_id_update_map for o%u\n",
            iter->second->o_id, iter->second->o_var, old_o_id, get_ovar_for_o_id(old_o_id));

        return iter->second;
    } else {
        dprint(DT_VM_MAPS, "...did not find o%u(%y) in o_id_update_map.\n", old_o_id, get_ovar_for_o_id(old_o_id));
        print_o_id_update_map(DT_VM_MAPS);
    }
    return 0;
}

void Variablization_Manager::add_updated_o_id_info(uint64_t old_o_id, Symbol* new_ovar, uint64_t new_o_id)
{
    assert(get_updated_o_id_info(old_o_id) == 0);
    o_id_update_info* new_o_id_info = new o_id_update_info();
    new_o_id_info->o_var = new_ovar;
    new_o_id_info->o_id = new_o_id;
    (*o_id_update_map)[old_o_id] = new_o_id_info;
}

void Variablization_Manager::add_updated_o_id_to_g_id_mapping(uint64_t old_o_id, uint64_t new_o_id, uint64_t pG_id)
{
    std::map< uint64_t, uint64_t >::iterator iter;
    uint64_t new_g_id=0;

    /* This should never be a STI, so should always have a g_id */
    assert(pG_id);

    dprint(DT_FIX_CONDITIONS, "Attempting to update o_id_to_g_id map from o%u to o%u.\n", old_o_id, new_o_id);
    print_o_id_to_gid_map(DT_FIX_CONDITIONS, true);
    iter = o_id_to_g_id_map->find(old_o_id);
    if (iter != o_id_to_g_id_map->end())
    {
        dprint(DT_FIX_CONDITIONS, "...found o_id->g_id mapping that needs updated: o%u = g%u -> o%u = g%u.\n", iter->first, iter->second, new_o_id, iter->second);
        new_g_id = iter->second;
        o_id_to_g_id_map->erase(old_o_id);
    } else {
        new_g_id = pG_id;
    }

    (*o_id_to_g_id_map)[new_o_id] = new_g_id;

}

void Variablization_Manager::update_o_id_for_new_instantiation(Symbol** pOvar, uint64_t* pO_id, uint64_t* pG_id, uint64_t pNew_i_id, bool pIsResult)
{
    uint64_t new_o_id = 0;
    Symbol* new_ovar = NULL;
    bool found_unique = false;

    if (!(*pO_id)) return;

    dprint(DT_OVAR_MAPPINGS, "update_o_id_for_new_instantiation called for %y o%u ", (*pOvar), (*pO_id));
    dprint_noprefix(DT_OVAR_MAPPINGS, "g%u i%u %s\n", (*pG_id), pNew_i_id, pIsResult ? "isResult" : "isNotResult");
    o_id_update_info* new_o_id_info = get_updated_o_id_info((*pO_id));
    if (new_o_id_info)
    {
        (*pO_id) = new_o_id_info->o_id;
        if ((*pOvar) != new_o_id_info->o_var)
        {
            dprint(DT_OVAR_MAPPINGS, "...found existing variable update %y(o%u)\n", new_o_id_info->o_var, new_o_id_info->o_id);
            symbol_remove_ref(thisAgent, (*pOvar));
            (*pOvar) = new_o_id_info->o_var;
            symbol_add_ref(thisAgent, (*pOvar));
        }
    } else {
        if (pIsResult)
        {
            /* A RHS variable that was local to the substate, so it won't be variablized and doesn't need
             * these values.*/
            dprint(DT_OVAR_MAPPINGS, "...did not find update info for result %y(o%u).  Must be ungrounded.\n", (*pOvar), (*pO_id));
            if ((*pOvar))
            {
                symbol_remove_ref(thisAgent, (*pOvar));
            } else {
                /* MToDo|  Remove logic.  There should always be an oVar. */
                assert(false);
            }
            (*pG_id) = 0;
            (*pOvar) = NULL;
            (*pO_id) = 0;
        } else {
            new_o_id = get_existing_o_id((*pOvar), pNew_i_id);
            if (new_o_id)
            {
                /* Ovar needs to be made unique.  This ovar has an existing o_id for new instantiation but no
                 * update info entry for the o_id.  That means that the previously seen case of this ovar
                 * had a different o_id. */
                std::string lVarName((*pOvar)->var->name+1);
                lVarName.erase(lVarName.length()-1);
                lVarName.append("-other");
                new_ovar = generate_new_variable(thisAgent, lVarName.c_str());
                //            dprint(DT_OVAR_MAPPINGS, "update_o_id_for_new_instantiation generated new variable %y from %s (%y ", new_ovar, lVarName.c_str(), ts);
                //            dprint_noprefix(DT_OVAR_MAPPINGS, "o%u g%u i%u %s).\n", tu1, (*pG_id), pNew_i_id, pIsResult ? "isResult" : "isNotResult");
                new_o_id = get_or_create_o_id(new_ovar, pNew_i_id);
                dprint(DT_OVAR_MAPPINGS, "...var name already used.  Generated new identity %y(%u) from varname root %s.\n", new_ovar, new_o_id, lVarName.c_str());
                add_updated_o_id_info((*pO_id), new_ovar, new_o_id);
                if ((*pOvar))
                {
                    symbol_remove_ref(thisAgent, (*pOvar));
                } else {
                    /* MToDo|  Remove logic.  There should always be an oVar. */
                    assert(false);
                }
                (*pOvar) = new_ovar;
            } else {
                /* First time this ovar has been encountered in the new instantiation.  So, create new
                 * o_id and add a new o_id_update_info entry for future tests that use the old o_id */
                new_o_id = get_or_create_o_id((*pOvar), pNew_i_id);
                dprint(DT_OVAR_MAPPINGS, "...var name not used.  Generated new identity for instantiation i%u %y(o%u).\n", pNew_i_id, (*pOvar), (*pO_id));
                add_updated_o_id_info((*pO_id), (*pOvar), new_o_id);
            }
            if ((*pG_id))
            {
                add_updated_o_id_to_g_id_mapping((*pO_id), new_o_id, (*pG_id));
            }
            (*pO_id) = new_o_id;
        }
    }
}

void Variablization_Manager::consolidate_variables_in_test(test t, tc_number tc_num, uint64_t pI_id)
{
    test found_test;
    uint64_t old_o_id;

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case DISJUNCTION_TEST:
            break;
        case CONJUNCTIVE_TEST:
        {
            dprint(DT_FIX_CONDITIONS, "Consolidating vars in conj test: %t [%g]\n", t, t);
            ::list* c = t->data.conjunct_list;
            test tt;
            while (c)
            {
                tt = static_cast<test>(c->first);
                consolidate_variables_in_test(tt, tc_num, pI_id);
                c = c->rest;
            }
            dprint(DT_FIX_CONDITIONS, "After consolidating vars in conj test: %t [%g]\n", t, t);
            break;
        }
        default:
            if (test_has_referent(t) && (t->data.referent->tc_num == tc_num))
            {
                dprint(DT_FIX_CONDITIONS, "Test marked as needing substitution found:: %t\n", t);
                found_test = get_substitution(t->data.referent);
                if (!found_test)
                {
                    print_variablization_tables(DT_FIX_CONDITIONS);
                    print_substitution_map(DT_FIX_CONDITIONS);
                    assert(false);
                }
                symbol_remove_ref(thisAgent, t->data.referent);
                symbol_add_ref(thisAgent, found_test->data.referent);
                t->data.referent = found_test->data.referent;
                t->identity->grounding_id = found_test->identity->grounding_id;
                dprint(DT_FIX_CONDITIONS, "Copying original vars: %y %y\n", t->identity->original_var, found_test->identity->original_var);
                if (t->identity->original_var)
                {
                    symbol_remove_ref(thisAgent, t->identity->original_var);
                }
                symbol_add_ref(thisAgent, found_test->identity->original_var);
                t->identity->original_var = found_test->identity->original_var;
                t->identity->original_var_id = found_test->identity->original_var_id;
            }

            break;
    }
}

void Variablization_Manager::consolidate_variables(condition* top_cond, tc_number tc_num, uint64_t pI_id)
{
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Consolidating variables in tests =\n");
    dprint_set_indents(DT_FIX_CONDITIONS, "          ");
    dprint_noprefix(DT_FIX_CONDITIONS, "%1", top_cond);
    dprint_clear_indents(DT_FIX_CONDITIONS);
    dprint_header(DT_FIX_CONDITIONS, PrintAfter, "");

    condition* next_cond, *last_cond = NULL;
    for (condition* cond = top_cond; cond;)
    {
        dprint(DT_FIX_CONDITIONS, "Fixing condition: %l\n", cond);
        next_cond = cond->next;
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            consolidate_variables_in_test(cond->data.tests.id_test, tc_num, pI_id);
            consolidate_variables_in_test(cond->data.tests.attr_test, tc_num, pI_id);
            consolidate_variables_in_test(cond->data.tests.value_test, tc_num, pI_id);
        }
        else
        {
            consolidate_variables(cond->data.ncc.top, tc_num, pI_id);
        }
        last_cond = cond;
        cond = next_cond;
//        dprint(DT_FIX_CONDITIONS, "...done fixing condition.\n");
    }
    dprint_header(DT_FIX_CONDITIONS, PrintBefore, "");
    dprint_set_indents(DT_FIX_CONDITIONS, "          ");
    dprint_noprefix(DT_FIX_CONDITIONS, "%1", top_cond);
    dprint_clear_indents(DT_FIX_CONDITIONS);
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Done consolidating variables in tests =\n");

}


void Variablization_Manager::fix_conditions(condition* top_cond, uint64_t pI_id, bool ignore_ungroundeds)
{
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Fixing conditions =\n");
    dprint_set_indents(DT_FIX_CONDITIONS, "          ");
    dprint_noprefix(DT_FIX_CONDITIONS, "%1", top_cond);
    dprint_clear_indents(DT_FIX_CONDITIONS);
    dprint_header(DT_FIX_CONDITIONS, PrintAfter, "");

    // get new tc_num to mark any variables that need to be substituted
    tc_number tc_num_subst = get_new_tc_number(thisAgent);;

    condition* next_cond, *last_cond = NULL;
    for (condition* cond = top_cond; cond;)
    {
        dprint(DT_FIX_CONDITIONS, "Finding redundancies in condition: %l\n", cond);
        next_cond = cond->next;
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            remove_redundancies_and_ungroundeds(&(cond->data.tests.id_test), tc_num_subst, ignore_ungroundeds);
            remove_redundancies_and_ungroundeds(&(cond->data.tests.attr_test), tc_num_subst, ignore_ungroundeds);
            remove_redundancies_and_ungroundeds(&(cond->data.tests.value_test), tc_num_subst, ignore_ungroundeds);
        }
        else
        {
            // Do we really need for NCCs?  I do think it might be possible to get
            // ungroundeds in NCCs
        }
        last_cond = cond;
        cond = next_cond;
//        dprint(DT_FIX_CONDITIONS, "...done finding redundancies in condition.\n");
    }

    consolidate_variables(top_cond, tc_num_subst, pI_id);
    clear_substitution_map();

    // get new tc_num to mark any variables that need to be literals
    tc_num_literalized = get_new_tc_number(thisAgent);;
//    install_literal_constraints(top_cond);

    dprint_header(DT_FIX_CONDITIONS, PrintBefore, "");
    dprint_set_indents(DT_FIX_CONDITIONS, "          ");
    dprint_noprefix(DT_FIX_CONDITIONS, "%1", top_cond);
    dprint_clear_indents(DT_FIX_CONDITIONS);
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Done fixing conditions =\n");
}

void Variablization_Manager::fix_results(preference* result, uint64_t pI_id)
{

    if (!result) return;

    dprint(DT_FIX_CONDITIONS, "Fixing result %p\n", result);

    if (pI_id)
    {
        if (result->original_symbols.id)
        {
            assert(result->original_symbols.id->is_variable());
            update_o_id_for_new_instantiation(&(result->original_symbols.id), &(result->o_ids.id), &(result->g_ids.id), pI_id, true);
        }
        if (result->original_symbols.attr)
        {
            assert(result->original_symbols.attr->is_variable());
            update_o_id_for_new_instantiation(&(result->original_symbols.attr), &(result->o_ids.attr), &(result->g_ids.attr), pI_id, true);
        }
        if (result->original_symbols.value)
        {
            assert(result->original_symbols.value->is_variable());
            update_o_id_for_new_instantiation(&(result->original_symbols.value), &(result->o_ids.value), &(result->g_ids.value), pI_id, true);
        }
    }
    fix_results(result->next_result, pI_id);
    /* MToDo | Do we need to fix o_ids in clones too? */
}
