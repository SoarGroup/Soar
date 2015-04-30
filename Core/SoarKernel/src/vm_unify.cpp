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

void Variablization_Manager::add_o_id_unification(uint64_t pOld_o_id, uint64_t pNew_o_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_substitution_map).find(pNew_o_id);
    // Do we need to check of old_id is there too?

    if (iter == (*o_id_substitution_map).end())
    {
        dprint(DT_OVAR_PROP, "Did not find o_id to o_id_substitution_map entry for o%u.  Adding %y[o%u] -> %y[o%u].\n", pOld_o_id, get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(pNew_o_id), pNew_o_id);
        (*o_id_substitution_map)[pOld_o_id] = pNew_o_id;
        print_o_id_substitution_map(DT_OVAR_PROP);
    }
    else
    {
        dprint(DT_OVAR_PROP, "o_id unification (%y[o%u] -> %y[o%u]) already exists.  Adding transitive mapping %y[o%u] -> %y[o%u].\n",
            get_ovar_for_o_id(pNew_o_id), pNew_o_id, get_ovar_for_o_id(iter->second), iter->second,
            get_ovar_for_o_id(pOld_o_id), pOld_o_id, get_ovar_for_o_id(iter->second), iter->second);
        (*o_id_substitution_map)[pOld_o_id] = iter->second;
    }
}

uint64_t Variablization_Manager::get_o_id_substitution(uint64_t pO_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_substitution_map).find(pO_id);
    if (iter != (*o_id_substitution_map).end())
    {
        dprint(DT_OVAR_PROP, "...found o%u in o_id_substitution_map table for o%u\n",
            iter->second, pO_id);

        return iter->second;
    } else {
//        dprint(DT_OVAR_PROP, "...did not find o%u in o_id_substitution_map.\n", pO_id);
//        print_o_id_substitution_map(DT_OVAR_PROP);
    }
    return 0;
}
