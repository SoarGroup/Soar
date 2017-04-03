#include "ebc.h"
#include "ebc_identity.h"

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

    if (result->inst_identities.id)
    {
        /* Match goal is not calculated yet, so we're passing in NULL, which would normally be a problem 
         * for the explainer since it wouldn't know where to record the identity set.  But in this function, 
         * we're creating actions, not preferences, so that means we should never be creating new unbound 
         * RHS values and these calls to get_or_add_identity should always be doing a look-up and won't need
         * the match goal. */
        if (!result->identities.id)
            result->identities.id = get_or_add_identity(result->inst_identities.id, result->identities.id, NULL);
        result->chunk_inst_identities.id = result->identities.id->update_clone_id();
    } else
        result->chunk_inst_identities.id = LITERAL_VALUE;

    if (!result->rhs_func_inst_identities.id)
    {
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id,  result->inst_identities.id, result->identities.id, result->was_unbound_vars.id);
    } else {
        result->rhs_func_chunk_inst_identities.id = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.id, false, true);
        a->id = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.id);
    }

    if (result->inst_identities.attr)
    {
        if (!result->identities.attr)
            result->identities.attr = get_or_add_identity(result->inst_identities.attr, result->identities.attr, NULL);
        result->chunk_inst_identities.attr = result->identities.attr->update_clone_id();
    } else
        result->chunk_inst_identities.attr = LITERAL_VALUE;

    if (!result->rhs_func_inst_identities.attr)
    {
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr,  result->inst_identities.attr, result->identities.attr, result->was_unbound_vars.attr);
    } else {
        result->rhs_func_chunk_inst_identities.attr = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.attr, false, true);
        a->attr = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.attr);
    }

    if (result->inst_identities.value)
    {
        if(!result->identities.value)
            result->identities.value = get_or_add_identity(result->inst_identities.value, result->identities.value, NULL);
        result->chunk_inst_identities.value = result->identities.value->update_clone_id();
    } else
        result->chunk_inst_identities.value = LITERAL_VALUE;

    if (!result->rhs_func_inst_identities.value)
    {
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value,  result->inst_identities.value, result->identities.value, result->was_unbound_vars.value);
    } else {
        result->rhs_func_chunk_inst_identities.value = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.value, false, true);
        a->value = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.value);
    }

    if (preference_is_binary(result->type))
    {
        if (result->inst_identities.referent)
        {
            if (!result->identities.referent)
                result->identities.referent = get_or_add_identity(result->inst_identities.referent, result->identities.referent, NULL);
            result->chunk_inst_identities.referent = result->identities.referent->update_clone_id();
        } else result->chunk_inst_identities.referent = LITERAL_VALUE;

        if (!result->rhs_func_inst_identities.referent)
        {
            a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent,  result->inst_identities.referent, result->identities.referent, result->was_unbound_vars.referent);
        } else {
            result->rhs_func_chunk_inst_identities.referent = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.referent, false, true);
            a->referent = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.referent);
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

    if (t->identity && t->identity->get_clone_identity())
    {
        dprint(DT_LHS_VARIABLIZATION, "Updating identity by lookup %t %g...with %u\n", t, t, t->identity->get_clone_identity());
        t->inst_identity = t->identity->get_clone_identity();
        clear_test_identity(thisAgent, t);
        dprint(DT_LHS_VARIABLIZATION, "--> t: %t %g\n", t, t);
    }
    else
    {
        t->inst_identity = LITERAL_VALUE;
        clear_test_identity(thisAgent, t);
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
                else if (tt->identity && !tt->data.referent->is_variable())
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
        if (test_has_referent(t) && t->identity)
        {
            update_identities_in_test_by_lookup(t, pSkipTopLevelEqualities);
        }
    }
}

void Explanation_Based_Chunker::update_identities_in_equality_tests(test pTest)
{
    if (!pTest->eq_test->data.referent->is_variable())
    {
        if (pTest->eq_test->identity && !pTest->eq_test->identity->literalized())
        {
            dprint(DT_LHS_VARIABLIZATION, "Updating equality test %t %g from %t %g\n", pTest->eq_test, pTest->eq_test, pTest, pTest);
            pTest->eq_test->inst_identity = pTest->eq_test->identity->get_clone_identity();
            if (!pTest->eq_test->inst_identity) pTest->eq_test->inst_identity = pTest->eq_test->identity->update_clone_id();
            clear_test_identity(thisAgent, pTest->eq_test);
            dprint(DT_LHS_VARIABLIZATION, "...to produce %t %g\n", pTest->eq_test, pTest->eq_test);
        } else {
            pTest->eq_test->inst_identity = LITERAL_VALUE;
            if (pTest->eq_test->identity) clear_test_identity(thisAgent, pTest->eq_test);
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

void Explanation_Based_Chunker::update_identities_in_rhs_value(const rhs_value pRhs_val)
{
    if (rhs_value_is_funcall(pRhs_val))
    {
        for (cons* c = rhs_value_to_funcall_list(pRhs_val)->rest; c != NIL; c = c->rest)
        {
            update_identities_in_rhs_value(static_cast<rhs_value>(c->first));
        }
    }
    else
    {
        rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);
        uint64_t lID;

        dprint(DT_RHS_VARIABLIZATION, "Updating %y [v%u/s%u].\n", rs->referent, rs->inst_identity, rs->identity ? rs->identity->get_identity() : 0);

        if (rs->identity)
        {
            lID = rs->identity->get_clone_identity();
            if (!lID)
                lID = rs->identity->get_identity();
        }
        else lID = rs->inst_identity;

        if (lID) rs->identity = thisAgent->explanationBasedChunker->get_identity_for_id(lID);

        dprint(DT_RHS_VARIABLIZATION, "...RHS value is now %y [v%u/s%u].\n", rs->referent, rs->inst_identity, rs->identity ? rs->identity->get_identity() : 0);
    }
}
