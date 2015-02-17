#include "portability.h"
#include "soar_rand.h" // provides SoarRand, a better random number generator (see bug 595)

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  decide.cpp
 *
 * =======================================================================
 *  Decider and Associated Routines
 *
 *  This file contains the decider as well as routine for managing
 *  slots, and the garbage collection of disconnected WMEs.
 * =======================================================================
 */


#include "decide.h"

#include "instantiations.h"
#include "mem.h"
#include "kernel.h"
#include "agent.h"
#include "symtab.h"
#include "wmem.h"
#include "init_soar.h"
#include "prefmem.h"
#include "production.h"
#include "print.h"
#include "trace.h"
#include "explain.h"
#include "tempmem.h"
#include "io_soar.h"
#include "xml.h"
#include "soar_TraceNames.h"

#include "soar_module.h"

#include "exploration.h"
#include "reinforcement_learning.h"
#include "decision_manipulation.h"
#include "wma.h"
#include "misc.h"

#include "episodic_memory.h"
#include "semantic_memory.h"
#ifndef NO_SVS
#include "svs_interface.h"
#endif
#include "test.h"
#include "debug.h"

#include "assert.h"

#include <algorithm>
#include <list>

using namespace soar_TraceNames;

#ifdef USE_MEM_POOL_ALLOCATORS
typedef std::list< Symbol*, soar_module::soar_memory_pool_allocator< Symbol* > > symbol_list;
#else
typedef std::list< Symbol* > symbol_list;
#endif

void print_candidates(agent* thisAgent, preference* candidates)
{
    preference* cand = 0;
    int max_count = 0;

    for (cand = candidates; cand != NIL; cand = cand->next_candidate)
    {
        max_count++;
        print(thisAgent,  "\n Candidate %d", cand);
        print_with_symbols(thisAgent, "\n    %y %y %y", cand->id, cand->attr, cand->value);
        if (max_count > 10)
        {
            break;
        }
    }
}

/* END: 2003-01-02 Behavior Variability Kernel Experiments */

//#endif

/* ------------------------------------------------------------------------
                     Decider Global Variables

   Top_goal and bottom_goal point to the top and bottom goal identifiers,
   respectively.  (If there is no goal stack at all, they're both NIL.)
   Top_state points to the top state (Symbol) if there is a top state, and
   is NIL of there isn't any top state selected.

   Highest_goal_whose_context_changed points to the identifier of the highest
   goal for which some context slot has changed preferences.  If no context
   slot has changed preferences, this variable is NIL.  This is used by
   the decider during decision phase to avoid scanning down the whole
   goal stack when (as is the usual case) it really only needs to look at
   the lowest context.

   Changed_slots is a dl_list of non-context slots with changed preferences.
   This is used by the decider during working memory phase to tell which
   slots need to be re-decided.

   Context_slots_with_changed_acceptable_preferences is a dl_list of
   context slots for which the set of acceptable or require preferences
   has changed.  This is used to update the acceptable preference WMEs.
------------------------------------------------------------------------ */

/* --------------------------------------------------
                   Decider Flags

   The decider often needs to mark symbols with
   certain flags, usually to record that the symbols
   are in certain sets or have a certain status.
   The "common.decider_flag" field on symbols is
   used for this, and is set to one of the following
   flag values.  (Usually only two or three of these
   values are used at once, and the meaning should
   be clear from the code.)
-------------------------------------------------- */

#define NOTHING_DECIDER_FLAG 0     /* Warning: code relies in this being 0 */
#define CANDIDATE_DECIDER_FLAG 1
#define CONFLICTED_DECIDER_FLAG 2
#define FORMER_CANDIDATE_DECIDER_FLAG 3
#define BEST_DECIDER_FLAG 4
#define WORST_DECIDER_FLAG 5
#define UNARY_INDIFFERENT_DECIDER_FLAG 6
#define ALREADY_EXISTING_WME_DECIDER_FLAG 7

/* REW: 2003-01-02 Behavior Variability Kernel Experiments
   A new preference type: unary indifferent + constant (probability) value
*/
#define UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG 8

/* ======================================================================

                  Acceptable Preference WME Routines

   Whenever some acceptable or require preference for a context slot
   changes, we call mark_context_slot_as_acceptable_preference_changed().

   At the end of the phase, do_buffered_acceptable_preference_wme_changes()
   is called to update the acceptable preference wmes.  This should be
   called *before* do_buffered_link_changes() and do_buffered_wm_changes().
====================================================================== */

void mark_context_slot_as_acceptable_preference_changed(agent* thisAgent, slot* s)
{
    dl_cons* dc;

    if (s->acceptable_preference_changed)
    {
        return;
    }
    allocate_with_pool(thisAgent, &thisAgent->dl_cons_pool, &dc);
    dc->item = s;
    s->acceptable_preference_changed = dc;
    insert_at_head_of_dll(thisAgent->context_slots_with_changed_acceptable_preferences, dc,
                          next, prev);
}

/* --- This updates the acceptable preference wmes for a single slot. --- */
void do_acceptable_preference_wme_changes_for_slot(agent* thisAgent, slot* s)
{
    wme* w, *next_w;
    preference* p;

    /* --- first, reset marks to "NOTHING" --- */
    for (w = s->acceptable_preference_wmes; w != NIL; w = w->next)
    {
        w->value->decider_flag = NOTHING_DECIDER_FLAG;
    }

    /* --- now mark values for which we WANT a wme as "CANDIDATE" values --- */
    for (p = s->preferences[REQUIRE_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        p->value->decider_flag = CANDIDATE_DECIDER_FLAG;
    }
    for (p = s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        p->value->decider_flag = CANDIDATE_DECIDER_FLAG;
    }

    /* --- remove any existing wme's that aren't CANDIDATEs; mark the
    rest as ALREADY_EXISTING --- */

    w = s->acceptable_preference_wmes;
    while (w)
    {
        next_w = w->next;
        if (w->value->decider_flag == CANDIDATE_DECIDER_FLAG)
        {
            w->value->decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
            w->value->decider_wme = w;
            w->preference = NIL;  /* we'll update this later */
        }
        else
        {
            remove_from_dll(s->acceptable_preference_wmes, w, next, prev);
            /* REW: begin 09.15.96 */
            /* IF we lose an acceptable preference for an operator, then that
            operator comes out of the slot immediately in OPERAND2.
            However, if the lost acceptable preference is not for item
            in the slot, then we don;t need to do anything special until
            mini-quiescence. */
            remove_operator_if_necessary(thisAgent, s, w);
            /* REW: end   09.15.96 */
            remove_wme_from_wm(thisAgent, w);
        }
        w = next_w;
    }

    /* --- add the necessary wme's that don't ALREADY_EXIST --- */

    for (p = s->preferences[REQUIRE_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        if (p->value->decider_flag == ALREADY_EXISTING_WME_DECIDER_FLAG)
        {
            /* --- found existing wme, so just update its trace --- */
            w = p->value->decider_wme;
            if (! w->preference)
            {
                w->preference = p;
            }
        }
        else
        {
            w = make_wme(thisAgent, p->id, p->attr, p->value, true);
            insert_at_head_of_dll(s->acceptable_preference_wmes, w, next, prev);
            w->preference = p;
            add_wme_to_wm(thisAgent, w);
            p->value->decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
            p->value->decider_wme = w;
        }
    }
    for (p = s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        if (p->value->decider_flag == ALREADY_EXISTING_WME_DECIDER_FLAG)
        {
            /* --- found existing wme, so just update its trace --- */
            w = p->value->decider_wme;
            if (! w->preference)
            {
                w->preference = p;
            }
        }
        else
        {
            w = make_wme(thisAgent, p->id, p->attr, p->value, true);
            insert_at_head_of_dll(s->acceptable_preference_wmes, w, next, prev);
            w->preference = p;
            add_wme_to_wm(thisAgent, w);
            p->value->decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
            p->value->decider_wme = w;
        }
    }
}

void do_buffered_acceptable_preference_wme_changes(agent* thisAgent)
{
    dl_cons* dc;
    slot* s;

    while (thisAgent->context_slots_with_changed_acceptable_preferences)
    {
        dc = thisAgent->context_slots_with_changed_acceptable_preferences;
        thisAgent->context_slots_with_changed_acceptable_preferences = dc->next;
        s = static_cast<slot_struct*>(dc->item);
        free_with_pool(&thisAgent->dl_cons_pool, dc);
        do_acceptable_preference_wme_changes_for_slot(thisAgent, s);
        s->acceptable_preference_changed = NIL;
    }
}


/* **********************************************************************

                         Ownership Calculations

   Whenever a link is added from one identifier to another (i.e.,
   (I37 ^x R26)), we call post_link_addition().  This records the link
   addition and buffers it for later processing.  Similarly, whenever a
   link is removed, we call post_link_removal(), which buffers the
   removal for later processing.  At the end of the phase, we call
   do_buffered_link_changes() to update the goal stack level of all
   identifiers, and garbage collect anything that's now disconnected.

   On each identifier, we maintain a count of how many links there are
   to it.  If the count is decremented to 0, the id must be disconnected,
   so we can GC it.  If the count is decremented but nonzero, we have
   to walk through WM to see whether it's connected--it could be disconnected
   but have a positive link count, if there's a circular structure in WM.

   Goal and impasse identifiers are handled specially.  We don't want to
   GC these even though WM may not have any pointers to them.  So instead
   of the normal link count stuff, we use a special "link" to each goal or
   impasse id.  This special link is added/removed by calling
   post_link_addition/removal (NIL, id).
********************************************************************** */

/* ======================================================================

                      Promotion (Upgrade) Routines

   The list "promoted_ids" indicates which identifiers need to be
   promoted at the end of the current phase.  On every id, we have a
   "promotion_level" field indicating the new goal_stack_level to which
   the id is going to be promoted.  When we actually do the promotion,
   we set the id's level to promotion_level.  We also promote anything
   in the id's transitive closure to the same level, if necessary.
====================================================================== */

/* ----------------------------------------------
   Post a link addition for later processing.
---------------------------------------------- */

void post_link_addition(agent* thisAgent, Symbol* from, Symbol* to)
{

    /* --- don't add links to goals/impasses, except the special one
       (NIL,goal) --- */
    if ((to->id->isa_goal || to->id->isa_impasse) && from)
    {
        return;
    }

    to->id->link_count++;

#ifdef DEBUG_LINKS
    if (from)
    {
        print_with_symbols(thisAgent, "\nAdding link from %y to %y", from, to);
    }
    else
    {
        print_with_symbols(thisAgent, "\nAdding special link to %y", to);
    }
    print(" (count=%lu)", to->id->link_count);
#endif

    if (!from)
    {
        return;    /* if adding a special link, we're done */
    }

    /* --- if adding link from same level, ignore it --- */
    if (from->id->promotion_level == to->id->promotion_level)
    {
        return;
    }

    /* --- if adding link from lower to higher, mark higher accordingly --- */
    if (from->id->promotion_level > to->id->promotion_level)
    {
        to->id->could_be_a_link_from_below = true;
        return;
    }

    /* --- otherwise buffer it for later --- */
    to->id->promotion_level = from->id->promotion_level;
    symbol_add_ref(thisAgent, to);
    push(thisAgent, to, thisAgent->promoted_ids);
}

/* ----------------------------------------------
   Promote an id and its transitive closure.
---------------------------------------------- */

#define promote_if_needed(thisAgent, sym) \
    { if ((sym)->symbol_type==IDENTIFIER_SYMBOL_TYPE) \
            promote_id_and_tc(thisAgent, sym,new_level); }

void promote_id_and_tc(agent* thisAgent, Symbol* id, goal_stack_level new_level)
{
    slot* s;
    preference* pref;
    wme* w;

    /* --- if it's already that high, or is going to be soon, don't bother -- */
    if (id->id->level <= new_level)
    {
        return;
    }
    if (id->id->promotion_level < new_level)
    {
        return;
    }

    /* --- update its level, etc. --- */
    id->id->level = new_level;
    id->id->promotion_level = new_level;
    id->id->could_be_a_link_from_below = true;

    /* --- sanity check --- */
    if (id->id->isa_goal || id->id->isa_impasse)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "decide.c: Internal error: tried to promote a goal or impasse id\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
        /* Note--since we can't promote a goal, we don't have to worry about
           slot->acceptable_preference_wmes below */
    }

    /* --- scan through all preferences and wmes for all slots for this id -- */
    for (w = id->id->input_wmes; w != NIL; w = w->next)
    {
        promote_if_needed(thisAgent, w->value);
    }
    for (s = id->id->slots; s != NIL; s = s->next)
    {
        for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
        {
            promote_if_needed(thisAgent, pref->value);
            if (preference_is_binary(pref->type))
            {
                promote_if_needed(thisAgent, pref->referent);
            }
        }
        for (w = s->wmes; w != NIL; w = w->next)
        {
            promote_if_needed(thisAgent, w->value);
        }
    } /* end of for slots loop */
}

/* ----------------------------------------------
   Do all buffered promotions.
---------------------------------------------- */

void do_promotion(agent* thisAgent)
{
    cons* c;
    Symbol* to;

    while (thisAgent->promoted_ids)
    {
        c = thisAgent->promoted_ids;
        to = static_cast<symbol_struct*>(c->first);
        thisAgent->promoted_ids = thisAgent->promoted_ids->rest;
        free_cons(thisAgent, c);
        promote_id_and_tc(thisAgent, to, to->id->promotion_level);
        symbol_remove_ref(thisAgent, to);
    }
}

/* ======================================================================

           Demotion (Downgrade) and Garbage Collection Routines

   Demotions happen in stages.  Post_link_removal() is called from
   various places; this decrements the link count on an identifier and
   adds it to the dl_list "ids_with_unknown_level".  (While this is going
   on, link_update_mode is always set to UPDATE_LINKS_NORMALLY.)

   At the end of the phase, do_demotion() is called.  This has three
   stages:
     (1) the ids with unknown level whose link count is 0 are moved
         over to a list of disconnected_ids -- these ids are definitely
         disconnected and are going to be GC'd
     (2) We GC the disconnected_ids.  While doing this GC, more wmes
         are removed from WM and more links are removed.  For these
         link removals, if the link count on an id goes to 0, we put
         it onto disconnected_ids (rather than ids_with_unknown_level).
         (Here link_update_mode is UPDATE_DISCONNECTED_IDS_LIST.)
         We keep GC-ing disconnected_ids until none are left.
     (3) Mark & Walk:  If there are still remaining ids_with_unknown_level,
         we mark each such id and its transitive closure, then walk the
         goal stack (or parts of it) to find out what's really still
         connected.
     (4) For each id now known to be disconnected, we GC it.  While doing
         this GC, more links are removed, but for those, we merely update
         the link count, nothing else--because we already took the TC of
         each id in step 3, so we're already certain of what's connected
         and what's not.  (Here link_update_mode is JUST_UPDATE_COUNT.)
====================================================================== */

/* ----------------------------------------------
   Post a link removal for later processing.
---------------------------------------------- */

void post_link_removal(agent* thisAgent, Symbol* from, Symbol* to)
{
    dl_cons* dc;

    /* --- don't remove links to goals/impasses, except the special one
       (NIL,goal) --- */
    if ((to->id->isa_goal || to->id->isa_impasse) && from)
    {
        return;
    }

    to->id->link_count--;

#ifdef DEBUG_LINKS
    if (from)
    {
        print_with_symbols(thisAgent, "\nRemoving link from %y to %y", from, to);
        print(" (%d to %d)", from->id->level, to->id->level);
    }
    else
    {
        print_with_symbols(thisAgent, S"\nRemoving special link to %y ", to);
        print(" (%d)", to->id->level);
    }
    print(" (count=%lu)", to->id->link_count);
#endif

    /* --- if a gc is in progress, handle differently --- */
    if (thisAgent->link_update_mode == JUST_UPDATE_COUNT)
    {
        return;
    }

    if ((thisAgent->link_update_mode == UPDATE_DISCONNECTED_IDS_LIST) &&
            (to->id->link_count == 0))
    {
        if (to->id->unknown_level)
        {
            dc = to->id->unknown_level;
            remove_from_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            insert_at_head_of_dll(thisAgent->disconnected_ids, dc, next, prev);
        }
        else
        {
            symbol_add_ref(thisAgent, to);
            allocate_with_pool(thisAgent, &thisAgent->dl_cons_pool, &dc);
            dc->item = to;
            to->id->unknown_level = dc;
            insert_at_head_of_dll(thisAgent->disconnected_ids, dc, next, prev);
        }
        return;
    }

    /* --- if removing a link from a different level, there must be some other
       link at the same level, so we can ignore this change --- */
    if (from && (from->id->level != to->id->level))
    {
        return;
    }

    if (! to->id->unknown_level)
    {
        symbol_add_ref(thisAgent, to);
        allocate_with_pool(thisAgent, &thisAgent->dl_cons_pool, &dc);
        dc->item = to;
        to->id->unknown_level = dc;
        insert_at_head_of_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
    }
}

/* ----------------------------------------------
   Garbage collect an identifier.  This removes
   all wmes, input wmes, and preferences for that
   id from TM.
---------------------------------------------- */

void garbage_collect_id(agent* thisAgent, Symbol* id)
{
    slot* s;
    preference* pref, *next_pref;

#ifdef DEBUG_LINKS
    print_with_symbols(thisAgent, "\n*** Garbage collecting id: %y", id);
#endif

    /* Note--for goal/impasse id's, this does not remove the impasse wme's.
        This is handled by remove_existing_such-and-such... */

    /* --- remove any input wmes from the id --- */
    remove_wme_list_from_wm(thisAgent, id->id->input_wmes, true);
    id->id->input_wmes = NIL;

    for (s = id->id->slots; s != NIL; s = s->next)
    {
        /* --- remove any existing attribute impasse for the slot --- */
        if (s->impasse_type != NONE_IMPASSE_TYPE)
        {
            remove_existing_attribute_impasse_for_slot(thisAgent, s);
        }

        /* --- remove all wme's from the slot --- */
        remove_wme_list_from_wm(thisAgent, s->wmes);
        s->wmes = NIL;

        /* --- remove all preferences for the slot --- */
        pref = s->all_preferences;
        while (pref)
        {
            next_pref = pref->all_of_slot_next;
            remove_preference_from_tm(thisAgent, pref);

            /* Note:  the call to remove_preference_from_slot handles the removal
            of acceptable_preference_wmes */
            pref = next_pref;
        }

        mark_slot_for_possible_removal(thisAgent, s);
    } /* end of for slots loop */
}

/* ----------------------------------------------
   During the mark & walk, these variables keep
   track of the highest goal stack level that
   any identifier could "fall from" and the lowest
   level any could "fall to".  These are used to
   delimit a range of goal stack levels that
   need to be walked.  (In many cases, much of
   the goal stack can be ignored.)
---------------------------------------------- */

/* ----------------------------------------------
   Mark an id and its transitive closure as having
   an unknown level.  Ids are marked by setting
   id.tc_num to mark_tc_number.  The starting id's
   goal stack level is recorded in
   level_at_which_marking_started by the caller.
   The marked ids are added to ids_with_unknown_level.
---------------------------------------------- */

inline bool mark_level_unknown_needed(agent* /*thisAgent*/, Symbol* sym)
{
    return (sym->symbol_type == IDENTIFIER_SYMBOL_TYPE);
}

void mark_id_and_tc_as_unknown_level(agent* thisAgent, Symbol* root)
{
    slot* s;
    preference* pref;
    wme* w;
    dl_cons* dc;

    Symbol* id;
#ifdef USE_MEM_POOL_ALLOCATORS
    symbol_list ids_to_walk = symbol_list(soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
#else
    symbol_list ids_to_walk;
#endif
    ids_to_walk.push_back(root);

    while (!ids_to_walk.empty())
    {
        id = ids_to_walk.back();
        ids_to_walk.pop_back();

        /* --- if id is already marked, do nothing --- */
        if (id->tc_num == thisAgent->mark_tc_number)
        {
            continue;
        }

        /* --- don't mark anything higher up as disconnected--in order to be higher
           up, it must have a link to it up there --- */
        if (id->id->level < thisAgent->level_at_which_marking_started)
        {
            continue;
        }

        /* --- mark id, so we won't do it again later --- */
        id->tc_num = thisAgent->mark_tc_number;

        /* --- update range of goal stack levels we'll need to walk --- */
        if (id->id->level < thisAgent->highest_level_anything_could_fall_from)
        {
            thisAgent->highest_level_anything_could_fall_from = id->id->level;
        }
        if (id->id->level > thisAgent->lowest_level_anything_could_fall_to)
        {
            thisAgent->lowest_level_anything_could_fall_to = id->id->level;
        }
        if (id->id->could_be_a_link_from_below)
        {
            thisAgent->lowest_level_anything_could_fall_to = LOWEST_POSSIBLE_GOAL_LEVEL;
        }

        /* --- add id to the set of ids with unknown level --- */
        if (! id->id->unknown_level)
        {
            allocate_with_pool(thisAgent, &thisAgent->dl_cons_pool, &dc);
            dc->item = id;
            id->id->unknown_level = dc;
            insert_at_head_of_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            symbol_add_ref(thisAgent, id);
        }

        /* -- scan through all preferences and wmes for all slots for this id -- */
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            if (mark_level_unknown_needed(thisAgent, w->value))
            {
                ids_to_walk.push_back(w->value);
            }
        }

        for (s = id->id->slots; s != NIL; s = s->next)
        {
            for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
            {
                if (mark_level_unknown_needed(thisAgent, pref->value))
                {
                    ids_to_walk.push_back(pref->value);
                }

                if (preference_is_binary(pref->type))
                {
                    if (mark_level_unknown_needed(thisAgent, pref->referent))
                    {
                        ids_to_walk.push_back(pref->referent);
                    }
                }
            }

            if (s->impasse_id)
            {
                if (mark_level_unknown_needed(thisAgent, s->impasse_id))
                {
                    ids_to_walk.push_back(s->impasse_id);
                }
            }

            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (mark_level_unknown_needed(thisAgent, w->value))
                {
                    ids_to_walk.push_back(w->value);
                }
            }
        } /* end of for slots loop */
    }
}

/* ----------------------------------------------
   After marking the ids with unknown level,
   we walk various levels of the goal stack,
   higher level to lower level.  If, while doing
   the walk, we encounter an id marked as having
   an unknown level, we update its level and
   remove it from ids_with_unknown_level.
---------------------------------------------- */

inline bool level_update_needed(agent* thisAgent, Symbol* sym)
{
    return ((sym->symbol_type == IDENTIFIER_SYMBOL_TYPE) && (sym->tc_num != thisAgent->walk_tc_number));
}

void walk_and_update_levels(agent* thisAgent, Symbol* root)
{
    slot* s;
    preference* pref;
    wme* w;
    dl_cons* dc;
    Symbol* id;

#ifdef USE_MEM_POOL_ALLOCATORS
    symbol_list ids_to_walk = symbol_list(soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
#else
    symbol_list ids_to_walk;
#endif
    ids_to_walk.push_back(root);

    while (!ids_to_walk.empty())
    {
        id = ids_to_walk.back();
        ids_to_walk.pop_back();

        /* --- mark id so we don't walk it twice --- */
        id->tc_num = thisAgent->walk_tc_number;

        /* --- if we already know its level, and it's higher up, then exit --- */
        if ((! id->id->unknown_level) && (id->id->level < thisAgent->walk_level))
        {
            continue;
        }

        /* --- if we didn't know its level before, we do now --- */
        if (id->id->unknown_level)
        {
            dc = id->id->unknown_level;
            remove_from_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            free_with_pool(&thisAgent->dl_cons_pool, dc);
            symbol_remove_ref(thisAgent, id);
            id->id->unknown_level = NIL;
            id->id->level = thisAgent->walk_level;
            id->id->promotion_level = thisAgent->walk_level;
        }

        /* -- scan through all preferences and wmes for all slots for this id -- */
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            if (level_update_needed(thisAgent, w->value))
            {
                ids_to_walk.push_back(w->value);
            }
        }

        for (s = id->id->slots; s != NIL; s = s->next)
        {
            for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
            {
                if (level_update_needed(thisAgent, pref->value))
                {
                    ids_to_walk.push_back(pref->value);

                    if (preference_is_binary(pref->type))
                    {
                        if (level_update_needed(thisAgent, pref->referent))
                        {
                            ids_to_walk.push_back(pref->referent);
                        }
                    }
                }
            }

            if (s->impasse_id)
            {
                if (level_update_needed(thisAgent, s->impasse_id))
                {
                    ids_to_walk.push_back(s->impasse_id);
                }
            }

            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (level_update_needed(thisAgent, w->value))
                {
                    ids_to_walk.push_back(w->value);
                }
            }
        }
    }
}

/* ----------------------------------------------
   Do all buffered demotions and gc's.
---------------------------------------------- */

void do_demotion(agent* thisAgent)
{
    Symbol* g, *id;
    dl_cons* dc, *next_dc;

    /* --- scan through ids_with_unknown_level, move the ones with link_count==0
     *  over to disconnected_ids --- */
    for (dc = thisAgent->ids_with_unknown_level; dc != NIL; dc = next_dc)
    {
        next_dc = dc->next;
        id = static_cast<symbol_struct*>(dc->item);
        if (id->id->link_count == 0)
        {
            remove_from_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            insert_at_head_of_dll(thisAgent->disconnected_ids, dc, next, prev);
        }
    }

    /* --- keep garbage collecting ids until nothing left to gc --- */
    thisAgent->link_update_mode = UPDATE_DISCONNECTED_IDS_LIST;
    while (thisAgent->disconnected_ids)
    {
        dc = thisAgent->disconnected_ids;
        thisAgent->disconnected_ids = thisAgent->disconnected_ids->next;
        id = static_cast<symbol_struct*>(dc->item);
        free_with_pool(&thisAgent->dl_cons_pool, dc);
        id->id->unknown_level = NIL;
        garbage_collect_id(thisAgent, id);
        symbol_remove_ref(thisAgent, id);
    }
    thisAgent->link_update_mode = UPDATE_LINKS_NORMALLY;

    /* --- if nothing's left with an unknown level, we're done --- */
    if (! thisAgent->ids_with_unknown_level)
    {
        return;
    }

    /* --- do the mark --- */
    thisAgent->highest_level_anything_could_fall_from =
        LOWEST_POSSIBLE_GOAL_LEVEL;
    thisAgent->lowest_level_anything_could_fall_to = -1;
    thisAgent->mark_tc_number = get_new_tc_number(thisAgent);
    for (dc = thisAgent->ids_with_unknown_level; dc != NIL; dc = dc->next)
    {
        id = static_cast<symbol_struct*>(dc->item);
        thisAgent->level_at_which_marking_started = id->id->level;
        mark_id_and_tc_as_unknown_level(thisAgent, id);
    }

    /* --- do the walk --- */
    g = thisAgent->top_goal;
    while (true)
    {
        if (!g)
        {
            break;
        }
        if (g->id->level > thisAgent->lowest_level_anything_could_fall_to)
        {
            break;
        }
        if (g->id->level >= thisAgent->highest_level_anything_could_fall_from)
        {
            thisAgent->walk_level = g->id->level;
            thisAgent->walk_tc_number = get_new_tc_number(thisAgent);
            walk_and_update_levels(thisAgent, g);
        }
        g = g->id->lower_goal;
    }

    /* --- GC anything left with an unknown level after the walk --- */
    thisAgent->link_update_mode = JUST_UPDATE_COUNT;
    while (thisAgent->ids_with_unknown_level)
    {
        dc = thisAgent->ids_with_unknown_level;
        thisAgent->ids_with_unknown_level =
            thisAgent->ids_with_unknown_level->next;
        id = static_cast<symbol_struct*>(dc->item);
        free_with_pool(&thisAgent->dl_cons_pool, dc);
        id->id->unknown_level = NIL;    /* AGR 640:  GAP set to NIL because */
        /* symbol may still have pointers to it */
        garbage_collect_id(thisAgent, id);
        symbol_remove_ref(thisAgent, id);
    }
    thisAgent->link_update_mode = UPDATE_LINKS_NORMALLY;
}

/* ------------------------------------------------------------------
                       Do Buffered Link Changes

   This routine does all the buffered link (ownership) changes, updating
   the goal stack level on all identifiers and garbage collecting
   disconnected wmes.
------------------------------------------------------------------ */

void do_buffered_link_changes(agent* thisAgent)
{

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    soar_timer local_timer;
    local_timer.set_enabled(&(thisAgent->sysparams[ TIMERS_ENABLED ]));
#endif
#endif

    /* --- if no promotions or demotions are buffered, do nothing --- */
    if (!(thisAgent->promoted_ids ||
            thisAgent->ids_with_unknown_level ||
            thisAgent->disconnected_ids))
    {
        return;
    }

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.start();
#endif
#endif
    do_promotion(thisAgent);
    do_demotion(thisAgent);
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    local_timer.stop();
    thisAgent->timers_ownership_cpu_time[thisAgent->current_phase].update(local_timer);
#endif
#endif
}

/* ------------------------------------------------------------------
                  Add a preference to a slot's CDPS
   This function adds a preference to a slots's context dependent
   preference set, checking to first see whether the pref is already
   there. If an operator The slot's CDPS is copied to conditions' bt structs in
   create_instatiation.  Those copies of the CDPS are used to
   backtrace through all relevant local evaluation rules that led to the
   selection of the operator that produced a result.
------------------------------------------------------------------ */

void add_to_CDPS(agent* thisAgent, slot* s, preference* pref, bool unique_value)
{

    bool already_exists = false;
    cons* CDPS;
    preference* p;

    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
    {
        print(thisAgent,  "--> Adding preference to CDPS: ");
        print_preference(thisAgent, pref);
    }

    for (CDPS = s->CDPS; CDPS != NIL; CDPS = CDPS->rest)
    {
        p = static_cast<preference*>(CDPS->first);
        if (p == pref)
        {
            already_exists = true;
            break;
        }

        if (unique_value)
        {
            /* Checking if a preference is unique differs depending on the preference type */

            /* Binary preferences can be considered equivalent if they point to the same
             * operators in the correct relative spots */
            if (((pref->type == BETTER_PREFERENCE_TYPE) || (pref->type == WORSE_PREFERENCE_TYPE)) &&
                    ((p->type == BETTER_PREFERENCE_TYPE) || (p->type == WORSE_PREFERENCE_TYPE)))
            {
                if (pref->type == p->type)
                {
                    already_exists = ((pref->value == p->value) && (pref->referent == p->referent));
                }
                else
                {
                    already_exists = ((pref->value == p->referent) && (pref->referent == p->value));
                }
            }
            else if ((pref->type == BINARY_INDIFFERENT_PREFERENCE_TYPE) &&
                     (p->type == BINARY_INDIFFERENT_PREFERENCE_TYPE))
            {
                already_exists = (((pref->value == p->value) && (pref->referent == p->referent)) ||
                                  ((pref->value == p->referent) && (pref->referent == p->value)));
            }
            else
            {
                /* Otherwise they are equivalent if they have the same value and type */
                already_exists = (pref->value == p->value) && (pref->type == p->type);
            }
            if (already_exists)
            {
                break;
            }
        }
    }
    if (!already_exists)
    {
        push(thisAgent, pref, s->CDPS);
        preference_add_ref(pref);
    }
    else if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
    {
        print(thisAgent,  "--> equivalent pref already exists.  Not adding.\n");
    }

}

/** Build our RL trace. -bazald **/

void build_rl_trace(agent* const& thisAgent, preference* const& candidates, preference* const& selected)   ///< bazald
{
    if (thisAgent->rl_params->trace->get_value() == off)
    {
        return;
    }

    agent::RL_Trace** next = NIL;

    for (preference* cand = candidates; cand; cand = cand->next_candidate)
    {
        if (cand->inst && cand->inst->prod)
        {
            const production* const& prod = cand->inst->prod;

//       std::cerr << "rl-trace: " << cand->inst->prod->name->sc->name << std::endl;

//       for(preference *pref = cand->inst->match_goal->id->operator_slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref; pref = pref->next) {
//         production * const &prod2 = pref->inst->prod;
//         if(cand->value == pref->value && prod2->rl_rule) {
//           std::cerr << "rl-trace: +" << prod2->name->sc->name << std::endl;
//         }
//       }

            std::vector<std::string> index_str;
            index_str.push_back("^name");
//       for(wme *w = thisAgent->all_wmes_in_rete; w; w = w->rete_next) {
            for (slot* s = cand->value->id->slots; s; s = s->next)
            {
                for (wme* w = s->wmes; w; w = w->next)
                {
                    if (cand->value == w->id)
                    {
                        const std::string attr = w->attr->to_string();
                        const std::string value = w->value->to_string();
//             std::cerr << "rl-trace: ^" << attr << ' ' << value << std::endl;

                        if (attr == "name")
                        {
                            index_str[0] += ' ' + value;
                        }
                        else
                        {
                            index_str.push_back('^' + attr + ' ' + value);
                        }

                        std::sort(++index_str.begin(), index_str.end());
                    }
                }
            }

            const double zero = 0.0;
            const double probability = cand->rl_contribution
                                       ? exploration_probability_according_to_policy(thisAgent, candidates->slot, candidates, cand)
                                       : zero / zero;

//       std::cerr << "rl-trace: =" << probability << std::endl;

            agent::RL_Trace* const rl_trace = static_cast<agent::RL_Trace*>(candidates->slot->id->id->rl_trace);
            rl_trace->split[index_str].init = thisAgent->rl_init_count;
            rl_trace->split[index_str].probability = probability;
            if (cand == selected)
            {
                next = &rl_trace->split[index_str].next;
            }
        }
    }

    if (next)
    {
        if (!*next)
        {
//       std::cerr << "rl-trace: Expanding" << std::endl;
            *next = new agent::RL_Trace;
        }
//     else {
//       std::cerr << "rl-trace: Traversing" << std::endl;
//     }

        candidates->slot->id->id->rl_trace = *next;
    }
}

/* Perform reinforcement learning update for one valid candidate. */

void rl_update_for_one_candidate(agent* thisAgent, slot* s, bool consistency, preference* candidates)
{

    if (!consistency && rl_enabled(thisAgent))
    {
        build_rl_trace(thisAgent, candidates, candidates);
        rl_tabulate_reward_values(thisAgent);
        exploration_compute_value_of_candidate(thisAgent, candidates, s, 0);
        rl_perform_update(thisAgent, candidates->numeric_value,
                          candidates->rl_contribution, s->id);
    }
}

/* **************************************************************************

                         Run Preference Semantics

   Run_preference_semantics (slot *s, preference **result_candidates) examines
   the preferences for a given slot, and returns an impasse type for thez
   slot.  The argument "result_candidates" is set to a list of candidate
   values for the slot--if the returned impasse type is NONE_IMPASSE_TYPE,
   this is the set of winners; otherwise it is the set of tied, conflicted,
   or constraint-failure values.  This list of values is a list of preferences
   for those values, linked via the "next_candidate" field on each preference
   structure.  If there is more than one preference for a given value,
   only one is returned in the result_candidates, with (first) require
   preferences being preferred over acceptable preferences, and (second)
   preferences from higher match goals being preferred over those from
   lower match goals.

   BUGBUG There is a problem here:  since the require/acceptable priority
   takes precedence over the match goal level priority, it's possible that
   we could return a require preference from lower in the goal stack than
   some acceptable preference.  If the goal stack gets popped soon
   afterwards (i.e., before the next time the slot is re-decided, I think),
   we would be left with a WME still in WM (not GC'd, because of the acceptable
   preference higher up) but with a trace pointing to a deallocated require
   preference.  This case is very obscure and unlikely to come up, but it
   could easily cause a core dump or worse.

************************************************************************** */

byte run_preference_semantics(agent* thisAgent,
                              slot* s,
                              preference** result_candidates,
                              bool consistency,
                              bool predict)
{
    preference* p, *p2, *cand, *prev_cand;
    bool match_found, not_all_indifferent, some_numeric, do_CDPS, some_not_worst;
    preference* candidates;
    Symbol* value;
    cons* CDPS, *prev_cons;

    /* Set a flag to determine if a context-dependent preference set makes sense in this context.
     * We can ignore the CDPS when:
     * - Run_preference_semantics is called for a consistency check (don't want side effects)
     * - For non-context slots (only makes sense for operators)
     * - For context-slots at the top level (will never be backtraced through)
     * - when the learning system parameter is set off (note, this is independent of whether learning is on) */

    do_CDPS = (s->isa_context_slot && !consistency && (s->id->id->level > TOP_GOAL_LEVEL) && thisAgent->sysparams[CHUNK_THROUGH_EVALUATION_RULES_SYSPARAM]);

    /* Empty the context-dependent preference set in the slot */

    if (do_CDPS && s->CDPS)
    {
        clear_CDPS(thisAgent, s);
    }

    /* If the slot has no preferences at all, things are trivial --- */

    if (!s->all_preferences)
    {
        if (!s->isa_context_slot)
        {
            mark_slot_for_possible_removal(thisAgent, s);
        }
        *result_candidates = NIL;
        return NONE_IMPASSE_TYPE;
    }

    /* If this is the true decision slot and selection has been made, attempt force selection */

    if (s->isa_context_slot && !consistency)
    {
        if (select_get_operator(thisAgent) != NULL)
        {
            preference* force_result = select_force(thisAgent,
                                                    s->preferences[ACCEPTABLE_PREFERENCE_TYPE], !predict);

            if (force_result)
            {
                force_result->next_candidate = NIL;
                *result_candidates = force_result;

                if (!predict && rl_enabled(thisAgent))
                {
                    build_rl_trace(thisAgent, force_result, force_result);
                    rl_tabulate_reward_values(thisAgent);
                    exploration_compute_value_of_candidate(thisAgent, force_result, s, 0);
                    rl_perform_update(thisAgent, force_result->numeric_value,
                                      force_result->rl_contribution, s->id);
                }

                return NONE_IMPASSE_TYPE;
            }
        }
    }

    /* If debugging a context-slot, print all preferences that we're deciding through */

    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM] && s->isa_context_slot)
    {

        print(thisAgent,
              "\n-------------------------------\nRUNNING PREFERENCE SEMANTICS...\n-------------------------------\n");
        print(thisAgent,  "All Preferences for slot:");

        for (int i = 0; i < NUM_PREFERENCE_TYPES; i++)
        {
            if (s->preferences[i])
            {
                print(thisAgent,  "\n   %ss:\n", preference_name[i]);
                for (p = s->preferences[i]; p; p = p->next)
                {
                    print(thisAgent,  "   ");
                    print_preference(thisAgent, p);
                }
            }
        }
        print(thisAgent,  "-------------------------------\n");
    }

    /* === Requires === */

    if (s->preferences[REQUIRE_PREFERENCE_TYPE])
    {

        /* Collect set of required items into candidates list */

        for (p = s->preferences[REQUIRE_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            p->value->decider_flag = NOTHING_DECIDER_FLAG;
        }
        candidates = NIL;
        for (p = s->preferences[REQUIRE_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            if (p->value->decider_flag == NOTHING_DECIDER_FLAG)
            {
                p->next_candidate = candidates;
                candidates = p;
                /* Unmark it, in order to prevent it from being added twice */
                p->value->decider_flag = CANDIDATE_DECIDER_FLAG;
            }
        }
        *result_candidates = candidates;

        /* Check if we have more than one required item. If so, return constraint failure. */

        if (candidates->next_candidate)
        {
            return CONSTRAINT_FAILURE_IMPASSE_TYPE;
        }

        /* Check if we have also have a prohibit preference. If so, return constraint failure.
         * Note that this is the one difference between prohibit and reject preferences. */

        value = candidates->value;
        for (p = s->preferences[PROHIBIT_PREFERENCE_TYPE]; p != NIL; p = p->next)
            if (p->value == value)
            {
                return CONSTRAINT_FAILURE_IMPASSE_TYPE;
            }

        /* --- We have a winner, so update RL --- */

        rl_update_for_one_candidate(thisAgent, s, consistency, candidates);

        /* Print a message that we're adding the require preference to the CDPS
         * even though we really aren't.  Requires aren't actually handled by
         * the CDPS mechanism since they are already backtraced through. */

        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM])
        {
            print(thisAgent,  "--> Adding preference to CDPS: ");
            print_preference(thisAgent, candidates);
        }

        return NONE_IMPASSE_TYPE;
    }

    /* === Acceptables, Prohibits, Rejects === */

    /* Mark every acceptable preference as a possible candidate */

    for (p = s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        p->value->decider_flag = CANDIDATE_DECIDER_FLAG;
    }

    /* Unmark any preferences that have a prohibit or reject.  Note that this may
     * remove the candidate_decider_flag set in the last loop */

    for (p = s->preferences[PROHIBIT_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        p->value->decider_flag = NOTHING_DECIDER_FLAG;
    }
    for (p = s->preferences[REJECT_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        p->value->decider_flag = NOTHING_DECIDER_FLAG;
    }

    /* Build list of candidates.  These are the acceptable prefs that didn't
     * have the CANDIDATE_DECIDER_FLAG reversed by prohibit or reject prefs. */

    candidates = NIL;
    for (p = s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p != NIL; p = p->next)
    {
        if (p->value->decider_flag == CANDIDATE_DECIDER_FLAG)
        {
            p->next_candidate = candidates;
            candidates = p;
            /* --- Unmark it, in order to prevent it from being added twice --- */
            p->value->decider_flag = NOTHING_DECIDER_FLAG;
        }
    }

    /* If this is not a decidable context slot, then we're done */

    if (!s->isa_context_slot)
    {
        *result_candidates = candidates;
        return NONE_IMPASSE_TYPE;
    }

    /* If there are reject or prohibit preferences, then
     * add all reject and prohibit preferences to CDPS */

    if (do_CDPS)
    {
        if (s->preferences[PROHIBIT_PREFERENCE_TYPE] || s->preferences[REJECT_PREFERENCE_TYPE])
        {
            for (p = s->preferences[PROHIBIT_PREFERENCE_TYPE]; p != NIL; p = p->next)
            {
                add_to_CDPS(thisAgent, s, p);
            }
            for (p = s->preferences[REJECT_PREFERENCE_TYPE]; p != NIL; p = p->next)
            {
                add_to_CDPS(thisAgent, s, p);
            }
        }
    }

    /* Exit point 1: Check if we're done, i.e. 0 or 1 candidates left */
    if ((!candidates) || (!candidates->next_candidate))
    {
        *result_candidates = candidates;
        if (candidates)
        {
            /* Update RL values for the winning candidate */
            rl_update_for_one_candidate(thisAgent, s, consistency, candidates);
        }
        else
        {
            if (do_CDPS && s->CDPS)
            {
                clear_CDPS(thisAgent, s);
            }
        }
        return NONE_IMPASSE_TYPE;
    }

    /* === Better/Worse === */

    if (s->preferences[BETTER_PREFERENCE_TYPE]
            || s->preferences[WORSE_PREFERENCE_TYPE])
    {
        Symbol* j, *k;

        /* Initialize decider flags */

        for (p = s->preferences[BETTER_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            p->value->decider_flag = NOTHING_DECIDER_FLAG;
            p->referent->decider_flag = NOTHING_DECIDER_FLAG;
        }
        for (p = s->preferences[WORSE_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            p->value->decider_flag = NOTHING_DECIDER_FLAG;
            p->referent->decider_flag = NOTHING_DECIDER_FLAG;
        }
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            cand->value->decider_flag = CANDIDATE_DECIDER_FLAG;
        }

        /* Mark any preferences that are worse than another as conflicted.  This
         * will either remove it from the candidate list or add it to the conflicted
         * list later.  We first do this for both the referent half of better and
         * then the value half of worse preferences. */

        for (p = s->preferences[BETTER_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            j = p->value;
            k = p->referent;
            if (j == k)
            {
                continue;
            }
            if (j->decider_flag && k->decider_flag)
            {
                if (j->decider_flag == CANDIDATE_DECIDER_FLAG
                        || k->decider_flag == CANDIDATE_DECIDER_FLAG)
                {
                    k->decider_flag = CONFLICTED_DECIDER_FLAG;
                }
            }
        }

        for (p = s->preferences[WORSE_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            j = p->value;
            k = p->referent;
            if (j == k)
            {
                continue;
            }
            if (j->decider_flag && k->decider_flag)
            {
                if (j->decider_flag == CANDIDATE_DECIDER_FLAG
                        || k->decider_flag == CANDIDATE_DECIDER_FLAG)
                {
                    j->decider_flag = CONFLICTED_DECIDER_FLAG;
                }
            }
        }

        /* Check if a valid candidate still exists. */

        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            if (cand->value->decider_flag == CANDIDATE_DECIDER_FLAG)
            {
                break;
            }
        }

        /* If no candidates exists, collect conflicted candidates and return as
         * the result candidates with a conflict impasse type. */

        if (!cand)
        {
            prev_cand = NIL;
            cand = candidates;
            while (cand)
            {
                if (cand->value->decider_flag != CONFLICTED_DECIDER_FLAG)
                {
                    if (prev_cand)
                    {
                        prev_cand->next_candidate = cand->next_candidate;
                    }
                    else
                    {
                        candidates = cand->next_candidate;
                    }
                }
                else
                {
                    prev_cand = cand;
                }
                cand = cand->next_candidate;
            }
            *result_candidates = candidates;
            if (do_CDPS && s->CDPS)
            {
                clear_CDPS(thisAgent, s);
            }
            return CONFLICT_IMPASSE_TYPE;
        }

        /* Otherwise, delete conflicted candidates from candidate list.
         * Also add better preferences to CDPS for every item in the candidate
         * list and delete acceptable preferences from the CDPS for those that
         * don't make the candidate list.*/

        prev_cand = NIL;
        cand = candidates;
        while (cand)
        {
            if (cand->value->decider_flag == CONFLICTED_DECIDER_FLAG)
            {
                /* Remove this preference from the candidate list */
                if (prev_cand)
                {
                    prev_cand->next_candidate = cand->next_candidate;
                }
                else
                {
                    candidates = cand->next_candidate;
                }

            }
            else
            {
                if (do_CDPS)
                {
                    /* Add better/worse preferences to CDPS */
                    for (p = s->preferences[BETTER_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == cand->value)
                        {
                            add_to_CDPS(thisAgent, s, p);
                        }
                    }
                    for (p = s->preferences[WORSE_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->referent == cand->value)
                        {
                            add_to_CDPS(thisAgent, s, p);
                        }
                    }
                }
                prev_cand = cand;
            }
            cand = cand->next_candidate;
        }
    }

    /* Exit point 2: Check if we're done, i.e. 0 or 1 candidates left */

    if ((!candidates) || (!candidates->next_candidate))
    {
        *result_candidates = candidates;
        if (candidates)
        {
            /* Update RL values for the winning candidate */
            rl_update_for_one_candidate(thisAgent, s, consistency, candidates);
        }
        else
        {
            if (do_CDPS && s->CDPS)
            {
                clear_CDPS(thisAgent, s);
            }
        }
        return NONE_IMPASSE_TYPE;
    }

    /* === Bests === */

    if (s->preferences[BEST_PREFERENCE_TYPE])
    {

        /* Initialize decider flags for all candidates */
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            cand->value->decider_flag = NOTHING_DECIDER_FLAG;
        }

        /* Mark flag for those with a best preference */
        for (p = s->preferences[BEST_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            p->value->decider_flag = BEST_DECIDER_FLAG;
        }

        /* Reduce candidates list to only those with best preference flag and add pref to CDPS */
        prev_cand = NIL;
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
            if (cand->value->decider_flag == BEST_DECIDER_FLAG)
            {
                if (do_CDPS)
                {
                    for (p = s->preferences[BEST_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == cand->value)
                        {
                            add_to_CDPS(thisAgent, s, p);
                        }
                    }
                }
                if (prev_cand)
                {
                    prev_cand->next_candidate = cand;
                }
                else
                {
                    candidates = cand;
                }
                prev_cand = cand;
            }
        if (prev_cand)
        {
            prev_cand->next_candidate = NIL;
        }
    }

    /* Exit point 3: Check if we're done, i.e. 0 or 1 candidates left */

    if ((!candidates) || (!candidates->next_candidate))
    {
        *result_candidates = candidates;
        if (candidates)
        {
            /* Update RL values for the winning candidate */
            rl_update_for_one_candidate(thisAgent, s, consistency, candidates);
        }
        else
        {
            if (do_CDPS && s->CDPS)
            {
                clear_CDPS(thisAgent, s);
            }
        }
        return NONE_IMPASSE_TYPE;
    }

    /* === Worsts === */

    if (s->preferences[WORST_PREFERENCE_TYPE])
    {

        /* Initialize decider flags for all candidates */
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            cand->value->decider_flag = NOTHING_DECIDER_FLAG;
        }

        /* Mark flag for those with a worst preference */
        for (p = s->preferences[WORST_PREFERENCE_TYPE]; p != NIL; p = p->next)
        {
            p->value->decider_flag = WORST_DECIDER_FLAG;
        }

        /* Because we only want to add worst preferences to the CDPS if they actually have an impact
        * on the candidate list, we must first see if there's at least one non-worst candidate. */

        if (do_CDPS)
        {
            some_not_worst = false;
            for (cand = candidates; cand != NIL; cand = cand->next_candidate)
            {
                if (cand->value->decider_flag != WORST_DECIDER_FLAG)
                {
                    some_not_worst = true;
                }
            }
        }

        prev_cand = NIL;
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            if (cand->value->decider_flag != WORST_DECIDER_FLAG)
            {
                if (prev_cand)
                {
                    prev_cand->next_candidate = cand;
                }
                else
                {
                    candidates = cand;
                }
                prev_cand = cand;
            }
            else
            {
                if (do_CDPS && some_not_worst)
                {
                    /* Add this worst preference to CDPS */
                    for (p = s->preferences[WORST_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == cand->value)
                        {
                            add_to_CDPS(thisAgent, s, p);
                        }
                    }
                }
            }
        }
        if (prev_cand)
        {
            prev_cand->next_candidate = NIL;
        }
    }

    /* Exit point 4: Check if we're done, i.e. 0 or 1 candidates left */

    if ((!candidates) || (!candidates->next_candidate))
    {
        *result_candidates = candidates;
        if (candidates)
        {
            /* Update RL values for the winning candidate */
            rl_update_for_one_candidate(thisAgent, s, consistency, candidates);
        }
        else
        {
            if (do_CDPS && s->CDPS)
            {
                clear_CDPS(thisAgent, s);
            }
        }
        return NONE_IMPASSE_TYPE;
    }

    /* === Indifferents === */

    /* Initialize decider flags for all candidates */

    for (cand = candidates; cand != NIL; cand = cand->next_candidate)
    {
        cand->value->decider_flag = NOTHING_DECIDER_FLAG;
    }

    /* Mark flag for unary or numeric indifferent preferences */

    for (p = s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]; p; p = p->next)
    {
        p->value->decider_flag = UNARY_INDIFFERENT_DECIDER_FLAG;
    }

    for (p = s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; p; p = p->next)
    {
        p->value->decider_flag = UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG;
    }

    /* Go through candidate list and check for a tie impasse.  All candidates
     * must either be unary indifferent or binary indifferent to every item on
     * the candidate list.  This will also catch when a candidate has no
     * indifferent preferences at all. */

    not_all_indifferent = false;
    some_numeric = false;

    for (cand = candidates; cand != NIL; cand = cand->next_candidate)
    {

        /* If this candidate has a unary indifferent preference, skip. Numeric indifferent
         * prefs are considered to have an implicit unary indifferent pref,
         * which is why they are skipped too. */

        if (cand->value->decider_flag == UNARY_INDIFFERENT_DECIDER_FLAG)
        {
            continue;
        }
        else if (cand->value->decider_flag == UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG)
        {
            some_numeric = true;
            continue;
        }

        /* Candidate has either only binary indifferences or no indifference prefs
         * at all, so make sure there is a binary preference between its operator
         * and every other preference's operator in the candidate list */

        for (p = candidates; p != NIL; p = p->next_candidate)
        {
            if (p == cand)
            {
                continue;
            }
            match_found = false;
            for (p2 = s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p2 != NIL;
                    p2 = p2->next)
                if (((p2->value == cand->value) && (p2->referent == p->value))
                        || ((p2->value == p->value) && (p2->referent == cand->value)))
                {
                    match_found = true;
                    break;
                }
            if (!match_found)
            {
                not_all_indifferent = true;
                break;
            }
        }
        if (not_all_indifferent)
        {
            break;
        }
    }

    if (!not_all_indifferent)
    {
        if (!consistency)
        {
            (*result_candidates) = exploration_choose_according_to_policy(thisAgent, s, candidates);
            if (!predict && rl_enabled(thisAgent))
            {
                build_rl_trace(thisAgent, candidates, *result_candidates);
            }
            (*result_candidates)->next_candidate = NIL;

            if (do_CDPS)
            {

                /* Add all indifferent preferences associated with the chosen candidate to the CDPS.*/

                if (some_numeric)
                {

                    /* Note that numeric indifferent preferences are never considered duplicates, so we
                    * pass an extra argument to add_to_cdps so that it does not check for duplicates.*/

                    for (p = s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == (*result_candidates)->value)
                        {
                            add_to_CDPS(thisAgent, s, p, false);
                        }
                    }

                    /* Now add any binary preferences with a candidate that does NOT have a numeric preference. */

                    for (p = s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if ((p->value == (*result_candidates)->value) || (p->referent == (*result_candidates)->value))
                        {
                            if ((p->referent->decider_flag != UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG) ||
                                    (p->value->decider_flag != UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG))
                            {
                                add_to_CDPS(thisAgent, s, p);
                            }
                        }
                    }
                }
                else
                {

                    /* This decision was non-numeric, so add all non-numeric preferences associated with the
                     * chosen candidate to the CDPS.*/

                    for (p = s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == (*result_candidates)->value)
                        {
                            add_to_CDPS(thisAgent, s, p);
                        }
                    }
                    for (p = s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if ((p->value == (*result_candidates)->value) || (p->referent == (*result_candidates)->value))
                        {
                            add_to_CDPS(thisAgent, s, p);
                        }
                    }
                }
            }
        }
        else
        {
            *result_candidates = candidates;
        }
        return NONE_IMPASSE_TYPE;
    }

    /* Candidates are not all indifferent, so we have a tie. */

    *result_candidates = candidates;
    if (do_CDPS && s->CDPS)
    {
        clear_CDPS(thisAgent, s);
    }
    return TIE_IMPASSE_TYPE;
}

/* **************************************************************************

                      Decider and Impasser Routines

************************************************************************** */

/* ------------------------------------------------------------------
                        Add Impasse Wme

   This creates a new wme and adds it to the given impasse object.
   "Id" indicates the goal/impasse id; (id ^attr value) is the impasse
   wme to be added.  The "preference" argument indicates the preference
   (if non-NIL) for backtracing.
------------------------------------------------------------------ */

void add_impasse_wme(agent* thisAgent, Symbol* id, Symbol* attr, Symbol* value, preference* p)
{
    wme* w;

    w = make_wme(thisAgent, id, attr, value, false);
    insert_at_head_of_dll(id->id->impasse_wmes, w, next, prev);
    w->preference = p;
    add_wme_to_wm(thisAgent, w);
}

/* ------------------------------------------------------------------
                         Create New Impasse

   This creates a new impasse, returning its identifier.  The caller is
   responsible for filling in either id->isa_impasse or id->isa_goal,
   and all the extra stuff for goal identifiers.
------------------------------------------------------------------ */

Symbol* create_new_impasse(agent* thisAgent, bool isa_goal, Symbol* object, Symbol* attr,
                           byte impasse_type, goal_stack_level level)
{
    Symbol* id;

    id = make_new_identifier(thisAgent, (isa_goal ? 'S' : 'I'), level);
    post_link_addition(thisAgent, NIL, id);   /* add the special link */

    add_impasse_wme(thisAgent, id, thisAgent->type_symbol, isa_goal ? thisAgent->state_symbol : thisAgent->impasse_symbol,
                    NIL);

    if (isa_goal)
    {
        add_impasse_wme(thisAgent, id, thisAgent->superstate_symbol, object, NIL);
        id->id->reward_header = make_new_identifier(thisAgent, 'R', level);
        soar_module::add_module_wme(thisAgent, id, thisAgent->rl_sym_reward_link, id->id->reward_header);

        id->id->epmem_header = make_new_identifier(thisAgent, 'E', level);
        soar_module::add_module_wme(thisAgent, id, thisAgent->epmem_sym, id->id->epmem_header);
        id->id->epmem_cmd_header = make_new_identifier(thisAgent, 'C', level);
        soar_module::add_module_wme(thisAgent, id->id->epmem_header, thisAgent->epmem_sym_cmd, id->id->epmem_cmd_header);
        id->id->epmem_result_header = make_new_identifier(thisAgent, 'R', level);
        soar_module::add_module_wme(thisAgent, id->id->epmem_header, thisAgent->epmem_sym_result, id->id->epmem_result_header);

        {
            int64_t my_time = static_cast<int64_t>(thisAgent->epmem_stats->time->get_value());
            if (my_time == 0)
            {
                // special case: pre-initialization
                my_time = 1;
            }

            Symbol* my_time_sym = make_int_constant(thisAgent, my_time);
            id->id->epmem_time_wme = soar_module::add_module_wme(thisAgent, id->id->epmem_header, thisAgent->epmem_sym_present_id, my_time_sym);
            symbol_remove_ref(thisAgent, my_time_sym);
        }

        id->id->smem_header = make_new_identifier(thisAgent, 'S', level);
        soar_module::add_module_wme(thisAgent, id, thisAgent->smem_sym, id->id->smem_header);
        id->id->smem_cmd_header = make_new_identifier(thisAgent, 'C', level);
        soar_module::add_module_wme(thisAgent, id->id->smem_header, thisAgent->smem_sym_cmd, id->id->smem_cmd_header);
        id->id->smem_result_header = make_new_identifier(thisAgent, 'R', level);
        soar_module::add_module_wme(thisAgent, id->id->smem_header, thisAgent->smem_sym_result, id->id->smem_result_header);

    }
    else
    {
        add_impasse_wme(thisAgent, id, thisAgent->object_symbol, object, NIL);
    }

    if (attr)
    {
        add_impasse_wme(thisAgent, id, thisAgent->attribute_symbol, attr, NIL);
    }

    switch (impasse_type)
    {
        case NONE_IMPASSE_TYPE:
            break;    /* this happens only when creating the top goal */
        case CONSTRAINT_FAILURE_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, id, thisAgent->impasse_symbol, thisAgent->constraint_failure_symbol, NIL);
            add_impasse_wme(thisAgent, id, thisAgent->choices_symbol, thisAgent->none_symbol, NIL);
            break;
        case CONFLICT_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, id, thisAgent->impasse_symbol, thisAgent->conflict_symbol, NIL);
            add_impasse_wme(thisAgent, id, thisAgent->choices_symbol, thisAgent->multiple_symbol, NIL);
            break;
        case TIE_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, id, thisAgent->impasse_symbol, thisAgent->tie_symbol, NIL);
            add_impasse_wme(thisAgent, id, thisAgent->choices_symbol, thisAgent->multiple_symbol, NIL);
            break;
        case NO_CHANGE_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, id, thisAgent->impasse_symbol, thisAgent->no_change_symbol, NIL);
            add_impasse_wme(thisAgent, id, thisAgent->choices_symbol, thisAgent->none_symbol, NIL);
            break;
    }

//   if(thisAgent->rl_trace.find(level) == thisAgent->rl_trace.end())
//     std::cerr << "rl-trace: Init level " << level << std::endl;
//   else
//     std::cerr << "rl-trace: Restore level " << level << std::endl;
    id->id->rl_trace = &thisAgent->rl_trace[level];

    return id;
}

/* ------------------------------------------------------------------
               Create/Remove Attribute Impasse for Slot

   These routines create and remove an attribute impasse for a given
   slot.
------------------------------------------------------------------ */

void create_new_attribute_impasse_for_slot(agent* thisAgent, slot* s, byte impasse_type)
{
    Symbol* id;

    s->impasse_type = impasse_type;
    id = create_new_impasse(thisAgent, false, s->id, s->attr, impasse_type,
                            ATTRIBUTE_IMPASSE_LEVEL);
    s->impasse_id = id;
    id->id->isa_impasse = true;

    soar_invoke_callbacks(thisAgent,
                          CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK,
                          static_cast<soar_call_data>(s));
}

void remove_existing_attribute_impasse_for_slot(agent* thisAgent, slot* s)
{
    Symbol* id;

    soar_invoke_callbacks(thisAgent,
                          REMOVE_ATTRIBUTE_IMPASSE_CALLBACK,
                          static_cast<soar_call_data>(s));

    id = s->impasse_id;
    s->impasse_id = NIL;
    s->impasse_type = NONE_IMPASSE_TYPE;
    remove_wme_list_from_wm(thisAgent, id->id->impasse_wmes);
    id->id->impasse_wmes = NIL;
    post_link_removal(thisAgent, NIL, id);   /* remove the special link */
    symbol_remove_ref(thisAgent, id);
}

/* ------------------------------------------------------------------
            Fake Preferences for Goal ^Item Augmentations

   When we backtrace through a (goal ^item) augmentation, we want
   to backtrace to the acceptable preference wme in the supercontext
   corresponding to that ^item.  A slick way to do this automagically
   is to set the backtracing preference pointer on the (goal ^item)
   wme to be a "fake" preference for a "fake" instantiation.  The
   instantiation has as its LHS a list of one condition, which matched
   the acceptable preference wme in the supercontext.

   Make_fake_preference_for_goal_item() builds such a fake preference
   and instantiation, given a pointer to the supergoal and the
   acceptable/require preference for the value, and returns a pointer
   to the fake preference.  *** for Soar 8.3, we changed the fake
   preference to be ACCEPTABLE instead of REQUIRE.  This could
   potentially break some code, but it avoids the BUGBUG condition
   that can occur when you have a REQUIRE lower in the stack than an
   ACCEPTABLE but the goal stack gets popped while the WME backtrace
   still points to the REQUIRE, instead of the higher ACCEPTABLE.
   See the section above on Preference Semantics.  It also allows
   the GDS to backtrace through ^items properly.

   Remove_fake_preference_for_goal_item() is called to clean up the
   fake stuff once the (goal ^item) wme is no longer needed.
------------------------------------------------------------------ */

preference* make_fake_preference_for_goal_item(agent* thisAgent,
        Symbol* goal,
        preference* cand)
{
    slot* s;
    wme* ap_wme;
    instantiation* inst;
    preference* pref;
    condition* cond;

    /* --- find the acceptable preference wme we want to backtrace to --- */
    s = cand->slot;
    for (ap_wme = s->acceptable_preference_wmes; ap_wme != NIL; ap_wme = ap_wme->next)
        if (ap_wme->value == cand->value)
        {
            break;
        }
    if (!ap_wme)
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg,
                "decide.c: Internal error: couldn't find acceptable pref wme\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    /* --- make the fake preference --- */
    /* kjc:  here's where we changed REQUIRE to ACCEPTABLE */
//    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, goal, thisAgent->item_symbol,
//                           cand->value, NIL,
//                           soar_module::symbol_triple(goal, thisAgent->item_symbol, cand->value),
//                           soar_module::g_id_triple(0,0,0));
    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, goal, thisAgent->item_symbol,
                           cand->value, NIL);
    symbol_add_ref(thisAgent, pref->id);
    symbol_add_ref(thisAgent, pref->attr);
    symbol_add_ref(thisAgent, pref->value);
    insert_at_head_of_dll(goal->id->preferences_from_goal, pref,
                          all_of_goal_next, all_of_goal_prev);
    pref->on_goal_list = true;
    preference_add_ref(pref);
    /* --- make the fake instantiation --- */
    allocate_with_pool(thisAgent, &thisAgent->instantiation_pool, &inst);
    pref->inst = inst;
    pref->inst_next = pref->inst_prev = NIL;
    inst->preferences_generated = pref;
    inst->prod = NIL;
    inst->next = inst->prev = NIL;
    inst->rete_token = NIL;
    inst->rete_wme = NIL;
    inst->match_goal = goal;
    inst->match_goal_level = goal->id->level;
    inst->reliable = true;
    inst->backtrace_number = 0;
    inst->in_ms = false;
    /* --- make the fake condition --- */
    allocate_with_pool(thisAgent, &thisAgent->condition_pool, &cond);
    init_condition(cond);
    cond->type = POSITIVE_CONDITION;
    inst->top_of_instantiated_conditions = cond;
    inst->bottom_of_instantiated_conditions = cond;

    cond->data.tests.id_test = make_test(thisAgent, ap_wme->id, EQUALITY_TEST);
//    cond->data.tests.id_test->original_test = copy_test(thisAgent, cond->data.tests.id_test);
//    cond->data.tests.id_test->identity->grounding_wme = ap_wme;
//    cond->data.tests.id_test->identity->grounding_field = ID_ELEMENT;
//    cond->data.tests.id_test->identity->original_var = cond->data.tests.id_test->data.referent;
//    symbol_add_ref(thisAgent, cond->data.tests.id_test->identity->original_var);

    cond->data.tests.attr_test = make_test(thisAgent, ap_wme->attr, EQUALITY_TEST);
//    cond->data.tests.attr_test->original_test = copy_test(thisAgent, cond->data.tests.attr_test);
//    cond->data.tests.attr_test->identity->grounding_wme = ap_wme;
//    cond->data.tests.attr_test->identity->grounding_field = ATTR_ELEMENT;
//    cond->data.tests.attr_test->identity->original_var = cond->data.tests.attr_test->data.referent;
//    symbol_add_ref(thisAgent, cond->data.tests.attr_test->identity->original_var);

    cond->data.tests.value_test = make_test(thisAgent, ap_wme->value, EQUALITY_TEST);
//    cond->data.tests.value_test->original_test = copy_test(thisAgent, cond->data.tests.value_test);
//    cond->data.tests.value_test->identity->grounding_wme = ap_wme;
//    cond->data.tests.value_test->identity->grounding_field = VALUE_ELEMENT;
//    cond->data.tests.value_test->identity->original_var = cond->data.tests.value_test->data.referent;
//    symbol_add_ref(thisAgent, cond->data.tests.value_test->identity->original_var);

    cond->test_for_acceptable_preference = true;
    cond->bt.wme_ = ap_wme;
#ifdef DO_TOP_LEVEL_REF_CTS
    wme_add_ref(ap_wme);
#else
    if (inst->match_goal_level > TOP_GOAL_LEVEL)
    {
        wme_add_ref(ap_wme);
    }
#endif
    cond->bt.level = ap_wme->id->id->level;

    /* --- return the fake preference --- */
    return pref;
}

void remove_fake_preference_for_goal_item(agent* thisAgent, preference* pref)
{
    preference_remove_ref(thisAgent, pref);  /* everything else happens automatically */
}

/* ------------------------------------------------------------------
                       Update Impasse Items

   This routine updates the set of ^item wmes on a goal or attribute
   impasse.  It takes the identifier of the goal/impasse, and a list
   of preferences (linked via the "next_candidate" field) for the new
   set of items that should be there.

   NLD 11/11: using this same basic framework to maintain a parallel
   list for those candidates that do not have numeric preferences.
------------------------------------------------------------------ */

void update_impasse_items(agent* thisAgent, Symbol* id, preference* items)
{
    enum item_types { regular, numeric };

    wme* w, *next_w;
    preference* cand;
    preference* bt_pref;
    unsigned int item_count;
    Symbol* loop_sym = NULL;
    Symbol* loop_count_sym = NULL;
    Symbol* count_sym = NULL;

    for (int it = regular; it <= numeric; it++)
    {
        if (it == regular)
        {
            loop_sym = thisAgent->item_symbol;
            loop_count_sym = thisAgent->item_count_symbol;
        }
        else
        {
            loop_sym = thisAgent->non_numeric_symbol;
            loop_count_sym = thisAgent->non_numeric_count_symbol;
        }

        // reset flags on existing items to NOTHING
        for (w = id->id->impasse_wmes; w != NIL; w = w->next)
            if (w->attr == loop_sym)
            {
                w->value->decider_flag = NOTHING_DECIDER_FLAG;
            }

        // reset flags on all items as CANDIDATEs
        for (cand = items; cand != NIL; cand = cand->next_candidate)
        {
            cand->value->decider_flag = CANDIDATE_DECIDER_FLAG;
        }

        // if numeric, block out candidates with numeric
        if ((it == numeric) && items)
        {
            for (cand = items->slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; cand; cand = cand->next)
            {
                cand->value->decider_flag = NOTHING_DECIDER_FLAG;
            }
        }

        // count up candidates (used for count WME)
        item_count = 0;
        for (cand = items; cand != NIL; cand = cand->next_candidate)
            if (cand->value->decider_flag == CANDIDATE_DECIDER_FLAG)
            {
                item_count++;
            }

        // for each existing item: if supposed to be there, ALREADY EXISTING; otherwise remove
        w = id->id->impasse_wmes;
        while (w)
        {
            next_w = w->next;
            if (w->attr == loop_sym)
            {
                if (w->value->decider_flag == CANDIDATE_DECIDER_FLAG)
                {
                    w->value->decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
                    w->value->decider_wme = w; // so we can update the pref later
                }
                else
                {
                    remove_from_dll(id->id->impasse_wmes, w, next, prev);

                    if (id->id->isa_goal)
                    {
                        remove_fake_preference_for_goal_item(thisAgent, w->preference);
                    }

                    remove_wme_from_wm(thisAgent, w);
                }
            }
            else if (w->attr == loop_count_sym)
            {
                remove_from_dll(id->id->impasse_wmes, w, next, prev);
                remove_wme_from_wm(thisAgent, w);
            }

            w = next_w;
        }

        // for each desired item: if doesn't ALREADY_EXIST, add it
        for (cand = items; cand != NIL; cand = cand->next_candidate)
        {
            // takes care of numerics
            if (cand->value->decider_flag == NOTHING_DECIDER_FLAG)
            {
                continue;
            }

            if (id->id->isa_goal)
            {
                bt_pref = make_fake_preference_for_goal_item(thisAgent, id, cand);
            }
            else
            {
                bt_pref = cand;
            }

            if (cand->value->decider_flag == ALREADY_EXISTING_WME_DECIDER_FLAG)
            {
                if (id->id->isa_goal)
                {
                    remove_fake_preference_for_goal_item(thisAgent, cand->value->decider_wme->preference);
                }

                cand->value->decider_wme->preference = bt_pref;
            }
            else
            {
                add_impasse_wme(thisAgent, id, loop_sym, cand->value, bt_pref);
            }
        }

        if (item_count > 0)
        {
            count_sym = make_int_constant(thisAgent, static_cast< int64_t >(item_count));
            add_impasse_wme(thisAgent, id, loop_count_sym, count_sym, NIL);
            symbol_remove_ref(thisAgent, count_sym);
        }
    }
}

/* ------------------------------------------------------------------
                       Decide Non Context Slot

   This routine decides a given slot, which must be a non-context
   slot.  It calls run_preference_semantics() on the slot, then
   updates the wmes and/or impasse for the slot accordingly.
------------------------------------------------------------------ */

void decide_non_context_slot(agent* thisAgent, slot* s)
{
    byte impasse_type;
    wme* w, *next_w;
    preference* candidates, *cand, *pref;

    impasse_type = run_preference_semantics(thisAgent, s, &candidates);

    if (impasse_type == NONE_IMPASSE_TYPE)
    {
        /* --- no impasse, so remove any existing one and update the wmes --- */
        if (s->impasse_type != NONE_IMPASSE_TYPE)
        {
            remove_existing_attribute_impasse_for_slot(thisAgent, s);
        }

        /* --- reset marks on existing wme values to "NOTHING" --- */
        for (w = s->wmes; w != NIL; w = w->next)
        {
            w->value->decider_flag = NOTHING_DECIDER_FLAG;
        }

        /* --- set marks on desired values to "CANDIDATES" --- */
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            cand->value->decider_flag = CANDIDATE_DECIDER_FLAG;
        }

        /* --- for each existing wme, if we want it there, mark it as
        ALREADY_EXISTING; otherwise remove it --- */
        w = s->wmes;
        while (w)
        {
            next_w = w->next;
            if (w->value->decider_flag == CANDIDATE_DECIDER_FLAG)
            {
                w->value->decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
                w->value->decider_wme = w; /* so we can set the pref later */
            }
            else
            {
                remove_from_dll(s->wmes, w, next, prev);
                /* REW: begin 09.15.96 */
                if (w->gds)
                {
                    if (w->gds->goal != NIL)
                    {
                        /* If the goal pointer is non-NIL, then goal is in the stack */
                        gds_invalid_so_remove_goal(thisAgent, w);
                    }
                }
                /* REW: end   09.15.96 */
                remove_wme_from_wm(thisAgent, w);
            }
            w = next_w;
        }  /* end while (W) */

        /* --- for each desired value, if it's not already there, add it --- */
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            if (cand->value->decider_flag == ALREADY_EXISTING_WME_DECIDER_FLAG)
            {
                /* REW: begin 11.22.97 */
                /* print(thisAgent, "\n This WME was marked as already existing...."); print_wme(cand->value->decider_wme); */

                /* REW: end   11.22.97 */
                cand->value->decider_wme->preference = cand;
            }
            else
            {
                if (cand->o_supported && (cand->inst->match_goal_level != 1) &&
                        (cand->inst->match_goal->id->gds == NIL) &&
                        (cand->inst->match_goal_level != cand->id->id->level))
                {
                    /* -- WME from duplicate production.  Skipping GDS processing for new wme.  This avoids
                     *    creating a GDS for a WME level different from the instantiation level.  This
                     *    solution seems to work well, but it's possible that there are subtle aspects
                     *    of the GDS that aren't being appreciated.  See comments below. -- */
                    continue;
                }
                else
                {
                    dprint_noprefix(DT_GDS, "\n");
                    dprint(DT_GDS, "wme not a duplicate.  Performing gds processing for newly made wme %p (id level = %d, mg level = %d)\n",
                           cand, cand->id->id->level, cand->inst->match_goal_level);
                    dprint(DT_GDS, "Generated from preference created by instantiation:\n");
                    dprint_set_params(DT_GDS, "           ");
                    dprint(DT_GDS, "%7", cand->inst);
                }
                w = make_wme(thisAgent, cand->id, cand->attr, cand->value, false);
                insert_at_head_of_dll(s->wmes, w, next, prev);
                w->preference = cand;

                if ((s->wma_val_references != NIL) && wma_enabled(thisAgent))
                {
                    wma_sym_reference_map::iterator it = s->wma_val_references->find(w->value);
                    if (it != s->wma_val_references->end())
                    {
                        // should only activate at this point if WME is o-supported
                        wma_activate_wme(thisAgent, w, it->second, NULL, true);

                        s->wma_val_references->erase(it);
                        if (s->wma_val_references->empty())
                        {
                            s->wma_val_references->~wma_sym_reference_map();
                            free_with_pool(&(thisAgent->wma_slot_refs_pool), s->wma_val_references);
                            s->wma_val_references = NIL;
                        }
                    }
                }

                /* REW: begin 09.15.96 */
                /* Whenever we add a WME to WM, we also want to check and see if
                this new WME is o-supported.  If so, then we want to add the
                supergoal dependencies of the new, o-supported element to the
                goal in which the element was created (as long as the o_supported
                element was not created in the top state -- the top goal has
                no gds).  */

                /* REW: begin 11.25.96 */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
                thisAgent->timers_gds.start();
#endif
#endif
                /* REW: end   11.25.96 */

                thisAgent->parent_list_head = NIL;

                /* If the working memory element being added is going to have
                o_supported preferences and the instantiation that created it
                is not in the top_level_goal (where there is no GDS), then
                loop over the preferences for this WME and determine which
                WMEs should be added to the goal's GDS (the goal here being the
                goal to which the added memory is attached). */

                if ((w->preference->o_supported == true) && (w->preference->inst->match_goal_level != 1))
                {
                    dprint(DT_GDS, "Checking GDS necessary for wme %w: %s (level = %d)\n", w,
                           (w->preference->o_supported ? ":o-support" : ":i-support"),
                           w->preference->id->id->level);
                    dprint(DT_GDS, "Generated from preference created by instantiation:\n");
                    dprint(DT_GDS, "%7", w->preference->inst);

                    if (w->preference->inst->match_goal->id->gds == NIL)
                    {
                        /* If there is no GDS yet for this goal,
                        * then we need to create one */
                        dprint(DT_GDS, "Creating new GDS for match goal...\n");
                        if (w->preference->inst->match_goal_level == w->preference->id->id->level)
                        {
                            /*
                            * NLD: BUG when the system has already halted and this code is reached, Soar will
                            * report a memory leak because the elaborate_gds call below will not execute (notice the
                            * check for system_halted), and hence the gds for this goal will not be populated,
                            * leading to the gds struct not being freed on quit.
                            *
                            * I'm not sure if this situation will come up anymore (after r12593), so not doing
                            * anything about it. However, if it does, this can lead to a memory leak. I know
                            * it can add up between calls to init-soar, but I'm not sure if it can increase
                            * more frequently than that.
                            */
                            create_gds_for_goal(thisAgent, w->preference->inst->match_goal);

                            /* REW: BUG When chunks and result instantiations both create
                            * preferences for the same WME, then we only want to create
                            * the GDS for the highest goal.  Right now I ensure that we
                            * elaborate the correct GDS with the tests in the loop just
                            * below this code, but the GDS creation above assumes that
                            * the chunk will be first on the GDS list.  This order
                            * appears to be always true, although I am not 100% certain
                            * (I think it occurs this way because the chunk is
                            * necessarily added to the instantiation list after the
                            * original instantiation and lists get built such older items
                            * appear further from the head of the list) . If not true,
                            * then we need to keep track of any GDS's that get created
                            * here to remove them later if we find a higher match goal
                            * for the WME. For now, the program just exits in this
                            * situation; otherwise, we would build a GDS for the wrong
                            * level and never elaborate it (resulting in a memory
                            * leak).
                            */
                        }
                        else
                        {
                            /* -- Should not be able to get here any more with new clause checking for this
                             *    cumulative state earlier. Leaving comment in case this solution doesn't
                             *    prove to be sufficient and we need to do something more complex, as
                             *    described in the previous comment by REW. -- */

                            // If this happens, we better be halted, see chunk.cpp:chunk_instantiation
                            // This can happen if a chunk can't be created, because then the match level
                            // of the preference instantiation can map back to the original matching
                            // production which can be at a different level than the id wme.
                            // Normally, there would be a chunk or justification firing at the higher
                            // goal with a match level equal to the id level.
                            // See more comments in chunk_instantiation.
                        }
                    } /* end if no GDS yet for goal... */

                    /* Loop over all the preferences for this WME:
                    *   If the instantiation that lead to the preference has not
                    *         been already explored; OR
                    *   If the instantiation is not an subgoal instantiation
                    *          for a chunk instantiation we are already exploring
                    *   Then
                    *      Add the instantiation to a list of instantiations that
                    *          will be explored in elaborate_gds().
                    */

                    // Added halt test because chunk_instantiation can cause problems,
                    // see comment a few lines above and in chunk_instantiation.
                    if (!thisAgent->system_halted)
                    {
                        for (pref = w->preference; pref != NIL; pref = pref->next)
                        {
#ifdef DEBUG_GDS_HIGH
                            print(thisAgent, "\n\n   ");
                            print_preference(pref);
                            print(thisAgent,  "   Goal level of preference: %d\n",
                                  pref->id->id->level);
#endif

                            if (pref->inst->GDS_evaluated_already == false)
                            {
#ifdef DEBUG_GDS_HIGH
                                print_with_symbols(thisAgent, "   Match goal lev of instantiation %y ",
                                                   pref->inst->prod->name);
                                print(thisAgent,  "is %d\n", pref->inst->match_goal_level);
#endif
                                if (pref->inst->match_goal_level > pref->id->id->level)
                                {
#ifdef DEBUG_GDS_HIGH
                                    print_with_symbols(thisAgent, "        %y is simply the instantiation that led to a chunk.\n        Not adding it the current instantiations.\n", pref->inst->prod->name);
#endif

                                }
                                else
                                {
#ifdef DEBUG_GDS_HIGH
                                    print_with_symbols(thisAgent, "\n   Adding %y to list of parent instantiations\n", pref->inst->prod->name);
#endif
                                    uniquely_add_to_head_of_dll(thisAgent, pref->inst);
                                    pref->inst->GDS_evaluated_already = true;
                                }
                            }  /* end if GDS_evaluated_already is false */
#ifdef DEBUG_GDS_HIGH
                            else
                            {
                                print_with_symbols(thisAgent, "\n    Instantiation %y was already explored; skipping it\n", pref->inst->prod->name);
                            }
#endif

                        }  /* end of forloop over preferences for this wme */

#ifdef DEBUG_GDS_HIGH
                        print(thisAgent,  "\n    CALLING ELABORATE GDS....\n");
#endif
                        elaborate_gds(thisAgent);

                        /* technically, the list should be empty at this point ??? */

                        free_parent_list(thisAgent);
#ifdef DEBUG_GDS_HIGH
                        print(thisAgent,  "    FINISHED ELABORATING GDS.\n\n");
#endif
                    } /* end if not halted */
                }  /* end if w->preference->o_supported == true ... */

                /* REW: begin 11.25.96 */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
                thisAgent->timers_gds.stop();
                thisAgent->timers_gds_cpu_time[thisAgent->current_phase].update(thisAgent->timers_gds);
#endif
#endif
                /* REW: end   11.25.96 */
                /* REW: end   09.15.96 */

                add_wme_to_wm(thisAgent, w);
            }
        }

        return;
    } /* end of if impasse type == NONE */

    /* --- impasse type != NONE --- */
    if (s->wmes)
    {
        /* --- remove any existing wmes --- */
        remove_wme_list_from_wm(thisAgent, s->wmes);
        s->wmes = NIL;
    }

    /* --- create and/or update impasse structure --- */
    if (s->impasse_type != NONE_IMPASSE_TYPE)
    {
        if (s->impasse_type != impasse_type)
        {
            remove_existing_attribute_impasse_for_slot(thisAgent, s);
            create_new_attribute_impasse_for_slot(thisAgent, s, impasse_type);
        }
        update_impasse_items(thisAgent, s->impasse_id, candidates);
    }
    else
    {
        create_new_attribute_impasse_for_slot(thisAgent, s, impasse_type);
        update_impasse_items(thisAgent, s->impasse_id, candidates);
    }
}

/* ------------------------------------------------------------------
                      Decide Non Context Slots

   This routine iterates through all changed non-context slots, and
   decides each one.
------------------------------------------------------------------ */

void decide_non_context_slots(agent* thisAgent)
{
    dl_cons* dc;
    slot* s;

    dprint(DT_EPMEM_CMD, "Deciding non-context slots.\n");
    while (thisAgent->changed_slots)
    {
        dc = thisAgent->changed_slots;
        thisAgent->changed_slots = thisAgent->changed_slots->next;
        s = static_cast<slot_struct*>(dc->item);
        dprint(DT_EPMEM_CMD, "Deciding non-context slot (%y ^%y ?)\n", s->id, s->attr);
        decide_non_context_slot(thisAgent, s);
        s->changed = NIL;
        dprint(DT_EPMEM_CMD, "Done deciding non-context slot (%y ^%y ?)\n", s->id, s->attr);
        free_with_pool(&thisAgent->dl_cons_pool, dc);
    }
    dprint(DT_EPMEM_CMD, "Done deciding non-context slots.\n");
}

/* ------------------------------------------------------------------
                      Context Slot Is Decidable

   This returns true iff the given slot (which must be a context slot)
   is decidable.  A context slot is decidable if it has no installed
   value but does have changed preferences
------------------------------------------------------------------ */

bool context_slot_is_decidable(slot* s)
{
    if (!s->wmes)
    {
        return (s->changed != NIL);
    }

    return false;
}

/* ------------------------------------------------------------------
                      Remove WMEs For Context Slot

   This removes the wmes (there can only be 0 or 1 of them) for the
   given context slot.
------------------------------------------------------------------ */

void remove_wmes_for_context_slot(agent* thisAgent, slot* s)
{
    wme* w;

    if (!s->wmes)
    {
        return;
    }
    /* Note that we only need to handle one wme--context slots never have
       more than one wme in them */
    w = s->wmes;
    preference_remove_ref(thisAgent, w->preference);
    remove_wme_from_wm(thisAgent, w);
    s->wmes = NIL;
}

/* ------------------------------------------------------------------
                 Remove Existing Context And Descendents

   This routine truncates the goal stack by removing the given goal
   and all its subgoals.  (If the given goal is the top goal, the
   entire context stack is removed.)
------------------------------------------------------------------ */

void remove_existing_context_and_descendents(agent* thisAgent, Symbol* goal)
{
    preference* p;

    ms_change* head, *tail;  /* REW:   08.20.97 */

    /* --- remove descendents of this goal --- */
    // BUGBUG this recursion causes a stack overflow if the goal depth is large
    if (goal->id->lower_goal)
    {
        remove_existing_context_and_descendents(thisAgent, goal->id->lower_goal);
    }

    /* --- invoke callback routine --- */
    soar_invoke_callbacks(thisAgent,
                          POP_CONTEXT_STACK_CALLBACK,
                          static_cast<soar_call_data>(goal));

    if ((goal != thisAgent->top_goal) && rl_enabled(thisAgent))
    {
        rl_tabulate_reward_value_for_goal(thisAgent, goal);
        rl_perform_update(thisAgent, 0, true, goal, false);   // this update only sees reward - there is no next state
        rl_clear_refs(goal);
    }

    /* --- disconnect this goal from the goal stack --- */
    if (goal == thisAgent->top_goal)
    {
        thisAgent->top_goal = NIL;
        thisAgent->bottom_goal = NIL;
    }
    else
    {
        thisAgent->bottom_goal = goal->id->higher_goal;
        thisAgent->bottom_goal->id->lower_goal = NIL;
    }

    /* --- remove any preferences supported by this goal --- */
#ifdef DO_TOP_LEVEL_REF_CTS
    while (goal->id->preferences_from_goal)
    {
        p = goal->id->preferences_from_goal;
        remove_from_dll(goal->id->preferences_from_goal, p,
                        all_of_goal_next, all_of_goal_prev);
        p->on_goal_list = false;
        if (! remove_preference_from_clones(thisAgent, p))
            if (p->in_tm)
            {
                remove_preference_from_tm(thisAgent, p);
            }
    }
#else
    /* KJC Aug 05: this seems to cure a potential for exceeding callstack
     * when popping soar's goal stack and not doing DO_TOP_LEVEL_REF_CTS
     * Probably should make this change for all cases, but needs testing.  */
    /* Prefs are added to head of dll, so try removing from tail */
    if (goal->id->preferences_from_goal)
    {
        p = goal->id->preferences_from_goal;
        while (p->all_of_goal_next)
        {
            p = p->all_of_goal_next;
        }
        while (p)
        {
            preference* p_next = p->all_of_goal_prev; // RPM 10/06 we need to save this because p may be freed by the end of the loop
            remove_from_dll(goal->id->preferences_from_goal, p,
                            all_of_goal_next, all_of_goal_prev);
            p->on_goal_list = false;
            if (! remove_preference_from_clones(thisAgent, p))
                if (p->in_tm)
                {
                    remove_preference_from_tm(thisAgent, p);
                }
            p = p_next;
        }
    }
#endif
    /* --- remove wmes for this goal, and garbage collect --- */
    remove_wmes_for_context_slot(thisAgent, goal->id->operator_slot);
    update_impasse_items(thisAgent, goal, NIL);  /* causes items & fake pref's to go away */

    epmem_reset(thisAgent, goal);
    smem_reset(thisAgent, goal);

    remove_wme_list_from_wm(thisAgent, goal->id->impasse_wmes);
    goal->id->impasse_wmes = NIL;
    /* REW: begin   09.15.96 */
    /* If there was a GDS for this goal, we want to set the pointer for the
       goal to NIL to indicate it no longer exists.
       BUG: We probably also need to make certain that the GDS doesn't need
       to be free'd here as well. */
    if (goal->id->gds != NIL)
    {
        goal->id->gds->goal = NIL;
    }
    /* REW: end   09.15.96 */

    /* REW: begin 08.20.97 */

    /* If we remove a goal WME, then we have to transfer any already existing
       retractions to the nil-goal list on the current agent.  We should be
       able to do this more efficiently but the most obvious way (below) still
       requires scanning over the whole list (to set the goal pointer of each
       msc to NIL); therefore this solution should be acceptably efficient. */

    if (goal->id->ms_retractions)   /* There's something on the retraction list */
    {

        head = goal->id->ms_retractions;
        tail = head;

        /* find the tail of this list */
        while (tail->next_in_level)
        {
            tail->goal = NIL;  /* force the goal to be NIL */
            tail = tail->next_in_level;
        }
        tail->goal = NIL;

        if (thisAgent->nil_goal_retractions)
        {
            /* There are already retractions on the list */

            /* Append this list to front of NIL goal list */
            thisAgent->nil_goal_retractions->prev_in_level = tail;
            tail->next_in_level = thisAgent->nil_goal_retractions;
            thisAgent->nil_goal_retractions = head;

        }
        else     /* If no retractions, make this list the NIL goal list */
        {
            thisAgent->nil_goal_retractions = head;
        }
    }

    goal->id->rl_info->eligibility_traces->~rl_et_map();
    free_with_pool(&(thisAgent->rl_et_pool), goal->id->rl_info->eligibility_traces);
    goal->id->rl_info->prev_op_rl_rules->~rl_rule_list();
    free_with_pool(&(thisAgent->rl_rule_pool), goal->id->rl_info->prev_op_rl_rules);
    symbol_remove_ref(thisAgent, goal->id->reward_header);
    free_with_pool(&(thisAgent->rl_info_pool), goal->id->rl_info);

    goal->id->epmem_info->epmem_wmes->~epmem_wme_stack();
    free_with_pool(&(thisAgent->epmem_wmes_pool), goal->id->epmem_info->epmem_wmes);
    symbol_remove_ref(thisAgent, goal->id->epmem_cmd_header);
    symbol_remove_ref(thisAgent, goal->id->epmem_result_header);
    symbol_remove_ref(thisAgent, goal->id->epmem_header);
    free_with_pool(&(thisAgent->epmem_info_pool), goal->id->epmem_info);

    goal->id->smem_info->smem_wmes->~smem_wme_stack();
    free_with_pool(&(thisAgent->smem_wmes_pool), goal->id->smem_info->smem_wmes);
    symbol_remove_ref(thisAgent, goal->id->smem_cmd_header);
    symbol_remove_ref(thisAgent, goal->id->smem_result_header);
    symbol_remove_ref(thisAgent, goal->id->smem_header);
    free_with_pool(&(thisAgent->smem_info_pool), goal->id->smem_info);

#ifndef NO_SVS
    thisAgent->svs->state_deletion_callback(goal);
#endif
    /* REW: BUG
     * Tentative assertions can exist for removed goals.  However, it looks
     * like the removal forces a tentative retraction, which then leads to
     * the deletion of the tentative assertion.  However, I have not tested
     * such cases exhaustively -- I would guess that some processing may be
     * necessary for the assertions here at some point?
     */

    /* REW: end   08.20.97 */

    /* We have to remove this state from the list of states to learn in (NLD: and free cons)
     * jzxu April 24, 2009 */
    free_list(thisAgent, extract_list_elements(thisAgent, &thisAgent->chunky_problem_spaces, cons_equality_fn, reinterpret_cast<void*>(goal)));
    free_list(thisAgent, extract_list_elements(thisAgent, &thisAgent->chunk_free_problem_spaces, cons_equality_fn, reinterpret_cast<void*>(goal)));

    post_link_removal(thisAgent, NIL, goal);   /* remove the special link */
    symbol_remove_ref(thisAgent, goal);

    if (goal->id->level <= thisAgent->substate_break_level)
    {
        thisAgent->stop_soar = true;
        thisAgent->substate_break_level = 0;
        thisAgent->reason_for_stopping = "Stopped due to substate (goal) retraction.";
    }
}

/* ------------------------------------------------------------------
                         Create New Context

   This routine creates a new goal context (becoming the new bottom
   goal) below the current bottom goal.  If there is no current
   bottom goal, this routine creates a new goal and makes it both
   the top and bottom goal.
------------------------------------------------------------------ */

void create_new_context(agent* thisAgent, Symbol* attr_of_impasse, byte impasse_type)
{
    Symbol* id;

    if (thisAgent->bottom_goal)
    {
        /* Creating a sub-goal (or substate) */
        id = create_new_impasse(thisAgent, true, thisAgent->bottom_goal,
                                attr_of_impasse, impasse_type,
                                static_cast<goal_stack_level>(thisAgent->bottom_goal->id->level + 1));
        id->id->higher_goal = thisAgent->bottom_goal;
        thisAgent->bottom_goal->id->lower_goal = id;
        thisAgent->bottom_goal = id;
        add_impasse_wme(thisAgent, id, thisAgent->quiescence_symbol,
                        thisAgent->t_symbol, NIL);
        if ((NO_CHANGE_IMPASSE_TYPE == impasse_type) &&
                (thisAgent->sysparams[MAX_GOAL_DEPTH] < thisAgent->bottom_goal->id->level))
        {
            // appear to be SNC'ing deep in goalstack, so interrupt and warn user
            // KJC note: we actually halt, because there is no interrupt function in SoarKernel
            // in the gSKI Agent code, if system_halted, MAX_GOAL_DEPTH is checked and if exceeded
            // then the interrupt is generated and system_halted is set to false so the user can recover.
            print(thisAgent,  "\nGoal stack depth exceeded %d on a no-change impasse.\n", thisAgent->sysparams[MAX_GOAL_DEPTH]);
            print(thisAgent,  "Soar appears to be in an infinite loop.  \nContinuing to subgoal may cause Soar to \nexceed the program stack of your system.\n");
            xml_generate_warning(thisAgent, "\nGoal stack depth exceeded on a no-change impasse.\n");
            xml_generate_warning(thisAgent, "Soar appears to be in an infinite loop.  \nContinuing to subgoal may cause Soar to \nexceed the program stack of your system.\n");
            thisAgent->stop_soar = true;
            thisAgent->system_halted = true;
            thisAgent->reason_for_stopping = "Max Goal Depth exceeded.";
        }
    }
    else
    {
        /* Creating the top state */
        id = create_new_impasse(thisAgent, true, thisAgent->nil_symbol,
                                NIL, NONE_IMPASSE_TYPE,
                                TOP_GOAL_LEVEL);
        thisAgent->top_goal = id;
        thisAgent->bottom_goal = id;
        thisAgent->top_state = thisAgent->top_goal;
        id->id->higher_goal = NIL;
        id->id->lower_goal = NIL;
    }

    id->id->isa_goal = true;
    id->id->operator_slot = make_slot(thisAgent, id, thisAgent->operator_symbol);
    id->id->allow_bottom_up_chunks = true;

    allocate_with_pool(thisAgent, &(thisAgent->rl_info_pool), &(id->id->rl_info));
    id->id->rl_info->previous_q = 0;
    id->id->rl_info->reward = 0;
    id->id->rl_info->gap_age = 0;
    id->id->rl_info->hrl_age = 0;
    allocate_with_pool(thisAgent, &(thisAgent->rl_et_pool), &(id->id->rl_info->eligibility_traces));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->rl_info->eligibility_traces = new(id->id->rl_info->eligibility_traces) rl_et_map(std::less< production* >(), soar_module::soar_memory_pool_allocator< std::pair< production*, double > >(thisAgent));
#else
    id->id->rl_info->eligibility_traces = new(id->id->rl_info->eligibility_traces) rl_et_map();
#endif
    allocate_with_pool(thisAgent, &(thisAgent->rl_rule_pool), &(id->id->rl_info->prev_op_rl_rules));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->rl_info->prev_op_rl_rules = new(id->id->rl_info->prev_op_rl_rules) rl_rule_list(soar_module::soar_memory_pool_allocator< production* >(thisAgent));
#else
    id->id->rl_info->prev_op_rl_rules = new(id->id->rl_info->prev_op_rl_rules) rl_rule_list();
#endif

    allocate_with_pool(thisAgent, &(thisAgent->epmem_info_pool), &(id->id->epmem_info));
    id->id->epmem_info->last_ol_time = 0;
    id->id->epmem_info->last_cmd_time = 0;
    id->id->epmem_info->last_cmd_count = 0;
    id->id->epmem_info->last_memory = EPMEM_MEMID_NONE;
    allocate_with_pool(thisAgent, &(thisAgent->epmem_wmes_pool), &(id->id->epmem_info->epmem_wmes));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->epmem_info->epmem_wmes = new(id->id->epmem_info->epmem_wmes) epmem_wme_stack(soar_module::soar_memory_pool_allocator< preference* >(thisAgent));
#else
    id->id->epmem_info->epmem_wmes = new(id->id->epmem_info->epmem_wmes) epmem_wme_stack();
#endif

    allocate_with_pool(thisAgent, &(thisAgent->smem_info_pool), &(id->id->smem_info));
    id->id->smem_info->last_cmd_time[0] = 0;
    id->id->smem_info->last_cmd_time[1] = 0;
    id->id->smem_info->last_cmd_count[0] = 0;
    id->id->smem_info->last_cmd_count[1] = 0;
    allocate_with_pool(thisAgent, &(thisAgent->smem_wmes_pool), &(id->id->smem_info->smem_wmes));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->smem_info->smem_wmes = new(id->id->smem_info->smem_wmes) smem_wme_stack(soar_module::soar_memory_pool_allocator< preference* >(thisAgent));
#else
    id->id->smem_info->smem_wmes = new(id->id->smem_info->smem_wmes) smem_wme_stack();
#endif

    /* --- invoke callback routine --- */
    soar_invoke_callbacks(thisAgent,
                          CREATE_NEW_CONTEXT_CALLBACK,
                          static_cast<soar_call_data>(id));

#ifndef NO_SVS
    thisAgent->svs->state_creation_callback(id);
#endif
}

/* ------------------------------------------------------------------
              Type and Attribute of Existing Impasse

   Given a goal, these routines return the type and attribute,
   respectively, of the impasse just below that goal context.  It
   does so by looking at the impasse wmes for the next lower goal
   in the goal stack.
------------------------------------------------------------------ */

byte type_of_existing_impasse(agent* thisAgent, Symbol* goal)
{
    wme* w;
    char msg[BUFFER_MSG_SIZE];

    if (! goal->id->lower_goal)
    {
        return NONE_IMPASSE_TYPE;
    }
    for (w = goal->id->lower_goal->id->impasse_wmes; w != NIL; w = w->next)
        if (w->attr == thisAgent->impasse_symbol)
        {
            if (w->value == thisAgent->no_change_symbol)
            {
                return NO_CHANGE_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->tie_symbol)
            {
                return TIE_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->constraint_failure_symbol)
            {
                return CONSTRAINT_FAILURE_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->conflict_symbol)
            {
                return CONFLICT_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->none_symbol)
            {
                return NONE_IMPASSE_TYPE;
            }
            strncpy(msg, "decide.c: Internal error: bad type of existing impasse.\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
    strncpy(msg, "decide.c: Internal error: couldn't find type of existing impasse.\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

Symbol* attribute_of_existing_impasse(agent* thisAgent, Symbol* goal)
{
    wme* w;

    if (! goal->id->lower_goal)
    {
        return NIL;
    }
    for (w = goal->id->lower_goal->id->impasse_wmes; w != NIL; w = w->next)
        if (w->attr == thisAgent->attribute_symbol)
        {
            return w->value;
        }
    {
        char msg[BUFFER_MSG_SIZE];
        strncpy(msg, "decide.c: Internal error: couldn't find attribute of existing impasse.\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    return NIL; /* unreachable, but without it, gcc -Wall warns here */
}

/* ------------------------------------------------------------------
                       Decide Context Slot

   This decides the given context slot.  It normally returns true,
   but returns false if the ONLY change as a result of the decision
   procedure was a change in the set of ^item's on the impasse below
   the given slot.
------------------------------------------------------------------ */

bool decide_context_slot(agent* thisAgent, Symbol* goal, slot* s, bool predict = false)
{
    byte impasse_type;
    Symbol* attribute_of_impasse;
    wme* w;
    preference* candidates;
    preference* temp;

    if (!context_slot_is_decidable(s))
    {
        /* --- the only time we decide a slot that's not "decidable" is when it's
        the last slot in the entire context stack, in which case we have a
        no-change impasse there --- */
        impasse_type = NO_CHANGE_IMPASSE_TYPE;
        candidates = NIL; /* we don't want any impasse ^item's later */

        if (predict)
        {
            predict_set(thisAgent, "none");
            return true;
        }
    }
    else
    {
        /* --- the slot is decidable, so run preference semantics on it --- */
        impasse_type = run_preference_semantics(thisAgent, s, &candidates);

        if (predict)
        {
            switch (impasse_type)
            {
                case CONSTRAINT_FAILURE_IMPASSE_TYPE:
                    predict_set(thisAgent, "constraint");
                    break;

                case CONFLICT_IMPASSE_TYPE:
                    predict_set(thisAgent, "conflict");
                    break;

                case TIE_IMPASSE_TYPE:
                    predict_set(thisAgent, "tie");
                    break;

                case NO_CHANGE_IMPASSE_TYPE:
                    predict_set(thisAgent, "none");
                    break;

                default:
                    if (!candidates || (candidates->value->symbol_type != IDENTIFIER_SYMBOL_TYPE))
                    {
                        predict_set(thisAgent, "none");
                    }
                    else
                    {
                        std::string temp = "";

                        // get first letter of id
                        temp += candidates->value->id->name_letter;

                        // get number
                        std::string temp2;
                        to_string(candidates->value->id->name_number, temp2);
                        temp += temp2;

                        predict_set(thisAgent, temp.c_str());
                    }
                    break;
            }

            return true;
        }

        remove_wmes_for_context_slot(thisAgent, s); /* must remove old wme before adding
                                                     the new one (if any) */
        if (impasse_type == NONE_IMPASSE_TYPE)
        {
            if (!candidates)
            {
                /* --- no winner ==> no-change impasse on the previous slot --- */
                impasse_type = NO_CHANGE_IMPASSE_TYPE;
            }
            else if (candidates->next_candidate)
            {
                /* --- more than one winner ==> internal error --- */
                char msg[BUFFER_MSG_SIZE];
                strncpy(msg, "decide.c: Internal error: more than one winner for context slot\n", BUFFER_MSG_SIZE);
                msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                abort_with_fatal_error(thisAgent, msg);
            }
        }
    }  /* end if !context_slot_is_decidable  */

    /* --- mark the slot as not changed --- */
    s->changed = NIL;

    /* --- determine the attribute of the impasse (if there is no impasse,
    * this doesn't matter) --- */
    if (impasse_type == NO_CHANGE_IMPASSE_TYPE)
    {
        if (s->wmes)
        {
            attribute_of_impasse = s->attr;
        }
        else
        {
            attribute_of_impasse = thisAgent->state_symbol;
        }
    }
    else
    {
        /* --- for all other kinds of impasses --- */
        attribute_of_impasse = s->attr;
    }

    /* --- remove wme's for lower slots of this context --- */
    if (attribute_of_impasse == thisAgent->state_symbol)
    {
        remove_wmes_for_context_slot(thisAgent, goal->id->operator_slot);
    }


    /* --- if we have a winner, remove any existing impasse and install the
    new value for the current slot --- */
    if (impasse_type == NONE_IMPASSE_TYPE)
    {
        for (temp = candidates; temp; temp = temp->next_candidate)
        {
            preference_add_ref(temp);
        }

        if (goal->id->lower_goal)
        {
            if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM])
            {
                print_with_symbols(thisAgent, "Removing state %y because of a decision.\n", goal->id->lower_goal);
            }

            remove_existing_context_and_descendents(thisAgent, goal->id->lower_goal);
        }

        w = make_wme(thisAgent, s->id, s->attr, candidates->value, false);
        insert_at_head_of_dll(s->wmes, w, next, prev);
        w->preference = candidates;
        preference_add_ref(w->preference);

        /* JC Adding an operator to working memory in the current state */
        add_wme_to_wm(thisAgent, w);

        for (temp = candidates; temp; temp = temp->next_candidate)
        {
            preference_remove_ref(thisAgent, temp);
        }

        if (rl_enabled(thisAgent))
        {
            rl_store_data(thisAgent, goal, candidates);
        }

        return true;
    }

    /* --- no winner; if an impasse of the right type already existed, just
    update the ^item set on it --- */
    if ((impasse_type == type_of_existing_impasse(thisAgent, goal)) &&
            (attribute_of_impasse == attribute_of_existing_impasse(thisAgent, goal)))
    {
        update_impasse_items(thisAgent, goal->id->lower_goal, candidates);
        return false;
    }

    /* --- no impasse already existed, or an impasse of the wrong type
    already existed --- */
    for (temp = candidates; temp; temp = temp->next_candidate)
    {
        preference_add_ref(temp);
    }

    if (goal->id->lower_goal)
    {
        if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM])
        {
            print_with_symbols(thisAgent, "Removing state %y because it's the wrong type of impasse.\n", goal->id->lower_goal);
        }

        remove_existing_context_and_descendents(thisAgent, goal->id->lower_goal);
    }

    /* REW: begin 10.24.97 */
    if (thisAgent->waitsnc && (impasse_type == NO_CHANGE_IMPASSE_TYPE) && (attribute_of_impasse == thisAgent->state_symbol))
    {
        thisAgent->waitsnc_detect = true;
    }
    else
    {
        /* REW: end     10.24.97 */
        create_new_context(thisAgent, attribute_of_impasse, impasse_type);
        update_impasse_items(thisAgent, goal->id->lower_goal, candidates);
    }

    for (temp = candidates; temp; temp = temp->next_candidate)
    {
        preference_remove_ref(thisAgent, temp);
    }

    return true;
}

/* ------------------------------------------------------------------
                       Decide Context Slots

   This scans down the goal stack and runs the decision procedure on
   the appropriate context slots.
------------------------------------------------------------------ */

void decide_context_slots(agent* thisAgent, bool predict = false)
{
    Symbol* goal;
    slot* s;

    if (thisAgent->highest_goal_whose_context_changed)
    {
        goal = thisAgent->highest_goal_whose_context_changed;
    }
    else
        /* no context changed, so jump right to the bottom */
    {
        goal = thisAgent->bottom_goal;
    }

    s = goal->id->operator_slot;

    /* --- loop down context stack --- */
    while (true)
    {
        /* --- find next slot to decide --- */
        while (true)
        {
            if (context_slot_is_decidable(s))
            {
                break;
            }

            if ((s == goal->id->operator_slot) || (! s->wmes))
            {
                /* --- no more slots to look at for this goal; have we reached
                the last slot in whole stack? --- */
                if (! goal->id->lower_goal)
                {
                    break;
                }

                /* --- no, go down one level --- */
                goal = goal->id->lower_goal;
                s = goal->id->operator_slot;
            }
        } /* end of while (true) find next slot to decide */

        /* --- now go and decide that slot --- */
        if (decide_context_slot(thisAgent, goal, s, predict))
        {
            break;
        }

    } /* end of while (true) loop down context stack */

    if (!predict)
    {
        thisAgent->highest_goal_whose_context_changed = NIL;
    }
}

/* **********************************************************************

                      Top-Level Decider Routines

   Init_decider() should be called at startup time to initialize this
   module.

   Do_buffered_wm_and_ownership_changes() does the end-of-phase processing
   of WM changes, ownership calculations, garbage collection, etc.

   Do_working_memory_phase() and do_decision_phase() are called from
   the top level to run those phases.

   Create_top_goal() creates the top goal in the goal stack.
   Clear_goal_stack() wipes out the whole goal stack--this is called
   during an init-soar.

   Print_lowest_slot_in_context_stack() is used for the watch 0 trace
   to print the context slot that was just decided.
********************************************************************** */

void init_decider(agent* thisAgent)
{
    init_memory_pool(thisAgent, &thisAgent->slot_pool, sizeof(slot), "slot");
    init_memory_pool(thisAgent, &thisAgent->wme_pool, sizeof(wme), "wme");
    init_memory_pool(thisAgent, &thisAgent->preference_pool,
                     sizeof(preference), "preference");
}

void do_buffered_wm_and_ownership_changes(agent* thisAgent)
{
    do_buffered_acceptable_preference_wme_changes(thisAgent);
    do_buffered_link_changes(thisAgent);
    do_buffered_wm_changes(thisAgent);
    remove_garbage_slots(thisAgent);
}

void do_working_memory_phase(agent* thisAgent)
{

    if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
    {
        if (thisAgent->current_phase == APPLY_PHASE)    /* it's always IE for PROPOSE */
        {
            xml_begin_tag(thisAgent, kTagSubphase);
            xml_att_val(thisAgent, kPhase_Name, kSubphaseName_ChangingWorkingMemory);
            switch (thisAgent->FIRING_TYPE)
            {
                case PE_PRODS:
                    print(thisAgent,  "\t--- Change Working Memory (PE) ---\n", 0);
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_PE);
                    break;
                case IE_PRODS:
                    print(thisAgent,  "\t--- Change Working Memory (IE) ---\n", 0);
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_IE);
                    break;
            }
            xml_end_tag(thisAgent, kTagSubphase);
        }
    }

    decide_non_context_slots(thisAgent);
    do_buffered_wm_and_ownership_changes(thisAgent);
}

void do_decision_phase(agent* thisAgent, bool predict)
{
    predict_srand_restore_snapshot(thisAgent, !predict);

    decide_context_slots(thisAgent, predict);

    if (!predict)
    {
        do_buffered_wm_and_ownership_changes(thisAgent);

        /*
         * Bob provided a solution to fix WME's hanging around unsupported
         * for an elaboration cycle.
         */
        decide_non_context_slots(thisAgent);
        do_buffered_wm_and_ownership_changes(thisAgent);

        exploration_update_parameters(thisAgent);
    }
}

void create_top_goal(agent* thisAgent)
{
    create_new_context(thisAgent, NIL, NONE_IMPASSE_TYPE);
    thisAgent->highest_goal_whose_context_changed = NIL;  /* nothing changed yet */
    do_buffered_wm_and_ownership_changes(thisAgent);
}

void clear_goal_stack(agent* thisAgent)
{
    if (!thisAgent->top_goal)
    {
        return;
    }

    remove_existing_context_and_descendents(thisAgent, thisAgent->top_goal);
    thisAgent->highest_goal_whose_context_changed = NIL;  /* nothing changed yet */
    do_buffered_wm_and_ownership_changes(thisAgent);
    thisAgent->top_state = NIL;
    thisAgent->active_goal = NIL;
    do_input_cycle(thisAgent);  /* tell input functions that the top state is gone */
    do_output_cycle(thisAgent); /* tell output functions that output commands are gone */
}

void print_lowest_slot_in_context_stack(agent* thisAgent)
{

    if (thisAgent->bottom_goal->id->operator_slot->wmes)
    {
        print_stack_trace(thisAgent, thisAgent->bottom_goal->id->operator_slot->wmes->value,
                          thisAgent->bottom_goal, FOR_OPERATORS_TF, true);

        /*
        this coded is needed just so that when an ONC is created in OPERAND
        (i.e. if the previous goal's operator slot is not empty), it's stack
        trace line doesn't get a number.  this is done because in OPERAND,
        ONCs are detected for "free".
        */

    }
    else
    {

        if (thisAgent->d_cycle_count == 0)
            print_stack_trace(thisAgent, thisAgent->bottom_goal,
                              thisAgent->bottom_goal, FOR_STATES_TF, true);
        else
        {
            if (thisAgent->bottom_goal->id->higher_goal &&
                    thisAgent->bottom_goal->id->higher_goal->id->operator_slot->wmes)
            {
                print_stack_trace(thisAgent, thisAgent->bottom_goal,
                                  thisAgent->bottom_goal,
                                  FOR_STATES_TF, true);
            }
            else
            {
                print_stack_trace(thisAgent, thisAgent->bottom_goal,
                                  thisAgent->bottom_goal,
                                  FOR_STATES_TF, true);
            }
        }
    }
}

void uniquely_add_to_head_of_dll(agent* thisAgent, instantiation* inst)
{

    parent_inst* new_pi, *curr_pi;

    /* print(thisAgent, "UNIQUE DLL:         scanning parent list...\n"); */

    for (curr_pi = thisAgent->parent_list_head;
            curr_pi;
            curr_pi = curr_pi->next)
    {
        if (curr_pi->inst == inst)
        {
#ifdef DEBUG_GDS
            print_with_symbols(thisAgent, "UNIQUE DLL:            %y is already in parent list\n", curr_pi->inst->prod->name);
#endif
            return;
        }
#ifdef DEBUG_GDS
        print_with_symbols(thisAgent, "UNIQUE DLL:            %y\n", curr_pi->inst->prod->name);
#endif
    } /* end for loop */

    new_pi = static_cast<parent_inst*>(malloc(sizeof(parent_inst)));
    new_pi->next = NIL;
    new_pi->prev = NIL;
    new_pi->inst = inst;

    new_pi->next = thisAgent->parent_list_head;

    if (thisAgent->parent_list_head != NIL)
    {
        thisAgent->parent_list_head->prev = new_pi;
    }

    thisAgent->parent_list_head = new_pi;
#ifdef DEBUG_GDS
    print_with_symbols(thisAgent, "UNIQUE DLL:         added: %y\n", inst->prod->name);
#endif
}

/* JC ADDED:  Added this function to make one place for wme's being added to
 *   the GDS.  Callback for wme added to GDS is made here.
 */
void add_wme_to_gds(agent* thisAgent, goal_dependency_set* gds, wme* wme_to_add)
{
    /* Set the correct GDS for this wme (wme's point to their gds) */
    wme_to_add->gds = gds;
    insert_at_head_of_dll(gds->wmes_in_gds, wme_to_add, gds_next, gds_prev);

    if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_GDS_SYSPARAM])
    {
        // BADBAD: the XML code makes this all very ugly
        char msgbuf[256];
        memset(msgbuf, 0, 256);
        snprintf_with_symbols(thisAgent, msgbuf, 255, "Adding to GDS for %y: ", wme_to_add->gds->goal);
        print_string(thisAgent,  msgbuf);

        xml_begin_tag(thisAgent, kTagVerbose);
        xml_att_val(thisAgent, kTypeString, msgbuf);
        print_wme(thisAgent, wme_to_add); // prints XML, too
        xml_end_tag(thisAgent, kTagVerbose);
    }
}

/*
========================

========================
*/
void elaborate_gds(agent* thisAgent)
{

    wme* wme_matching_this_cond;
    goal_stack_level  wme_goal_level;
    preference* pref_for_this_wme, *pref;
    condition* cond;
    parent_inst* curr_pi, *temp_pi;
    slot* s;
    instantiation* inst;

    for (curr_pi = thisAgent->parent_list_head; curr_pi; curr_pi = temp_pi)
    {

        inst = curr_pi->inst;

#ifdef DEBUG_GDS
        print_with_symbols(thisAgent, "\n      EXPLORING INSTANTIATION: %y\n", curr_pi->inst->prod->name);
        print(thisAgent,  "      ");
        print_instantiation_with_wmes(thisAgent, curr_pi->inst , TIMETAG_WME_TRACE, -1);
#endif

        for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        {

            if (cond->type != POSITIVE_CONDITION)
            {
                continue;
            }

            /* We'll deal with negative instantiations after we get the
            * positive ones figured out */

            wme_matching_this_cond = cond->bt.wme_;
            wme_goal_level         = cond->bt.level;
            pref_for_this_wme      = wme_matching_this_cond->preference;

#ifdef DEBUG_GDS
            print(thisAgent,  "\n       wme_matching_this_cond at goal_level = %d : ",
                  wme_goal_level);
            print_wme(thisAgent, wme_matching_this_cond);

            if (pref_for_this_wme)
            {
                print(thisAgent,  "       pref_for_this_wme                        : ");
                print_preference(thisAgent, pref_for_this_wme);
            }
#endif


            /* WME is in a supergoal or is arch-supported WME
            *  (except for fake instantiations, which do have prefs, so
            *  they get handled under "wme is local and i-supported")
            */
            if ((pref_for_this_wme == NIL) ||
                    (wme_goal_level < inst->match_goal_level))
            {

#ifdef DEBUG_GDS
                if (pref_for_this_wme == NIL)
                {
                    print(thisAgent,  "         this wme has no preferences (it's an arch-created wme)\n");
                }
                else if (wme_goal_level < inst->match_goal_level)
                {
                    print(thisAgent,  "         this wme is in the supergoal\n");
                }
                print_with_symbols(thisAgent, "inst->match_goal [%y]\n" , inst->match_goal);
#endif

                if (wme_matching_this_cond->gds != NIL)
                {
                    /* Then we want to check and see if the old GDS value
                    * should be changed */
                    if (wme_matching_this_cond->gds->goal == NIL)
                    {
                        /* The goal is NIL: meaning that the goal for the GDS
                        * is no longer around */
                        fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds, \
                                             wme_matching_this_cond, wme,
                                             gds_next, gds_prev);

                        /* We have to check for GDS removal anytime we take a
                        * WME off the GDS wme list, not just when a WME is
                        * removed from memory. */
                        if (!wme_matching_this_cond->gds->wmes_in_gds)
                        {
                            if (wme_matching_this_cond->gds->goal)
                            {
                                wme_matching_this_cond->gds->goal->id->gds = NIL;
                            }
                            free_with_pool(&(thisAgent->gds_pool), wme_matching_this_cond->gds);

#ifdef DEBUG_GDS
                            print(thisAgent,  "\n  REMOVING GDS FROM MEMORY.");
#endif
                        }

                        /* JC ADDED: Separate adding wme to GDS as a function */
                        add_wme_to_gds(thisAgent, inst->match_goal->id->gds, wme_matching_this_cond);

                        //                  wme_matching_this_cond->gds = inst->match_goal->id->gds;
                        //                  insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
                        //                     wme_matching_this_cond, gds_next,
                        //                     gds_prev);
#ifdef DEBUG_GDS
                        print(thisAgent,  "\n       .....GDS' goal is NIL so switching from old to new GDS list....\n");
#endif

                    }
                    else if (wme_matching_this_cond->gds->goal->id->level >
                             inst->match_goal_level)
                    {
                        /* if the WME currently belongs to the GDS of a goal below
                        * the current one */
                        /* 1. Take WME off old (current) GDS list
                        * 2. Check to see if old GDS WME list is empty.  If so,
                        *         remove(free) it.
                        * 3. Add WME to new GDS list
                        * 4. Update WME pointer to new GDS list
                        */
                        if (inst->match_goal_level == 1)
                        {
                            print(thisAgent,  "\n\n\n HELLO! HELLO! The inst->match_goal_level is 1");
                        }

                        fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds, \
                                             wme_matching_this_cond, wme,
                                             gds_next, gds_prev);
                        if (!wme_matching_this_cond->gds->wmes_in_gds)
                        {
                            if (wme_matching_this_cond->gds->goal)
                            {
                                wme_matching_this_cond->gds->goal->id->gds = NIL;
                            }
                            free_with_pool(&(thisAgent->gds_pool), wme_matching_this_cond->gds);

#ifdef DEBUG_GDS
                            print(thisAgent,  "\n  REMOVING GDS FROM MEMORY.");
#endif
                        }
                        /* JC ADDED: Separate adding wme to GDS as a function */
                        add_wme_to_gds(thisAgent, inst->match_goal->id->gds, wme_matching_this_cond);

                        //                  wme_matching_this_cond->gds = inst->match_goal->id->gds;
                        //                  insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
                        //                     wme_matching_this_cond, gds_next,
                        //                     gds_prev);
#ifdef DEBUG_GDS
                        print(thisAgent,  "\n       ....switching from old to new GDS list....\n");
#endif
                        wme_matching_this_cond->gds = inst->match_goal->id->gds;
                    }
                }
                else
                {
                    /* We know that the WME should be in the GDS of the current
                    * goal if the WME's GDS does not already exist.
                    * (i.e., if NIL GDS) */

                    /* JC ADDED: Separate adding wme to GDS as a function */
                    add_wme_to_gds(thisAgent, inst->match_goal->id->gds, wme_matching_this_cond);

                    //               wme_matching_this_cond->gds = inst->match_goal->id->gds;
                    //               insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
                    //                  wme_matching_this_cond, gds_next, gds_prev);

                    if (wme_matching_this_cond->gds->wmes_in_gds->gds_prev)
                    {
                        print(thisAgent,  "\nDEBUG DEBUG : The new header should never have a prev value.\n");
                    }
#ifdef DEBUG_GDS
                    print_with_symbols(thisAgent, "\n       ......WME did not have defined GDS.  Now adding to goal [%y].\n", wme_matching_this_cond->gds->goal);
#endif
                } /* end else clause for "if wme_matching_this_cond->gds != NIL" */


#ifdef DEBUG_GDS
                print(thisAgent,  "            Added WME to GDS for goal = %d",
                      wme_matching_this_cond->gds->goal->id->level);
                print_with_symbols(thisAgent, " [%y]\n", wme_matching_this_cond->gds->goal);
#endif
            } /* end "wme in supergoal or arch-supported" */
            else
            {
                /* wme must be local */

                /* if wme's pref is o-supported, then just ignore it and
                * move to next condition */
                if (pref_for_this_wme->o_supported == true)
                {
#ifdef DEBUG_GDS
                    print(thisAgent,  "         this wme is local and o-supported\n");
#endif
                    continue;
                }

                else
                {
                    /* wme's pref is i-supported, so remember it's instantiation
                    * for later examination */

                    /* this test avoids "backtracing" through the top state */
                    if (inst->match_goal_level == 1)
                    {
#ifdef DEBUG_GDS
                        print(thisAgent,  "         don't back up through top state\n");
                        if (inst->prod)
                            if (inst->prod->name)
                            {
                                print_with_symbols(thisAgent, "         don't back up through top state for instantiation %y\n", inst->prod->name);
                            }
#endif
                        continue;
                    }

                    else   /* (inst->match_goal_level != 1) */
                    {
#ifdef DEBUG_GDS
                        print(thisAgent,  "         this wme is local and i-supported\n");
#endif
                        s = find_slot(pref_for_this_wme->id, pref_for_this_wme->attr);
                        if (s == NIL)
                        {
                            /* this must be an arch-wme from a fake instantiation */

#ifdef DEBUG_GDS
                            print(thisAgent,  "here's the wme with no slot:\t");
                            print_wme(thisAgent, pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme_);
#endif

                            /* this is the same code as above, just using the
                            * differently-named pointer.  it probably should
                            * be a subroutine */
                            {
                                wme* fake_inst_wme_cond;

                                fake_inst_wme_cond = pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme_;
                                if (fake_inst_wme_cond->gds != NIL)
                                {
                                    /* Then we want to check and see if the old GDS
                                    * value should be changed */
                                    if (fake_inst_wme_cond->gds->goal == NIL)
                                    {
                                        /* The goal is NIL: meaning that the goal for
                                        * the GDS is no longer around */

                                        fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds,
                                                             fake_inst_wme_cond, wme,
                                                             gds_next, gds_prev);

                                        /* We have to check for GDS removal anytime we take
                                        * a WME off the GDS wme list, not just when a WME
                                        * is removed from memory. */
                                        if (!fake_inst_wme_cond->gds->wmes_in_gds)
                                        {
                                            if (fake_inst_wme_cond->gds->goal)
                                            {
                                                fake_inst_wme_cond->gds->goal->id->gds = NIL;
                                            }
                                            free_with_pool(&(thisAgent->gds_pool), fake_inst_wme_cond->gds);

#ifdef DEBUG_GDS
                                            print(thisAgent,  "\n  REMOVING GDS FROM MEMORY.");
#endif
                                        }

                                        /* JC ADDED: Separate adding wme to GDS as a function */
                                        add_wme_to_gds(thisAgent, inst->match_goal->id->gds, fake_inst_wme_cond);

                                        //                                 fake_inst_wme_cond->gds = inst->match_goal->id->gds;
                                        //                                 insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds,
                                        //                                                       fake_inst_wme_cond, gds_next, gds_prev);
#ifdef DEBUG_GDS
                                        print(thisAgent,  "\n       .....GDS' goal is NIL so switching from old to new GDS list....\n");
#endif
                                    }
                                    else if (fake_inst_wme_cond->gds->goal->id->level > inst->match_goal_level)
                                    {
                                        /* if the WME currently belongs to the GDS of a
                                        *goal below the current one */
                                        /* 1. Take WME off old (current) GDS list
                                        * 2. Check to see if old GDS WME list is empty.
                                        *    If so, remove(free) it.
                                        * 3. Add WME to new GDS list
                                        * 4. Update WME pointer to new GDS list
                                        */
                                        if (inst->match_goal_level == 1)
                                        {
                                            print(thisAgent,  "\n\n\n\n\n HELLO! HELLO! The inst->match_goal_level is 1");
                                        }

                                        fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds, \
                                                             fake_inst_wme_cond, wme,
                                                             gds_next, gds_prev);
                                        if (!fake_inst_wme_cond->gds->wmes_in_gds)
                                        {
                                            if (fake_inst_wme_cond->gds->goal)
                                            {
                                                fake_inst_wme_cond->gds->goal->id->gds = NIL;
                                            }
                                            free_with_pool(&(thisAgent->gds_pool), fake_inst_wme_cond->gds);

#ifdef DEBUG_GDS
                                            print(thisAgent,  "\n  REMOVING GDS FROM MEMORY.");
#endif
                                        }

                                        /* JC ADDED: Separate adding wme to GDS as a function */
                                        add_wme_to_gds(thisAgent, inst->match_goal->id->gds, fake_inst_wme_cond);

                                        //                                 fake_inst_wme_cond->gds = inst->match_goal->id->gds;
                                        //                                 insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds,
                                        //                                    fake_inst_wme_cond, gds_next,
                                        //                                    gds_prev);
#ifdef DEBUG_GDS
                                        print(thisAgent,  "\n       .....switching from old to new GDS list....\n");
#endif
                                        fake_inst_wme_cond->gds = inst->match_goal->id->gds;
                                    }
                                }
                                else
                                {
                                    /* We know that the WME should be in the GDS of
                                    * the current goal if the WME's GDS does not
                                    * already exist. (i.e., if NIL GDS) */

                                    /* JC ADDED: Separate adding wme to GDS as a function */
                                    add_wme_to_gds(thisAgent, inst->match_goal->id->gds, fake_inst_wme_cond);

                                    //                              fake_inst_wme_cond->gds = inst->match_goal->id->gds;
                                    //                              insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds,
                                    //                                                    fake_inst_wme_cond,
                                    //                                                    gds_next, gds_prev);

                                    if (fake_inst_wme_cond->gds->wmes_in_gds->gds_prev)
                                    {
                                        print(thisAgent,  "\nDEBUG DEBUG : The new header should never have a prev value.\n");
                                    }
#ifdef DEBUG_GDS
                                    print_with_symbols(thisAgent, "\n       ......WME did not have defined GDS.  Now adding to goal [%y].\n", fake_inst_wme_cond->gds->goal);
#endif
                                }
#ifdef DEBUG_GDS
                                print(thisAgent,  "            Added WME to GDS for goal = %d", fake_inst_wme_cond->gds->goal->id->level);
                                print_with_symbols(thisAgent, " [%y]\n",
                                                   fake_inst_wme_cond->gds->goal);
#endif
                            }  /* matches { wme *fake_inst_wme_cond  */
                        }
                        else
                        {
                            /* this was the original "local & i-supported" action */
                            for (pref = s->preferences[ACCEPTABLE_PREFERENCE_TYPE];
                                    pref; pref = pref->next)
                            {

#ifdef DEBUG_GDS
                                print(thisAgent,  "           looking at pref for the wme: ");
                                print_preference(thisAgent, pref);
#endif


                                /* REW: 2004-05-27: Bug fix
                                   We must check that the value with acceptable pref for the slot
                                   is the same as the value for the wme in the condition, since
                                   operators can have acceptable preferences for values other than
                                   the WME value.  We dont want to backtrack thru acceptable prefs
                                   for other operators */

                                if (pref->value == wme_matching_this_cond->value)
                                {


                                    /* REW BUG: may have to go over all insts regardless
                                    * of this visited_already flag... */

                                    if (pref->inst->GDS_evaluated_already == false)
                                    {

#ifdef DEBUG_GDS
                                        print_with_symbols(thisAgent, "\n           adding inst that produced the pref to GDS: %y\n", pref->inst->prod->name);
#endif
                                        //////////////////////////////////////////////////////
                                        /* REW: 2003-12-07 */
                                        /* If the preference comes from a lower level inst, then
                                        ignore it. */
                                        /* Preferences from lower levels must come from result
                                        instantiations;
                                        we just want to use the justification/chunk
                                        instantiations at the match goal level*/
                                        if (pref->inst->match_goal_level <= inst->match_goal_level)
                                        {



                                            //////////////////////////////////////////////////////
                                            uniquely_add_to_head_of_dll(thisAgent, pref->inst);
                                            pref->inst->GDS_evaluated_already = true;
                                            //////////////////////////////////////////////////////
                                        }
#ifdef DEBUG_GDS
                                        else
                                        {
                                            print_with_symbols(thisAgent, "\n           ignoring inst %y because it is at a lower level than the GDS\n", pref->inst->prod->name);
                                            pref->inst->GDS_evaluated_already = true;
                                        }
#endif
                                        /* REW: 2003-12-07 */

                                        //////////////////////////////////////////////////////
                                    }
#ifdef DEBUG_GDS
                                    else
                                    {
                                        print(thisAgent,  "           the inst producing this pref was already explored; skipping it\n");
                                    }
#endif

                                }
#ifdef DEBUG_GDS
                                else
                                {
                                    print(thisAgent,  "        this inst is for a pref with a differnt value than the condition WME; skippint it\n");
                                }
#endif
                            }  /* for pref = s->pref[ACCEPTABLE_PREF ...*/
                        }
                    }
                }
            }
        }  /* for (cond = inst->top_of_instantiated_cond ...  *;*/


        /* remove just used instantiation from list */

#ifdef DEBUG_GDS
        print_with_symbols(thisAgent, "\n      removing instantiation: %y\n",
                           curr_pi->inst->prod->name);
#endif

        if (curr_pi->next != NIL)
        {
            curr_pi->next->prev = curr_pi->prev;
        }

        if (curr_pi->prev != NIL)
        {
            curr_pi->prev->next = curr_pi->next;
        }

        if (thisAgent->parent_list_head == curr_pi)
        {
            thisAgent->parent_list_head = curr_pi->next;
        }

        temp_pi = curr_pi->next;
        free(curr_pi);

    } /* end of "for (curr_pi = thisAgent->parent_list_head ... */


    if (thisAgent->parent_list_head != NIL)
    {

#ifdef DEBUG_GDS
        print(thisAgent,  "\n    RECURSING using these parents:\n");
        for (curr_pi = thisAgent->parent_list_head;
                curr_pi;
                curr_pi = curr_pi->next)
        {
            print_with_symbols(thisAgent, "      %y\n", curr_pi->inst->prod->name);
        }
#endif

        /* recursively explore the parents of all the instantiations */

        elaborate_gds(thisAgent);

        /* free the parent instantiation list.  technically, the list
        * should be empty at this point ??? */
        free_parent_list(thisAgent);
    }

} /* end of elaborate_gds   */




/* REW BUG: this needs to be smarter to deal with wmes that get support from
multiple instantiations.  for example ^enemy-out-there could be made by 50
instantiations.  if one of those instantiations goes, should the goal be
killed????  This routine says "yes" -- anytime a dependent item gets changed,
we're gonna yank out the goal -- even when that i-supported element itself
may not be removed (due to multiple preferences).  So, we'll say that this is
a "twitchy" version of OPERAND2, and leave open the possibility that other
approaches may be better */

void gds_invalid_so_remove_goal(agent* thisAgent, wme* w)
{

    if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_GDS_SYSPARAM])
    {
        // BADBAD: the XML code makes this all very ugly
        char msgbuf[256];
        memset(msgbuf, 0, 256);
        snprintf_with_symbols(thisAgent, msgbuf, 255, "Removing state %y because element in GDS changed. WME: ", w->gds->goal);
        print_string(thisAgent, msgbuf);

        xml_begin_tag(thisAgent, soar_TraceNames::kTagVerbose);
        xml_att_val(thisAgent, soar_TraceNames::kTypeString, msgbuf);
        print_wme(thisAgent, w); // prints XML, too
        xml_end_tag(thisAgent, soar_TraceNames::kTagVerbose);
    }

    /* REW: begin 11.25.96 */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    thisAgent->timers_gds.start();
#endif
#endif
    /* REW: end   11.25.96 */

    /* This call to GDS_PrintCmd will have to be uncommented later. -ajc */
    //if (thisAgent->soar_verbose_flag) {} //GDS_PrintCmd();

    /* REW: BUG.  I have no idea right now if this is a terrible hack or
    * actually what we want to do.  The idea here is that the context of
    * the immediately higher goal above a retraction should be marked as
    * having its context changed in order that the architecture doesn't
    * look below this level for context changes.  I think it's a hack b/c
    * it seems like there should aready be mechanisms for doing this in
    * the architecture but I couldn't find any.
    */
    /* Note: the inner 'if' is correct -- we only want to change
    * highest_goal_whose_context_changed if the pointer is currently at
    * or below (greater than) the goal which we are going to retract.
    * However, I'm not so sure about the outer 'else.'  If we don't set
    * this to the goal above the retraction, even if the current value
    * is NIL, we still seg fault in certain cases.  But setting it as we do
    * in the inner 'if' seems to clear up the difficulty.
    */

    if (thisAgent->highest_goal_whose_context_changed)
    {
        if (thisAgent->highest_goal_whose_context_changed->id->level >= w->gds->goal->id->level)
        {
            thisAgent->highest_goal_whose_context_changed = w->gds->goal->id->higher_goal;
        }
    }
    else
    {
        /* If nothing has yet changed (highest_ ... = NIL) then set
        * the goal automatically */
        thisAgent->highest_goal_whose_context_changed = w->gds->goal->id->higher_goal;

        // Tell those slots they are changed so that the impasses can be regenerated
        // bug 1011
        for (slot* s = thisAgent->highest_goal_whose_context_changed->id->slots; s != 0; s = s->next)
        {
            if (s->isa_context_slot && !s->changed)
            {
                s->changed = reinterpret_cast<dl_cons*>(1);    // use non-zero value to indicate change, see definition of slot::changed
            }
        }
    }

    if (thisAgent->sysparams[TRACE_OPERAND2_REMOVALS_SYSPARAM])
    {
        print_with_symbols(thisAgent, "\n    REMOVING GOAL [%y] due to change in GDS WME ", w->gds->goal);
        print_wme(thisAgent, w);
    }

    remove_existing_context_and_descendents(thisAgent, w->gds->goal);

    /* BUG: Need to reset highest_goal here ???*/

    /* usually, we'd call do_buffered_wm_and_ownership_changes() here, but
    * we don't need to because it will be done at the end of the working
    * memory phase; cf. the end of do_working_memory_phase().
    */

    /* REW: begin 11.25.96 */
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    thisAgent->timers_gds.stop();
    thisAgent->timers_gds_cpu_time[thisAgent->current_phase].update(thisAgent->timers_gds);
#endif
#endif
    /* REW: end   11.25.96 */
}


void free_parent_list(agent* thisAgent)
{
    parent_inst* curr_pi;

    for (curr_pi = thisAgent->parent_list_head;
            curr_pi;
            curr_pi = curr_pi->next)
    {
        free(curr_pi);
    }

    thisAgent->parent_list_head = NIL;
}

void create_gds_for_goal(agent* thisAgent, Symbol* goal)
{
    goal_dependency_set* gds;

    allocate_with_pool(thisAgent, &(thisAgent->gds_pool), &gds);

    gds->goal = goal;
    gds->wmes_in_gds = NIL;
    goal->id->gds = gds;
#ifdef DEBUG_GDS
    print_with_symbols(thisAgent, "\nCreated GDS for goal [%y].\n", gds->goal);
#endif
}
