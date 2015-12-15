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
#include <assert.h>
#include <ebc_variablize.h>
#include "test.h"
#include "debug.h"
#include "kernel.h"
#include "symtab.h"
#include "agent.h"
#include "print.h"
#include "rete.h"
#include "instantiations.h"
#include "output_manager.h"
#include "wmem.h"
#include "prefmem.h"

void EBC_Manager::add_identity_to_original_id_test(condition* cond,
                                       byte field_num,
                                       rete_node_level levels_up)
{
//    Symbol* temp;
	test t = 0;//, New = 0;

    t = var_test_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    cond->data.tests.id_test->identity = t->identity;
    dprint(DT_ADD_ADDITIONALS, "add_hash_info_to_original_id_test added o_id o%u(%y) from %t.\n",
        cond->data.tests.id_test->identity,
        thisAgent->variablizationManager->get_ovar_for_o_id(cond->data.tests.id_test->identity),
        t);
}


/* ----------------------------------------------------------------------
                 add_additional_tests_and_originals

   These two functions create an explanation trace for used by EBC.
   It does this by annotating individual tests with variablization identity
   information and adding any additional in the original rule that was
   fired to the appropriate tests.

---------------------------------------------------------------------- */
void EBC_Manager::explain_RL_condition(rete_node* node,
    condition* cond,
    wme* w,
    node_varnames* nvn,
    uint64_t pI_id,
    AddAdditionalTestsMode additional_tests)
{

    Symbol* referent = NULL;
    test chunk_test = NULL;
    TestType test_type;
    rete_test* rt = node->b.posneg.other_tests;

    /* --- Store original referent information.  Note that sometimes the
     *     original referent equality will be stored in the beta nodes extra tests
     *     data structure rather than the alpha memory --- */
    dprint(DT_ADD_ADDITIONALS, "-=-=-=-=-=-\n");
    dprint(DT_ADD_ADDITIONALS, "add_inequalities called for rl instantiation %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
    dprint(DT_ADD_ADDITIONALS, "%l\n", cond);

    /* -- Now process any additional relational test -- */
    dprint(DT_ADD_ADDITIONALS, "Processing additional tests to add to condition %l...\n", cond);
    for (; rt != NIL; rt = rt->next)
    {
        dprint(DT_ADD_ADDITIONALS, "Processing additional test...\n");
        chunk_test = NULL;
        if (test_is_constant_relational_test(rt->type))
        {
            dprint(DT_ADD_ADDITIONALS, "Creating constant relational test.\n");
            test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
            chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
        }
        else if (test_is_variable_relational_test(rt->type))
        {
            test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
            /* We may need to add equality tests if we start variablizing STIs like
             * constants, because we'd need the identity from the equality test processed
             * as a relational here because of weird rete case. Then again, we would need to
             * generate identities in this function if we had to do that.  Right now, identities
             * are not needed for RL because we only use them to variablize constants and RL
             * only variablizes STis.*/
            //if ((test_type == EQUALITY_TEST) || (test_type == NOT_EQUAL_TEST))
            if (test_type == NOT_EQUAL_TEST)
            {
                dprint(DT_ADD_ADDITIONALS, "Creating variable relational rl test.\n");

                test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                    rt->data.variable_referent.field_num,
                    rt->data.variable_referent.levels_up);
                referent = ref_test->data.referent;
                if(referent->is_identifier())
                {
                    chunk_test = make_test(thisAgent, referent, test_type);
                    dprint(DT_RL_VARIABLIZATION, "Creating valid relational test for template %t [%g].\n", chunk_test, chunk_test);
                }
                else
                {
                    dprint(DT_RL_VARIABLIZATION, "Relational test referent is not an STI.  Ignoring.\n");
                }
            }
            else
            {
                dprint(DT_RL_VARIABLIZATION, "Relational test type is not a valid template relational test.  Ignoring.\n");
            }
        }
        if (chunk_test)
        {
            if (rt->right_field_num == 0)
            {
                explain_constraint(&(cond->data.tests.id_test), chunk_test, pI_id);

                dprint(DT_ADD_ADDITIONALS, "Added relational test to id element resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);
            }
            else if (rt->right_field_num == 1)
            {
                explain_constraint(&(cond->data.tests.attr_test), chunk_test, pI_id);

                dprint(DT_ADD_ADDITIONALS, "Added relational test to attribute element resulting in: %t [%g]\n", cond->data.tests.attr_test, cond->data.tests.attr_test);
            }
            else
            {
                explain_constraint(&(cond->data.tests.value_test), chunk_test, pI_id);

                dprint(DT_ADD_ADDITIONALS, "Added relational test to value element resulting in: %t [%g]\n", cond->data.tests.value_test, cond->data.tests.value_test);
            }
        }

    }

    dprint(DT_ADD_ADDITIONALS, "add_additional_tests_and_originals finished for %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
    dprint(DT_ADD_ADDITIONALS, "Final test after add_additional_tests and creating identity: %l\n", cond);
}

void EBC_Manager::explain_condition(rete_node* node,
                                        condition* cond,
                                        wme* w,
                                        node_varnames* nvn,
                                        uint64_t pI_id,
                                        AddAdditionalTestsMode additional_tests)
{
    if (additional_tests == JUST_INEQUALITIES)
    {
        explain_RL_condition(node, cond, w, nvn, pI_id, additional_tests);
        return;
    }

    if (!m_learning_on) return;
    rete_test* rt = node->b.posneg.other_tests;

    /* --- Store original referent information.  Note that sometimes the
     *     original referent equality will be stored in the beta nodes extra tests
     *     data structure rather than the alpha memory --- */
    alpha_mem* am;
    am = node->b.posneg.alpha_mem_;

    dprint(DT_ADD_ADDITIONALS, "-=-=-=-=-=-\n");
    dprint(DT_ADD_ADDITIONALS, "add_constraints_and_identities called for %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
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
        add_identity_to_original_id_test(cond,
            node->left_hash_loc_field_num,
            node->left_hash_loc_levels_up);
        dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

    }
    else if (node->node_type == POSITIVE_BNODE)
    {
        dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for POSITIVE_BNODE...\n");
        add_identity_to_original_id_test(cond,
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
            chunk_test->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, rt->data.disjunction_list);
            has_referent = false;
        } else {
            if (test_is_constant_relational_test(rt->type))
            {
                dprint(DT_ADD_ADDITIONALS, "Creating constant relational test.\n");
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
            }
            else if (test_is_variable_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                dprint(DT_ADD_ADDITIONALS, "Creating variable relational test.\n");

                test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                    rt->data.variable_referent.field_num,
                    rt->data.variable_referent.levels_up);
                chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                chunk_test->identity = ref_test->identity;
                dprint(DT_ADD_ADDITIONALS, "Created relational test for chunk: %t [%g].\n", chunk_test, chunk_test);
            }
        }
        if (chunk_test)
        {
            if (rt->right_field_num == 0)
            {
                explain_constraint(&(cond->data.tests.id_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to id element resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);
            }
            else if (rt->right_field_num == 1)
            {
                explain_constraint(&(cond->data.tests.attr_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to attribute element resulting in: %t [%g]\n", cond->data.tests.attr_test, cond->data.tests.attr_test);
            }
            else
            {
                explain_constraint(&(cond->data.tests.value_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to value element resulting in: %t [%g]\n", cond->data.tests.value_test, cond->data.tests.value_test);
            }
        }
    }

    dprint(DT_ADD_ADDITIONALS, "Final test after add_constraints_and_identities: %l\n", cond);
}

/* -- This function is a special purpose function for extracting information for the explanation
 *    trace from the rete's other tests lists.  The function is odd because the rete can store
 *    more than just constraints in its other tests lists.  It can also store anan equality test
 *    that contains the original symbol name, which we use to assign a variablization identity.
 *    Normally, that information is stored in the varlist, but for certain condtitions that are
 *    one or two levels deep, it seems to propagate those varnames down via its other test structure.
 * -- */

//void add_relational_test(agent* thisAgent, test* dest_test_address, test new_test, uint64_t pI_id, bool has_referent = true)
void EBC_Manager::explain_constraint(test* dest_test_address, test new_test, uint64_t pI_id, bool has_referent)
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
                            destination, thisAgent->variablizationManager->get_ovar_for_o_id(destination->identity));
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
                                    thisAgent->variablizationManager->get_ovar_for_o_id(check_test->identity));
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
