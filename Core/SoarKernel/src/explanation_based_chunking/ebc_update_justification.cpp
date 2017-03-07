#include "ebc.h"
#include "ebc_identity_set.h"

#include "agent.h"
#include "dprint.h"
#include "condition.h"
#include "preference.h"
#include "symbol_manager.h"
#include "test.h"
#include "rhs.h"

action* Explanation_Based_Chunker::convert_result_into_action(preference* result)
{
    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    uint64_t lIdentity = 0;

    action* a = make_action(thisAgent);
    a->type = MAKE_ACTION;
    a->preference_type = result->type;

    if (result->identities.id)
    {
        if (!result->identity_sets.id)
            result->identity_sets.id = get_or_add_id_set(result->identities.id, result->identity_sets.id);
        result->identity_sets.id->update_clone_id();
        lIdentity = result->identity_sets.id->get_clone_identity();
    } else lIdentity = LITERAL_VALUE;

    if (!result->rhs_funcs.id)
    {
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id,  lIdentity, NULL, result->was_unbound_vars.id);
        result->clone_identities.id = lIdentity;
    } else {
        result->clone_identities.id = lIdentity;
        a->id = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.id);
        result->cloned_rhs_funcs.id = a->id;
        a->id = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.id, false, true);
    }

    if (result->identities.attr)
    {
        if (!result->identity_sets.attr)
            result->identity_sets.attr = get_or_add_id_set(result->identities.attr, result->identity_sets.attr);
        result->identity_sets.attr->update_clone_id();
        lIdentity = result->identity_sets.attr->get_clone_identity();
    } else lIdentity = LITERAL_VALUE;

    if (!result->rhs_funcs.attr)
    {
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr, lIdentity, NULL, result->was_unbound_vars.attr);
        result->clone_identities.attr = lIdentity;
    } else {
        result->clone_identities.attr = lIdentity;
        a->attr = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.attr);
        result->cloned_rhs_funcs.attr = a->attr;
        a->attr = copy_rhs_value(thisAgent, result->rhs_funcs.attr, false, true);
    }

    if (result->identities.value)
    {
        if(!result->identity_sets.value)
            result->identity_sets.value = get_or_add_id_set(result->identities.value, result->identity_sets.value);
        result->identity_sets.value->update_clone_id();
        lIdentity = result->identity_sets.value->get_clone_identity();
    } else lIdentity = LITERAL_VALUE;
    if (!result->rhs_funcs.value)
    {
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value, lIdentity, NULL, result->was_unbound_vars.value);
        result->clone_identities.value = lIdentity;
    } else {
        result->clone_identities.value = lIdentity;
        a->value = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.value);
        result->cloned_rhs_funcs.value = a->value;
        a->value = copy_rhs_value(thisAgent, result->rhs_funcs.value, false, true);
    }

    if (result->identities.referent)
    {
        if (!result->identity_sets.referent)
            result->identity_sets.referent = get_or_add_id_set(result->identities.referent, result->identity_sets.referent);
        result->identity_sets.referent->update_clone_id();
        lIdentity = result->identity_sets.referent->get_clone_identity();
    } else lIdentity = LITERAL_VALUE;
    if (preference_is_binary(result->type))
    {
        if (!result->rhs_funcs.referent)
        {
            a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent, lIdentity, NULL, result->was_unbound_vars.referent);
            result->clone_identities.referent = lIdentity;
        } else {
            result->clone_identities.referent = lIdentity;
            a->referent = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.referent);
            result->cloned_rhs_funcs.referent = a->referent;
            a->referent = copy_rhs_value(thisAgent, result->rhs_funcs.referent, false, true);
        }
    }

    dprint(DT_RHS_VARIABLIZATION, "Converted result: %a\n", a);
    return a;
}

action* Explanation_Based_Chunker::convert_results_into_actions()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Result preferences before conversion: \n%6", NULL, m_results);

    action* returnAction, *lAction, *lLastAction;
    preference* lPref;

    thisAgent->symbolManager->reset_variable_generator(m_lhs, NIL);
    returnAction = lAction = lLastAction = NULL;

    for (lPref = m_results; lPref; lPref = lPref->next_result)
    {
        lAction = convert_result_into_action(lPref);
        if (!returnAction)  returnAction = lAction;
        if (lLastAction) lLastAction->next = lAction;
        lLastAction = lAction;
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Actions after conversion: \n%2", returnAction);
    return returnAction;
}

bool Explanation_Based_Chunker::update_identities_in_test_by_lookup(test t, bool pSkipTopLevelEqualities)
{
    if (pSkipTopLevelEqualities && (t->type == EQUALITY_TEST)) return true;

    if (t->identity_set && t->identity_set->get_clone_identity())
    {
        dprint(DT_LHS_VARIABLIZATION, "Updating identity by lookup %t %g...with %u\n", t, t, t->identity_set->get_clone_identity());
        t->identity = t->identity_set->get_clone_identity();
        clear_test_identity_set(thisAgent, t);
        dprint(DT_LHS_VARIABLIZATION, "--> t: %t %g\n", t, t);
    }
    else
    {
        t->identity = LITERAL_VALUE;
        clear_test_identity_set(thisAgent, t);
        return false;
    }

    return true;
}

void Explanation_Based_Chunker::update_identities_in_tests_by_lookup(test t, bool pSkipTopLevelEqualities)
{

    cons* c;
    test tt;

    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; )
        {
            /* Any ungrounded tests on non-STIs do not need to be deleted.  We just leave them as a literal. */
            tt = reinterpret_cast<test>(c->first);
            if (test_has_referent(tt))
            {
                if (tt->data.referent->is_sti())
                {
                    if (!update_identities_in_test_by_lookup(tt, pSkipTopLevelEqualities))
                    {
                        c = delete_test_from_conjunct(thisAgent, &t, c);
                        continue;
                    }
                }
                else if (tt->identity_set && !tt->data.referent->is_variable())
                {
                    update_identities_in_test_by_lookup(tt, pSkipTopLevelEqualities);
                }
            }
            c = c->rest;
        }
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        if (test_has_referent(t) && t->identity_set)
        {
            update_identities_in_test_by_lookup(t, pSkipTopLevelEqualities);
        }
    }
}

void Explanation_Based_Chunker::update_identities_in_equality_tests(test pTest)
{
    cons* c;
    test tt;
    char prefix[2];
    Symbol* lNewVariable = NULL;
    Symbol* lOldSym;

    if (!pTest->eq_test->data.referent->is_variable())
    {
        if (pTest->eq_test->identity_set && !pTest->eq_test->identity_set->literalized())
        {
            dprint(DT_LHS_VARIABLIZATION, "Updating equality test %t %g from %t %g\n", pTest->eq_test, pTest->eq_test, pTest, pTest);
            if (!pTest->eq_test->identity_set->get_clone_identity())
            {
                pTest->eq_test->identity_set->update_clone_id();
                dprint(DT_LHS_VARIABLIZATION, "...with newly created cloned identity %u for identity set %u\n", pTest->eq_test->identity_set->get_clone_identity(), pTest->eq_test->identity_set->super_join->idset_id);
            }
            pTest->eq_test->identity = pTest->eq_test->identity_set->get_clone_identity();
            clear_test_identity_set(thisAgent, pTest->eq_test);
            dprint(DT_LHS_VARIABLIZATION, "...to produce %t %g\n", pTest->eq_test, pTest->eq_test);
        } else {
            /* Literalized identity, so set identity in chunk to 0 */
            pTest->eq_test->identity = LITERAL_VALUE;
            if (pTest->eq_test->identity_set) clear_test_identity_set(thisAgent, pTest->eq_test);
        }
    }
}

void Explanation_Based_Chunker::update_identities_in_condition_list(condition* top_cond, bool pInNegativeCondition)
{
    dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Updating identities in justification's LHS condition list:\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Updating equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
            {
                dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Updating equality test in LHS positive condition: %l\n", cond);
                update_identities_in_equality_tests(cond->data.tests.id_test);
                update_identities_in_equality_tests(cond->data.tests.attr_test);
                update_identities_in_equality_tests(cond->data.tests.value_test);
                dprint(DT_LHS_VARIABLIZATION, "-->Updated equalities in condition: %l\n", cond);
            }
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Pass 2: Updating all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Updating LHS positive non-equality tests in: %l\n", cond);
            if ((cond->data.tests.id_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                update_identities_in_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            if ((cond->data.tests.attr_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                update_identities_in_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            if ((cond->data.tests.value_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                update_identities_in_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "-->Updated condition: %l\n", cond);
            #ifdef EBC_SANITY_CHECK_RULES
            sanity_justification_test(cond->data.tests.id_test, pInNegativeCondition);
            sanity_justification_test(cond->data.tests.attr_test, pInNegativeCondition);
            sanity_justification_test(cond->data.tests.value_test, pInNegativeCondition);
            #endif
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Updating LHS negative condition: %l\n", cond);
            update_identities_in_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            update_identities_in_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            update_identities_in_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "-->Updated negative condition: %l\n", cond);
            #ifdef EBC_SANITY_CHECK_RULES
            sanity_justification_test(cond->data.tests.id_test, pInNegativeCondition);
            sanity_justification_test(cond->data.tests.attr_test, pInNegativeCondition);
            sanity_justification_test(cond->data.tests.value_test, pInNegativeCondition);
            #endif
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint_header(DT_NCC_VARIABLIZATION, PrintBefore, "Updating LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            update_identities_in_condition_list(cond->data.ncc.top, true);
            dprint(DT_LHS_VARIABLIZATION, "-->Updated NCC: %l\n", cond);
        }
    }
    dprint_header(DT_LHS_VARIABLIZATION, PrintAfter, "Done updating LHS condition list.\n");
}


