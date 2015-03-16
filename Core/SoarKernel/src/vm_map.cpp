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


void Variablization_Manager::clear_data()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing all variablization manager maps.\n");
    clear_cached_constraints();
    clear_oid_to_gid_map();
    clear_variablization_maps();
    clear_merge_map();
    clear_substitution_map();
    clear_dnvl();
    clear_ovar_to_o_id_map();
    clear_o_id_substitution_map();
    clear_o_id_to_ovar_debug_map();
}

void Variablization_Manager::clear_oid_to_gid_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing o_id to g_id table...\n");
    o_id_to_g_id_map->clear();
}

void Variablization_Manager::clear_variablization_maps()
{

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing symbol->variablization map...\n");
    /* -- Clear symbol->variablization map -- */
    for (std::map< Symbol*, variablization* >::iterator it = (*sym_to_var_map).begin(); it != (*sym_to_var_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %y -> %y(%u)/%y(%u)\n",
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
    for (std::map< uint64_t, variablization* >::iterator it = (*g_id_to_var_map).begin(); it != (*g_id_to_var_map).end(); ++it)
    {
        dprint(DT_VARIABLIZATION_MANAGER, "Clearing %u -> %y(%u)/%y(%u)\n",
               it->first,
               it->second->instantiated_symbol, it->second->instantiated_symbol->reference_count,
               it->second->variablized_symbol,  it->second->variablized_symbol->reference_count);
        symbol_remove_ref(thisAgent, it->second->instantiated_symbol);
        symbol_remove_ref(thisAgent, it->second->variablized_symbol);
        delete it->second;
    }
    g_id_to_var_map->clear();
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing variablization data.\n");
}

variablization* Variablization_Manager::get_variablization(uint64_t index_id)
{
    if (index_id == 0)
    {
        return NULL;
    }

    std::map< uint64_t, variablization* >::iterator iter = (*g_id_to_var_map).find(index_id);
    if (iter != (*g_id_to_var_map).end())
    {
        dprint(DT_VARIABLIZATION_MANAGER, "...found %u in g_id variablization table: %y/%y\n", index_id,
               iter->second->variablized_symbol, iter->second->instantiated_symbol);
        return iter->second;
    }
    else
    {
        dprint(DT_VARIABLIZATION_MANAGER, "...did not find %u in g_id variablization table.\n", index_id);
        print_variablization_tables(DT_LHS_VARIABLIZATION, 2);
        return NULL;
    }
}

variablization* Variablization_Manager::get_variablization_for_symbol(std::map< Symbol*, variablization* >* pMap, Symbol* index_sym)
{
    std::map< Symbol*, variablization* >::iterator iter = (*pMap).find(index_sym);
    if (iter != (*pMap).end())
    {
        dprint(DT_VARIABLIZATION_MANAGER, "...found %y in variablization table: %y/%y\n", index_sym,
               iter->second->variablized_symbol, iter->second->instantiated_symbol);
        return iter->second;
    }
    else
    {
        dprint(DT_VARIABLIZATION_MANAGER, "...did not find %y in variablization table.\n", index_sym);
        print_variablization_tables(DT_VARIABLIZATION_MANAGER, 1);
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
        return get_variablization(t->identity->grounding_id);
    }
}

uint64_t Variablization_Manager::get_gid_for_o_id(uint64_t pO_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_to_g_id_map).find(pO_id);
    if (iter != (*o_id_to_g_id_map).end())
    {
        dprint(DT_VARIABLIZATION_MANAGER, "...found g%u in o_id_to_g_id table for o%u\n",
            iter->second, pO_id);

        return iter->second;
    } else {
        dprint(DT_VARIABLIZATION_MANAGER, "...did not find o%u in o_id_to_g_id.\n", pO_id);
        print_ovar_gid_propogation_table(DT_LHS_VARIABLIZATION);
    }
    return 0;
}

uint64_t Variablization_Manager::get_gid_for_orig_var(Symbol* index_sym, uint64_t pI_id)
{
    uint64_t found_o_id;
    found_o_id = get_existing_o_id(index_sym, pI_id);
    if (found_o_id)
    {
        return get_gid_for_o_id(found_o_id);
    } else {
        return 0;
    }
}

uint64_t Variablization_Manager::add_o_id_to_gid_mapping(uint64_t pO_id, uint64_t pG_id)
{
    std::map< uint64_t, uint64_t >::iterator iter = (*o_id_to_g_id_map).find(pO_id);
    if (iter == (*o_id_to_g_id_map).end())
    {
        dprint(DT_OVAR_MAPPINGS, "Did not find o_id to g_id mapping for %u.  Adding.\n", pO_id);
        (*o_id_to_g_id_map)[pO_id] = pG_id;
//        symbol_add_ref(thisAgent, index_sym);
        /* -- returning 0 indicates that the mapping was added -- */
        return 0;
    }
    else
    {
        dprint(DT_OVAR_MAPPINGS,
               "...g%u already exists for o%u(%y).  add_o_id_to_gid_mapping returning existing g%u.\n",
               iter->second, pO_id, get_ovar_for_o_id(pO_id), iter->second);
    }
    return iter->second;
}

uint64_t Variablization_Manager::add_orig_var_to_gid_mapping(Symbol* index_sym, uint64_t index_g_id, uint64_t pI_id)
{
    uint64_t lO_id = get_or_create_o_id(index_sym, pI_id);
//    uint64_t lO_id = get_existing_o_id(index_sym, pI_id);
    if (!lO_id)
        print_o_id_tables(DT_OVAR_MAPPINGS);
    assert(lO_id);
    return add_o_id_to_gid_mapping(lO_id, index_g_id);
}


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
            dprint(DT_OVAR_PROP, "...get_existing_o_id found mapping for %y in instantiation %u.  Returning existing o_id %u\n", orig_var, inst_id, iter_inst->second);
            return iter_inst->second;
        }
    }

    dprint(DT_OVAR_PROP, "...get_existing_o_id did not find mapping for %y in instantiation %u.\n", orig_var, inst_id);
    /* MToDo | Remove */
    std::string strName(orig_var->to_string());
    if ((inst_id == 1) && (strName == "<x>"))
        assert(false);
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
        dprint(DT_OVAR_PROP, "...Created and returning new o_id %u for orig var %y in instantiation %u.\n", ovar_id_counter, orig_var, inst_id);
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

void Variablization_Manager::clear_dnvl()
{
    dprint(DT_DNVL, "Clearing LTI DNVL set.\n");
     dnvl_set->clear();
}
void Variablization_Manager::add_dnvl(Symbol* sym)
{
    dprint(DT_DNVL, "...adding symbol %y to DNVL set.\n", sym);
    dnvl_set->insert(sym);
}

bool Variablization_Manager::is_in_dnvl(Symbol* sym)
{
    std::set< Symbol* >::iterator iter;
    iter = dnvl_set->find(sym);
    dprint(DT_DNVL, "Looking for symbol %y in DNVL set...%s.\n", sym, (iter != dnvl_set->end()) ? "found" : "not found");

    return (iter != dnvl_set->end());
}
