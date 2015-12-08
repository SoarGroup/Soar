#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* ======================================================================
        Preference Memory routines
   ====================================================================== */

#include <stdlib.h>
#include "prefmem.h"

#include "mem.h"
#include "kernel.h"
#include "agent.h"
#include "debug.h"
#include "instantiations.h"
#include "symtab.h"
#include "recmem.h"
#include "tempmem.h"
#include "decide.h"
#include "print.h"
#include "wmem.h"

/* Note that these must be in the same order as the #define variables
 * in gdatastructs.h */

const char* preference_name[] =
{
    "acceptable",
    "require",
    "reject",
    "prohibit",
    "reconsider",
    "unary indifferent",
    "unary parallel",
    "best",
    "worst",
    "binary indifferent",
    "binary parallel",
    "better",
    "worse",
    "numeric indifferent"
};


/*                     Preference Management Routines

====================================================================== */

/* ----------------------------------------------------------------------
   Make_preference() creates a new preference structure of the given type
   with the given id/attribute/value/referent.  (Referent is only used
   for binary preferences.)  The preference is not yet added to preference
   memory, however.

   The last three parameters are original variable names used by the chunker
   and are optional/have default nil values.
---------------------------------------------------------------------- */

preference* make_preference(agent* thisAgent, byte type, Symbol* id, Symbol* attr,
                            Symbol* value, Symbol* referent,
                            const soar_module::identity_triple o_ids,
                            const soar_module::rhs_triple rhs_funcs)
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

    p->o_ids.id = o_ids.id;
    p->o_ids.attr = o_ids.attr;
    p->o_ids.value = o_ids.value;

    p->rhs_funcs.id = rhs_funcs.id;
    p->rhs_funcs.attr = rhs_funcs.attr;
    p->rhs_funcs.value = rhs_funcs.value;

#ifdef DEBUG_PREFS
    print(thisAgent, "\nAllocating preference at 0x%8x: ", reinterpret_cast<uintptr_t>(p));
    print_preference(thisAgent, p);
#endif

    return p;

    /* BUGBUG check to make sure the pref doesn't have
          value or referent .isa_goal or .isa_impasse; */
}

/* ----------------------------------------------------------------------
   Deallocate_preference() deallocates a given preference.
---------------------------------------------------------------------- */

void deallocate_preference(agent* thisAgent, preference* pref)
{
    dprint(DT_DEALLOCATES, "Deallocating preference %p\n", pref);

    assert(pref->reference_count == 0);

    /* --- remove it from the list of pref's for its match goal --- */
    if (pref->on_goal_list)
        remove_from_dll(pref->inst->match_goal->id->preferences_from_goal,
                        pref, all_of_goal_next, all_of_goal_prev);

    /* --- remove it from the list of pref's from that instantiation --- */
    remove_from_dll(pref->inst->preferences_generated, pref,
                    inst_next, inst_prev);
    possibly_deallocate_instantiation(thisAgent, pref->inst);

    /* --- dereference component symbols --- */
    symbol_remove_ref(thisAgent, pref->id);
    symbol_remove_ref(thisAgent, pref->attr);
    symbol_remove_ref(thisAgent, pref->value);
    if (preference_is_binary(pref->type))
    {
        symbol_remove_ref(thisAgent, pref->referent);
    }
    if (pref->wma_o_set)
    {
        wma_remove_pref_o_set(thisAgent, pref);
    }

    /* --- free the memory --- */
    thisAgent->memoryManager->free_with_pool(MP_preference, pref);
}

/* ----------------------------------------------------------------------
   Possibly_deallocate_preference_and_clones() checks whether a given
   preference and all its clones have reference_count 0, and deallocates
   them all if they do.  It returns true if they were actually
   deallocated, false otherwise.
---------------------------------------------------------------------- */

bool possibly_deallocate_preference_and_clones(agent* thisAgent, preference* pref)
{
    preference* clone, *next;

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

    /* --- deallocate all the clones --- */
    clone = pref->next_clone;
    while (clone)
    {
        next = clone->next_clone;
        deallocate_preference(thisAgent, clone);
        clone = next;
    }
    clone = pref->prev_clone;
    while (clone)
    {
        next = clone->prev_clone;
        deallocate_preference(thisAgent, clone);
        clone = next;
    }

    /* --- deallocate pref --- */
    deallocate_preference(thisAgent, pref);

    return true;
}

/* ----------------------------------------------------------------------
   Remove_preference_from_clones() splices a given preference out of the
   list of clones.  If the preference's reference_count is 0, it also
   deallocates it and returns true.  Otherwise it returns false.
---------------------------------------------------------------------- */

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
    if (! pref->reference_count)
    {
        deallocate_preference(thisAgent, pref);
        return true;
    }
    else
    {
        return false;
    }
}

/* ------------------------------------------------------------------------
   Add_preference_to_tm() adds a given preference to preference memory (and
   hence temporary memory).
------------------------------------------------------------------------ */

bool add_preference_to_tm(agent* thisAgent, preference* pref)
{
#ifdef DEBUG_PREFS
    print(thisAgent, "\nAdd preference at 0x%8x:  ", reinterpret_cast<uintptr_t>(pref));
    print_preference(thisAgent, pref);
#endif

    slot* s = make_slot(thisAgent, pref->id, pref->attr);
    preference* p2;

	if (!s->isa_context_slot && pref->o_supported && (pref->type == ACCEPTABLE_PREFERENCE_TYPE) && (pref->inst->match_goal == thisAgent->top_state))
    {
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
            // NLD: if it is suspected that this code is causing an issue, simply comment out the following line to debug.
            return false;
        }
    }

	pref->slot = s;

    insert_at_head_of_dll(s->all_preferences, pref,
                          all_of_slot_next, all_of_slot_prev);

    /* --- add preference to the list (in the right place, according to match
           goal level of the instantiations) for the slot --- */

    if (!s->preferences[pref->type])
    {
        /* --- this is the only pref. of its type, just put it at the head --- */
        insert_at_head_of_dll(s->preferences[pref->type], pref, next, prev);
    }
    else if (s->preferences[pref->type]->inst->match_goal_level >= pref->inst->match_goal_level)
    {
        /* --- it belongs at the head of the list, so put it there --- */
        insert_at_head_of_dll(s->preferences[pref->type], pref, next, prev);
    }
    else
    {
        /* --- scan through the pref. list, find the one to insert after --- */
        for (p2 = s->preferences[pref->type]; p2->next != NIL; p2 = p2->next)
        {
            if (p2->next->inst->match_goal_level >= pref->inst->match_goal_level)
            {
                break;
            }
        }

        /* --- insert pref after p2 --- */
        pref->next = p2->next;
        pref->prev = p2;
        p2->next = pref;
        if (pref->next)
        {
            pref->next->prev = pref;
        }
    }

    /* --- other miscellaneous stuff --- */
    pref->in_tm = true;
    preference_add_ref(pref);

    // if it's the case that the slot is unchanged, but has
    // some references laying around, clear them
    // this doesn't cause immediate memory deallocate/allocate
    // but once the WMEs are resolved, this should free the
    // memory, as opposed to lead to a "leak"
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

        // if wme exists, it should already have been updated
        // during assertion of new preferences
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

    /* --- update identifier levels --- */
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

    /* --- if acceptable/require pref for context slot, we may need to add a
    wme later --- */
    if ((s->isa_context_slot) &&
            ((pref->type == ACCEPTABLE_PREFERENCE_TYPE) ||
             (pref->type == REQUIRE_PREFERENCE_TYPE)))
    {
        mark_context_slot_as_acceptable_preference_changed(thisAgent, s);
    }

    return true;
}

/* ------------------------------------------------------------------------
   Remove_preference_from_tm() removes a given preference from PM and TM.
------------------------------------------------------------------------ */

void remove_preference_from_tm(agent* thisAgent, preference* pref)
{
    slot* s;

    s = pref->slot;

#ifdef DEBUG_PREFS
    print(thisAgent, "\nRemove preference at 0x%8x:  ", reinterpret_cast<uintptr_t>(pref));
    print_preference(thisAgent, pref);
#endif

    /* --- remove preference from the list for the slot --- */
    remove_from_dll(s->all_preferences, pref,
                    all_of_slot_next, all_of_slot_prev);
    remove_from_dll(s->preferences[pref->type], pref, next, prev);

    /* --- other miscellaneous stuff --- */
    pref->in_tm = false;
    pref->slot = NIL;      /* BUG shouldn't we use pref->slot in place of pref->in_tm? */
    mark_slot_as_changed(thisAgent, s);

    /* --- if acceptable/require pref for context slot, we may need to remove
       a wme later --- */
    if ((s->isa_context_slot) &&
            ((pref->type == ACCEPTABLE_PREFERENCE_TYPE) ||
             (pref->type == REQUIRE_PREFERENCE_TYPE)))
    {
        mark_context_slot_as_acceptable_preference_changed(thisAgent, s);
    }

    /* --- update identifier levels --- */
    if (pref->value->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        post_link_removal(thisAgent, pref->id, pref->value);
    }
    if (preference_is_binary(pref->type))
        if (pref->referent->symbol_type == IDENTIFIER_SYMBOL_TYPE)
        {
            post_link_removal(thisAgent, pref->id, pref->referent);
        }

    /* --- deallocate it and clones if possible --- */
    preference_remove_ref(thisAgent, pref);
}

/* ------------------------------------------------------------------------
   Process_o_rejects_and_deallocate_them() handles the processing of
   o-supported reject preferences.  This routine is called from the firer
   and passed a list of all the o-rejects generated in the current
   preference phase (the list is linked via the "next" fields on the
   preference structures).  This routine removes all preferences for
   matching values from TM, and deallocates the o-reject preferences when
   done.
------------------------------------------------------------------------ */

void process_o_rejects_and_deallocate_them(agent* thisAgent, preference* o_rejects, pref_buffer_list& bufdeallo)
{
    preference* pref, *next_pref, *p, *next_p;
    slot* s;

    for (pref = o_rejects; pref != NIL; pref = pref->next)
    {
        preference_add_ref(pref);  /* prevents it from being deallocated if it's
                                   a clone of some other pref we're about to
                                   remove */
#ifdef DEBUG_PREFS
        print(thisAgent, "\nO-reject posted at 0x%8x:  ", reinterpret_cast<uintptr_t>(pref));
        print_preference(thisAgent, pref);
#endif
    }

    pref = o_rejects;
    while (pref)
    {
        next_pref = pref->next;
        s = find_slot(pref->id, pref->attr);
        if (s)
        {
            /* --- remove all pref's in the slot that have the same value --- */
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

