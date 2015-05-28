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

    dprint(DT_FIX_CONDITIONS, "Fixing result %p\n", result);
    dprint_o_id_tables(DT_FIX_CONDITIONS);

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
    /* MToDo | Do we need to fix o_ids in clones too? */
}

void Variablization_Manager::update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id, uint64_t pOld_o_id_2)
{
    std::map< uint64_t, uint64_t >::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {

        if ((iter->second == pOld_o_id) || (pOld_o_id_2 && (iter->second == pOld_o_id_2)))
        {
            dprint(DT_FIX_CONDITIONS, "...found secondary o_id unification mapping that needs updated: o%u = o%u -> o%u = o%u.\n", iter->first, iter->second, iter->first, pNew_o_id );
            (*unification_map)[iter->first] = pNew_o_id;
        }
    }
}

void Variablization_Manager::add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    std::map< uint64_t, uint64_t >::iterator iter;
    uint64_t newID;

    assert(pOld_o_id);
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
                /* MToDo | This might be redundant.  Wouldn't it be literalized already? */
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

