/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "prefmem.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "wmem.h"
#include "debug.h"


void Variablization_Manager::unify_identity(agent* thisAgent, test t)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*unification_map).find(t->identity);
    if (iter != (*unification_map).end())
    {
        dprint(DT_UNIFICATION, "...found variablization unification o%u -> o%u\n",
            t->identity, iter->second);

        t->identity = iter->second;
    }
}
void Variablization_Manager::unify_identity_for_result_element(agent* thisAgent, preference* result, WME_Field field)
{

    uint64_t lO_id = 0;

    if (field == ID_ELEMENT)
    {
        lO_id = result->o_ids.id;
    } else if (field == ATTR_ELEMENT) {
        lO_id = result->o_ids.attr;
    } else if (field == VALUE_ELEMENT) {
        lO_id = result->o_ids.value;
    }
    std::map< uint64_t, uint64_t >::iterator iter = (*unification_map).find(lO_id);
    if (iter != (*unification_map).end())
    {
        dprint(DT_UNIFICATION, "...found variablization unification o%u (%y) -> o%u (%y)\n",
            lO_id, get_ovar_for_o_id(lO_id), iter->second, get_ovar_for_o_id(iter->second));

        lO_id = iter->second;
        if (field == ID_ELEMENT)
        {
            result->o_ids.id = lO_id;
        } else if (field == ATTR_ELEMENT) {
            result->o_ids.attr = lO_id;
        } else if (field == VALUE_ELEMENT) {
            result->o_ids.value = lO_id;
        }
    }
}

void Variablization_Manager::unify_identities_for_results(preference* result)
{

    if (!result) return;

    dprint(DT_UNIFICATION, "Fixing result %p\n", result);
    dprint_o_id_tables(DT_UNIFICATION);

    if (result->o_ids.id)
    {
        unify_identity_for_result_element(thisAgent, result, ID_ELEMENT);
    }
    if (result->o_ids.attr)
    {
        unify_identity_for_result_element(thisAgent, result, ATTR_ELEMENT);
    }
    if (result->o_ids.value)
    {
        unify_identity_for_result_element(thisAgent, result, VALUE_ELEMENT);
    }
    unify_identities_for_results(result->next_result);
}

void Variablization_Manager::update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id, uint64_t pOld_o_id_2)
{
    std::map< uint64_t, uint64_t >::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {

        if ((iter->second == pOld_o_id) || (pOld_o_id_2 && (iter->second == pOld_o_id_2)))
        {
            dprint(DT_UNIFICATION, "...found secondary o_id unification mapping that needs updated: o%u = o%u -> o%u = o%u.\n", iter->first, iter->second, iter->first, pNew_o_id );
            (*unification_map)[iter->first] = pNew_o_id;
        }
    }
}

void Variablization_Manager::add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    std::map< uint64_t, uint64_t >::iterator iter;
    uint64_t newID;

    assert(pOld_o_id);
    if (pOld_o_id == pNew_o_id)
    {
        dprint(DT_UNIFICATION, "Attempting to unify identical conditions %y[o%u].  Skipping.\n", pNew_o_id);
        return;
    }
    if (pNew_o_id == 0)
    {
        /* Do not check if a unification already exists if we're propagating back a literalization */
        dprint(DT_UNIFICATION, "Adding literalization substitution in o_id to o_id_substitution_map for o%u.  Adding %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id);
        newID = 0;
    } else {
        /* See if a unification already exists for the new identity propagating back*/
        iter = (*unification_map).find(pNew_o_id);

        if (iter == (*unification_map).end())
        {
            /* Map all cases of this identity with its parent identity */
            dprint(DT_UNIFICATION, "Did not find o_id to o_id_substitution_map entry for o%u.  Adding %y[o%u] -> %y[o%u].\n", pNew_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            newID = pNew_o_id;
            dprint(DT_UNIFICATION, "Old identity propagation map:\n");
            dprint_o_id_substitution_map(DT_UNIFICATION);
        }
        else
        {
            /* Map all cases of what this identity is already remapped to with its parent identity */
            dprint(DT_UNIFICATION, "o_id unification (%y[o%u] -> %y[o%u]) already exists.  Adding transitive mapping %y[o%u] -> %y[o%u].\n",
                get_ovar_for_o_id(pNew_o_id), pNew_o_id, get_ovar_for_o_id(iter->second), iter->second,
                get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
            newID = iter->second;
        }
    }

    /* See if a unification already exists for the identity being replaced in this instantiation*/
    iter = (*unification_map).find(pOld_o_id);
    uint64_t existing_mapping;
    if (iter != (*unification_map).end())
    {
        existing_mapping = iter->second;
        if (existing_mapping == 0)
        {
            if (newID != 0)
            {
                /* The existing identity we're unifying with is already literalized from a different trace.  So,
                 * literalize any tests with identity of parent in this trace */
                dprint(DT_UNIFICATION, "Literalization exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
                (*unification_map)[newID] = 0;
                update_unification_table(newID, 0);
            } else {
                dprint(DT_UNIFICATION, "Literalizing something already literalized o%u.  Skipping %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            }
        } else {
            if (newID == 0)
            {
                /* The existing identity we're literalizing is already unified with another identity from
                 * a different trace.  So, literalize the identity, that it is already remapped to.*/
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(existing_mapping), existing_mapping);
                (*unification_map)[existing_mapping] = 0;
                update_unification_table(existing_mapping, 0, pOld_o_id);
            } else {
                /* The existing identity we're unifying with is already unified with another identity from
                 * a different trace.  So, unify the identity that it is already remapped to with identity
                 * of the parent in this trace */
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Adding %y[o%u] -> %y[o%u].\n", pOld_o_id, get_ovar_for_o_id(existing_mapping), existing_mapping, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
                (*unification_map)[pNew_o_id] = existing_mapping;
                update_unification_table(newID, existing_mapping);
            }
        }
    } else {
        (*unification_map)[pOld_o_id] = newID;
        update_unification_table(pOld_o_id, newID);
    }

    /* Unify identity in this instantiation with final identity */
    dprint(DT_UNIFICATION, "New identity propagation map:\n");
    dprint_o_id_substitution_map(DT_UNIFICATION);
}

bool Variablization_Manager::unify_backtraced_dupe_conditions(condition* ground_cond, condition* new_cond)
{
    dprint(DT_IDENTITY_PROP, "Adding identity mappings for dupe match condition: %l from %l\n", new_cond, ground_cond);
    test new_cond_id, new_cond_attr, new_cond_value, ground_cond_id, ground_cond_attr, ground_cond_value;

    new_cond_id = new_cond_attr = new_cond_value = NULL;
    if (new_cond->data.tests.id_test->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
        return false;
        new_cond_id = equality_test_found_in_test(new_cond->data.tests.id_test);
    } else {
        new_cond_id = new_cond->data.tests.id_test;
        assert(new_cond_id->type == EQUALITY_TEST);
    }
    if (new_cond->data.tests.attr_test->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
        return false;
        new_cond_attr = equality_test_found_in_test(new_cond->data.tests.attr_test);
    } else {
        new_cond_attr = new_cond->data.tests.attr_test;
        assert(new_cond_attr->type == EQUALITY_TEST);
    }
    if (new_cond->data.tests.value_test->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
        return false;
        new_cond_value = equality_test_found_in_test(new_cond->data.tests.value_test);
    } else {
        new_cond_value = new_cond->data.tests.value_test;
        assert(new_cond_value->type == EQUALITY_TEST);
    }

    if (!new_cond_id && !new_cond_attr && !new_cond_value)
    {
        dprint(DT_IDENTITY_PROP, "Condition has additional constraints.  Not unifying.\n");
        return false;
    }

    bool mismatched_literal = false;
    ground_cond_id = equality_test_found_in_test(ground_cond->data.tests.id_test);
    ground_cond_attr = equality_test_found_in_test(ground_cond->data.tests.attr_test);
    ground_cond_value = equality_test_found_in_test(ground_cond->data.tests.value_test);

    if (!new_cond_id->identity || !ground_cond_id->identity)
    {
        if (new_cond_id->identity != ground_cond_id->identity)
        {
            mismatched_literal = true;
        }
    }
    if (!mismatched_literal && (!new_cond_attr->identity || !ground_cond_attr->identity))
    {
        if (new_cond_attr->identity != ground_cond_attr->identity)
        {
            mismatched_literal = true;
        }
    }
    if (!mismatched_literal && (!new_cond_value->identity || !ground_cond_value->identity))
    {
        if (new_cond_value->identity != ground_cond_value->identity)
        {
            mismatched_literal = true;
        }
    }


    if (mismatched_literal)
    {
        dprint(DT_IDENTITY_PROP, "One of the conditions has a literal not in the other condition.  Not unifying.\n");
        return false;
    }
    /* We now know either both conds are literal or both have identities */
    if (new_cond_id->identity)
    {
        thisAgent->variablizationManager->add_identity_unification(new_cond_id->identity, ground_cond_id->identity);
    }
    if (new_cond_attr->identity)
    {
        thisAgent->variablizationManager->add_identity_unification(new_cond_attr->identity, ground_cond_attr->identity);
    }
    if (new_cond_value->identity)
    {
        thisAgent->variablizationManager->add_identity_unification(new_cond_value->identity, ground_cond_value->identity);
    }
    dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    return true;
}

void Variablization_Manager::unify_backtraced_conditions(condition* parent_cond,
                                                         const soar_module::identity_triple o_ids_to_replace)
{
    test lId = 0, lAttr = 0, lValue = 0;
    lId = equality_test_found_in_test(parent_cond->data.tests.id_test);
    lAttr = equality_test_found_in_test(parent_cond->data.tests.attr_test);
    lValue = equality_test_found_in_test(parent_cond->data.tests.value_test);
    if (!lId->data.referent->is_sti() && o_ids_to_replace.id)
    {
        if (lId->identity)
        {
            dprint(DT_IDENTITY_PROP, "Found an o_id to replace for identifier element: %y [o%u] -> %y [o%u]\n", thisAgent->variablizationManager->get_ovar_for_o_id(o_ids_to_replace.id), o_ids_to_replace.id,
                thisAgent->variablizationManager->get_ovar_for_o_id(lId->identity), lId->identity);
        } else {
            dprint(DT_IDENTITY_PROP, "Found an o_id to literalize for identifier element: %y [o%u] -> %t\n", thisAgent->variablizationManager->get_ovar_for_o_id(o_ids_to_replace.id), o_ids_to_replace.id, lId);
        }
        thisAgent->variablizationManager->add_identity_unification(o_ids_to_replace.id, lId->identity);
        dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    } else {
        dprint(DT_IDENTITY_PROP, "Did not unify because %s%s\n",
                lId->data.referent->is_sti() ? "is STI " : "",
                !o_ids_to_replace.id ? "RHS pref is literal " : "");
    }
    if (!lAttr->data.referent->is_sti() && o_ids_to_replace.attr)
    {
        if (lAttr->identity)
        {
            dprint(DT_IDENTITY_PROP, "Found an o_id to replace for attribute element: %y [o%u] -> %y [o%u]\n", thisAgent->variablizationManager->get_ovar_for_o_id(o_ids_to_replace.attr), o_ids_to_replace.attr,
                thisAgent->variablizationManager->get_ovar_for_o_id(lAttr->identity), lAttr->identity);
        } else {
            dprint(DT_IDENTITY_PROP, "Found an o_id to literalize for attribute element: %y [o%u] -> %t\n", thisAgent->variablizationManager->get_ovar_for_o_id(o_ids_to_replace.attr), o_ids_to_replace.attr, lAttr);
        }
        thisAgent->variablizationManager->add_identity_unification(o_ids_to_replace.attr, lAttr->identity);
        dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    } else {
        dprint(DT_IDENTITY_PROP, "Did not unify because %s%s\n",
                lAttr->data.referent->is_sti() ? "is STI " : "",
                !o_ids_to_replace.attr ? "RHS pref is literal " : "");
    }
    if (!lValue->data.referent->is_sti() && o_ids_to_replace.value)
    {
        if (lValue->identity)
        {
            dprint(DT_IDENTITY_PROP, "Found an o_id to replace for value element: %y [o%u] -> %y [o%u]\n", thisAgent->variablizationManager->get_ovar_for_o_id(o_ids_to_replace.value), o_ids_to_replace.value,
                thisAgent->variablizationManager->get_ovar_for_o_id(lValue->identity), lValue->identity);
        } else {
            dprint(DT_IDENTITY_PROP, "Found an o_id to literalize for value element: %y [o%u] -> %t\n", thisAgent->variablizationManager->get_ovar_for_o_id(o_ids_to_replace.value), o_ids_to_replace.value, lValue);
        }
        thisAgent->variablizationManager->add_identity_unification(o_ids_to_replace.value, lValue->identity);
        dprint_o_id_substitution_map(DT_IDENTITY_PROP);
    } else {
        dprint(DT_IDENTITY_PROP, "Did not unify because %s%s\n",
                lValue->data.referent->is_sti() ? "is STI " : "",
                !o_ids_to_replace.value ? "RHS pref is literal " : "");
    }
}
