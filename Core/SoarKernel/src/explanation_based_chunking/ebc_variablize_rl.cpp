#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "symbol_manager.h"
#include "test.h"
#include "rhs.h"

// creates an action for a template instantiation
action* Explanation_Based_Chunker::variablize_rl_action(action* pRLAction, struct token_struct* tok, wme* w, double & initial_value)
{
    action* rhs;
    Symbol* id_sym, *attr_sym, *val_sym, *ref_sym;
    char first_letter;

    // get the preference value
    bool_quadruple was_unbound_vars;
    id_sym = instantiate_rhs_value(thisAgent, pRLAction->id, -1, 's', tok, w, was_unbound_vars.id);
    attr_sym = instantiate_rhs_value(thisAgent, pRLAction->attr, id_sym->id->level, 'a', tok, w, was_unbound_vars.attr);
    first_letter = first_letter_from_symbol(attr_sym);
    val_sym = instantiate_rhs_value(thisAgent, pRLAction->value, id_sym->id->level, first_letter, tok, w, was_unbound_vars.value);
    ref_sym = instantiate_rhs_value(thisAgent, pRLAction->referent, id_sym->id->level, first_letter, tok, w, was_unbound_vars.referent);

    rhs = make_action(thisAgent);
    rhs->type = MAKE_ACTION;
    rhs->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

    rhs_symbol lRS = rhs_value_to_rhs_symbol(pRLAction->id);
    rhs->id = allocate_rhs_value_for_symbol(thisAgent, id_sym, lRS->inst_identity, NULL, was_unbound_vars.id);
    lRS = rhs_value_to_rhs_symbol(pRLAction->attr);
    rhs->attr = allocate_rhs_value_for_symbol(thisAgent, attr_sym, lRS->inst_identity, NULL, was_unbound_vars.attr);
    lRS = rhs_value_to_rhs_symbol(pRLAction->value);
    rhs->value = allocate_rhs_value_for_symbol(thisAgent, val_sym, lRS->inst_identity, NULL, was_unbound_vars.value);
    lRS = rhs_value_to_rhs_symbol(pRLAction->referent);
    rhs->referent = allocate_rhs_value_for_symbol(thisAgent, ref_sym, lRS->inst_identity, NULL, was_unbound_vars.referent);

    /* instantiate and allocate both increased refcount by 1.  Decrease one here.  Variablize may decrease also */
    thisAgent->symbolManager->symbol_remove_ref(&id_sym);
    thisAgent->symbolManager->symbol_remove_ref(&attr_sym);
    thisAgent->symbolManager->symbol_remove_ref(&val_sym);
    thisAgent->symbolManager->symbol_remove_ref(&ref_sym);

    if (ref_sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
    {
        initial_value = static_cast< double >(ref_sym->ic->value);
    }
    else if (ref_sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
    {
        initial_value = ref_sym->fc->value;
    } else {
        deallocate_action_list(thisAgent, rhs);
        return NULL;
    }

    tc_number lti_link_tc = get_new_tc_number(thisAgent);
    sti_variablize_rhs_symbol(rhs->id, false);
    sti_variablize_rhs_symbol(rhs->attr, false);
    sti_variablize_rhs_symbol(rhs->value, false);
    sti_variablize_rhs_symbol(rhs->referent, false);

    return rhs;
}

void Explanation_Based_Chunker::variablize_rl_test(test pTest)
{
    cons* c;
    test tt;

    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (c = pTest->data.conjunct_list; c != NIL; )
        {
            tt = reinterpret_cast<test>(c->first);
            if (test_has_referent(tt) && tt->data.referent->is_sti())
            {
                sti_variablize_test(tt, false);
            }
            c = c->rest;
        }
    }
    else
    {
        if (test_has_referent(pTest) && pTest->data.referent->is_sti())
        {
            sti_variablize_test(pTest, false);
        }
    }
}


void Explanation_Based_Chunker::variablize_rl_condition_list(condition* top_cond)
{
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if ((cond->type == POSITIVE_CONDITION) || (cond->type == NEGATIVE_CONDITION))
        {
            variablize_rl_test(cond->data.tests.id_test);
            variablize_rl_test(cond->data.tests.attr_test);
            variablize_rl_test(cond->data.tests.value_test);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            variablize_rl_condition_list(cond->data.ncc.top);
        }
    }
}
