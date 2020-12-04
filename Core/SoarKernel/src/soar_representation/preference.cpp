#include "preference.h"

#include "agent.h"
#include "decide.h"
#include "decider.h"
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

    p->inst_identities = { o_ids.id, o_ids.attr, o_ids.value,  o_ids.referent };
    p->identities = { NULL_IDENTITY_SET, NULL_IDENTITY_SET, NULL_IDENTITY_SET, NULL_IDENTITY_SET };
    p->rhs_func_inst_identities = { NULL, NULL, NULL, NULL };
    p->chunk_inst_identities = { LITERAL_VALUE, LITERAL_VALUE, LITERAL_VALUE, LITERAL_VALUE };
    p->rhs_func_chunk_inst_identities = { NULL, NULL, NULL, NULL };
    p->was_unbound_vars.id = pWas_unbound_vars.id;
    p->was_unbound_vars.attr = pWas_unbound_vars.attr;
    p->was_unbound_vars.value = pWas_unbound_vars.value;
    p->was_unbound_vars.referent = pWas_unbound_vars.referent;

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

    p->rhs_func_chunk_inst_identities = { NULL, NULL, NULL, NULL };
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

    p->inst_identities = {pPref->inst_identities.id, pPref->inst_identities.attr, pPref->inst_identities.value, pPref->inst_identities.referent};
    p->chunk_inst_identities = {pPref->chunk_inst_identities.id, pPref->chunk_inst_identities.attr, pPref->chunk_inst_identities.value, pPref->chunk_inst_identities.referent};

    p->identities = { NULL, NULL, NULL, NULL };
    set_pref_identity(thisAgent, p, ID_ELEMENT, pPref->identities.id);
    set_pref_identity(thisAgent, p, ATTR_ELEMENT, pPref->identities.attr);
    set_pref_identity(thisAgent, p, VALUE_ELEMENT, pPref->identities.value);
    set_pref_identity(thisAgent, p, REFERENT_ELEMENT, pPref->identities.referent);

    p->rhs_func_inst_identities.id = copy_rhs_value(thisAgent, pPref->rhs_func_inst_identities.id);
    p->rhs_func_inst_identities.attr = copy_rhs_value(thisAgent, pPref->rhs_func_inst_identities.attr);
    p->rhs_func_inst_identities.value = copy_rhs_value(thisAgent, pPref->rhs_func_inst_identities.value);
    p->rhs_func_inst_identities.referent = copy_rhs_value(thisAgent, pPref->rhs_func_inst_identities.referent);

    return p;

    /* BUGBUG check to make sure the pref doesn't have
          value or referent .isa_goal or .isa_impasse; */
}

void cache_preference_if_necessary(agent* thisAgent, preference* pref)
{
    if ((pref->inst->match_goal_level != TOP_GOAL_LEVEL) && thisAgent->explanationMemory->is_any_enabled())
    {
        preference* lNewPref = shallow_copy_preference(thisAgent, pref);
        insert_at_head_of_dll(pref->inst->preferences_cached, lNewPref, inst_next, inst_prev);
    }
}

void deallocate_preference_contents(agent* thisAgent, preference* pref, bool dont_cache)
{
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

    if (pref->identities.id) IdentitySet_remove_ref(thisAgent, pref->identities.id);
    if (pref->identities.attr) IdentitySet_remove_ref(thisAgent, pref->identities.attr);
    if (pref->identities.value) IdentitySet_remove_ref(thisAgent, pref->identities.value);
    if (pref->identities.referent) IdentitySet_remove_ref(thisAgent, pref->identities.referent);

    if (pref->rhs_func_inst_identities.id) deallocate_rhs_value(thisAgent, pref->rhs_func_inst_identities.id);
    if (pref->rhs_func_inst_identities.attr) deallocate_rhs_value(thisAgent, pref->rhs_func_inst_identities.attr);
    if (pref->rhs_func_inst_identities.value) deallocate_rhs_value(thisAgent, pref->rhs_func_inst_identities.value);
    if (pref->rhs_func_inst_identities.referent) deallocate_rhs_value(thisAgent, pref->rhs_func_inst_identities.referent);
    if (pref->rhs_func_chunk_inst_identities.id) deallocate_rhs_value(thisAgent, pref->rhs_func_chunk_inst_identities.id);
    if (pref->rhs_func_chunk_inst_identities.attr) deallocate_rhs_value(thisAgent, pref->rhs_func_chunk_inst_identities.attr);
    if (pref->rhs_func_chunk_inst_identities.value) deallocate_rhs_value(thisAgent, pref->rhs_func_chunk_inst_identities.value);
    if (pref->rhs_func_chunk_inst_identities.referent) deallocate_rhs_value(thisAgent, pref->rhs_func_chunk_inst_identities.referent);

    thisAgent->memoryManager->free_with_pool(MP_preference, pref);

}

/* IMPORTANT: Any changes made to deallocate_preference should also be made to corresponding code in deallocate_instantiation */
void deallocate_preference(agent* thisAgent, preference* pref, bool dont_cache)
{
    /*  Remove from temporary memory and match goal if necessary */
    if (pref->in_tm) remove_preference_from_tm(thisAgent, pref);
    if (pref->on_goal_list) remove_from_dll(pref->inst->match_goal->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);

    if (pref->inst)
    {
        if (!dont_cache) cache_preference_if_necessary(thisAgent, pref);
        remove_from_dll(pref->inst->preferences_generated, pref, inst_next, inst_prev);
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

    if (pref->reference_count) return false;

    for (clone = pref->next_clone; clone != NIL; clone = clone->next_clone)
        if (clone->reference_count) return false;
    for (clone = pref->prev_clone; clone != NIL; clone = clone->prev_clone)
        if (clone->reference_count) return false;

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
        post_link_addition(thisAgent, pref->id, pref->value);
    }
    if (preference_is_binary(pref->type))
    {
        if (pref->referent->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
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
        post_link_removal(thisAgent, pref->id, pref->value);
    }
    if (preference_is_binary(pref->type))
        if (pref->referent->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
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
