/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "wmem.h"
#include "debug.h"


void Variablization_Manager::unify_identity(agent* thisAgent, test t)
{
    uint64_t found_o_id = 0;

    found_o_id = thisAgent->variablizationManager->get_identity_unification(t->identity->original_var_id);
    if (found_o_id)
    {
        t->identity->original_var_id = found_o_id;
        symbol_remove_ref(thisAgent, t->identity->original_var);
        /* MToDo | This was originally a debug table to make o_ids more intelligible, so probably should find
         *         a better way to set ovar here. */
        t->identity->original_var = get_ovar_for_o_id(t->identity->original_var_id);
        if (t->identity->original_var)
        {
            symbol_add_ref(thisAgent, t->identity->original_var);
        }
    }
}

void Variablization_Manager::add_identity_unification(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    std::map< uint64_t, uint64_t >::iterator iter;
    uint64_t newID;

    assert(pOld_o_id);
    if (!pNew_o_id)
    {
        /* Do not check if a unification already exists if we're propagating back a literalization */
        dprint(DT_UNIFICATION, "Adding literalization substitution in o_id to o_id_substitution_map for o%u.  Adding %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id);
        newID = 0;
    } else {
        /* See if a unification already exists for the new identity propagating back*/
        iter = (*o_id_substitution_map).find(pNew_o_id);

        if (iter == (*o_id_substitution_map).end())
        {
            /* Map all cases of this identity with its parent identity */
            dprint(DT_UNIFICATION, "Did not find o_id to o_id_substitution_map entry for o%u.  Adding %y[o%u] -> %y[o%u].\n", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            newID = pNew_o_id;
            print_o_id_substitution_map(DT_OVAR_PROP);
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
    iter = (*o_id_substitution_map).find(pOld_o_id);
    if (iter != (*o_id_substitution_map).end())
    {
        if (iter->second == 0)
        {
            /* The existing identity we're unifying with is already literalized from a different trace.  So,
             * literalize any tests with identity of parent in this trace */
            dprint(DT_UNIFICATION, "Literalization exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            /* MToDo | This might be redundant.  Wouldn't it be literalized already? */
            (*o_id_substitution_map)[newID] = 0;
        } else {
            if (newID == 0)
            {
                /* The existing identity we're literalizing is already unified with another identity from
                 * a different trace.  So, literalize the identity, that it is already remapped to.*/
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
                (*o_id_substitution_map)[iter->second] = 0;
            } else {
                /* The existing identity we're unifying with is already unified with another identity from
                 * a different trace.  So, unify the identity that it is already remapped to with identity
                 * of the parent in this trace */
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Adding %y[o%u] -> %y[o%u].\n", pOld_o_id, get_ovar_for_o_id(iter->second), iter->second, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
                (*o_id_substitution_map)[iter->second] = newID;
            }
        }
        print_o_id_substitution_map(DT_UNIFICATION);
        return;
    }

    /* No existing unification for the existing identity, so just unify it with the identity of the parent. */
    (*o_id_substitution_map)[pOld_o_id] = newID;
    print_o_id_substitution_map(DT_UNIFICATION);
}

uint64_t Variablization_Manager::get_identity_unification(uint64_t pO_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_substitution_map).find(pO_id);
    if (iter != (*o_id_substitution_map).end())
    {
        dprint(DT_UNIFICATION, "...found o%u in o_id_substitution_map table for o%u\n",
            iter->second, pO_id);

        return iter->second;
    } else {
//        dprint(DT_UNIFICATION, "...did not find o%u in o_id_substitution_map.\n", pO_id);
//        print_o_id_substitution_map(DT_UNIFICATION);
    }
    return 0;
}
