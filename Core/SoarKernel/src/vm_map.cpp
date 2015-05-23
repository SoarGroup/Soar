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
#include "wmem.h"
#include "print.h"
#include "debug.h"


void Variablization_Manager::clear_data()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing all variablization manager maps.\n");
    clear_cached_constraints();
    clear_variablization_maps();
    clear_merge_map();
    clear_ovar_to_o_id_map();
    clear_o_id_substitution_map();
    clear_o_id_to_ovar_debug_map();
    clear_o_id_update_map();
    clear_attachment_map();
}

void Variablization_Manager::clear_o_id_update_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing o_id_update table...\n");
//    for (std::map< uint64_t, uint64_t >::iterator it = (*o_id_update_map).begin(); it != (*o_id_update_map).end(); ++it)
//    {
//        dprint(DT_VM_MAPS, "Clearing %u -> %y(%u)\n", it->first, get_ovar_for_o_id(it->second), it->second);
//        delete it->second;
//    }
    o_id_update_map->clear();
}

void Variablization_Manager::clear_attachment_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing attachment map...\n");
    for (std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %u -> %s of %l\n", it->first, field_to_string(it->second->field), it->second->cond);
        delete it->second;
    }
    attachment_points->clear();
}

void Variablization_Manager::clear_variablization_maps()
{

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing symbol->variablization map...\n");
    /* -- Clear symbol->variablization map -- */
    for (std::map< Symbol*, variablization* >::iterator it = (*sym_to_var_map).begin(); it != (*sym_to_var_map).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %y -> %y(%u)/%y(%u)\n",
               it->first,
               it->second->instantiated_symbol, it->second->instantiated_symbol->reference_count,
               it->second->variablized_symbol,  it->second->variablized_symbol->reference_count);
        symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
        symbol_remove_ref(thisAgent, it->second->variablized_symbol);
        delete it->second;
    }
    sym_to_var_map->clear();

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing grounding_id->variablization map...\n");
    /* -- Clear grounding_id->variablization map -- */
    for (std::map< uint64_t, variablization* >::iterator it = (*o_id_to_var_map).begin(); it != (*o_id_to_var_map).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %u -> %y(%u)/%y(%u)\n",
               it->first,
               it->second->instantiated_symbol, it->second->instantiated_symbol->reference_count,
               it->second->variablized_symbol,  it->second->variablized_symbol->reference_count);
        symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
        symbol_remove_ref(thisAgent, it->second->variablized_symbol);
        delete it->second;
    }
    o_id_to_var_map->clear();
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing variablization data.\n");
}

variablization* Variablization_Manager::get_variablization(uint64_t index_id)
{
    if (index_id == 0)
    {
        return NULL;
    }

    std::map< uint64_t, variablization* >::iterator iter = (*o_id_to_var_map).find(index_id);
    if (iter != (*o_id_to_var_map).end())
    {
        dprint(DT_VM_MAPS, "...found o%u in non-STI variablization table: %y/%y\n", index_id,
               iter->second->variablized_symbol, iter->second->instantiated_symbol);
        return iter->second;
    }
    else
    {
        dprint(DT_VM_MAPS, "...did not find o%u in non-STI variablization table.\n", index_id);
        print_variablization_tables(DT_LHS_VARIABLIZATION, 2);
        return NULL;
    }
}

variablization* Variablization_Manager::get_variablization_for_symbol(std::map< Symbol*, variablization* >* pMap, Symbol* index_sym)
{
    std::map< Symbol*, variablization* >::iterator iter = (*pMap).find(index_sym);
    if (iter != (*pMap).end())
    {
        dprint(DT_VM_MAPS, "...found %y in STI variablization table: %y/%y\n", index_sym,
               iter->second->variablized_symbol, iter->second->instantiated_symbol);
        return iter->second;
    }
    else
    {
        dprint(DT_VM_MAPS, "...did not find %y in STI variablization table.\n", index_sym);
        print_variablization_tables(DT_VM_MAPS, 1);
        return NULL;
    }
}
variablization* Variablization_Manager::get_variablization(Symbol* index_sym)
{
    return get_variablization_for_symbol(sym_to_var_map, index_sym);
}

variablization* Variablization_Manager::get_variablization(test t)
{
    assert(t->data.referent);
    if (t->data.referent->is_sti())
    {
        return get_variablization(t->data.referent);
    }
    else
    {
        return get_variablization(t->identity->o_id);
    }
}

void Variablization_Manager::clear_o_id_to_ovar_debug_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    o_id_to_ovar_debug_map->clear();
}

void Variablization_Manager::clear_o_id_substitution_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    unification_map->clear();
}

void Variablization_Manager::clear_ovar_to_o_id_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    /* -- Clear original variable map -- */
    for (std::map< Symbol*, std::map< uint64_t, uint64_t > >::iterator it = (*ovar_to_o_id_map).begin(); it != (*ovar_to_o_id_map).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %y\n", it->first);
        symbol_remove_ref(thisAgent, it->first);
    }
    ovar_to_o_id_map->clear();
}

uint64_t Variablization_Manager::get_existing_o_id(Symbol* orig_var, uint64_t pI_id)
{
    std::map< Symbol*, std::map< uint64_t, uint64_t > >::iterator iter_sym;
    std::map< uint64_t, uint64_t >::iterator iter_inst;

//    dprint(DT_VM_MAPS, "...looking for symbol %y\n", orig_var);
    assert(orig_var && pI_id);

    iter_sym = ovar_to_o_id_map->find(orig_var);
    if (iter_sym != ovar_to_o_id_map->end())
    {
//        dprint(DT_VM_MAPS, "...Found.  Looking  for instantiation id %u\n", inst_id);
        iter_inst = iter_sym->second.find(pI_id);
        if (iter_inst != iter_sym->second.end())
        {
            dprint(DT_VM_MAPS, "%f...get_existing_o_id found mapping for %y in instantiation %u.  Returning existing o_id %u\n", orig_var, pI_id, iter_inst->second);
            return iter_inst->second;
        }
    }

    dprint(DT_VM_MAPS, "%f...get_existing_o_id did not find mapping for %y in instantiation %u.\n", orig_var, pI_id);
    /* MToDo | Remove */
    return 0;

}

uint64_t Variablization_Manager::get_or_create_o_id(Symbol* orig_var, uint64_t pI_id)
{
    int64_t existing_o_id = 0;

    existing_o_id = get_existing_o_id(orig_var, pI_id);
    if (!existing_o_id)
    {
        ++ovar_id_counter;
        (*ovar_to_o_id_map)[orig_var][pI_id] = ovar_id_counter;
        symbol_add_ref(thisAgent, orig_var);
        (*o_id_to_ovar_debug_map)[ovar_id_counter] = orig_var;
        dprint(DT_VM_MAPS, "%f...Created and returning new o_id %u for orig var %y in instantiation %u.\n", ovar_id_counter, orig_var, pI_id);
        return ovar_id_counter;
    } else {
        return existing_o_id;
    }
}

Symbol * Variablization_Manager::get_ovar_for_o_id(uint64_t o_id)
{
    if (o_id == 0) return NULL;

//    dprint(DT_VM_MAPS, "...looking for ovar for o_id %u...", o_id);
    std::map< uint64_t, Symbol* >::iterator iter = o_id_to_ovar_debug_map->find(o_id);
    if (iter != o_id_to_ovar_debug_map->end())
    {
//        dprint_noprefix(DT_IDENTITY_PROP, "found.  Returning %y\n", iter->second);
        return iter->second;
    }
//    dprint_noprefix(DT_IDENTITY_PROP, "not found.  Returning NULL.\n");
    return NULL;

}
