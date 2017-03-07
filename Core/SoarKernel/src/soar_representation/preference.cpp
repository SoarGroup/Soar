#include "preference.h"

#include "agent.h"
#include "debug_inventories.h"
#include "decide.h"
#include "decider.h"
#include "dprint.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "mem.h"
#include "output_manager.h"
#include "print.h"
#include "rhs.h"
#include "slot.h"
#include "symbol.h"
#include "working_memory.h"
#include "working_memory_activation.h"

#include <stdlib.h>

/* -------------------------------------------------------------------
   Make_preference() creates a new preference structure of the given type
   with the given id/attribute/value/referent.  (Referent is only used
   for binary preferences.)  The preference is not yet added to preference
   memory, however.
-------------------------------------------------------------------*/

preference* make_preference(agent* thisAgent, PreferenceType type, 
                            Symbol* id, Symbol* attr, Symbol* value, Symbol* referent,
                            const identity_quadruple &o_ids, const bool_quadruple &pWas_unbound_vars)
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
    p->level = 0;

    p->identities = { o_ids.id, o_ids.attr, o_ids.value,  o_ids.referent };
    p->identity_sets = { NULL_IDENTITY_SET, NULL_IDENTITY_SET, NULL_IDENTITY_SET, NULL_IDENTITY_SET };
    p->rhs_funcs = { NULL, NULL, NULL, NULL };
    p->clone_identities = { LITERAL_VALUE, LITERAL_VALUE, LITERAL_VALUE, LITERAL_VALUE };
    p->cloned_rhs_funcs = { NULL, NULL, NULL, NULL };
    p->was_unbound_vars = { pWas_unbound_vars.id, pWas_unbound_vars.attr, pWas_unbound_vars.value, pWas_unbound_vars.referent };

    PDI_add(thisAgent, p);
    dprint(DT_PREFS, "Created preference %p\n", p);

    return p;
}

/* This function just copies the elements of a preferences we need for the EBC explanation mechanism */
preference* shallow_copy_preference(agent* thisAgent, preference* pPref)
{
    preference* p;

    thisAgent->memoryManager->allocate_with_pool(MP_preference, &p);

    /* Initialize stuff we don't want to copy over */
    p->inst = NULL;
    p->in_tm = false;
    p->on_goal_list = false;
    p->reference_count = 0;
    p->slot = NIL;
    p->total_preferences_for_candidate = 0;
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

    p->cloned_rhs_funcs = { NULL, NULL, NULL, NULL };
    p->was_unbound_vars = { false, false, false, false };

    /* Now copy over stuff that we do want */

    p->type = pPref->type;
    p->numeric_value = pPref->numeric_value;
    p->o_supported = pPref->o_supported;
    p->level = pPref->level;

    p->id = pPref->id;
    p->attr = pPref->attr;
    p->value = pPref->value;
    p->referent = pPref->referent;
    thisAgent->symbolManager->symbol_add_ref(p->id);
    thisAgent->symbolManager->symbol_add_ref(p->attr);
    thisAgent->symbolManager->symbol_add_ref(p->value);
    if (p->referent) thisAgent->symbolManager->symbol_add_ref(p->referent);

    p->identities = {pPref->identities.id, pPref->identities.attr, pPref->identities.value, pPref->identities.referent};
    p->clone_identities = {pPref->clone_identities.id, pPref->clone_identities.attr, pPref->clone_identities.value, pPref->clone_identities.referent};

    set_pref_identity_set(thisAgent, p, ID_ELEMENT, pPref->identity_sets.attr);
    set_pref_identity_set(thisAgent, p, ATTR_ELEMENT, pPref->identity_sets.id);
    set_pref_identity_set(thisAgent, p, VALUE_ELEMENT, pPref->identity_sets.value);
    set_pref_identity_set(thisAgent, p, REFERENT_ELEMENT, pPref->identity_sets.referent);

    PDI_add(thisAgent, p, true);
    break_if_id_matches(pPref->p_id, 26);

    p->rhs_funcs.id = copy_rhs_value(thisAgent, pPref->rhs_funcs.id);
    p->rhs_funcs.attr = copy_rhs_value(thisAgent, pPref->rhs_funcs.attr);
    p->rhs_funcs.value = copy_rhs_value(thisAgent, pPref->rhs_funcs.value);
    p->rhs_funcs.referent = copy_rhs_value(thisAgent, pPref->rhs_funcs.referent);

    dprint(DT_PREFS, "Created shallow copy of preference %p (%u)\n", p, p->p_id);

    return p;

    /* BUGBUG check to make sure the pref doesn't have
          value or referent .isa_goal or .isa_impasse; */
}
/* -------------------------------------------------------------------
   Deallocate_preference() deallocates a given preference.
-------------------------------------------------------------------*/
void cache_preference_if_necessary(agent* thisAgent, preference* pref)
{
    if ((pref->inst->match_goal_level != TOP_GOAL_LEVEL) && thisAgent->explanationMemory->is_any_enabled())
    {
        preference* lNewPref = shallow_copy_preference(thisAgent, pref);
        dprint(DT_EXPLAIN_CACHE, "Caching preference for instantiation %u (match of %y): %p\n", pref->inst->i_id, pref->inst->prod_name, pref);
        insert_at_head_of_dll(pref->inst->preferences_cached, lNewPref, inst_next, inst_prev);
    }
}

void deallocate_preference_contents(agent* thisAgent, preference* pref, bool dont_cache)
{
    PDI_remove(thisAgent, pref);
    debug_refcount_change_start(thisAgent, false);

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

    if (pref->identity_sets.id) IdentitySet_remove_ref(thisAgent, pref->identity_sets.id);
    if (pref->identity_sets.attr) IdentitySet_remove_ref(thisAgent, pref->identity_sets.attr);
    if (pref->identity_sets.value) IdentitySet_remove_ref(thisAgent, pref->identity_sets.value);
    if (pref->identity_sets.referent) IdentitySet_remove_ref(thisAgent, pref->identity_sets.referent);

    if (pref->rhs_funcs.id) deallocate_rhs_value(thisAgent, pref->rhs_funcs.id);
    if (pref->rhs_funcs.attr) deallocate_rhs_value(thisAgent, pref->rhs_funcs.attr);
    if (pref->rhs_funcs.value) deallocate_rhs_value(thisAgent, pref->rhs_funcs.value);
    if (pref->rhs_funcs.referent) deallocate_rhs_value(thisAgent, pref->rhs_funcs.referent);
    if (pref->cloned_rhs_funcs.id) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.id);
    if (pref->cloned_rhs_funcs.attr) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.attr);
    if (pref->cloned_rhs_funcs.value) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.value);
    if (pref->cloned_rhs_funcs.referent) deallocate_rhs_value(thisAgent, pref->cloned_rhs_funcs.referent);

    debug_refcount_change_end(thisAgent, (std::string((pref->inst && pref->in_tm) ? pref->inst->prod_name ? pref->inst->prod_name->sc->name : "DEALLOCATED INST" : "DEALLOCATED INST" ) + std::string(" preference deallocation")).c_str(), false);

    /* Sometimes I turn this on for debugging. */
    // pref->p_id = 23;

    thisAgent->memoryManager->free_with_pool(MP_preference, pref);

}

/* IMPORTANT: Any changes made to deallocate_preference should also be made to corresponding code in deallocate_instantiation */
void deallocate_preference(agent* thisAgent, preference* pref, bool dont_cache)
{
    /* We don't print the preference out directly with %p because identity set pointer may not be valid */
    dprint(DT_DEALLOCATE_PREF, "Deallocating preference p%u (^%y ^%y ^%y) at level %d\n", pref->p_id, pref->id, pref->attr, pref->value, static_cast<int64_t>(pref->level));
    assert(pref->reference_count == 0);
//    break_if_pref_matches_string(pref, "L5", "value", "bar");
    /*  Remove from temporary memory and match goal if necessary */
    if (pref->in_tm) remove_preference_from_tm(thisAgent, pref);
    if (pref->on_goal_list) remove_from_dll(pref->inst->match_goal->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);

    /* Cache preference for explainer if necessary, remove from preferences
     * asserted by instantiation and possibly deallocate parent
     * instantiation if this is the last pref that it is asserting */
    if (pref->inst)
    {
        if (!dont_cache) cache_preference_if_necessary(thisAgent, pref);
        remove_from_dll(pref->inst->preferences_generated, pref, inst_next, inst_prev);
        dprint(DT_DEALLOCATE_INST, "Possibly deallocating instantiation %u (match of %y) for preference.\n", pref->inst->i_id, pref->inst->prod_name);
        possibly_deallocate_instantiation(thisAgent, pref->inst);
    }

    /* Clean up contents and deallocate */
    deallocate_preference_contents(thisAgent, pref, dont_cache);

}

/* -------------------------------------------------------------------
   Possibly_deallocate_preference_and_clones() checks whether a given
   preference and all its clones have reference_count 0, and deallocates
   them all if they do.  It returns true if they were actually
   deallocated, false otherwise.
-------------------------------------------------------------------*/
/* IMPORTANT: Any changes made to possibly_deallocate_preference_and_clones should also be made to corresponding code in deallocate_instantiation */

bool possibly_deallocate_preference_and_clones(agent* thisAgent, preference* pref, bool dont_cache)
{
    preference* clone, *next;

    dprint(DT_DEALLOCATE_PREF, "Possibly deallocating preference and clones of %p (%u) at level %d...\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
    if (pref->reference_count) return false;

    for (clone = pref->next_clone; clone != NIL; clone = clone->next_clone)
        if (clone->reference_count) return false;
    for (clone = pref->prev_clone; clone != NIL; clone = clone->prev_clone)
        if (clone->reference_count) return false;

    dprint(DT_DEALLOCATE_PREF, "...deallocating clones...\n");
    clone = pref->next_clone;
    while (clone)
    {
        next = clone->next_clone;
        dprint(DT_DEALLOCATE_PREF, "...deallocating clone %p (%u) at level %d \n", clone, clone->p_id, static_cast<int64_t>(clone->level));
        deallocate_preference(thisAgent, clone, dont_cache);
        clone = next;
    }
    clone = pref->prev_clone;
    while (clone)
    {
        next = clone->prev_clone;
        dprint(DT_DEALLOCATE_PREF, "...deallocating clone %p (%u) at level %d \n", clone, clone->p_id, static_cast<int64_t>(clone->level));
        deallocate_preference(thisAgent, clone, dont_cache);
        clone = next;
    }

    deallocate_preference(thisAgent, pref, dont_cache);

    return true;
}

/* -------------------------------------------------------------------
   Remove_preference_from_clones() splices a given preference out of the
   list of clones.  If the preference's reference_count is 0, it also
   deallocates it and returns true.  Otherwise it returns false.
-------------------------------------------------------------------*/

bool remove_preference_from_clones_and_deallocate(agent* thisAgent, preference* pref)
{
    dprint(DT_DEALLOCATE_PREF, "Removing preference %p (%u) at level %d from clones...\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
    preference* any_clone = NIL;
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
        dprint(DT_DEALLOCATE_PREF, "...found clone %p (%u) at level %d to possibly deallocate...\n", any_clone, any_clone->p_id, static_cast<int64_t>(any_clone->level));
        possibly_deallocate_preference_and_clones(thisAgent, any_clone);
    }
    if (!pref->reference_count)
    {
        dprint(DT_DEALLOCATE_PREF, "...deallocating preference %p (%u) at level %d.\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
        deallocate_preference(thisAgent, pref);
        return true;
    }
    else
    {
        dprint(DT_DEALLOCATE_PREF, "...preference %p has been removed from clones.\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
        return false;
    }
}

/* ---------------------------------------------------------------------
   Add_preference_to_tm() adds a given preference to preference memory (and
   hence temporary memory).
---------------------------------------------------------------------*/

bool add_preference_to_tm(agent* thisAgent, preference* pref)
{
    dprint(DT_PREFS, "Adding preference  %p (%u) at level %d to temporary memory\n", pref, pref->p_id, static_cast<int64_t>(pref->level));

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
            dprint(DT_DEALLOCATE_PREF, "...not adding pref %p (%u) at level %d because already o-supported on top state.\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
            if (thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "%e+ ");
                print_preference(thisAgent, pref, false);
                thisAgent->outputManager->printa_sf(thisAgent,  " (%y) ALREADY SUPPORTED ON TOP LEVEL.  IGNORING.\n", pref->inst->prod_name);
            }
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
    if (thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "%e+ ");
        print_preference(thisAgent, pref, false);
        thisAgent->outputManager->printa_sf(thisAgent,  " (%y)\n", pref->inst->prod_name);
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

    dprint(DT_PREFS, "Removing preference %p (%u) at level %d from temporary memory\n", pref, pref->p_id, static_cast<int64_t>(pref->level));

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
    if (thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "%e- ");
        print_preference(thisAgent, pref, false);
        thisAgent->outputManager->printa_sf(thisAgent,  " (%y)\n", pref->inst->prod_name);
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
        dprint(DT_PREFS, "O-reject preferences posted: %p (%u) at level %d\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
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
                    dprint(DT_DEALLOCATE_PREF, "Pushing o-rejected preference %p (%u) at level %d to bufdeallo\n", p, p->p_id, static_cast<int64_t>(p->level));
                        bufdeallo.push_back(p);
                    remove_preference_from_tm(thisAgent, p);
                }
                p = next_p;
            }
        }
        dprint(DT_DEALLOCATE_PREF, "Removing ref for o-rejected preference %p (%u) at level %d to bufdeallo\n", pref, pref->p_id, static_cast<int64_t>(pref->level));
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
            dprint(DT_OSK, "Cleaning up OSK preference %p\n", lPref);
            preference_remove_ref(thisAgent, lPref, true);
        }
        free_list(thisAgent, pPrefList);
        pPrefList = NULL;
    }
}
