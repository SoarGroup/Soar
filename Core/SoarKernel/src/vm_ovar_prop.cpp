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
#include "debug.h"

void Variablization_Manager::clear_o_id_to_ovar_debug_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    o_id_to_ovar_debug_map->clear();
}

void Variablization_Manager::clear_o_id_substitution_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    o_id_substitution_map->clear();
}

void Variablization_Manager::clear_ovar_to_o_id_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    /* -- Clear original variable map -- */
    for (std::map< Symbol*, std::map< uint64_t, uint64_t > >::iterator it = (*ovar_to_o_id_map).begin(); it != (*ovar_to_o_id_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %y\n", it->first);
        symbol_remove_ref(thisAgent, it->first);
    }
    ovar_to_o_id_map->clear();
}

uint64_t Variablization_Manager::get_existing_o_id(Symbol* orig_var, uint64_t inst_id)
{
    std::map< Symbol*, std::map< uint64_t, uint64_t > >::iterator iter_sym;
    std::map< uint64_t, uint64_t >::iterator iter_inst;

//    dprint(DT_OVAR_PROP, "...looking for symbol %y\n", orig_var);
    iter_sym = ovar_to_o_id_map->find(orig_var);
    if (iter_sym != ovar_to_o_id_map->end())
    {
//        dprint(DT_OVAR_PROP, "...Found.  Looking  for instantiation id %u\n", inst_id);
        iter_inst = iter_sym->second.find(inst_id);
        if (iter_inst != iter_sym->second.end())
        {
//            dprint(DT_OVAR_PROP, "...Found.  Returning existing o_id %u\n", iter_inst->second);
            return iter_inst->second;
        }
    }

    return 0;

}

uint64_t Variablization_Manager::get_or_create_o_id(Symbol* orig_var, uint64_t inst_id)
{
    int64_t existing_o_id = 0;

    existing_o_id = get_existing_o_id(orig_var, inst_id);
    if (!existing_o_id)
    {
        ++ovar_id_counter;
        (*ovar_to_o_id_map)[orig_var][inst_id] = ovar_id_counter;
        symbol_add_ref(thisAgent, orig_var);
        (*o_id_to_ovar_debug_map)[ovar_id_counter] = orig_var;
        dprint(DT_OVAR_PROP, "...Not found.  Stored and returning new o_id %u for orig var %y.\n", ovar_id_counter, orig_var);
        return ovar_id_counter;
    } else {
        return existing_o_id;
    }
}

Symbol * Variablization_Manager::get_ovar_for_o_id(uint64_t o_id)
{
    if (o_id == 0) return NULL;

//    dprint(DT_OVAR_PROP, "...looking for ovar for o_id %u...", o_id);
    std::map< uint64_t, Symbol* >::iterator iter = o_id_to_ovar_debug_map->find(o_id);
    if (iter != o_id_to_ovar_debug_map->end())
    {
//        dprint_noprefix(DT_OVAR_PROP, "found.  Returning %y\n", iter->second);
        return iter->second;
    }
//    dprint_noprefix(DT_OVAR_PROP, "not found.  Returning NULL.\n");
    return NULL;

}
