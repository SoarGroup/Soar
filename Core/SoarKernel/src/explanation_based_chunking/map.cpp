/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"

#include "agent.h"
#include "dprint.h"
#include "instantiation.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"
#include "print.h"

#include <assert.h>

void Explanation_Based_Chunker::clear_data()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Clearing all chunking maps.\n");
    clear_cached_constraints();
    clear_variablization_maps();
    clear_merge_map();
    clear_rulesym_to_identity_map();
    clear_o_id_substitution_map();
    clear_o_id_to_ovar_debug_map();
    clear_attachment_map();
    clear_singletons();
}

void Explanation_Based_Chunker::clear_singletons()
{
    if (local_singleton_superstate_identity)
    {
        delete local_singleton_superstate_identity;
        local_singleton_superstate_identity = NULL;
    }
}

void Explanation_Based_Chunker::clear_attachment_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing attachment map...\n");
    for (attachment_points_map::iterator it = (*attachment_points).begin(); it != (*attachment_points).end(); ++it)
    {
        // Don't print anything from condition b/c it could be deallocated when this is being cleared
        dprint(DT_VM_MAPS, "Clearing %u -> %s of a condition in chunk.\n", it->first, field_to_string(it->second->field));
        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
    }
    attachment_points->clear();
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing attachment data.\n");
}

void Explanation_Based_Chunker::clear_variablization_maps()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing o_id_to_var_map map...\n");
    /* -- Clear grounding_id->variablization map -- */
    for (id_to_sym_map::iterator it = (*o_id_to_var_map).begin(); it != (*o_id_to_var_map).end(); ++it)
    {
        dprint(DT_VM_MAPS, "Clearing %u -> %y\n", it->first, it->second);
        thisAgent->symbolManager->symbol_remove_ref(&it->second);
    }
    o_id_to_var_map->clear();
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager done clearing o_id_to_var_map data.\n");
}


void Explanation_Based_Chunker::clear_o_id_to_ovar_debug_map()
{
    Symbol* lSym;
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing id_to_rule_sym_debug_map...\n");
    for (auto it = (*id_to_rule_sym_debug_map).begin(); it != (*id_to_rule_sym_debug_map).end(); ++it)
    {
        lSym = it->second;
        dprint(DT_VM_MAPS, "Clearing %u -> %y\n", it->first, it->second);
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
    }
    id_to_rule_sym_debug_map->clear();
}

void Explanation_Based_Chunker::clear_o_id_substitution_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing unification_map...\n");
    unification_map->clear();
}


void Explanation_Based_Chunker::clear_rulesym_to_identity_map()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager clearing instantiation_identities...\n");
    instantiation_identities->clear();
}

uint64_t Explanation_Based_Chunker::get_existing_o_id(Symbol* orig_var, uint64_t pI_id)
{
    assert(orig_var && pI_id);

    inst_to_id_map::iterator iter_inst;
    sym_to_id_map::iterator iter_sym;
    iter_inst = instantiation_identities->find(pI_id);
    if (iter_inst != instantiation_identities->end())
    {
        //    dprint(DT_VM_MAPS, "...Found.  Looking for symbol %y\n", orig_var);
        iter_sym = iter_inst->second.find(orig_var);
        if (iter_sym != iter_inst->second.end())
        {
            dprint(DT_IDENTITY_PROP, "%f...get_existing_o_id found mapping for %y in instantiation %u.  Returning existing o_id o%u\n", orig_var, pI_id, iter_sym->second);
            return iter_sym->second;
        }
    }

//    dprint(DT_IDENTITY_PROP, "%f...get_existing_o_id did not find mapping for %y in instantiation %u.\n", orig_var, pI_id);
    return NULL_IDENTITY_SET;

}

/* Why would we need to do this after instantiation creation? */
void Explanation_Based_Chunker::cleanup_after_instantiation_creation(uint64_t pI_id)
{
    /* Since we're cleaning up instantiation identities to symbol mappings after inst
     * creation, then the inst_identity list won't be here to clear out debug symbols
     * when the instantiation is deallocated.  So the debug symbol table will grow
     * with each chunk/justification attempted.  Originally, these were just for debugging
     * but they're nice for the explainer.  We should save them to the instantiation and
     * have the explainer copy them as needed.*/

    if (instantiation_identities->size() == 0) return;
    dprint(DT_EBC_CLEANUP, "Cleaning up after creating instantiation %u\n", pI_id);
    instantiation_identities->erase(pI_id);
}

void Explanation_Based_Chunker::cleanup_for_instantiation_deallocation(uint64_t pI_id)
{
    #ifdef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS

        if ((instantiation_identities->size() == 0) || (id_to_rule_sym_debug_map->size() == 0)) return;

        // I don't think this function can get this far any more until we get smarter cleanup

        dprint(DT_EBC_CLEANUP, "Cleaning up for deallocation of instantiation %u\n", pI_id);
        //    dprint_o_id_to_ovar_debug_map(DT_EBC_CLEANUP);

        inst_to_id_map::iterator iter_inst;
        sym_to_id_map::iterator iter_sym;
        Symbol* lSym;

        iter_inst = instantiation_identities->find(pI_id);

        if (iter_inst != instantiation_identities->end())
        {
            assert(false);
            for (iter_sym = iter_inst->second.begin(); iter_sym != iter_inst->second.end(); ++iter_sym)
            {
                lSym = iter_sym->first;
                id_to_rule_sym_debug_map->erase(iter_sym->second);
                thisAgent->symbolManager->symbol_remove_ref(&(lSym));
            }
        }
        instantiation_identities->erase(pI_id);
        //    dprint_o_id_to_ovar_debug_map(DT_EBC_CLEANUP);
        dprint(DT_EBC_CLEANUP, "Done cleaning up for deallocation of instantiation %u\n-------\n", pI_id);

    #endif
}

uint64_t Explanation_Based_Chunker::get_or_create_o_id(Symbol* orig_var, uint64_t pI_id)
{
    /* Fake instantiations are currently getting identities, so this may no longer hold */
//    assert(orig_var->is_variable());
    int64_t existing_o_id = 0;

    existing_o_id = get_existing_o_id(orig_var, pI_id);
    if (!existing_o_id)
    {
        increment_counter(ovar_id_counter);
        (*instantiation_identities)[pI_id][orig_var] = ovar_id_counter;
#ifdef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
        /* We add a refcount for debug map because we may create a dummy variable for architectural
         * instantiations, which needs to create a refcount.  So we create them from both and clean
         * up on instantiation deallocation. */
        if (orig_var->is_variable())
        {
            (*id_to_rule_sym_debug_map)[ovar_id_counter] = orig_var;
            thisAgent->symbolManager->symbol_add_ref(orig_var);
        } else {
            (*id_to_rule_sym_debug_map)[ovar_id_counter] = thisAgent->symbolManager->generate_new_variable("A");
        }
#endif
        dprint(DT_IDENTITY_PROP, "%f...Created and returning new o_id o%u for orig var %y in instantiation %u.\n", ovar_id_counter, orig_var, pI_id);
        return ovar_id_counter;
    } else {
        return existing_o_id;
    }
}

Symbol* Explanation_Based_Chunker::get_match_for_rhs_var(Symbol* pRHS_var)
{
    sym_to_sym_map::iterator iter = rhs_var_to_match_map->find(pRHS_var);
    if (iter != rhs_var_to_match_map->end())
    {
        return iter->second;
    }
    return NULL;
}

void Explanation_Based_Chunker::add_matched_sym_for_rhs_var(Symbol* pRHS_var, Symbol* pMatched_sym)
{
    Symbol* lSym = get_match_for_rhs_var(pRHS_var);
    if (!lSym)
    {
        (*rhs_var_to_match_map)[pRHS_var] = pMatched_sym;
    }
}

Symbol * Explanation_Based_Chunker::get_ovar_for_o_id(uint64_t o_id)
{
#ifndef DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS
    return NULL;
#else
    if (o_id == 0) return NULL;
    if (!ebc_settings[SETTING_EBC_LEARNING_ON]) return NULL;

//    dprint(DT_VM_MAPS, "...looking for ovar for o_id %u...", o_id);
    id_to_sym_map::iterator iter = id_to_rule_sym_debug_map->find(o_id);
    if (iter != id_to_rule_sym_debug_map->end())
    {
//        dprint_noprefix(DT_IDENTITY_PROP, "found.  Returning %y\n", iter->second);
        return iter->second;
    }
//    dprint_noprefix(DT_IDENTITY_PROP, "not found.  Returning NULL.\n");
    return NULL;
#endif
}
