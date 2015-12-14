/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include <ebc_variablize.h>
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
    clear_rulesym_to_identity_map();
    clear_o_id_substitution_map();
    clear_o_id_to_ovar_debug_map();
    clear_attachment_map();
}

void Variablization_Manager::clear_attachment_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing attachment map...\n");
    for (std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        // Don't print anything from condition b/c it could be deallocated when this is being cleared
        dprint(DT_VM_MAPS, "Clearing %u -> %s of a condition in chunk.\n", it->first, field_to_string(it->second->field));
        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
    }
    attachment_points->clear();
}

void Variablization_Manager::clear_variablization_maps()
{

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing symbol->variablization map...\n");
    /* -- Clear symbol->variablization map -- */
    for (std::unordered_map< Symbol*, Symbol* >::iterator it = (*sym_to_var_map).begin(); it != (*sym_to_var_map).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %y -> %y\n", it->first, it->second);
        symbol_remove_ref(thisAgent, it->first);
        symbol_remove_ref(thisAgent, it->second);
    }
    sym_to_var_map->clear();

    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing grounding_id->variablization map...\n");
    /* -- Clear grounding_id->variablization map -- */
    for (std::unordered_map< uint64_t, Symbol* >::iterator it = (*o_id_to_var_map).begin(); it != (*o_id_to_var_map).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %u -> %y\n", it->first, it->second);
        symbol_remove_ref(thisAgent, it->second);
    }
    o_id_to_var_map->clear();
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing variablization data.\n");
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


void Variablization_Manager::clear_rulesym_to_identity_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing ovar_to_o_id_map...\n");
    rulesym_to_identity_map->clear();
}

uint64_t Variablization_Manager::get_existing_o_id(Symbol* orig_var, uint64_t pI_id)
{
    if (!m_learning_on) return NULL_IDENTITY_SET;
    std::unordered_map< uint64_t, std::unordered_map< Symbol*, uint64_t > >::iterator iter_sym;
    std::unordered_map< Symbol*, uint64_t >::iterator iter_inst;

    //        dprint(DT_VM_MAPS, "...Looking  for instantiation id %u\n", inst_id);
    assert(orig_var && pI_id);

    iter_sym = rulesym_to_identity_map->find(pI_id);
    if (iter_sym != rulesym_to_identity_map->end())
    {
        //    dprint(DT_VM_MAPS, "...Found.  Looking for symbol %y\n", orig_var);
        iter_inst = iter_sym->second.find(orig_var);
        if (iter_inst != iter_sym->second.end())
        {
            dprint(DT_IDENTITY_PROP, "%f...get_existing_o_id found mapping for %y in instantiation %u.  Returning existing o_id o%u\n", orig_var, pI_id, iter_inst->second);
            return iter_inst->second;
        }
    }

//    dprint(DT_IDENTITY_PROP, "%f...get_existing_o_id did not find mapping for %y in instantiation %u.\n", orig_var, pI_id);
    return NULL_IDENTITY_SET;

}

void Variablization_Manager::cleanup_for_instantiation_deallocation(uint64_t pI_id)
{
    assert(m_learning_on || rulesym_to_identity_map->size() == 0);
    assert(m_learning_on || o_id_to_ovar_debug_map->size() == 0);
    if (!m_learning_on) return;

    dprint(DT_EBC_CLEANUP, "Cleaning up for deallocation of instantiation %u\n", pI_id);
//    dprint_ovar_to_o_id_map(DT_EBC_CLEANUP);

#ifdef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
//    dprint_o_id_to_ovar_debug_map(DT_EBC_CLEANUP);

    std::unordered_map< uint64_t, std::unordered_map< Symbol*, uint64_t > >::iterator iter_sym;
    std::unordered_map< Symbol*, uint64_t >::iterator iter_inst;
    iter_sym = rulesym_to_identity_map->find(pI_id);
    if (iter_sym != rulesym_to_identity_map->end())
    {
        for (iter_inst = iter_sym->second.begin(); iter_inst != iter_sym->second.end(); ++iter_inst)
        {
            o_id_to_ovar_debug_map->erase(iter_inst->second);
        }
    }
//    dprint_o_id_to_ovar_debug_map(DT_EBC_CLEANUP);
#endif
    rulesym_to_identity_map->erase(pI_id);
//    dprint_ovar_to_o_id_map(DT_EBC_CLEANUP);
    dprint(DT_EBC_CLEANUP, "Done cleaning up for deallocation of instantiation %u\n-------\n", pI_id);
}

uint64_t Variablization_Manager::get_or_create_o_id(Symbol* orig_var, uint64_t pI_id)
{
    if (!m_learning_on) return 0;
    assert(orig_var->is_variable());
    int64_t existing_o_id = 0;

    existing_o_id = get_existing_o_id(orig_var, pI_id);
    if (!existing_o_id)
    {
        ++ovar_id_counter;
        (*rulesym_to_identity_map)[pI_id][orig_var] = ovar_id_counter;
#ifdef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
        (*o_id_to_ovar_debug_map)[ovar_id_counter] = orig_var;
#endif
        dprint(DT_IDENTITY_PROP, "%f...Created and returning new o_id o%u for orig var %y in instantiation %u.\n", ovar_id_counter, orig_var, pI_id);
        return ovar_id_counter;
    } else {
        return existing_o_id;
    }
}

Symbol * Variablization_Manager::get_ovar_for_o_id(uint64_t o_id)
{
#ifndef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
    return NULL;
#else
    if (o_id == 0) return NULL;
    if (!m_learning_on) return NULL;

//    dprint(DT_VM_MAPS, "...looking for ovar for o_id %u...", o_id);
    std::unordered_map< uint64_t, Symbol* >::iterator iter = o_id_to_ovar_debug_map->find(o_id);
    if (iter != o_id_to_ovar_debug_map->end())
    {
//        dprint_noprefix(DT_IDENTITY_PROP, "found.  Returning %y\n", iter->second);
        return iter->second;
    }
//    dprint_noprefix(DT_IDENTITY_PROP, "not found.  Returning NULL.\n");
    return NULL;
#endif
}
