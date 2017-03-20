/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
//#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "rete.h"
//#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
//#include "working_memory.h"

#include <assert.h>

/* Methods for generating variable identities during instantiation creation */

uint64_t Explanation_Based_Chunker::get_or_create_identity_for_sym(Symbol* pSym)
{
    int64_t existing_o_id = 0;

    auto iter_sym = instantiation_identities->find(pSym);
    if (iter_sym != instantiation_identities->end())
    {
        existing_o_id = iter_sym->second;
    }

    if (!existing_o_id)
    {
        increment_counter(variablization_identity_counter);
        (*instantiation_identities)[pSym] = variablization_identity_counter;
        return variablization_identity_counter;
    }
    return existing_o_id;
}

void Explanation_Based_Chunker::force_add_identity(Symbol* pSym, uint64_t pID)
{
    if (pSym->is_sti()) (*instantiation_identities)[pSym] = pID;
}

void Explanation_Based_Chunker::add_identity_to_test(test pTest)
{
    if (!pTest->identity) pTest->identity = get_or_create_identity_for_sym(pTest->data.referent);
}

void Explanation_Based_Chunker::add_var_test_bound_identity_to_id_test(condition* cond, byte field_num, rete_node_level levels_up)
{
    test t = var_test_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    cond->data.tests.id_test->identity = t->identity;
}

void Explanation_Based_Chunker::add_explanation_to_condition(rete_node* node,
                                        condition* cond,
                                        node_varnames* nvn,
                                        ExplainTraceType ebcTraceType)
{
    if (ebcTraceType == WM_Trace_w_Inequalities)
    {
        add_explanation_to_RL_condition(node, cond);
        return;
    }

    rete_test* rt = node->b.posneg.other_tests;

    alpha_mem* am;
    am = node->b.posneg.alpha_mem_;

    dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition called for %s.\n%l\n",  thisAgent->newly_created_instantiations->prod_name->sc->name, cond);

    if (nvn)
    {
        add_varname_identity_to_test(thisAgent, nvn->data.fields.id_varnames, cond->data.tests.id_test);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.attr_varnames, cond->data.tests.attr_test);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.value_varnames, cond->data.tests.value_test);
    }
    if (!am->id && (!cond->data.tests.id_test->eq_test || !cond->data.tests.id_test->eq_test->data.referent->is_variable()) add_new_chunk_variable(&(cond->data.tests.id_test), 's');
    if (!am->attr && !cond->data.tests.attr_test->eq_test->data.referent->is_variable()) add_new_chunk_variable(&(cond->data.tests.attr_test), 'a');
    if (!am->value && !cond->data.tests.value_test->eq_test->data.referent->is_variable()) add_new_chunk_variable(&(cond->data.tests.value_test), first_letter_from_test(cond->data.tests.attr_test));

    /* --- on hashed nodes, add equality test for the hash function --- */
    if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
    {
        add_var_test_bound_identity_to_id_test(cond, node->left_hash_loc_field_num, node->left_hash_loc_levels_up);
    }
    else if (node->node_type == POSITIVE_BNODE)
    {
        add_var_test_bound_identity_to_id_test(cond, node->parent->left_hash_loc_field_num, node->parent->left_hash_loc_levels_up);
    }

    /* -- Now process any additional relational test -- */
    test chunk_test = NULL;
    TestType test_type;
    bool has_referent;
    for (; rt != NIL; rt = rt->next)
    {
        chunk_test = NULL;
        has_referent = true;
        if (rt->type ==DISJUNCTION_RETE_TEST)
        {
            chunk_test = make_test(thisAgent, NIL, DISJUNCTION_TEST);
            chunk_test->data.disjunction_list = thisAgent->symbolManager->copy_symbol_list_adding_references(rt->data.disjunction_list);
            has_referent = false;
        } else {
            if (test_is_constant_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
            }
            else if (test_is_variable_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond, rt->data.variable_referent.field_num, rt->data.variable_referent.levels_up);
                chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                chunk_test->identity = ref_test->identity;
            }
        }
        if (chunk_test)
        {
            if (rt->right_field_num == 0) add_constraint_to_explanation(&(cond->data.tests.id_test), chunk_test, has_referent);
            else if (rt->right_field_num == 1) add_constraint_to_explanation(&(cond->data.tests.attr_test), chunk_test, has_referent);
            else add_constraint_to_explanation(&(cond->data.tests.value_test), chunk_test, has_referent);
        }
    }

    dprint(DT_ADD_EXPLANATION_TRACE, "Final condition after add_explanation_to_condition: %l\n", cond);
}

void Explanation_Based_Chunker::add_new_chunk_variable(test* pTest, char pChar)
{
    if (!(*pTest) || !(*pTest)->eq_test)
    {
        char prefix[2];
        prefix[0] = pChar;
        prefix[1] = 0;
        add_test(thisAgent, pTest, make_test(thisAgent, thisAgent->symbolManager->generate_new_variable(prefix), EQUALITY_TEST));
    }
    (*pTest)->eq_test->identity = thisAgent->explanationBasedChunker->get_or_create_identity_for_sym((*pTest)->eq_test->data.referent);
    dprint(DT_ADD_EXPLANATION_TRACE, "add_varname_identity_to_test adding identity %u for newly generated chunk var %y\n", (*pTest)->eq_test->identity, (*pTest)->eq_test->data.referent);
}

/* -- This function adds a constraint test from the rete's other tests lists.  This function
 *    is odd because, although in most cases the rete only uses the other-tests list for
 *    tests that place additional constraints on the value, sometimes, for as of yet not
 *    understood reason, it can also store an equality test that contains the original rule
 *    symbol, which is normally stored in the varlist for the node.  This seems to happen for
 *    certain conditions that are linked more than one or two levels away from the state.  We
 *    need that symbol to assign an identity, so we look for that case here.
 * -- */

void Explanation_Based_Chunker::add_constraint_to_explanation(test* dest_test_address, test new_test, bool has_referent)
{
    if (has_referent)
    {
        // Handle case where relational test is equality test
        if ((*dest_test_address) && new_test && (new_test->type == EQUALITY_TEST))
        {
            test destination = *dest_test_address;
            if (destination->type == EQUALITY_TEST)
            {
                if (destination->data.referent == new_test->data.referent)
                {
                    if (!destination->identity && new_test->identity)
                    {
                        /* This is the special case */
                        destination->identity = new_test->identity;
                    }
                    deallocate_test(thisAgent, new_test);
                    return;
                } // else different referents and should be added as new test
            }
            else if (destination->type == CONJUNCTIVE_TEST)
            {
                if (destination->eq_test)
                {
                    if (destination->eq_test->data.referent == new_test->data.referent)
                    {
                        if (!destination->eq_test->identity && new_test->identity)
                        {
                            /* This is the special case */
                            destination->eq_test->identity = new_test->identity;
                            return;
                        }
                        deallocate_test(thisAgent, new_test);
                    }
                }
            }
        }
    }
    add_test(thisAgent, dest_test_address, new_test);
}

void Explanation_Based_Chunker::add_explanation_to_RL_condition(rete_node* node, condition* cond)
{
        rete_test* rt = node->b.posneg.other_tests;
        dprint(DT_RL_VARIABLIZATION, "Adding explanation to RL condition %l\n", cond);

        test chunk_test = NULL;
        TestType test_type;
        bool has_referent;
        for (; rt != NIL; rt = rt->next)
        {
            dprint(DT_RL_VARIABLIZATION, "...found rete tests for RL condition");
            chunk_test = NULL;
            has_referent = true;
            if (test_is_variable_relational_test(rt->type))
            {
                dprint(DT_RL_VARIABLIZATION, "...variable relational");
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                if ((test_type == EQUALITY_TEST) || (test_type == NOT_EQUAL_TEST))
                {
                    dprint(DT_RL_VARIABLIZATION, "...equality or not equal");
                    test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond, rt->data.variable_referent.field_num, rt->data.variable_referent.levels_up);
                    if (ref_test->data.referent->is_sti() || ref_test->data.referent->is_variable())
                    {
                        dprint(DT_RL_VARIABLIZATION, "...STI or variable...Adding!");
                        chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                    }
                }
            }
            dprint(DT_RL_VARIABLIZATION, "\n");
            if (chunk_test)
            {
                if (rt->right_field_num == 0) add_constraint_to_explanation(&(cond->data.tests.id_test), chunk_test, has_referent);
                else if (rt->right_field_num == 1) add_constraint_to_explanation(&(cond->data.tests.attr_test), chunk_test, has_referent);
                else add_constraint_to_explanation(&(cond->data.tests.value_test), chunk_test, has_referent);
            }
        }
        dprint(DT_RL_VARIABLIZATION, "Final RL condition with explanation: %l\n", cond);

}

