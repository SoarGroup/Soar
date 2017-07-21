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

#include "assert.hpp"

/* Methods for generating variable identities during instantiation creation */

uint64_t Explanation_Based_Chunker::get_or_create_inst_identity_for_sym(Symbol* pSym)
{
    int64_t existing_o_id = 0;

    auto iter_sym = instantiation_identities->find(pSym);
    if (iter_sym != instantiation_identities->end())
    {
        existing_o_id = iter_sym->second;
    }

    if (!existing_o_id)
    {
        increment_counter(inst_identity_counter);
        (*instantiation_identities)[pSym] = inst_identity_counter;
        return inst_identity_counter;
    }
    return existing_o_id;
}

void Explanation_Based_Chunker::force_add_inst_identity(Symbol* pSym, uint64_t pID)
{
    if (pSym->is_sti()) (*instantiation_identities)[pSym] = pID;
}

void Explanation_Based_Chunker::add_inst_identity_to_test(test pTest)
{
    if (!pTest->inst_identity) pTest->inst_identity = get_or_create_inst_identity_for_sym(pTest->data.referent);
}

void Explanation_Based_Chunker::add_var_test_bound_identity_to_id_test(condition* cond, byte field_num, rete_node_level levels_up)
{
    test t = var_test_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    cond->data.tests.id_test->inst_identity = t->inst_identity;
    dprint(DT_ADD_EXPLANATION_TRACE, "add_var_test_bound_identity_to_id_test adding identity %u to id test of cond %l\n", t->inst_identity, cond);
}

void Explanation_Based_Chunker::add_explanation_to_condition(rete_node* node,
                                        condition* cond,
                                        node_varnames* nvn,
                                        ExplainTraceType ebcTraceType,
                                        bool inNegativeNodes)
{
    if (ebcTraceType == WM_Trace_w_Inequalities)
    {
        add_explanation_to_RL_condition(node, cond);
        return;
    }

    test        chunk_test, ref_test;
    TestType    test_type;
    bool        has_referent;
    alpha_mem*  am = node->b.posneg.alpha_mem_;
    rete_test*  rt = node->b.posneg.other_tests;

    dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition called for %s.\n%l\n",  thisAgent->newly_created_instantiations->prod_name->sc->name, cond);

    /* For hashed nodes, copy identity from relative condition position --- */
    if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
    {
        add_var_test_bound_identity_to_id_test(cond, node->left_hash_loc_field_num, node->left_hash_loc_levels_up);
    }
    else if (node->node_type == POSITIVE_BNODE)
    {
        add_var_test_bound_identity_to_id_test(cond, node->parent->left_hash_loc_field_num, node->parent->left_hash_loc_levels_up);
    }

    bool needs_eq_tests = (
        (!am->id && cond->data.tests.id_test && !cond->data.tests.id_test->inst_identity && !cond->data.tests.id_test->data.referent->is_variable()) ||
        (!am->attr && cond->data.tests.attr_test && !cond->data.tests.attr_test->inst_identity && !cond->data.tests.attr_test->data.referent->is_variable()) ||
        (!am->value && cond->data.tests.value_test && !cond->data.tests.value_test->inst_identity && !cond->data.tests.value_test->data.referent->is_variable()));

    if (needs_eq_tests)
    {
        /* -- Now process any additional relational test -- */
        for (; rt != NIL; rt = rt->next)
        {
            if (test_is_variable_relational_test(rt->type) && (relational_test_type_to_test_type(kind_of_relational_test(rt->type)) == EQUALITY_TEST))
            {
                ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond, rt->data.variable_referent.field_num, rt->data.variable_referent.levels_up);

                if (rt->right_field_num == 0)
                {
                    dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition adding identity %u for non-variable from relational test %t\n", ref_test->inst_identity, cond->data.tests.id_test);
                    cond->data.tests.id_test->inst_identity = ref_test->inst_identity;
                }
                else if (rt->right_field_num == 1)
                {
                    dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition adding identity %u for non-variable from relational test %t\n", ref_test->inst_identity, cond->data.tests.attr_test);
                    cond->data.tests.attr_test->inst_identity = ref_test->inst_identity;
                }
                else
                {
                    dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition adding identity %u for non-variable from relational test  %t\n", ref_test->inst_identity, cond->data.tests.value_test);
                    cond->data.tests.value_test->inst_identity = ref_test->inst_identity;
                }
            }
        }

        /* Check for non-constants that still don't have an identity */
        if (!am->id && cond->data.tests.id_test && !cond->data.tests.id_test->inst_identity && !cond->data.tests.id_test->data.referent->is_variable())
        {
            cond->data.tests.id_test->inst_identity = thisAgent->explanationBasedChunker->get_new_inst_identity_id();
            dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition adding identity %u for non-variable with alpha memory %t\n", cond->data.tests.id_test->inst_identity, cond->data.tests.id_test);
        }
        if (!am->attr && cond->data.tests.attr_test && !cond->data.tests.attr_test->inst_identity && !cond->data.tests.attr_test->data.referent->is_variable())
        {
            cond->data.tests.attr_test->inst_identity = thisAgent->explanationBasedChunker->get_new_inst_identity_id();
            dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition adding identity %u for non-variable with alpha memory %t\n", cond->data.tests.attr_test->inst_identity, cond->data.tests.attr_test);
        }
        if (!am->value && cond->data.tests.value_test && !cond->data.tests.value_test->inst_identity && !cond->data.tests.value_test->data.referent->is_variable())
        {
            cond->data.tests.value_test->inst_identity = thisAgent->explanationBasedChunker->get_new_inst_identity_id();
            dprint(DT_ADD_EXPLANATION_TRACE, "add_explanation_to_condition adding identity %u for non-variable with alpha memory %t\n", cond->data.tests.value_test->inst_identity, cond->data.tests.value_test);
        }
    }

    /* -- Now process any additional relational test -- */
    rt = node->b.posneg.other_tests;
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
            test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
            if (test_is_constant_relational_test(rt->type))
            {
                chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
            }
            else if (test_is_variable_relational_test(rt->type))
            {
                ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond, rt->data.variable_referent.field_num, rt->data.variable_referent.levels_up);
                chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                chunk_test->inst_identity = ref_test->inst_identity;
            }
        }
        if (chunk_test)
        {
            if (rt->right_field_num == 0) add_constraint_to_explanation(&(cond->data.tests.id_test), chunk_test, has_referent);
            else if (rt->right_field_num == 1) add_constraint_to_explanation(&(cond->data.tests.attr_test), chunk_test, has_referent);
            else add_constraint_to_explanation(&(cond->data.tests.value_test), chunk_test, has_referent);
        }
    }

    /* Make sure there's an equality test in each of the three fields.  These are necessary for NCCs and NCs */
    if (! nvn)
    {
        if (!cond->data.tests.id_test || !cond->data.tests.id_test->eq_test)
        {
            add_new_chunk_variable(&(cond->data.tests.id_test), 's', inNegativeNodes);
        }
        if (!cond->data.tests.attr_test || !cond->data.tests.attr_test->eq_test)
        {
            add_new_chunk_variable(&(cond->data.tests.attr_test), 'a', inNegativeNodes);
        }
        if (!cond->data.tests.value_test || !cond->data.tests.value_test->eq_test)
        {
            add_new_chunk_variable(&(cond->data.tests.value_test), first_letter_from_test(cond->data.tests.attr_test), inNegativeNodes);
        }
    }

    dprint(DT_ADD_EXPLANATION_TRACE, "Final condition after add_explanation_to_condition: %l\n", cond);
}

void Explanation_Based_Chunker::add_new_chunk_variable(test* pTest, char pChar, bool inNegativeNodes)
{
    char prefix[2];
    prefix[0] = pChar;
    prefix[1] = 0;
    Symbol* newVar = thisAgent->symbolManager->generate_new_variable(prefix);
    test newTest = make_test(thisAgent, newVar, EQUALITY_TEST);
    thisAgent->symbolManager->symbol_remove_ref(&newVar);
    if (*pTest) add_test(thisAgent, pTest, newTest);
    else *pTest = newTest;
    (*pTest)->eq_test->inst_identity = LITERAL_VALUE;
}

void Explanation_Based_Chunker::add_constraint_to_explanation(test* dest_test_address, test new_test, bool has_referent)
{
    if (has_referent)
    {
        if ((*dest_test_address) && new_test && (new_test->type == EQUALITY_TEST))
        {
            test destination = *dest_test_address;
            if (((destination->type == EQUALITY_TEST) && (destination->data.referent == new_test->data.referent)) ||
                ((destination->type == CONJUNCTIVE_TEST) && (destination->eq_test->data.referent == new_test->data.referent)))
            {
                dprint(DT_ADD_EXPLANATION_TRACE, "add_constraint_to_explanation found already processed equality test %t[%u] for %t\n", new_test, new_test->inst_identity, destination->eq_test);
                deallocate_test(thisAgent, new_test);
                return;
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

