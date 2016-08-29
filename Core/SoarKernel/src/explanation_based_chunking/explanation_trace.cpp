/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =======================================================================
 *                    Test Utilities
 * This file contains various utility routines for tests.
 *
 * =======================================================================
 */
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "rete.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"

#include <assert.h>

void Explanation_Based_Chunker::add_identity_to_id_test(condition* cond,
                                       byte field_num,
                                       rete_node_level levels_up)
{
    test t = 0;

    t = var_test_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    cond->data.tests.id_test->identity = t->identity;
    dprint(DT_ADD_ADDITIONALS, "add_hash_info_to_original_id_test added o_id o%u(%y) from %t.\n",
        cond->data.tests.id_test->identity,
        thisAgent->explanationBasedChunker->get_ovar_for_o_id(cond->data.tests.id_test->identity),
        t);
}

void Explanation_Based_Chunker::add_explanation_to_RL_condition(rete_node* node,
    condition* cond,
    node_varnames* nvn,
    uint64_t pI_id,
    AddAdditionalTestsMode additional_tests)
{
        rete_test* rt = node->b.posneg.other_tests;

        /* --- Store original referent information.  Note that sometimes the
         *     original referent equality will be stored in the beta nodes extra tests
         *     data structure rather than the alpha memory --- */
        alpha_mem* am;
        am = node->b.posneg.alpha_mem_;

        dprint(DT_ADD_ADDITIONALS, "-=-=-=-=-=-\n");
        dprint(DT_ADD_ADDITIONALS, "add_explanation_to_RL_condition called for %s.\n",
               thisAgent->newly_created_instantiations->prod_name->sc->name);
        dprint(DT_ADD_ADDITIONALS, "%l\n", cond);
        dprint(DT_ADD_ADDITIONALS, "AM: (%y ^%y %y)\n", am->id , am->attr, am->value);

        if (nvn)
        {
            dprint(DT_ADD_ADDITIONALS, "adding var names node to original tests:\n");
            dprint_varnames_node(DT_ADD_ADDITIONALS, nvn);
            add_varname_identity_to_test(thisAgent, nvn->data.fields.id_varnames, cond->data.tests.id_test, pI_id, true);
            add_varname_identity_to_test(thisAgent, nvn->data.fields.attr_varnames, cond->data.tests.attr_test, pI_id, true);
            add_varname_identity_to_test(thisAgent, nvn->data.fields.value_varnames, cond->data.tests.value_test, pI_id, true);
            dprint(DT_ADD_ADDITIONALS, "Done adding var names to original tests resulting in: %l\n", cond);
        }

        /* --- on hashed nodes, add equality test for the hash function --- */
        if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
        {
            dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for MP_BNODE or NEGATIVE_BNODE...\n");
            add_identity_to_id_test(cond,
                node->left_hash_loc_field_num,
                node->left_hash_loc_levels_up);
            dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

        }
        else if (node->node_type == POSITIVE_BNODE)
        {
            dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for POSITIVE_BNODE...\n");
            add_identity_to_id_test(cond,
                node->parent->left_hash_loc_field_num,
                node->parent->left_hash_loc_levels_up);
            dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

        }

        /* -- Now process any additional relational test -- */
        dprint(DT_ADD_ADDITIONALS, "Processing additional tests to add to condition %l...\n", cond);

        test chunk_test = NULL;
        TestType test_type;
        bool has_referent;
        for (; rt != NIL; rt = rt->next)
        {
            chunk_test = NULL;
            has_referent = true;
            if (test_is_variable_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                if ((test_type == EQUALITY_TEST) || (test_type == NOT_EQUAL_TEST))
                {
                    dprint(DT_ADD_ADDITIONALS, "Creating variable relational test.\n");

                    test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                        rt->data.variable_referent.field_num,
                        rt->data.variable_referent.levels_up);
                    if (ref_test->data.referent->is_identifier())
                    {
                        chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                        chunk_test->identity = ref_test->identity;
                        dprint(DT_ADD_ADDITIONALS, "Created relational test for chunk: %t [%g].\n", chunk_test, chunk_test);
                    }
                }
            }
            if (chunk_test)
            {
                if (rt->right_field_num == 0)
                {
                    add_constraint_to_explanation(&(cond->data.tests.id_test), chunk_test, pI_id, has_referent);
                    dprint(DT_ADD_ADDITIONALS, "Added relational test to id element resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);
                }
                else if (rt->right_field_num == 1)
                {
                    add_constraint_to_explanation(&(cond->data.tests.attr_test), chunk_test, pI_id, has_referent);
                    dprint(DT_ADD_ADDITIONALS, "Added relational test to attribute element resulting in: %t [%g]\n", cond->data.tests.attr_test, cond->data.tests.attr_test);
                }
                else
                {
                    add_constraint_to_explanation(&(cond->data.tests.value_test), chunk_test, pI_id, has_referent);
                    dprint(DT_ADD_ADDITIONALS, "Added relational test to value element resulting in: %t [%g]\n", cond->data.tests.value_test, cond->data.tests.value_test);
                }
            }
        }

        dprint(DT_ADD_ADDITIONALS, "Final test after add_constraints_and_identities: %l\n", cond);
}

void Explanation_Based_Chunker::add_explanation_to_condition(rete_node* node,
                                        condition* cond,
                                        node_varnames* nvn,
                                        uint64_t pI_id,
                                        AddAdditionalTestsMode additional_tests)
{
    if (additional_tests == JUST_INEQUALITIES)
    {
        add_explanation_to_RL_condition(node, cond, nvn, pI_id, additional_tests);
        return;
    }

    if (!ebc_settings[SETTING_EBC_LEARNING_ON]) return;

    rete_test* rt = node->b.posneg.other_tests;

    /* --- Store original referent information.  Note that sometimes the
     *     original referent equality will be stored in the beta nodes extra tests
     *     data structure rather than the alpha memory --- */
    alpha_mem* am;
    am = node->b.posneg.alpha_mem_;

    dprint(DT_ADD_ADDITIONALS, "-=-=-=-=-=-\n");
    dprint(DT_ADD_ADDITIONALS, "add_constraints_and_identities called for %s.\n",
           thisAgent->newly_created_instantiations->prod_name->sc->name);
    dprint(DT_ADD_ADDITIONALS, "%l\n", cond);
    dprint(DT_ADD_ADDITIONALS, "AM: (%y ^%y %y)\n", am->id , am->attr, am->value);

    if (nvn)
    {
        dprint(DT_ADD_ADDITIONALS, "adding var names node to original tests:\n");
        dprint_varnames_node(DT_ADD_ADDITIONALS, nvn);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.id_varnames, cond->data.tests.id_test, pI_id);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.attr_varnames, cond->data.tests.attr_test, pI_id);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.value_varnames, cond->data.tests.value_test, pI_id);
        dprint(DT_ADD_ADDITIONALS, "Done adding var names to original tests resulting in: %l\n", cond);
    }

    /* --- on hashed nodes, add equality test for the hash function --- */
    if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
    {
        dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for MP_BNODE or NEGATIVE_BNODE...\n");
        add_identity_to_id_test(cond,
            node->left_hash_loc_field_num,
            node->left_hash_loc_levels_up);
        dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

    }
    else if (node->node_type == POSITIVE_BNODE)
    {
        dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for POSITIVE_BNODE...\n");
        add_identity_to_id_test(cond,
            node->parent->left_hash_loc_field_num,
            node->parent->left_hash_loc_levels_up);
        dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

    }

    /* -- Now process any additional relational test -- */
    dprint(DT_ADD_ADDITIONALS, "Processing additional tests to add to condition %l...\n", cond);

    test chunk_test = NULL;
    TestType test_type;
    bool has_referent;
    for (; rt != NIL; rt = rt->next)
    {
        chunk_test = NULL;
        has_referent = true;
        if (rt->type ==DISJUNCTION_RETE_TEST)
        {
            dprint(DT_ADD_ADDITIONALS, "Creating disjunction test.\n");
            chunk_test = make_test(thisAgent, NIL, DISJUNCTION_TEST);
            chunk_test->data.disjunction_list = thisAgent->symbolManager->copy_symbol_list_adding_references(rt->data.disjunction_list);
            has_referent = false;
        } else {
            if (test_is_constant_relational_test(rt->type))
            {
                dprint(DT_ADD_ADDITIONALS, "Creating constant relational test.\n");
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                if (test_type == SMEM_LINK_TEST)
                {
                    chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
                } else {
                    chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
                }
            }
            else if (test_is_variable_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                dprint(DT_ADD_ADDITIONALS, "Creating variable relational test.\n");
                if (test_type == SMEM_LINK_TEST)
                {
                    test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                        rt->data.variable_referent.field_num,
                        rt->data.variable_referent.levels_up);
                    chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                    chunk_test->identity = ref_test->identity;
                    dprint(DT_ADD_ADDITIONALS, "Created relational test for chunk: %t [%g].\n", chunk_test, chunk_test);
                } else {
                    test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                        rt->data.variable_referent.field_num,
                        rt->data.variable_referent.levels_up);
                    chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                    chunk_test->identity = ref_test->identity;
                    dprint(DT_ADD_ADDITIONALS, "Created relational test for chunk: %t [%g].\n", chunk_test, chunk_test);
                }
            }
        }
        if (chunk_test)
        {
            if (rt->right_field_num == 0)
            {
                add_constraint_to_explanation(&(cond->data.tests.id_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to id element resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);
            }
            else if (rt->right_field_num == 1)
            {
                add_constraint_to_explanation(&(cond->data.tests.attr_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to attribute element resulting in: %t [%g]\n", cond->data.tests.attr_test, cond->data.tests.attr_test);
            }
            else
            {
                add_constraint_to_explanation(&(cond->data.tests.value_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to value element resulting in: %t [%g]\n", cond->data.tests.value_test, cond->data.tests.value_test);
            }
        }
    }

    dprint(DT_ADD_ADDITIONALS, "Final test after add_constraints_and_identities: %l\n", cond);
}

/* -- This function adds a constraint test from the rete's other tests lists.  This function
 *    is odd because, although in most cases the rete only uses the other-tests list for
 *    tests that place additional constraints on the value, sometimes, for as of yet not
 *    understood reason, it can also store an equality test that contains the original rule
 *    symbol, which is normally stored in the varlist for the node.  This seems to happen for
 *    certain conditions that are linked more than one or two levels away from the state.  We
 *    need that symbol to assign an identity, so we look for that case here.
 * -- */

void Explanation_Based_Chunker::add_constraint_to_explanation(test* dest_test_address, test new_test, uint64_t pI_id, bool has_referent)
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
                        dprint(DT_IDENTITY_PROP, "Copying identity to equality test for add_relational_test special case %t: %y\n",
                            destination, thisAgent->explanationBasedChunker->get_ovar_for_o_id(destination->identity));
                        deallocate_test(thisAgent, new_test);
                        return;
                    }
                    else
                    {
                        /* Identical referents and possibly identical originals.  Ignore. */
                        return;
                    }
                } // else different referents and should be added as new test
            }
            else if (destination->type == CONJUNCTIVE_TEST)
            {
                cons* c;
                test check_test;
                for (c = destination->data.conjunct_list; c != NIL; c = c->rest)
                {
                    check_test = static_cast<test>(c->first);
                    if (check_test->type == EQUALITY_TEST)
                    {
                        if (check_test->data.referent == new_test->data.referent)
                        {
                            if (!check_test->identity && new_test->identity)
                            {
                                /* This is the special case */
                                check_test->identity = new_test->identity;
                                dprint(DT_IDENTITY_PROP, "Copying identity to equality test for add_relational_test special case %t: %s\n", check_test,
                                    thisAgent->explanationBasedChunker->get_ovar_for_o_id(check_test->identity));
                                deallocate_test(thisAgent, new_test);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    add_test(thisAgent, dest_test_address, new_test);
}
