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
    action* a = make_action(thisAgent);
    a->type = MAKE_ACTION;
    a->preference_type = result->type;

    if (result->identities.id)
    {
        if (!result->identity_sets.id)
            result->identity_sets.id = get_or_add_id_set(result->identities.id, result->identity_sets.id);
        result->clone_identities.id = result->identity_sets.id->update_clone_id();
    } else
        result->clone_identities.id = LITERAL_VALUE;

    if (!result->rhs_funcs.id)
    {
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id,  result->identities.id, result->identity_sets.id, result->was_unbound_vars.id);
    } else {
        result->cloned_rhs_funcs.id = copy_rhs_value(thisAgent, result->rhs_funcs.id, false, true);
        a->id = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.id);
    }

    if (result->identities.attr)
    {
        if (!result->identity_sets.attr)
            result->identity_sets.attr = get_or_add_id_set(result->identities.attr, result->identity_sets.attr);
        result->clone_identities.attr = result->identity_sets.attr->update_clone_id();
    } else
        result->clone_identities.attr = LITERAL_VALUE;

    if (!result->rhs_funcs.attr)
    {
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr,  result->identities.attr, result->identity_sets.attr, result->was_unbound_vars.attr);
    } else {
        result->cloned_rhs_funcs.attr = copy_rhs_value(thisAgent, result->rhs_funcs.attr, false, true);
        a->attr = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.attr);
    }

    if (result->identities.value)
    {
        if(!result->identity_sets.value)
            result->identity_sets.value = get_or_add_id_set(result->identities.value, result->identity_sets.value);
        result->clone_identities.value = result->identity_sets.value->update_clone_id();
    } else
        result->clone_identities.value = LITERAL_VALUE;

    if (!result->rhs_funcs.value)
    {
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value,  result->identities.value, result->identity_sets.value, result->was_unbound_vars.value);
    } else {
        result->cloned_rhs_funcs.value = copy_rhs_value(thisAgent, result->rhs_funcs.value, false, true);
        a->value = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.value);
    }

    if (preference_is_binary(result->type))
    {
        if (result->identities.referent)
        {
            if (!result->identity_sets.referent)
                result->identity_sets.referent = get_or_add_id_set(result->identities.referent, result->identity_sets.referent);
            result->clone_identities.referent = result->identity_sets.referent->update_clone_id();
        } else result->clone_identities.referent = LITERAL_VALUE;

        if (!result->rhs_funcs.referent)
        {
            a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent,  result->identities.referent, result->identity_sets.referent, result->was_unbound_vars.referent);
        } else {
            result->cloned_rhs_funcs.referent = copy_rhs_value(thisAgent, result->rhs_funcs.referent, false, true);
            a->referent = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.referent);
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
    if (!pTest->eq_test->data.referent->is_variable())
    {
        if (pTest->eq_test->identity_set && !pTest->eq_test->identity_set->literalized())
        {
            dprint(DT_LHS_VARIABLIZATION, "Updating equality test %t %g from %t %g\n", pTest->eq_test, pTest->eq_test, pTest, pTest);
            pTest->eq_test->identity = pTest->eq_test->identity_set->get_clone_identity();
            if (!pTest->eq_test->identity) pTest->eq_test->identity = pTest->eq_test->identity_set->update_clone_id();
            clear_test_identity_set(thisAgent, pTest->eq_test);
            dprint(DT_LHS_VARIABLIZATION, "...to produce %t %g\n", pTest->eq_test, pTest->eq_test);
        } else {
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

uint64_t Explanation_Based_Chunker::update_identities_in_rhs_value(rhs_value &pRhs_val)
{
    uint64_t lCloneIdentity;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;
        rhs_value lRhsValue, *lc;

        dprint(DT_RHS_FUN_VARIABLIZATION, "Updating RHS funcall %r\n", pRhs_val);
        //dprint_identity_to_id_set_map(DT_RHS_FUN_VARIABLIZATION);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            lRhsValue = static_cast<rhs_value>(c->first);
            dprint(DT_RHS_FUN_VARIABLIZATION, "Updating RHS funcall argument %r\n", lRhsValue);
            update_identities_in_rhs_value(lRhsValue);
            dprint(DT_RHS_FUN_VARIABLIZATION, "... RHS funcall argument is now   %r\n", static_cast<char*>(c->first));
        }
        return LITERAL_VALUE;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);
    IdentitySet* lIDSet = rs->identity_set;

    dprint(DT_RHS_VARIABLIZATION, "Updating %y [%u].\n", rs->referent, lIDSet ? lIDSet->get_identity() : 0);

    if (rs->identity_set && !rs->identity_set->literalized())
    {
        lCloneIdentity = rs->identity_set->get_clone_identity();
        if (!lCloneIdentity) lCloneIdentity = rs->identity_set->update_clone_id();

        rs->identity = rs->identity_set->get_identity();
        rs->identity_set = NULL;
        dprint(DT_LHS_VARIABLIZATION, "...to produce %r [%u].  Returning clone identity %u.\n", rs->referent, rs->identity, lCloneIdentity);
        return lCloneIdentity;
    }

    dprint(DT_LHS_VARIABLIZATION, "...to produce %r [0]\n", rs->referent);
    rs->identity = LITERAL_VALUE;
    rs->identity_set = NULL_IDENTITY_SET;
    return LITERAL_VALUE;
}
