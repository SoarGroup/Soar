#include "preference.h"

#include "agent.h"
#include "decide.h"
#include "decider.h"
#include "dprint.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "mem.h"
#include "print.h"
#include "rhs.h"
#include "slot.h"
#include "symbol.h"
#include "working_memory.h"
#include "working_memory_activation.h"

#include <stdlib.h>

#ifdef DEBUG_PREF_DEALLOCATION_INVENTORY
    id_to_string_map pref_deallocation_map;
    id_to_pref_map pref_deallocation_map2;

    uint64_t PDI_id_counter = 0;

    void PDI_add(agent* thisAgent, preference* pPref, bool isShallow = false)
    {
        std::string lPrefString;
        pPref->p_id = ++PDI_id_counter;
        thisAgent->outputManager->sprinta_sf(thisAgent, lPrefString, "%u: %p", pPref->p_id, pPref);
//        dprint(DT_DEBUG, "%u:%s%p\n", pPref->p_id, isShallow ? " shallow " : " ", pPref);
        pref_deallocation_map[pPref->p_id] = lPrefString;
        pref_deallocation_map2[pPref->p_id] = pPref;
    }
    void PDI_remove(agent* thisAgent, preference* pPref)
    {
        auto it = pref_deallocation_map.find(pPref->p_id);
        assert (it != pref_deallocation_map.end());
//        if (it == pref_deallocation_map.end())
//        {
//            dprint(DT_DEBUG, "Did not find preference to remove!  %p\nRemaining preference deallocation map:\n", pPref);
//            std::string lPrefString;
//            for (auto it = pref_deallocation_map.begin(); it != pref_deallocation_map.end(); ++it)
//            {
//                lPrefString = it->second;
//                if (!lPrefString.empty())
//                {
//                    dprint(DT_DEBUG, "%u: %s\n", it->first, lPrefString.c_str());
//                }
//            }
//            return;
//        }
        std::string lPrefString = it->second;
        if (!lPrefString.empty())
        {
//            dprint(DT_DEBUG, "Clearing pref from %u: %p", pPref->p_id, pPref);
            pref_deallocation_map[pPref->p_id].clear();
//            dprint(DT_DEBUG, "--> %p\n", pPref);
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "Preferences %u was deallocated twice!\n", it->first);
        }
    }

    void PDI_print_and_cleanup(agent* thisAgent)
    {
        std::string lPrefString;
        uint64_t bugCount = 0;
        thisAgent->outputManager->printa_sf(thisAgent, "Looking for preferences that were not deallocated...\n");
        for (auto it = pref_deallocation_map.begin(); it != pref_deallocation_map.end(); ++it)
        {
            lPrefString = it->second;
            if (!lPrefString.empty())
            {
                bugCount++;
            }
        }
        if (bugCount <= 23)
        {
            for (auto it = pref_deallocation_map.begin(); it != pref_deallocation_map.end(); ++it)
            {
                lPrefString = it->second;
                if (!lPrefString.empty())
                {
                    thisAgent->outputManager->printa_sf(thisAgent, "Preference %u was not deallocated: %s!\n", it->first, lPrefString.c_str());
                    auto it2 = pref_deallocation_map2.find(it->first);
                    if (it2 != pref_deallocation_map2.end())
                    {
                        preference* lPref = it2->second;
                        thisAgent->outputManager->printa_sf(thisAgent, "- created by instantiation %u %y.  Refcount of %d.\n", lPref->inst ? lPref->inst->i_id : 0, lPref->inst ? lPref->inst->prod_name : thisAgent->symbolManager->soarSymbols.nil_symbol, lPref->reference_count);
                    }

                }
            }
        }
        thisAgent->outputManager->printa_sf(thisAgent, "\n\nPreference inventory result:  %u/%u were not deallocated.\n", bugCount, PDI_id_counter);
        pref_deallocation_map.clear();
    }
#else
    void PDI_add(agent* thisAgent, instantiation* pInst, bool isShallow = false) {}
    void PDI_remove(agent* thisAgent, uint64_t pID) {}
    void PDI_print_and_cleanup(agent* thisAgent) {}
#endif

/* -------------------------------------------------------------------
   Make_preference() creates a new preference structure of the given type
   with the given id/attribute/value/referent.  (Referent is only used
   for binary preferences.)  The preference is not yet added to preference
   memory, however.
-------------------------------------------------------------------*/

preference* make_preference(agent* thisAgent, PreferenceType type, 
                            Symbol* id, Symbol* attr, Symbol* value, Symbol* referent,
                            const identity_quadruple o_ids, bool pUnify_identities, const bool_quadruple pWas_unbound_vars)
{
    preference* p;

    thisAgent->memoryManager->allocate_with_pool(MP_preference, &p);
    p->type = type;
    p->in_tm = false;
    p->o_supported = false;
    p->on_goal_list = false;
    p->reference_count = 0;
    p->id = id;
    p->attr = attr;
    p->value = value;
    p->referent = referent;
    p->slot = NIL;
    p->total_preferences_for_candidate = 0;
    p->numeric_value = 0;
    p->rl_contribution = false;
    p->rl_rho = 1.0;
    p->wma_o_set = NIL;
    p->next_clone = NIL;
    p->prev_clone = NIL;
    p->next = NIL;
    p->prev = NIL;
    p->inst_next = NIL;
    p->inst_prev = NIL;
    p->all_of_slot_next = NIL;
    p->all_of_slot_prev = NIL;
    p->all_of_goal_next = NIL;
    p->all_of_goal_prev = NIL;
    p->next_candidate = NIL;
    p->next_result = NIL;
    p->parent_action = NULL;

    if (pUnify_identities)
    {
        if (o_ids.id) p->identities.id = thisAgent->explanationBasedChunker->get_identity(o_ids.id); else p->identities.id = 0;
        if (o_ids.attr) p->identities.attr = thisAgent->explanationBasedChunker->get_identity(o_ids.attr); else p->identities.attr = 0;
        if (o_ids.value) p->identities.value = thisAgent->explanationBasedChunker->get_identity(o_ids.value); else p->identities.value = 0;
        if (o_ids.referent) p->identities.referent = thisAgent->explanationBasedChunker->get_identity(o_ids.referent); else p->identities.referent = 0;
    }
    else
    {
        p->identities.id = o_ids.id;
        p->identities.attr = o_ids.attr;
        p->identities.value = o_ids.value;
        p->identities.referent = o_ids.referent;
    }
    p->clone_identities.id = p->identities.id;
    p->clone_identities.attr = p->identities.attr;
    p->clone_identities.value = p->identities.value;
    p->clone_identities.referent = p->identities.referent;

    /* We set these to NULL an leave the code creating this preference responsible
     * for allocating these rhs_values if needed. These rhs values are used to
     * store the variablization identities of variables used in the rhs functions */
    p->rhs_funcs.id = NULL;
    p->rhs_funcs.attr = NULL;
    p->rhs_funcs.value = NULL;
    p->rhs_funcs.referent = NULL;
    p->cloned_rhs_funcs.id = NULL;
    p->cloned_rhs_funcs.attr = NULL;
    p->cloned_rhs_funcs.value = NULL;
    p->cloned_rhs_funcs.referent = NULL;

    p->was_unbound_vars.id = pWas_unbound_vars.id;
    p->was_unbound_vars.attr = pWas_unbound_vars.attr;
    p->was_unbound_vars.value = pWas_unbound_vars.value;
    p->was_unbound_vars.referent = pWas_unbound_vars.referent;

    PDI_add(thisAgent, p);

    dprint(DT_PREFS, "Created preference %p\n", p);

    return p;
}

/* This function just copies the elements of a preferences we need for the EBC explanation mechanism */
preference* shallow_copy_preference(agent* thisAgent, preference* pPref)
{
    preference* p;

    thisAgent->memoryManager->allocate_with_pool(MP_preference, &p);
    p->type = pPref->type;
    p->numeric_value = pPref->numeric_value;
    p->o_supported = pPref->o_supported;
    p->id = pPref->id;
    p->attr = pPref->attr;
    p->value = pPref->value;
    p->referent = pPref->referent;
    thisAgent->symbolManager->symbol_add_ref(p->id);
    thisAgent->symbolManager->symbol_add_ref(p->attr);
    thisAgent->symbolManager->symbol_add_ref(p->value);
    if (p->referent) thisAgent->symbolManager->symbol_add_ref(p->referent);
    p->identities.id = pPref->identities.id;
    p->identities.attr = pPref->identities.attr;
    p->identities.value = pPref->identities.value;
    p->identities.referent = pPref->identities.referent;
    p->clone_identities.id = pPref->clone_identities.id;
    p->clone_identities.attr = pPref->clone_identities.attr;
    p->clone_identities.value = pPref->clone_identities.value;
    p->clone_identities.referent = pPref->clone_identities.referent;

    p->rhs_funcs.id = copy_rhs_value(thisAgent, pPref->rhs_funcs.id);
    p->rhs_funcs.attr = copy_rhs_value(thisAgent, pPref->rhs_funcs.attr);
    p->rhs_funcs.value = copy_rhs_value(thisAgent, pPref->rhs_funcs.value);
    p->rhs_funcs.referent = copy_rhs_value(thisAgent, pPref->rhs_funcs.referent);

    p->cloned_rhs_funcs.id = NULL;
    p->cloned_rhs_funcs.attr = NULL;
    p->cloned_rhs_funcs.value = NULL;
    p->cloned_rhs_funcs.referent = NULL;

    /* Don't want this information or have the other things cleaned up*/
    p->inst = NULL;
    p->in_tm = false;
    p->on_goal_list = false;
    p->reference_count = 0;
    p->slot = NULL;
    p->total_preferences_for_candidate = 1;
    p->rl_contribution = false;
    p->rl_rho = 0;
    p->wma_o_set = NULL;

    /* Don't want to copy links to other preferences */
    p->next_clone = NULL;
    p->prev_clone = NULL;
    p->next = NULL;
    p->prev = NULL;
    p->inst_next = NULL;
    p->inst_prev = NULL;
    p->all_of_slot_next = NULL;
    p->all_of_slot_prev = NULL;
    p->all_of_goal_next = NULL;
    p->all_of_goal_prev = NULL;
    p->next_candidate = NULL;
    p->next_result = NULL;

    dprint(DT_PREFS, "Created shallow copy of preference %p\n", p);
    PDI_add(thisAgent, p, true);

    return p;

    /* BUGBUG check to make sure the pref doesn't have
          value or referent .isa_goal or .isa_impasse; */
}
/* -------------------------------------------------------------------
   Deallocate_preference() deallocates a given preference.
-------------------------------------------------------------------*/

void deallocate_preference(agent* thisAgent, preference* pref, bool dont_cache)
{
//    dprint(DT_DEALLOCATE_PREF, "Deallocating preference %p\n", pref);
    dprint(DT_DEALLOCATE_PREF, "Deallocating preference %p (%u)\n", pref, pref->p_id);
    break_if_id_matches(pref->p_id, 2);
    assert(pref->reference_count == 0);

    /*  remove it from the list of pref's for its match goal */
    if (pref->on_goal_list)
        remove_from_dll(pref->inst->match_goal->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);

    if (pref->inst)
    {
        /* The following caches the preference if there's a chance that it will be
         * needed for an explanation of an instantiation.  (Might be able to avoid
         * some of this caching.  At one point we passed in a pDoNotCache flag.  */
        //if (!pDoNotCache && (pref->inst->match_goal_level != TOP_GOAL_LEVEL) && thisAgent->explanationMemory->enabled())
        if ((pref->inst->match_goal_level != TOP_GOAL_LEVEL) && thisAgent->explanationMemory->is_any_enabled() && !dont_cache)
        {
            preference* lNewPref = shallow_copy_preference(thisAgent, pref);
            dprint(DT_EXPLAIN_CACHE, "Caching preference for instantiation %u (match of %y): %p\n", pref->inst->i_id, pref->inst->prod_name, pref);
            insert_at_head_of_dll(pref->inst->preferences_cached, lNewPref, inst_next, inst_prev);
        }
        /*  remove it from the list of pref's from that instantiation */
        remove_from_dll(pref->inst->preferences_generated, pref, inst_next, inst_prev);
        dprint(DT_DEALLOCATE_INST, "Possibly deallocating instantiation %u (match of %y) for preference.\n", pref->inst->i_id, pref->inst->prod_name);
        possibly_deallocate_instantiation(thisAgent, pref->inst);
    }

//    /* The following code re-uses the preference instead of copying it.  It worked
//     * but there were certain cases where instantiations weren't being deallocated
//     * (arithmetic agent, chunk always, interrupt on, allow-local-negations off,
//     * explain all, explain just, init after it interrupts). Couldn't quite sort
//     * out why but reverting to copying version resolved it. Since copying is more
//     * expensive, keeping the re-use code in case we have time to figure it out later. */
//        if (pref->inst)
//        {
//            /*  remove it from the list of pref's from that instantiation */
//            remove_from_dll(pref->inst->preferences_generated, pref, inst_next, inst_prev);
//            instantiation* prefInst = pref->inst;
//            if ((pref->inst->match_goal_level != TOP_GOAL_LEVEL) && thisAgent->explanationMemory->is_any_enabled())
//            {
//                /* We erase some stuff and stash this preference in inst->preferences_cached
//                 * This is needed in case preferences are retracted for an instantiation that is
//                 * part of an explanation */
//                insert_at_head_of_dll(pref->inst->preferences_cached, pref, inst_next, inst_prev);
//                if (pref->wma_o_set)
//                {
//                    wma_remove_pref_o_set(thisAgent, pref);
//                }
//                pref->wma_o_set = NULL;
//                /* Don't want this information or have the other things cleaned up.  This will
//                 * also force the preference to be cleaned up when the instantiation gets
//                 * deallocated.  (b/c pref->inst is null, so it won't go into this part) */
//                pref->inst = NULL;
//                pref->in_tm = false;
//                pref->on_goal_list = false;
//                pref->reference_count = 0;
//                pref->slot = NULL;
//                pref->total_preferences_for_candidate = 1;
//                pref->rl_contribution = false;
//                pref->rl_rho = 0;
//
//                /* Don't want to copy links to other preferences, except inst_next/prev b/c we're using that
//                 * to link cached preferences */
//                pref->next_clone = NULL;
//                pref->prev_clone = NULL;
//                pref->next = NULL;
//                pref->prev = NULL;
//                pref->all_of_slot_next = NULL;
//                pref->all_of_slot_prev = NULL;
//                pref->all_of_goal_next = NULL;
//                pref->all_of_goal_prev = NULL;
//                pref->next_candidate = NULL;
//                pref->next_result = NULL;
//                return;
//            }
//            dprint(DT_DEALLOCATE_INST, "Possibly deallocating instantiation %u (match of %y) for preference.\n", prefInst->i_id, prefInst->prod_name);
//            possibly_deallocate_instantiation(thisAgent, prefInst);
//        }

    PDI_remove(thisAgent, pref);

    /*  dereference component symbols */
    thisAgent->symbolManager->symbol_remove_ref(&pref->id);
    thisAgent->symbolManager->symbol_remove_ref(&pref->attr);
    thisAgent->symbolManager->symbol_remove_ref(&pref->value);
    if (preference_is_binary(pref->type))
    {
        thisAgent->symbolManager->symbol_remove_ref(&pref->referent);
    }
    if (pref->wma_o_set)
    {
        wma_remove_pref_o_set(thisAgent, pref);
    }

    if (pref->rhs_funcs.id) deallocate_rhs_value(thisAgent, pref->rhs_funcs.id);
    if (pref->rhs_funcs.attr) deallocate_rhs_value(thisAgent, pref->rhs_funcs.attr);
    if (pref->rhs_funcs.value) deallocate_rhs_value(thisAgent, pref->rhs_funcs.value);
    if (pref->rhs_funcs.referent) deallocate_rhs_value(thisAgent, pref->rhs_funcs.referent);
    if (pref->cloned_rhs_funcs.id) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.id);
    if (pref->cloned_rhs_funcs.attr) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.attr);
    if (pref->cloned_rhs_funcs.value) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.value);
    if (pref->cloned_rhs_funcs.referent) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.referent);

    /*  free the memory */
    thisAgent->memoryManager->free_with_pool(MP_preference, pref);
}

/* -------------------------------------------------------------------
   Possibly_deallocate_preference_and_clones() checks whether a given
   preference and all its clones have reference_count 0, and deallocates
   them all if they do.  It returns true if they were actually
   deallocated, false otherwise.

   Note:  If this code changes, similar changes may need to be made in
          deallocate_instantiation, which had to use a flattened out
          version of this code to avoid a stack overflow in certain
          cases.
-------------------------------------------------------------------*/

bool possibly_deallocate_preference_and_clones(agent* thisAgent, preference* pref, bool dont_cache)
{
    preference* clone, *next;

    dprint(DT_DEALLOCATE_PREF, "Possibly deallocating preference %p and clones...\n", pref);
    if (pref->reference_count)
    {
        return false;
    }
    for (clone = pref->next_clone; clone != NIL; clone = clone->next_clone)
        if (clone->reference_count)
        {
            return false;
        }
    for (clone = pref->prev_clone; clone != NIL; clone = clone->prev_clone)
        if (clone->reference_count)
        {
            return false;
        }

    dprint(DT_DEALLOCATE_PREF, "Deallocating clones of %p...\n", pref);
    /*  deallocate all the clones */
    clone = pref->next_clone;
    while (clone)
    {
        next = clone->next_clone;
        deallocate_preference(thisAgent, clone, dont_cache);
        clone = next;
    }
    clone = pref->prev_clone;
    while (clone)
    {
        next = clone->prev_clone;
        deallocate_preference(thisAgent, clone, dont_cache);
        clone = next;
    }

    /*  deallocate pref */
    deallocate_preference(thisAgent, pref, dont_cache);

    return true;
}

/* -------------------------------------------------------------------
   Remove_preference_from_clones() splices a given preference out of the
   list of clones.  If the preference's reference_count is 0, it also
   deallocates it and returns true.  Otherwise it returns false.
-------------------------------------------------------------------*/

bool remove_preference_from_clones(agent* thisAgent, preference* pref)
{
    preference* any_clone;

    any_clone = NIL;
    if (pref->next_clone)
    {
        any_clone = pref->next_clone;
        pref->next_clone->prev_clone = pref->prev_clone;
    }
    if (pref->prev_clone)
    {
        any_clone = pref->prev_clone;
        pref->prev_clone->next_clone = pref->next_clone;
    }
    pref->next_clone = pref->prev_clone = NIL;
    if (any_clone)
    {
        possibly_deallocate_preference_and_clones(thisAgent, any_clone);
    }
    if (!pref->reference_count)
    {
        deallocate_preference(thisAgent, pref);
        return true;
    }
    else
    {
        return false;
    }
}

/* ---------------------------------------------------------------------
   Add_preference_to_tm() adds a given preference to preference memory (and
   hence temporary memory).
---------------------------------------------------------------------*/

bool add_preference_to_tm(agent* thisAgent, preference* pref)
{
    dprint(DT_PREFS, "Adding preference %p to temporary memory\n", pref);

    slot* s = make_slot(thisAgent, pref->id, pref->attr);
    preference* p2;

    if (!thisAgent->Decider->settings[DECIDER_KEEP_TOP_OPREFS] && (pref->inst->match_goal == thisAgent->top_state) && pref->o_supported && !s->isa_context_slot && (pref->type == ACCEPTABLE_PREFERENCE_TYPE) )
    {
        /* We could potentially cache a list of all o-supported values in the slot so that we don't have to iterate
         * through all of the preferences. We would need to maintain the list as we decide non-context slots */
        bool already_top_o_supported = false;

        for (p2 = s->all_preferences; (p2 && !already_top_o_supported); p2 = p2->all_of_slot_next)
        {
            if ((p2->value == pref->value) && p2->o_supported && (p2->inst->match_goal == thisAgent->top_state))
            {
                already_top_o_supported = true;
            }
        }

        if (already_top_o_supported)
        {
            dprint(DT_PREFS, "...not adding because already o-supported on top state.\n");
            dprint(DT_DEALLOCATE_PREF, "...not adding pref %p because already o-supported on top state.\n", pref);
            return false;
        }
    }

    pref->slot = s;

    insert_at_head_of_dll(s->all_preferences, pref, all_of_slot_next, all_of_slot_prev);

    /*  add to preference list of slot (according to match goal level of the instantiations) */

    if (!s->preferences[pref->type])
    {
        /*  this is the only pref. of its type, just put it at the head */
        insert_at_head_of_dll(s->preferences[pref->type], pref, next, prev);
    }
    else if (s->preferences[pref->type]->inst->match_goal_level >= pref->inst->match_goal_level)
    {
        /*  it belongs at the head of the list, so put it there */
        insert_at_head_of_dll(s->preferences[pref->type], pref, next, prev);
    }
    else
    {
        /*  scan through the pref. list, find the one to insert after */
        for (p2 = s->preferences[pref->type]; p2->next != NIL; p2 = p2->next)
        {
            if (p2->next->inst->match_goal_level >= pref->inst->match_goal_level)
            {
                break;
            }
        }

        /*  insert pref after p2 */
        pref->next = p2->next;
        pref->prev = p2;
        p2->next = pref;
        if (pref->next)
        {
            pref->next->prev = pref;
        }
    }

    pref->in_tm = true;
    preference_add_ref(pref);

    /* If the slot is unchanged but has some references laying around, we clear them
     * This does not lead to immediate memory deallocate/allocate but once the WMEs are
     * resolved, this should free the memory. */
    if (wma_enabled(thisAgent) && !s->isa_context_slot)
    {
        if (!s->changed)
        {
            if (s->wma_val_references != NIL)
            {
                s->wma_val_references->clear();
            }
        }
    }

    mark_slot_as_changed(thisAgent, s);

    if (wma_enabled(thisAgent) && !s->isa_context_slot)
    {
        bool exists = false;
        wme* w = pref->slot->wmes;
        while (!exists && w)
        {
            if (w->value == pref->value)
            {
                exists = true;
            }

            w = w->next;
        }

        /* If wme exists, it should already have been updated during assertion of new preferences */
        if (!exists)
        {
            if (s->wma_val_references == NIL)
            {
                thisAgent->memoryManager->allocate_with_pool(MP_wma_slot_refs, &(s->wma_val_references));
#ifdef USE_MEM_POOL_ALLOCATORS
                s->wma_val_references = new(s->wma_val_references) wma_sym_reference_map(std::less< Symbol* >(), soar_module::soar_memory_pool_allocator< std::pair< Symbol*, uint64_t > >());
#else
                s->wma_val_references = new(s->wma_val_references) wma_sym_reference_map();
#endif
            }

            (*s->wma_val_references)[ pref->value ]++;
        }
    }

    /*  update identifier levels */
    if (pref->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link addition for id %y and value %y.\n", pref->id, pref->value);
        post_link_addition(thisAgent, pref->id, pref->value);
    }
#ifdef DEBUG_ATTR_AS_LINKS
    if (pref->attr->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link addition for id %y and attr %y.\n", pref->id, pref->attr);
        post_link_addition(thisAgent, pref->id, pref->attr);
        /* Do we need to link to value if it's an identifier? If so may need to link referent to attribute and value as well */
        //        if (pref->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        //        {
        //            post_link_addition(thisAgent, pref->id, pref->value);
        //        }
    }
#endif

    if (preference_is_binary(pref->type))
    {
        if (pref->referent->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
            dprint(DT_WME_CHANGES, "Calling post-link addition for id %y and referent %y.\n", pref->id, pref->referent);
            post_link_addition(thisAgent, pref->id, pref->referent);
        }
    }

    /*  if acceptable/require pref for context slot, we may need to add a wme later */
    if ((s->isa_context_slot) &&  ((pref->type == ACCEPTABLE_PREFERENCE_TYPE) || (pref->type == REQUIRE_PREFERENCE_TYPE)))
    {
        mark_context_slot_as_acceptable_preference_changed(thisAgent, s);
    }

    return true;
}

/* ---------------------------------------------------------------------
   Remove_preference_from_tm() removes a given preference from PM and TM.
---------------------------------------------------------------------*/

void remove_preference_from_tm(agent* thisAgent, preference* pref)
{
    slot* s;

    s = pref->slot;

    dprint(DT_PREFS, "Removing preference %p from temporary memory\n", pref);

    /*  remove preference from the list for the slot */
    remove_from_dll(s->all_preferences, pref, all_of_slot_next, all_of_slot_prev);
    remove_from_dll(s->preferences[pref->type], pref, next, prev);

    /*  other miscellaneous stuff */
    pref->in_tm = false;
    pref->slot = NIL;      /* BUG shouldn't we use pref->slot in place of pref->in_tm? */
    mark_slot_as_changed(thisAgent, s);

    /*  if acceptable/require pref for context slot, we may need to remove
       a wme later */
    if ((s->isa_context_slot) &&
            ((pref->type == ACCEPTABLE_PREFERENCE_TYPE) ||
             (pref->type == REQUIRE_PREFERENCE_TYPE)))
    {
        mark_context_slot_as_acceptable_preference_changed(thisAgent, s);
    }

    /*  update identifier levels */
    if (pref->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link removal for id %y and value %y.\n", pref->id, pref->value);
        post_link_removal(thisAgent, pref->id, pref->value);
    }
#ifdef DEBUG_ATTR_AS_LINKS
    if (pref->attr->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        dprint(DT_WME_CHANGES, "Calling post-link removal for id %y and attr %y.\n", pref->id, pref->attr);
        post_link_removal(thisAgent, pref->id, pref->attr);
        /* Do we need to link to value if it's an identifier? If so may need to link referent to attribute and value as well */
//        if (pref->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
//        {
//            post_link_addition(thisAgent, pref->id, pref->value);
//        }
    }
#endif
    if (preference_is_binary(pref->type))
        if (pref->referent->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
            dprint(DT_WME_CHANGES, "Calling post-link removal for id %y and referent %y.\n", pref->id, pref->referent);
            post_link_removal(thisAgent, pref->id, pref->referent);
        }

    /*  deallocate it and clones if possible */
    preference_remove_ref(thisAgent, pref);
}

/* ---------------------------------------------------------------------
   Process_o_rejects_and_deallocate_them() handles the processing of
   o-supported reject preferences.  This routine is called from the firer
   and passed a list of all the o-rejects generated in the current
   preference phase (the list is linked via the "next" fields on the
   preference structures).  This routine removes all preferences for
   matching values from TM, and deallocates the o-reject preferences when
   done.
---------------------------------------------------------------------*/

void process_o_rejects_and_deallocate_them(agent* thisAgent, preference* o_rejects, preference_list& bufdeallo)
{
    preference* pref, *next_pref, *p, *next_p;
    slot* s;

    for (pref = o_rejects; pref != NIL; pref = pref->next)
    {
        preference_add_ref(pref);  /* prevents it from being deallocated if it's
                                   a clone of some other pref we're about to
                                   remove */
        dprint(DT_PREFS, "O-reject preferences posted: %p\n", pref);
    }

    pref = o_rejects;
    while (pref)
    {
        next_pref = pref->next;
        s = find_slot(pref->id, pref->attr);
        if (s)
        {
            /*  remove all pref's in the slot that have the same value */
            p = s->all_preferences;
            while (p)
            {
                next_p = p->all_of_slot_next;
                if (p->value == pref->value)
                {
                    // Buffer deallocation by adding a reference here and putting it
                    // on a list. These are deallocated after the inner elaboration
                    // loop completes.
                    preference_add_ref(p);
                    bufdeallo.push_back(p);
                    remove_preference_from_tm(thisAgent, p);
                }
                p = next_p;
            }
        }
        preference_remove_ref(thisAgent, pref);
        pref = next_pref;
    }
}

void clear_preference_list(agent* thisAgent, cons* &pPrefList)
{
    cons *c;
    preference* lPref;

    if (pPrefList)
    {
        lPref = NIL;
        for (c = pPrefList; c != NIL; c = c->rest)
        {
            lPref = static_cast<preference*>(c->first);
            preference_remove_ref(thisAgent, lPref, true);
        }
        free_list(thisAgent, pPrefList);
        pPrefList = NULL;
    }
}
