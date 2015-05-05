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
    std::map< uint64_t, uint64_t >::iterator iter = (*unification_map).find(t->identity->o_id);
    if (iter != (*unification_map).end())
    {
        dprint(DT_UNIFICATION, "...found variablization unification o%u -> o%u\n",
            t->identity->o_id, iter->second);

        t->identity->o_id = iter->second;
        if (t->identity->rule_symbol)
        {
            symbol_remove_ref(thisAgent, t->identity->rule_symbol);
            t->identity->rule_symbol = NULL;
        }
        if (iter->second)
        {
            /* MToDo | This was originally a debug table to make o_ids more intelligible, so probably should find
             *         a better way to set ovar here. */
            t->identity->rule_symbol = get_ovar_for_o_id(t->identity->o_id);
            if (t->identity->rule_symbol)
            {
                symbol_add_ref(thisAgent, t->identity->rule_symbol);
            }
        }
    }
}
/* MToDo | Verify that this is necessary in certain cases */
void Variablization_Manager::update_unification_table(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    std::map< uint64_t, uint64_t >::iterator iter;

    for (iter = unification_map->begin(); iter != unification_map->end(); ++iter)
    {

        if (iter->second == pOld_o_id)
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
    if (!pNew_o_id)
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
            print_o_id_to_ovar_debug_map(DT_UNIFICATION);
            dprint(DT_UNIFICATION, "Did not find o_id to o_id_substitution_map entry for o%u.  Adding %y[o%u] ", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id);
            dprint_noprefix(DT_UNIFICATION, "-> %y[o%u].\n", get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            newID = pNew_o_id;
            print_o_id_substitution_map(DT_OVAR_PROP);
        }
        else
        {
            /* Map all cases of what this identity is already remapped to with its parent identity */
            dprint(DT_UNIFICATION, "o_id unification (%y[o%u] -> ", get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            dprint_noprefix(DT_UNIFICATION, "%y[o%u]) already exists.  Adding transitive mapping ", get_ovar_for_o_id(iter->second), iter->second);
            dprint_noprefix(DT_UNIFICATION, "%y[o%u] -> ", get_ovar_for_o_id(pOld_o_id), pOld_o_id);
            dprint_noprefix(DT_UNIFICATION, "%y[o%u].\n", get_ovar_for_o_id(iter->second), iter->second);
            newID = iter->second;
        }
    }

    /* See if a unification already exists for the identity being replaced in this instantiation*/
    iter = (*unification_map).find(pOld_o_id);
    if (iter != (*unification_map).end())
    {
        if (iter->second == 0)
        {
            /* The existing identity we're unifying with is already literalized from a different trace.  So,
             * literalize any tests with identity of parent in this trace */
            dprint(DT_UNIFICATION, "Literalization exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
            /* MToDo | This might be redundant.  Wouldn't it be literalized already? */
            (*unification_map)[newID] = 0;
            update_unification_table(newID, 0);
        } else {
            if (newID == 0)
            {
                /* The existing identity we're literalizing is already unified with another identity from
                 * a different trace.  So, literalize the identity, that it is already remapped to.*/
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Propagating literalization substitution with %y[o%u] -> 0.\n", pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
                (*unification_map)[iter->second] = 0;
                update_unification_table(iter->second, 0);
            } else {
                /* The existing identity we're unifying with is already unified with another identity from
                 * a different trace.  So, unify the identity that it is already remapped to with identity
                 * of the parent in this trace */
                dprint(DT_UNIFICATION, "Unification with another identity exists for o%u.  Adding %y[o%u] -> ", pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
                dprint_noprefix(DT_UNIFICATION, "%y[o%u].\n", get_ovar_for_o_id(pNew_o_id), pNew_o_id);
                (*unification_map)[iter->second] = newID;
                update_unification_table(iter->second, newID);
            }
        }
    }

    /* Unify identity in this instantiation with final identity */
    (*unification_map)[pOld_o_id] = newID;
    update_unification_table(pOld_o_id, newID);
    print_o_id_substitution_map(DT_UNIFICATION);
}

