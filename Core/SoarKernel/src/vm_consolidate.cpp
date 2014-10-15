///*
// * variablization_manager_merge.cpp
// *
// *  Created on: Jul 25, 2013
// *      Author: mazzin
// */
//
//#include "variablization_manager.h"
//#include "agent.h"
//#include "instantiations.h"
//#include "assert.h"
//#include "test.h"
//#include "print.h"
//#include "debug.h"
//
//void Variablization_Manager::clear_substitution_map()
//{
//    substitution_map->clear();
//}
//
//test Variablization_Manager::get_substitution(Symbol* sym)
//{
//    std::map< Symbol*, test >::iterator iter;
//    iter = substitution_map->find(sym);
//    if (iter != substitution_map->end())
//    {
//        return (iter->second);
//    }
//    return NULL;
//}
//
//
//void Variablization_Manager::consolidate_variables_in_test(test t, tc_number tc_num)
//{
//    test found_test;
//    switch (t->type)
//    {
//        case GOAL_ID_TEST:
//        case IMPASSE_ID_TEST:
//        case DISJUNCTION_TEST:
//            break;
//        case CONJUNCTIVE_TEST:
//        {
//            dprint_test(DT_FIX_CONDITIONS, t, true, false, false, "          Consolidating vars in conj test: ", "\n");
//            ::list* c = t->data.conjunct_list;
//            while (c)
//            {
//                test tt = static_cast<test>(c->first);
//                consolidate_variables_in_test(tt, tc_num);
//                c = c->rest;
//            }
//            dprint_test(DT_FIX_CONDITIONS, t, true, false, false, "          After consolidating vars in conj test: ", "\n");
//            break;
//        }
//        default:
//            if (test_has_referent(t) && (t->data.referent->tc_num == tc_num))
//            {
//                dprint_test(DT_FIX_CONDITIONS, t, true, false, true, "          Test needing substitution found:: ", "\n");
//                found_test = get_substitution(t->data.referent);
//                assert(found_test);
//                symbol_remove_ref(thisAgent, t->data.referent);
//                symbol_add_ref(thisAgent, found_test->data.referent);
//                t->data.referent = found_test->data.referent;
//                t->identity->grounding_id = found_test->identity->grounding_id;
//            }
//            break;
//    }
//}
//
//void Variablization_Manager::consolidate_variables(condition* top_cond, tc_number tc_num)
//{
//    dprint(DT_FIX_CONDITIONS, "====================================\n");
//    dprint(DT_FIX_CONDITIONS, "= Consolidating variables in tests =\n");
//    dprint(DT_FIX_CONDITIONS, "====================================\n");
//    dprint_condition_list(DT_FIX_CONDITIONS, top_cond, "          ", true, false, true);
//    dprint(DT_FIX_CONDITIONS, "====================================\n");
//
//    condition* next_cond, *last_cond = NULL;
//    for (condition* cond = top_cond; cond;)
//    {
//        dprint_condition(DT_FIX_CONDITIONS, cond, "Fixing condition: ", true, false, true);
//        next_cond = cond->next;
//        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
//        {
//            consolidate_variables_in_test(cond->data.tests.id_test, tc_num);
//            consolidate_variables_in_test(cond->data.tests.attr_test, tc_num);
//            consolidate_variables_in_test(cond->data.tests.value_test, tc_num);
//        }
//        else
//        {
//            consolidate_variables(cond->data.ncc.top, tc_num);
//        }
//        last_cond = cond;
//        cond = next_cond;
//        dprint(DT_FIX_CONDITIONS, "...done fixing condition.\n");
//    }
//    dprint(DT_FIX_CONDITIONS, "=========================================\n");
//    dprint_condition_list(DT_FIX_CONDITIONS, top_cond, "          ", true, false, true);
//    dprint(DT_FIX_CONDITIONS, "=========================================\n");
//    dprint(DT_FIX_CONDITIONS, "= Done consolidating variables in tests =\n");
//    dprint(DT_FIX_CONDITIONS, "=========================================\n");
//
//}
//
//void Variablization_Manager::update_ovar_table_for_sub(test sacrificeSymTest, test survivorSymTest)
//{
//    std::map< Symbol*, uint64_t >::iterator iter;
//
//    print_ovar_gid_propogation_table(DT_FIX_CONDITIONS, true);
//    for (iter = orig_var_to_g_id_map->begin(); iter != orig_var_to_g_id_map->end(); ++iter)
//    {
//
//        if (iter->second == sacrificeSymTest->identity->grounding_id)
//        {
//            dprint_noprefix(DT_FIX_CONDITIONS, "...found ovar->g_id mapping that needs updated: %s = g%llu -> g%llu.\n", iter->first->to_string(), iter->second, survivorSymTest->identity->grounding_id);
//            iter->second = survivorSymTest->identity->grounding_id;
//        }
//    }
//}
//
//// Requires:  Two equality tests
//void Variablization_Manager::set_substitution(test sacrificeSymTest, test survivorSymTest, tc_number tc_num)
//{
//    dprint_noprefix(DT_FIX_CONDITIONS, "Storing substitution %s(g%llu)->%s(g%llu) (tc_num %llu)...\n",
//                    sacrificeSymTest->data.referent->to_string(), sacrificeSymTest->identity->grounding_id,
//                    survivorSymTest->data.referent->to_string(), survivorSymTest->identity->grounding_id,
//                    tc_num);
//
//    /* -- If we're already supposed to substitute the survivor sym for another, we need to
//          scan ovar->g_id list looking for g_ids that match previous survivor and redirect
//          to new survivor -- */
//    test existing_survivor = get_substitution(survivorSymTest->data.referent);
//    if (existing_survivor)
//    {
//        dprint_noprefix(DT_FIX_CONDITIONS, "...found existing survivor %s.  Fixing ovar table and deleting entry.\n", existing_survivor->data.referent->to_string());
//        update_ovar_table_for_sub(existing_survivor, survivorSymTest);
//        existing_survivor->data.referent->tc_num = tc_num;
//        substitution_map->erase(survivorSymTest->data.referent);
//    }
//
//    (*substitution_map)[sacrificeSymTest->data.referent] = survivorSymTest;
//
//    sacrificeSymTest->data.referent->tc_num = tc_num;
//    survivorSymTest->data.referent->tc_num = 0;
//
//    dprint_noprefix(DT_FIX_CONDITIONS, "...fixing ovar table...\n");
//    update_ovar_table_for_sub(sacrificeSymTest, survivorSymTest);
//
//    // Scan substitution map looking for values that match sacrifice and redirect to survivor
//    dprint_noprefix(DT_FIX_CONDITIONS, "...updating existing substitutions...\n");
//    std::map< Symbol*, test >::iterator iter;
//    for (iter = substitution_map->begin(); iter != substitution_map->end(); ++iter)
//    {
//        if (iter->second->data.referent == sacrificeSymTest->data.referent)
//        {
//            dprint_noprefix(DT_FIX_CONDITIONS, "...found substitution that needs updated: %s = %s (g%llu).\n", iter->first->to_string(), iter->second->data.referent->to_string(), survivorSymTest->identity->grounding_id);
//            iter->second = survivorSymTest;
//        }
//    }
//}
//
//void Variablization_Manager::remove_redundancies_and_ungroundeds(test* t, tc_number tc_num)
//{
//    assert(t);
//
//    if ((*t)->type == EQUALITY_TEST)
//    {
//        /* Cache main equality test.  Clearly redundant for an equality test, but simplifies code. */
//        (*t)->eq_test = (*t);
//        return;
//    }
//    else if ((*t)->type == CONJUNCTIVE_TEST)
//    {
//        test found_eq_test = NULL, tt, survivor, sacrifice;
//        ::list* c = (*t)->data.conjunct_list;
//        while (c)
//        {
//            tt = static_cast<test>(c->first);
//
//            // For all tests, check if referent is STI.  If so, it's ungrounded.  Delete.
//            if (test_has_referent(tt) && (tt->data.referent->is_sti()))
//            {
//                dprint(DT_FIX_CONDITIONS, "Ungrounded STI found: %s\n", tt->data.referent->to_string());
//                c = delete_test_from_conjunct(thisAgent, t, c);
//                dprint_test(DT_FIX_CONDITIONS, (*t), true, false, true, "          ...after deletion: ", "\n");
//            }
//
//            // Code to detect a conjunction that contain more than one equality tests.  If found, eliminate the redundancy.
//            else if (tt->type == EQUALITY_TEST)
//            {
//                if (found_eq_test != NULL)
//                {
//                    dprint(DT_FIX_CONDITIONS, "Found second equality test %s.\n", tt->data.referent->to_string());
//                    /* -- Choose the symbol with the lower string value as the survivor, to be consistent -- */
//                    if (strcmp(found_eq_test->data.referent->to_string(), tt->data.referent->to_string()) > 0)
//                    {
//                        survivor = tt;
//                        sacrifice = found_eq_test;
//                    }
//                    else
//                    {
//                        survivor = found_eq_test;
//                        sacrifice = tt;
//
//                    }
//                    dprint(DT_FIX_CONDITIONS, "Surviving test = %s, sacrificed test = %s.\n", survivor->data.referent->to_string(), sacrifice->data.referent->to_string());
//                    // MToDo | If there's a problem, make sure we have g_id in this variablized test.
//                    set_substitution(sacrifice, survivor, tc_num);
//                    c = delete_test_from_conjunct(thisAgent, t, c);
//                }
//                else
//                {
//                    dprint(DT_FIX_CONDITIONS, "Found first equality test %s.\n", tt->data.referent->to_string());
//                    found_eq_test = survivor = tt;
//                    c = c->rest;
//                }
//            }
//            else
//            {
//                c = c->rest;
//            }
//        }
//
//        /* -- We also cache the main equality test for each element in each condition
//         *    since it's easy to do here. This allows us to avoid searching conjunctive
//         *    tests repeatedly during merging. -- */
//        (*t)->eq_test = survivor;
//        survivor->eq_test = survivor;
//    }
//}
//
//void Variablization_Manager::fix_conditions(condition* top_cond)
//{
//    dprint(DT_FIX_CONDITIONS, "========================\n");
//    dprint(DT_FIX_CONDITIONS, "= Finding redundancies =\n");
//    dprint(DT_FIX_CONDITIONS, "========================\n");
//    dprint_condition_list(DT_FIX_CONDITIONS, top_cond, "          ", true, false, true);
//    dprint(DT_FIX_CONDITIONS, "========================\n");
//
//    // get new tc_num to mark any variables that need to be substituted
//    tc_number tc_num = get_new_tc_number(thisAgent);;
//
//    condition* next_cond, *last_cond = NULL;
//    for (condition* cond = top_cond; cond;)
//    {
//        dprint_condition(DT_FIX_CONDITIONS, cond, "Finding redundancies in condition: ", true, false, true);
//        next_cond = cond->next;
//        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
//        {
//            remove_redundancies_and_ungroundeds(&(cond->data.tests.id_test), tc_num);
//            remove_redundancies_and_ungroundeds(&(cond->data.tests.attr_test), tc_num);
//            remove_redundancies_and_ungroundeds(&(cond->data.tests.value_test), tc_num);
//        }
//        else
//        {
//            // Do we really need for NCCs?  I do think it might be possible to get
//            // ungroundeds in NCCs
//        }
//        last_cond = cond;
//        cond = next_cond;
//        dprint(DT_FIX_CONDITIONS, "...done finding redundancies in condition.\n");
//    }
//
//    consolidate_variables(top_cond, tc_num);
//    clear_substitution_map();
//
//    dprint(DT_FIX_CONDITIONS, "=============================\n");
//    dprint_condition_list(DT_FIX_CONDITIONS, top_cond, "          ", true, false, true);
//    dprint(DT_FIX_CONDITIONS, "=============================\n");
//    dprint(DT_FIX_CONDITIONS, "= Done finding redundancies =\n");
//    dprint(DT_FIX_CONDITIONS, "=============================\n");
//}
//
//
