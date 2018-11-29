#include "ebc.h"
#include "ebc_identity.h"

#include "agent.h"
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
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id,  result->chunk_inst_identities.id, result->inst_identities.id, NULL, result->was_unbound_vars.id);
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
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr, result->chunk_inst_identities.attr, result->inst_identities.attr, NULL, result->was_unbound_vars.attr);
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
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value,  result->chunk_inst_identities.value, result->inst_identities.value, NULL, result->was_unbound_vars.value);
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
            a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent, result->chunk_inst_identities.referent, result->inst_identities.referent, NULL, result->was_unbound_vars.referent);
        } else {
            result->rhs_func_chunk_inst_identities.referent = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.referent, false, true);
            a->referent = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.referent);
        }
    }

    return a;
}

action* Explanation_Based_Chunker::convert_results_into_actions()
{

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

    return returnAction;
}

bool Explanation_Based_Chunker::update_identities_in_test_by_lookup(test t, bool pSkipTopLevelEqualities)
{
    if (pSkipTopLevelEqualities && (t->type == EQUALITY_TEST)) return true;

    if (t->identity)
    {
        if (!t->identity->literalized() && t->identity->get_clone_identity())
        {
            t->inst_identity = t->identity->get_clone_identity();
        } else {
            t->inst_identity = LITERAL_VALUE;
        }
        t->chunk_inst_identity = t->identity->get_identity();
        clear_test_identity(thisAgent, t);
    }
    else
    {
        t->inst_identity = LITERAL_VALUE;
        t->chunk_inst_identity = LITERAL_VALUE;
        clear_test_identity(thisAgent, t);
        return false;
    }

    return (t->inst_identity != LITERAL_VALUE);
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
        if (pTest->eq_test->identity)
        {
            if (!pTest->eq_test->identity->literalized())
            {
                pTest->eq_test->inst_identity = pTest->eq_test->identity->update_clone_id();
                pTest->eq_test->chunk_inst_identity = pTest->eq_test->identity->get_identity();
            } else {
                pTest->eq_test->inst_identity = LITERAL_VALUE;
                pTest->eq_test->chunk_inst_identity = LITERAL_VALUE;
            }
            clear_test_identity(thisAgent, pTest->eq_test);
        } else {
            pTest->eq_test->inst_identity = LITERAL_VALUE;
            pTest->eq_test->chunk_inst_identity = LITERAL_VALUE;
        }
    }
}

void Explanation_Based_Chunker::update_identities_in_condition_list(condition* top_cond, bool pInNegativeCondition)
{
    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
            {
                update_identities_in_equality_tests(cond->data.tests.id_test);
                update_identities_in_equality_tests(cond->data.tests.attr_test);
                update_identities_in_equality_tests(cond->data.tests.value_test);
            }
        }
    }
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            if ((cond->data.tests.id_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                update_identities_in_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            if ((cond->data.tests.attr_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                update_identities_in_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            if ((cond->data.tests.value_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                update_identities_in_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            update_identities_in_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            update_identities_in_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            update_identities_in_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            update_identities_in_condition_list(cond->data.ncc.top, true);
        }
    }
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
        uint64_t lID = LITERAL_VALUE;

        if (rs->identity) lID = rs->identity->get_clone_identity();
        if (!lID) lID = rs->inst_identity;
        if (!lID) lID = rs->cv_id;

        if (lID)
        {
            rs->identity = thisAgent->explanationBasedChunker->get_identity_for_id(lID);
            rs->cv_id = rs->inst_identity;
            rs->inst_identity = lID;
        } else {
            rs->identity = NULL_IDENTITY_SET;
            rs->inst_identity = LITERAL_VALUE;
            rs->cv_id = LITERAL_VALUE;
        }
    }
}
