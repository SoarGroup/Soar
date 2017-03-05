#include "ebc.h"

#include "agent.h"
#include "dprint.h"
#include "condition.h"
#include "symbol_manager.h"
#include "test.h"
#include "rhs.h"

void Explanation_Based_Chunker::reinstantiate_test (test pTest, bool pIsInstantiationCond)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            reinstantiate_test(static_cast<test>(c->first), pIsInstantiationCond);
        }
    }
    else if (test_has_referent(pTest) && pTest->data.referent->is_variable() && pTest->identity)
    {
        Symbol* oldSym = pTest->data.referent;
        pTest->data.referent = pTest->data.referent->var->instantiated_sym;
        thisAgent->symbolManager->symbol_add_ref(pTest->data.referent);
        thisAgent->symbolManager->symbol_remove_ref(&oldSym);
    }
    if (test_has_referent(pTest) && pTest->identity && pIsInstantiationCond) pTest->identity = pTest->clone_identity;
}

void Explanation_Based_Chunker::reinstantiate_condition_list(condition* top_cond, bool pIsInstantiationCond, bool pIsNCC)
{
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        reinstantiate_condition(cond, pIsInstantiationCond, pIsNCC);
    }
}

void Explanation_Based_Chunker::reinstantiate_condition(condition* cond, bool pIsInstantiationCond, bool pIsNCC)
{
    if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        reinstantiate_test(cond->data.tests.id_test, pIsInstantiationCond);
        reinstantiate_test(cond->data.tests.attr_test, pIsInstantiationCond);
        reinstantiate_test(cond->data.tests.value_test, pIsInstantiationCond);
        dprint(DT_REINSTANTIATE, "Reinstantiated condition is now %l\n", cond);
        #ifdef EBC_SANITY_CHECK_RULES
        sanity_justification_test(cond->data.tests.id_test, pIsNCC);
        sanity_justification_test(cond->data.tests.attr_test, pIsNCC);
        sanity_justification_test(cond->data.tests.value_test, pIsNCC);
        #endif
    } else {
        reinstantiate_condition_list(cond->data.ncc.top, pIsInstantiationCond, true);
    }

}

condition* Explanation_Based_Chunker::reinstantiate_lhs(condition* top_cond)
{
    dprint_header(DT_REINSTANTIATE, PrintBoth, "Reversing variablization of condition list\n");

    condition* last_cond, *lCond, *inst_top;
    last_cond = inst_top = lCond = NULL;

    for (condition* cond = m_lhs; cond != NIL; cond = cond->next)
    {
        dprint(DT_REINSTANTIATE, "Reversing variablization of condition: %l\n", cond);

        if (m_rule_type == ebc_justification)
        {
            /* This case is rare (if it should happen at all any more).  It occurs when a learning attempt fails
             * and is being reverted to a justification instead */
            reinstantiate_condition(cond, false);
            lCond = copy_condition(thisAgent, cond, false, false, false);
            reinstantiate_condition(cond, true);
            lCond->inst = m_chunk_inst;
            lCond->bt = cond->bt;
        } else {
            lCond = copy_condition(thisAgent, cond, false, false, false);
            lCond->bt = cond->bt;
            lCond->inst = m_chunk_inst;
            reinstantiate_condition(lCond, true);
        }

        if (last_cond)
        {
            last_cond->next = lCond;
        } else {
            inst_top = lCond;
        }
        lCond->prev = last_cond;
        last_cond = lCond;
    }
    if (last_cond)
    {
        last_cond->next = NULL;
    }
    else
    {
        inst_top = NULL;
    }

    dprint_header(DT_REINSTANTIATE, PrintAfter, "Done reversing variablization of LHS condition list.\n");
    return inst_top;
}

void Explanation_Based_Chunker::reinstantiate_rhs_symbol(rhs_value pRhs_val)
{

    Symbol* var;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;

        for (c = fl->rest; c != NULL; c = c->rest)
        {
            dprint(DT_RHS_FUN_VARIABLIZATION, "Reversing variablization of funcall RHS value %r\n", static_cast<char*>(c->first));
            reinstantiate_rhs_symbol(static_cast<char*>(c->first));
            dprint(DT_RHS_FUN_VARIABLIZATION, "... RHS value is now %r\n", static_cast<char*>(c->first));
        }
        return;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    if (rs->referent->is_variable())
    {
        dprint(DT_REINSTANTIATE, "Reversing variablization for RHS symbol %y [%u] -> %y.\n", rs->referent, rs->identity, rs->referent->var->instantiated_sym);
        Symbol* oldSym = rs->referent;
        rs->referent = rs->referent->var->instantiated_sym;
        thisAgent->symbolManager->symbol_add_ref(rs->referent);
        thisAgent->symbolManager->symbol_remove_ref(&oldSym);
        IdentitySetSharedPtr lIDSet = rs->identity_set_wp.lock();
        if (lIDSet)
        {
            rs->identity = lIDSet->get_clone_identity();
            rs->identity_set_wp.reset();
        }
    } else {
        dprint(DT_REINSTANTIATE, "Not a variable.  Ignoring %y [%u]\n", rs->referent, rs->identity);
    }
}

void Explanation_Based_Chunker::reinstantiate_actions(action* pActionList)
{
    for (action* lAction = pActionList; lAction != NULL; lAction = lAction->next)
    {
        if (lAction->type == MAKE_ACTION)
        {
            reinstantiate_rhs_symbol(lAction->id);
            reinstantiate_rhs_symbol(lAction->attr);
            reinstantiate_rhs_symbol(lAction->value);
            if (lAction->referent)
            {
                reinstantiate_rhs_symbol(lAction->referent);
            }
        }
    }
}

condition* Explanation_Based_Chunker::reinstantiate_current_rule()
{
    dprint(DT_REINSTANTIATE, "Before reinstantiation: \n%1-->\n%2", m_lhs, m_rhs);

    condition* returnConds = reinstantiate_lhs(m_lhs);

    /* If this was a chunk failure that is being reverted to a justification, we must
     * reinstantiate the actions as well */
    if (m_rule_type == ebc_justification)
    {
        reinstantiate_actions(m_rhs);
    }
    dprint(DT_REINSTANTIATE, "After reinstantiation: \n%1-->\n%2", m_lhs, m_rhs);

    return returnConds;
}
