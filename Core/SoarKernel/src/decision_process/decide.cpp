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
#include "decider.h"

#include "agent.h"
#include "condition.h"
#include "consistency.h"
#include "debug_inventories.h"
#include "decision_manipulation.h"
#include "dprint.h"
#include "ebc.h"
#include "episodic_memory.h"
#include "exploration.h"
#include "instantiation.h"
#include "io_link.h"
#include "mem.h"
#include "misc.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "reinforcement_learning.h"
#include "rete.h"
#include "rhs.h"
#include "run_soar.h"
#include "semantic_memory.h"
#include "slot.h"
#include "smem_structs.h"
#include "soar_module.h"
#include "soar_rand.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "test.h"
#include "trace.h"
#include "working_memory_activation.h"
#include "working_memory.h"
#include "xml.h"

#ifndef NO_SVS
#include "svs_interface.h"
#endif

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <list>

using namespace soar_TraceNames;

void print_candidates(agent* thisAgent, preference* candidates)
{
    preference* cand = 0;
    int max_count = 0;

    for (cand = candidates; cand != NIL; cand = cand->next_candidate)
    {
        max_count++;
        thisAgent->outputManager->printa_sf(thisAgent, "\n Candidate %d", cand);
        thisAgent->outputManager->printa_sf(thisAgent, "\n    %y %y %y", cand->id, cand->attr, cand->value);
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
#define UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG 8

Symbol* find_goal_at_goal_stack_level(agent* thisAgent, goal_stack_level level)
{
    Symbol* g;

    for (g = thisAgent->top_goal; g != NIL; g = g->id->lower_goal)
        if (g->id->level == level)
        {
            return (g);
        }
    return (NIL);
}

Symbol* find_impasse_wme_value(Symbol* id, Symbol* attr)
{
    wme* w;

    for (w = id->id->impasse_wmes; w != NIL; w = w->next)
        if (w->attr == attr)
        {
            return w->value;
        }
    return NIL;
}

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
    thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
    dc->item = s;
    s->acceptable_preference_changed = dc;
    insert_at_head_of_dll(thisAgent->context_slots_with_changed_accept_prefs, dc, next, prev);
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
            /* IF we lose an acceptable preference for an operator, then that
            operator comes out of the slot immediately in OPERAND2.
            However, if the lost acceptable preference is not for item
            in the slot, then we don;t need to do anything special until
            mini-quiescence. */
            remove_operator_if_necessary(thisAgent, s, w);
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

    while (thisAgent->context_slots_with_changed_accept_prefs)
    {
        dc = thisAgent->context_slots_with_changed_accept_prefs;
        thisAgent->context_slots_with_changed_accept_prefs = dc->next;
        s = static_cast<slot_struct*>(dc->item);
        thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
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

//    dprint(DT_LINKS, "Adding %slink %s%y to %y (%d to %d) (link count=%u)",
//        (from ? "" : "special "), (from ? "from " : ""), from, to,
//        (from ? from->id->level : 0), to->id->level, to->id->link_count);

    if (!from)
    {
        return;  /* if adding a special link, we're done */
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
    thisAgent->symbolManager->symbol_add_ref(to);
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
        thisAgent->symbolManager->symbol_remove_ref(&to);
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

//    dprint(DT_LINKS, "Removing %slink %s%y to %y (%d to %d) (link count=%u)",
//        (from ? "" : "special "), (from ? "from " : ""), from, to,
//        (from ? from->id->level : 0), to->id->level, to->id->link_count);

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
            //dprint(DT_UNKNOWN_LEVEL, "Removing %y from ids_with_unknown_level in post_link_removal() while adding to disconnected list.\n", to);
            remove_from_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            insert_at_head_of_dll(thisAgent->disconnected_ids, dc, next, prev);
            //dprint(DT_LINKS, "Disconnecting %y in do_demotion.\n", static_cast<Symbol *>(dc->item));
        }
        else
        {
            thisAgent->symbolManager->symbol_add_ref(to);
            thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
            dc->item = to;
            to->id->unknown_level = dc;
            //dprint(DT_UNKNOWN_LEVEL, "Setting %y as unknown_level in post_link_removal() while adding to disconnected list. (problem?)\n", to);
            insert_at_head_of_dll(thisAgent->disconnected_ids, dc, next, prev);
            //dprint(DT_LINKS, "Disconnecting %y in do_demotion.\n", to);
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
        thisAgent->symbolManager->symbol_add_ref(to);
        thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
        dc->item = to;
        to->id->unknown_level = dc;
//        dprint(DT_UNKNOWN_LEVEL,
//            "Setting %y as unknown_level in post_link_removal() because identifiers in link at "
//            "same level: %s%y (lvl %d) --/--> %y (lvl %d)\n", to,
//            (from ? "" : "special "), from, (from ? from->id->level : 0), to, to->id->level);
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

    //dprint(DT_LINKS, "*** Garbage collecting id: %y", id);

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

void mark_id_and_tc_as_unknown_level(agent* thisAgent, Symbol* root)
{
    slot* s;
    preference* pref;
    wme* w;
    dl_cons* dc;

    Symbol* id;
    symbol_list ids_to_walk;

    //dprint(DT_UNKNOWN_LEVEL, "mark_id_and_tc_as_unknown_level called on %y.  (mark tc = %u)", root, thisAgent->mark_tc_number);
    ids_to_walk.push_back(root);

    while (!ids_to_walk.empty())
    {
        id = ids_to_walk.back();
        ids_to_walk.pop_back();

        //dprint(DT_UNKNOWN_LEVEL, "   Processing %y (level = %d, tc_num = %u)...\n", id, id->id->level, id->tc_num);

        /* --- if id is already marked, do nothing --- */
        if (id->tc_num == thisAgent->mark_tc_number)
        {
            //dprint(DT_UNKNOWN_LEVEL, "      Skipping because already marked %d.\n", thisAgent->mark_tc_number);
            continue;
        }

        /* --- don't mark anything higher up as disconnected--in order to be higher
           up, it must have a link to it up there --- */
        if (id->id->level < thisAgent->level_at_which_marking_started)
        {
            //dprint(DT_UNKNOWN_LEVEL, "      Skipping because level is greater than %d( level_at_which_marking_started).\n", thisAgent->level_at_which_marking_started);
            continue;
        }

        /* --- mark id, so we won't do it again later --- */
        //dprint(DT_UNKNOWN_LEVEL, "      Marking %y with tc %d.\n", id, thisAgent->mark_tc_number);
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
        //dprint(DT_UNKNOWN_LEVEL, "      Highest level = %d, Lowest level = %d\n", thisAgent->highest_level_anything_could_fall_from, thisAgent->lowest_level_anything_could_fall_to);

        /* --- add id to the set of ids with unknown level --- */
        if (! id->id->unknown_level)
        {
            thisAgent->memoryManager->allocate_with_pool(MP_dl_cons, &dc);
            dc->item = id;
            id->id->unknown_level = dc;
            //dprint(DT_UNKNOWN_LEVEL, "      Setting %y as unknown_level and adding to list ids_with_unknown_level.\n", id);
            insert_at_head_of_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            thisAgent->symbolManager->symbol_add_ref(id);
        } else {
            //dprint(DT_UNKNOWN_LEVEL, "      Not setting %y as unknown_level because already set.\n", id);
        }

        /* -- scan through all preferences and wmes for all slots for this id -- */
        //dprint(DT_UNKNOWN_LEVEL, "      Adding IDs from input wme's to walk list:");
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            if (w->value->is_sti())
            {
                //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", w->value);
                ids_to_walk.push_back(w->value);
            }
        }
        //dprint_noprefix(DT_UNKNOWN_LEVEL, "\n");

        //dprint(DT_UNKNOWN_LEVEL, "      Adding IDs from involved slots to walk list:");
        for (s = id->id->slots; s != NIL; s = s->next)
        {
            for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
            {
                if (pref->value->is_sti())
                {
                    //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", pref->value);
                    ids_to_walk.push_back(pref->value);
                }

                if (preference_is_binary(pref->type))
                {
                    if (pref->referent->is_sti())
                    {
                        //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", pref->referent);
                        ids_to_walk.push_back(pref->referent);
                    }
                }
            }

            if (s->impasse_id)
            {
                if (s->impasse_id->is_sti())
                {
                    //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", s->impasse_id);
                    ids_to_walk.push_back(s->impasse_id);
                }
            }

            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (w->value->is_sti())
                {
                    //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", w->value);
                    ids_to_walk.push_back(w->value);
                }
            }
        } /* end of for slots loop */
        //dprint_noprefix(DT_UNKNOWN_LEVEL, "\n");
        //dprint(DT_UNKNOWN_LEVEL, "   Done processing %y (level = %d, tc_num = %u)...\n", id, id->id->level, id->tc_num);
    }
    //dprint(DT_UNKNOWN_LEVEL, "mark_id_and_tc_as_unknown_level DONE for %y.\n", root);
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

    //dprint(DT_UNKNOWN_LEVEL, "walk_and_update_levels called for %y.\n", root);

    symbol_list ids_to_walk;
    ids_to_walk.push_back(root);

    while (!ids_to_walk.empty())
    {
        id = ids_to_walk.back();
        ids_to_walk.pop_back();

        /* --- mark id so we don't walk it twice --- */
        id->tc_num = thisAgent->walk_tc_number;
//        dprint(DT_UNKNOWN_LEVEL, "   processing %y.  level = %d, walk_level = %d, walk tc = %u\n",
//            id, id->id->level, thisAgent->walk_level, thisAgent->walk_tc_number);

        /* --- if we already know its level, and it's higher up, then exit --- */
        if ((! id->id->unknown_level) && (id->id->level < thisAgent->walk_level))
        {
            //dprint(DT_UNKNOWN_LEVEL, "...skipping because symbol not unknown level, and level is higher than walk level %y.\n", root);
            continue;
        }

        /* --- if we didn't know its level before, we do now --- */
        if (id->id->unknown_level)
        {
            //dprint(DT_UNKNOWN_LEVEL, "   ...removing unknown level for %y and removing from ids_with unknown level\n", id);
            //dprint(DT_UNKNOWN_LEVEL, "   ...setting level, promotion_level to %d\n", thisAgent->walk_level);
            dc = id->id->unknown_level;
            remove_from_dll(thisAgent->ids_with_unknown_level, dc, next, prev);
            thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
            thisAgent->symbolManager->symbol_remove_ref(&id);
            id->id->unknown_level = NIL;
            id->id->level = thisAgent->walk_level;
            id->id->promotion_level = thisAgent->walk_level;
        }

        /* -- scan through all preferences and wmes for all slots for this id -- */
        //dprint(DT_UNKNOWN_LEVEL, "      Adding IDs from input wme's to walk list:");
        for (w = id->id->input_wmes; w != NIL; w = w->next)
        {
            if (level_update_needed(thisAgent, w->value))
            {
                //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", w->value);
                ids_to_walk.push_back(w->value);
            }
        }
        //dprint_noprefix(DT_UNKNOWN_LEVEL, "\n");

        //dprint(DT_UNKNOWN_LEVEL, "      Adding IDs from slots to walk list:");
        for (s = id->id->slots; s != NIL; s = s->next)
        {
            for (pref = s->all_preferences; pref != NIL; pref = pref->all_of_slot_next)
            {
                if (level_update_needed(thisAgent, pref->value))
                {
                    //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", pref->value);
                    ids_to_walk.push_back(pref->value);

                    if (preference_is_binary(pref->type))
                    {
                        if (level_update_needed(thisAgent, pref->referent))
                        {
                            //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", pref->referent);
                            ids_to_walk.push_back(pref->referent);
                        }
                    }
                }
            }

            if (s->impasse_id)
            {
                if (level_update_needed(thisAgent, s->impasse_id))
                {
                    //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", s->impasse_id);
                    ids_to_walk.push_back(s->impasse_id);
                }
            }

            for (w = s->wmes; w != NIL; w = w->next)
            {
                if (level_update_needed(thisAgent, w->value))
                {
                    //dprint_noprefix(DT_UNKNOWN_LEVEL, " %y", w->value);
                    ids_to_walk.push_back(w->value);
                }
            }
        }
        //dprint_noprefix(DT_UNKNOWN_LEVEL, "\n");
    }
    //dprint(DT_UNKNOWN_LEVEL, "walk_and_update_levels DONE for %y.\n", root);
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
            //dprint(DT_LINKS, "Disconnecting %y in do_demotion.\n", static_cast<Symbol *>(dc->item));
        }
    }

    /* --- keep garbage collecting ids until nothing left to gc --- */
    thisAgent->link_update_mode = UPDATE_DISCONNECTED_IDS_LIST;
    while (thisAgent->disconnected_ids)
    {
        dc = thisAgent->disconnected_ids;
        thisAgent->disconnected_ids = thisAgent->disconnected_ids->next;
        id = static_cast<symbol_struct*>(dc->item);
        thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
        id->id->unknown_level = NIL;
        garbage_collect_id(thisAgent, id);
        thisAgent->symbolManager->symbol_remove_ref(&id);
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
        thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
        id->id->unknown_level = NIL;    /* AGR 640:  GAP set to NIL because */
        /* symbol may still have pointers to it */
        garbage_collect_id(thisAgent, id);
        thisAgent->symbolManager->symbol_remove_ref(&id);
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
    local_timer.set_enabled(&(thisAgent->trace_settings[ TIMERS_ENABLED ]));
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


/** Build our RL trace. -bazald **/

void build_rl_trace(agent* const& thisAgent, preference* const& candidates, preference* const& selected)   ///< bazald
{
    if (thisAgent->RL->rl_params->trace->get_value() == off)
    {
        return;
    }

    RL_Trace** next = NIL;

    for (preference* cand = candidates; cand; cand = cand->next_candidate)
    {
        if (cand->inst && cand->inst->prod)
        {
//       std::cerr << "rl-trace: " << cand->inst->prod_name->sc->name << std::endl;

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

            const double probability = cand->rl_contribution
                                       ? exploration_probability_according_to_policy(thisAgent, candidates->slot, candidates, cand)
                                       : nan("");

//       std::cerr << "rl-trace: =" << probability << std::endl;

            RL_Trace* const rl_trace = static_cast<RL_Trace*>(candidates->slot->id->id->rl_trace);
            rl_trace->split[index_str].init = thisAgent->RL->rl_init_count;
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
            *next = new RL_Trace;
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
   the preferences for a given slot, and returns an impasse type for the
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
    bool match_found, not_all_indifferent, some_numeric, add_OSK, some_not_worst = false;
    preference* candidates;
    Symbol* value;

    /* Set a flag to determine if a context-dependent preference set makes sense in this context.
     * We can ignore OSK prefs when:
     * - Run_preference_semantics is called for a consistency check (don't want side effects)
     * - For non-context slots (only makes sense for operators)
     * - For context-slots at the top level (will never be backtraced through)
     * - when the learning system parameter is set off (note, this is independent of whether learning is on) */

    add_OSK = (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ADD_OSK] && s->isa_context_slot && !consistency && (s->id->id->level > TOP_GOAL_LEVEL));

    /* Empty the context-dependent preference set in the slot */

    if (add_OSK && s->OSK_prefs)
    {
        clear_preference_list(thisAgent, s->OSK_prefs);
    }

    /* If the slot has no preferences at all, things are trivial --- */

    if (!s->all_preferences)
    {
        if (!s->isa_context_slot)
        {
            mark_slot_for_possible_removal(thisAgent, s);
        }
        *result_candidates = NIL;
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, NULL);
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
                if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, force_result);
                return NONE_IMPASSE_TYPE;
            }
        }
    }

    /* If debugging a context-slot, print all preferences that we're deciding through */

//    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM] && s->isa_context_slot)
//    {
//
//        thisAgent->outputManager->printa_sf(thisAgent,
//              "\n-------------------------------\nRUNNING PREFERENCE SEMANTICS...\n-------------------------------\n");
//        thisAgent->outputManager->printa_sf(thisAgent, "All Preferences for slot:");
//
//        for (int i = 0; i < NUM_PREFERENCE_TYPES; i++)
//        {
//            if (s->preferences[i])
//            {
//                thisAgent->outputManager->printa_sf(thisAgent, "\n   %ss:\n", preference_name(i));
//                for (p = s->preferences[i]; p; p = p->next)
//                {
//                    thisAgent->outputManager->printa_sf(thisAgent, "   ");
//                    print_preference(thisAgent, p);
//                }
//            }
//        }
//        thisAgent->outputManager->printa_sf(thisAgent, "-------------------------------\n");
//    }

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
            if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, NULL);
            return CONSTRAINT_FAILURE_IMPASSE_TYPE;
        }

        /* Check if we have also have a prohibit preference. If so, return constraint failure.
         * Note that this is the one difference between prohibit and reject preferences. */

        value = candidates->value;
        for (p = s->preferences[PROHIBIT_PREFERENCE_TYPE]; p != NIL; p = p->next)
            if (p->value == value)
            {
                if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, NULL);
                return CONSTRAINT_FAILURE_IMPASSE_TYPE;
            }

        /* --- We have a winner, so update RL --- */

        rl_update_for_one_candidate(thisAgent, s, consistency, candidates);

        /* Print a message that we're adding the require preference to the OSK prefs
         * even though we really aren't.  Requires aren't actually handled by
         * the OSK prefs mechanism since they are already backtraced through. */

//        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
//        {
//            thisAgent->outputManager->printa_sf(thisAgent, "--> Adding preference to OSK prefs: ");
//            print_preference(thisAgent, candidates);
//        }
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, candidates);
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
     * add all reject and prohibit preferences to OSK prefs */

    if (add_OSK)
    {
        if (s->preferences[PROHIBIT_PREFERENCE_TYPE] || s->preferences[REJECT_PREFERENCE_TYPE])
        {
            for (p = s->preferences[PROHIBIT_PREFERENCE_TYPE]; p != NIL; p = p->next)
            {
                thisAgent->explanationBasedChunker->add_to_OSK(s, p);
            }
            for (p = s->preferences[REJECT_PREFERENCE_TYPE]; p != NIL; p = p->next)
            {
                thisAgent->explanationBasedChunker->add_to_OSK(s, p);
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
            if (add_OSK && s->OSK_prefs)
            {
                clear_preference_list(thisAgent, s->OSK_prefs);
            }
        }
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, candidates);

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
                if (j->decider_flag == CANDIDATE_DECIDER_FLAG || k->decider_flag == CANDIDATE_DECIDER_FLAG)
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
                if (j->decider_flag == CANDIDATE_DECIDER_FLAG || k->decider_flag == CANDIDATE_DECIDER_FLAG)
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
            if (add_OSK && s->OSK_prefs)
            {
                clear_preference_list(thisAgent, s->OSK_prefs);
            }
            if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, NULL);

            return CONFLICT_IMPASSE_TYPE;
        }

        /* Otherwise, delete conflicted candidates from candidate list.
         * Also add better preferences to OSK prefs for every item in the candidate
         * list and delete acceptable preferences from the OSK prefs for those that
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
                if (add_OSK)
                {
                    /* Add better/worse preferences to OSK prefs */
                    for (p = s->preferences[BETTER_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == cand->value)
                        {
                            thisAgent->explanationBasedChunker->add_to_OSK(s, p);
                        }
                    }
                    for (p = s->preferences[WORSE_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->referent == cand->value)
                        {
                            thisAgent->explanationBasedChunker->add_to_OSK(s, p);
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
            if (add_OSK && s->OSK_prefs)
            {
                clear_preference_list(thisAgent, s->OSK_prefs);
            }
        }
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, candidates);
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

        /* Reduce candidates list to only those with best preference flag and add pref to OSK prefs */
        prev_cand = NIL;
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
            if (cand->value->decider_flag == BEST_DECIDER_FLAG)
            {
                if (add_OSK)
                {
                    for (p = s->preferences[BEST_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == cand->value)
                        {
                            thisAgent->explanationBasedChunker->add_to_OSK(s, p);
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
            if (add_OSK && s->OSK_prefs)
            {
                clear_preference_list(thisAgent, s->OSK_prefs);
            }
        }
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, candidates);
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

        /* Because we only want to add worst preferences to the OSK prefs if they actually have an impact
        * on the candidate list, we must first see if there's at least one non-worst candidate. */

        if (add_OSK)
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
                if (add_OSK && some_not_worst)
                {
                    /* Add this worst preference to OSK prefs */
                    for (p = s->preferences[WORST_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == cand->value)
                        {
                            thisAgent->explanationBasedChunker->add_to_OSK(s, p);
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
            if (add_OSK && s->OSK_prefs)
            {
                clear_preference_list(thisAgent, s->OSK_prefs);
            }
        }
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, candidates);
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
            for (p2 = s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p2 != NIL; p2 = p2->next)
                if (((p2->value == cand->value) && (p2->referent == p->value)) ||
                    ((p2->value == p->value) && (p2->referent == cand->value)))
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

            if (add_OSK)
            {

                /* Add all indifferent preferences associated with the chosen candidate to the OSK prefs.*/

                if (some_numeric)
                {

                    /* Note that numeric indifferent preferences are never considered duplicates, so we
                    * pass an extra argument to add_to_OSK so that it does not check for duplicates.*/

                    for (p = s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if (p->value == (*result_candidates)->value)
                        {
                            thisAgent->explanationBasedChunker->add_to_OSK(s, p, false);
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
                                thisAgent->explanationBasedChunker->add_to_OSK(s, p);
                            }
                        }
                    }
                }
                else
                {

                    /* This decision was non-numeric, so add all non-numeric preferences associated with the
                     * chosen candidate to the OSK prefs.*/
                    /* MToDo | Temporarily removed because it was causing problems for John in demo agents.  All of the OSK
                     *         prefs that involve uncertainty now seem weird. Will need to reconsider how we handle them now
                     *         that we have a better handle for correctness issues and are thinking more about probabilistic
                     *         chunks.*/
//                    for (p = s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
//                    {
//                        if (p->value == (*result_candidates)->value)
//                        {
//                            thisAgent->explanationBasedChunker->add_to_OSK(s, p);
//                        }
//                    }
                    for (p = s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p != NIL; p = p->next)
                    {
                        if ((p->value == (*result_candidates)->value) || (p->referent == (*result_candidates)->value))
                        {
                            thisAgent->explanationBasedChunker->add_to_OSK(s, p);
                        }
                    }
                }
            }
        }
        else
        {
            *result_candidates = candidates;
        }
        if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, *result_candidates);

        return NONE_IMPASSE_TYPE;
    }

    /* Candidates are not all indifferent, so we have a tie. */

    *result_candidates = candidates;
    if (add_OSK && s->OSK_prefs)
    {
        clear_preference_list(thisAgent, s->OSK_prefs);
    }
    if (add_OSK) thisAgent->explanationBasedChunker->update_proposal_OSK(s, NULL);

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

void add_impasse_wme(agent* thisAgent, Symbol* id, Symbol* attr, Symbol* value, preference* p, bool isSingleton = false)
{
    wme* w;

    w = make_wme(thisAgent, id, attr, value, false);
    insert_at_head_of_dll(id->id->impasse_wmes, w, next, prev);
    w->preference = p;
    add_wme_to_wm(thisAgent, w);
    if (isSingleton)
    {
        thisAgent->explanationBasedChunker->add_to_singletons(w);
    }
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
    Symbol* impasseID;

    impasseID = thisAgent->symbolManager->make_new_identifier((isa_goal ? 'S' : 'I'), level, 0, false);
    post_link_addition(thisAgent, NIL, impasseID);   /* add the special link */

    add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.type_symbol, isa_goal ? thisAgent->symbolManager->soarSymbols.state_symbol : thisAgent->symbolManager->soarSymbols.impasse_symbol, NIL, true);

    if (isa_goal)
    {
        thisAgent->memoryManager->allocate_with_pool(MP_rl_info, &(impasseID->id->rl_info));
        thisAgent->memoryManager->allocate_with_pool(MP_smem_info, &(impasseID->id->smem_info));
        thisAgent->memoryManager->allocate_with_pool(MP_epmem_info, &(impasseID->id->epmem_info));

        add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.superstate_symbol, object, NIL, true);

        Symbol* lreward_header = thisAgent->symbolManager->make_new_identifier('R', level);
        impasseID->id->rl_info->rl_link_wme = soar_module::add_module_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.rl_sym_reward_link, lreward_header, true);

        Symbol* lepmem_header = thisAgent->symbolManager->make_new_identifier('E', level);
        impasseID->id->epmem_info->epmem_link_wme = soar_module::add_module_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.epmem_sym, lepmem_header, true);
        Symbol* lepmem_cmd_header = thisAgent->symbolManager->make_new_identifier('C', level);
        impasseID->id->epmem_info->cmd_wme = soar_module::add_module_wme(thisAgent, lepmem_header, thisAgent->symbolManager->soarSymbols.epmem_sym_cmd, lepmem_cmd_header);
        Symbol* lepmem_result_header = thisAgent->symbolManager->make_new_identifier('R', level);
        impasseID->id->epmem_info->result_wme = soar_module::add_module_wme(thisAgent, lepmem_header, thisAgent->symbolManager->soarSymbols.epmem_sym_result, lepmem_result_header);


        {
            int64_t my_time = static_cast<int64_t>(thisAgent->EpMem->epmem_stats->time->get_value());
            if (my_time == 0) my_time = 1;

            Symbol* my_time_sym = thisAgent->symbolManager->make_int_constant(my_time);
            impasseID->id->epmem_info->epmem_time_wme = soar_module::add_module_wme(thisAgent, lepmem_header, thisAgent->symbolManager->soarSymbols.epmem_sym_present_id, my_time_sym);
            thisAgent->symbolManager->symbol_remove_ref(&my_time_sym);
        }

        Symbol* lsmem_header = thisAgent->symbolManager->make_new_identifier('L', level);
        Symbol* lsmem_cmd_header = thisAgent->symbolManager->make_new_identifier('C', level);
        Symbol* lsmem_result_header = thisAgent->symbolManager->make_new_identifier('R', level);
        impasseID->id->smem_info->smem_link_wme = soar_module::add_module_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.smem_sym, lsmem_header, true);
        impasseID->id->smem_info->cmd_wme = soar_module::add_module_wme(thisAgent, lsmem_header, thisAgent->symbolManager->soarSymbols.smem_sym_cmd, lsmem_cmd_header);
        impasseID->id->smem_info->result_wme = soar_module::add_module_wme(thisAgent, lsmem_header, thisAgent->symbolManager->soarSymbols.smem_sym_result, lsmem_result_header);

        thisAgent->symbolManager->symbol_remove_ref(&lreward_header);
        thisAgent->symbolManager->symbol_remove_ref(&lepmem_header);
        thisAgent->symbolManager->symbol_remove_ref(&lepmem_cmd_header);
        thisAgent->symbolManager->symbol_remove_ref(&lepmem_result_header);
        thisAgent->symbolManager->symbol_remove_ref(&lsmem_header);
        thisAgent->symbolManager->symbol_remove_ref(&lsmem_cmd_header);
        thisAgent->symbolManager->symbol_remove_ref(&lsmem_result_header);

    }
    else
    {
        add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.object_symbol, object, NIL, true);
    }

    if (attr)
    {
        add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.attribute_symbol, attr, NIL, true);
    }

    switch (impasse_type)
    {
        case NONE_IMPASSE_TYPE:
            break;    /* this happens only when creating the top goal */
        case CONSTRAINT_FAILURE_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.impasse_symbol, thisAgent->symbolManager->soarSymbols.constraint_failure_symbol, NIL, true);
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.choices_symbol, thisAgent->symbolManager->soarSymbols.none_symbol, NIL);
            break;
        case CONFLICT_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.impasse_symbol, thisAgent->symbolManager->soarSymbols.conflict_symbol, NIL, true);
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.choices_symbol, thisAgent->symbolManager->soarSymbols.multiple_symbol, NIL);
            break;
        case TIE_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.impasse_symbol, thisAgent->symbolManager->soarSymbols.tie_symbol, NIL, true);
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.choices_symbol, thisAgent->symbolManager->soarSymbols.multiple_symbol, NIL);
            break;
        case NO_CHANGE_IMPASSE_TYPE:
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.impasse_symbol, thisAgent->symbolManager->soarSymbols.no_change_symbol, NIL, true);
            add_impasse_wme(thisAgent, impasseID, thisAgent->symbolManager->soarSymbols.choices_symbol, thisAgent->symbolManager->soarSymbols.none_symbol, NIL);
            break;
    }

//   if(thisAgent->rl_trace.find(level) == thisAgent->rl_trace.end())
//     std::cerr << "rl-trace: Init level " << level << std::endl;
//   else
//     std::cerr << "rl-trace: Restore level " << level << std::endl;
    impasseID->id->rl_trace = &thisAgent->RL->rl_trace[level];

    return impasseID;
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
    thisAgent->symbolManager->symbol_remove_ref(&id);
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
            loop_sym = thisAgent->symbolManager->soarSymbols.item_symbol;
            loop_count_sym = thisAgent->symbolManager->soarSymbols.item_count_symbol;
        }
        else
        {
            loop_sym = thisAgent->symbolManager->soarSymbols.non_numeric_symbol;
            loop_count_sym = thisAgent->symbolManager->soarSymbols.non_numeric_count_symbol;
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
                        /* Remove fake preference for goal item */
                        preference_remove_ref(thisAgent, w->preference);
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
                bt_pref = make_architectural_instantiation_for_impasse_item(thisAgent, id, cand);
            }
            else
            {
                bt_pref = cand;
            }

            if (cand->value->decider_flag == ALREADY_EXISTING_WME_DECIDER_FLAG)
            {
                if (id->id->isa_goal)
                {
                    /* Remove fake preference for goal item */
                    preference_remove_ref(thisAgent, cand->value->decider_wme->preference);
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
            count_sym = thisAgent->symbolManager->make_int_constant(static_cast< int64_t >(item_count));
            add_impasse_wme(thisAgent, id, loop_count_sym, count_sym, NIL);
            thisAgent->symbolManager->symbol_remove_ref(&count_sym);
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

    dprint(DT_WME_CHANGES, "Deciding non-context slot (%y ^%y _?_)\n", s->id, s->attr);
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

        /* --- for each existing wme, if we want it there, mark it as ALREADY_EXISTING; otherwise remove it --- */
        w = s->wmes;
        while (w)
        {
            next_w = w->next;
            if (w->value->decider_flag == CANDIDATE_DECIDER_FLAG)
            {
                w->value->decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
                w->value->decider_wme = w; /* so we can set the pref later */
                dprint(DT_WME_CHANGES, "WME %w already an existing wme.  Marking as ALREADY_EXISTING_WME_DECIDER_FLAG.\n", w);

            }
            else
            {
                remove_from_dll(s->wmes, w, next, prev);
                if (w->gds)
                {
                    if (w->gds->goal != NIL)
                    {
                        /* If the goal pointer is non-NIL, then goal is in the stack */
                        gds_invalid_so_remove_goal(thisAgent, w);
                    }
                }
                remove_wme_from_wm(thisAgent, w);
            }
            w = next_w;
        }  /* end while (W) */

        /* --- for each desired value, if it's not already there, add it --- */
        for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        {
            if (cand->value->decider_flag == ALREADY_EXISTING_WME_DECIDER_FLAG)
            {
                cand->value->decider_wme->preference = cand;
            }
            else
            {
                dprint(DT_WME_CHANGES, "Adding non-context %s WME at %y (lvl %d) from instantiation i%u (%y) and pref %p to slot.\n",
                    cand->o_supported ? "o-supported" : "i-supported",
                        cand->inst->match_goal, static_cast<int64_t>(cand->id->id->level), cand->inst->i_id, cand->inst->prod_name, cand);

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
                            thisAgent->memoryManager->free_with_pool(MP_wma_slot_refs, s->wma_val_references);
                            s->wma_val_references = NIL;
                        }
                    }
                }

                /* Whenever we add a WME to WM, we also want to check and see if
                this new WME is o-supported.  If so, then we want to add the
                supergoal dependencies of the new, o-supported element to the
                goal in which the element was created (as long as the o_supported
                element was not created in the top state -- the top goal has
                no gds).  */

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
                thisAgent->timers_gds.start();
#endif
#endif
                thisAgent->parent_list_head = NIL;

                /* If the working memory element being added is going to have
                o_supported preferences and the instantiation that created it
                is not in the top_level_goal (where there is no GDS), then
                loop over the preferences for this WME and determine which
                WMEs should be added to the goal's GDS (the goal here being the
                goal to which the added memory is attached). */

                /* Find the highest level preference that is not the top level */
                goal_stack_level    current_highest_level = LOWEST_POSSIBLE_GOAL_LEVEL;
                Symbol*             current_highest_goal = NULL;
                for (pref = w->preference; pref != NIL; pref = pref->next)
                {
                    if ((w->preference->o_supported == true) && (w->preference->level != 1) &&
                        (w->preference->inst->match_goal->id->gds == NIL) &&
                        (w->preference->inst->match_goal_level < current_highest_level))
                    {
                        dprint(DT_GDS, "GDS creation detected o-supported WME being added for subgoal %y (l%d). %s.  %w\n",
                            w->preference->inst->match_goal, static_cast<int64_t>(w->preference->id->id->level),
                            (w->preference->inst->match_goal->id->gds == NIL) ? "GDS exists" : "GDS does not exist", w);
                        //                    dprint(DT_GDS, "- created as a result of instantiation i%u preference: %p\n", w->preference->inst->i_id, w->preference);
                        current_highest_level = w->preference->inst->match_goal_level;
                        current_highest_goal = w->preference->inst->match_goal;
                    }
                }
                /* Create a GDS for that level */
                if (current_highest_goal)
                {
                    dprint(DT_GDS, "...creating new GDS for match goal %y\n", w->preference->inst->match_goal);
                    create_gds_for_goal(thisAgent, w->preference->inst->match_goal);
                }
                /* Loop over all the preferences for this WME:
                 *   If the instantiation that lead to the preference has not been already explored
                 *         OR
                 *   If the instantiation is not an subgoal instantiation
                 *          for a chunk instantiation we are already exploring
                 *   Then
                 *      Add the instantiation to a list of instantiations that
                 *          will be explored in elaborate_gds().
                 */

                for (pref = w->preference; pref != NIL; pref = pref->next)
                {
                    dprint(DT_GDS_HIGH, "   %p   Goal level of preference: %d\n", pref, static_cast<int64_t>(pref->id->id->level));

                    if ((pref->inst->GDS_evaluated_already == false) && (pref->inst->match_goal_level == current_highest_level))
                    {
                        dprint(DT_GDS_HIGH, "   Match goal lev of instantiation %y is %d\n", pref->inst->prod_name , static_cast<int64_t>(pref->level));
                        dprint(DT_GDS_HIGH, "   Adding %y to list of parent instantiations\n", pref->inst->prod_name);
                        uniquely_add_to_head_of_dll(thisAgent, pref->inst);
                        pref->inst->GDS_evaluated_already = true;
                    }
                    else
                    {
                        dprint(DT_GDS_HIGH, "    Instantiation %y was already explored; skipping it\n", pref->inst->prod_name);
                    }
                }

                if (thisAgent->parent_list_head)
                {
                    dprint(DT_GDS_HIGH, "    CALLING ELABORATE GDS....\n");
                    elaborate_gds(thisAgent);
                }
                /* technically, the list should be empty at this point ??? */
                assert(!thisAgent->parent_list_head);
                if (thisAgent->parent_list_head) free_parent_list(thisAgent);
                dprint(DT_GDS_HIGH, "    FINISHED ELABORATING GDS.\n\n");

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
                thisAgent->timers_gds.stop();
                thisAgent->timers_gds_cpu_time[thisAgent->current_phase].update(thisAgent->timers_gds);
#endif
#endif
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

    dprint(DT_WME_CHANGES, "Deciding non-context slots.\n");
    while (thisAgent->changed_slots)
    {
        dc = thisAgent->changed_slots;
        thisAgent->changed_slots = thisAgent->changed_slots->next;
        s = static_cast<slot_struct*>(dc->item);
        dprint(DT_WME_CHANGES, "Deciding non-context slot (%y ^%y ?)\n", s->id, s->attr);
        decide_non_context_slot(thisAgent, s);
        s->changed = NIL;
        dprint(DT_WME_CHANGES, "Done deciding non-context slot (%y ^%y ?)\n", s->id, s->attr);
        thisAgent->memoryManager->free_with_pool(MP_dl_cons, dc);
    }
    dprint(DT_WME_CHANGES, "Done deciding non-context slots.\n");
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

    ms_change* head, *tail;

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
#ifdef DO_TOP_LEVEL_COND_REF_CTS
    while (goal->id->preferences_from_goal)
    {
        p = goal->id->preferences_from_goal;
        remove_from_dll(goal->id->preferences_from_goal, p, all_of_goal_next, all_of_goal_prev);
        p->on_goal_list = false;
        if (! remove_preference_from_clones_and_deallocate(thisAgent, p))
            if (p->in_tm)
            {
                remove_preference_from_tm(thisAgent, p);
            }
    }
#else
    /* KJC Aug 05: this seems to cure a potential for exceeding callstack
     * when popping soar's goal stack and not doing DO_TOP_LEVEL_COND_REF_CTS
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
            remove_from_dll(goal->id->preferences_from_goal, p, all_of_goal_next, all_of_goal_prev);
            p->on_goal_list = false;
            if (! remove_preference_from_clones_and_deallocate(thisAgent, p))
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
    thisAgent->SMem->reset(goal);

    remove_wme_list_from_wm(thisAgent, goal->id->impasse_wmes);
    goal->id->impasse_wmes = NIL;
    /* If there was a GDS for this goal, we want to set the pointer for the
       goal to NIL to indicate it no longer exists.
       BUG: We probably also need to make certain that the GDS doesn't need
       to be free'd here as well. */
    if (goal->id->gds != NIL)
    {
        goal->id->gds->goal = NIL;
    }
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
    thisAgent->memoryManager->free_with_pool(MP_rl_et, goal->id->rl_info->eligibility_traces);
    goal->id->rl_info->prev_op_rl_rules->~production_list();
    thisAgent->memoryManager->free_with_pool(MP_rl_rule, goal->id->rl_info->prev_op_rl_rules);
    thisAgent->memoryManager->free_with_pool(MP_rl_info, goal->id->rl_info);

    goal->id->epmem_info->epmem_wmes->~epmem_wme_stack();
    thisAgent->memoryManager->free_with_pool(MP_epmem_wmes, goal->id->epmem_info->epmem_wmes);
    thisAgent->memoryManager->free_with_pool(MP_epmem_info, goal->id->epmem_info);

    goal->id->smem_info->smem_wmes->~preference_list();
    thisAgent->memoryManager->free_with_pool(MP_smem_wmes, goal->id->smem_info->smem_wmes);
    thisAgent->memoryManager->free_with_pool(MP_smem_info, goal->id->smem_info);

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

    free_list(thisAgent, extract_list_elements(thisAgent, &(thisAgent->explanationBasedChunker->chunky_problem_spaces), cons_equality_fn, reinterpret_cast<void*>(goal)));
    free_list(thisAgent, extract_list_elements(thisAgent, &(thisAgent->explanationBasedChunker->chunk_free_problem_spaces), cons_equality_fn, reinterpret_cast<void*>(goal)));

    post_link_removal(thisAgent, NIL, goal);   /* remove the special link */

    if (goal->id->level <= thisAgent->substate_break_level)
    {
        thisAgent->stop_soar = true;
        thisAgent->substate_break_level = 0;
        thisAgent->reason_for_stopping = "Stopped due to substate (goal) retraction.";
    }

    thisAgent->symbolManager->symbol_remove_ref(&goal);
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
        add_impasse_wme(thisAgent, id, thisAgent->symbolManager->soarSymbols.quiescence_symbol,
                        thisAgent->symbolManager->soarSymbols.t_symbol, NIL);
        if ((NO_CHANGE_IMPASSE_TYPE == impasse_type) &&
                (thisAgent->Decider->settings[DECIDER_MAX_GOAL_DEPTH] < thisAgent->bottom_goal->id->level))
        {
            // appear to be SNC'ing deep in goalstack, so interrupt and warn user
            // KJC note: we actually halt, because there is no interrupt function in SoarKernel
            // in the gSKI Agent code, if system_halted, MAX_GOAL_DEPTH is checked and if exceeded
            // then the interrupt is generated and system_halted is set to false so the user can recover.
            thisAgent->outputManager->printa_sf(thisAgent, "\nGoal stack depth exceeded %u on a no-change impasse.\n", thisAgent->Decider->settings[DECIDER_MAX_GOAL_DEPTH]);
            thisAgent->outputManager->printa_sf(thisAgent, "Soar appears to be in an infinite loop.  \nContinuing to subgoal may cause Soar to \nexceed the program stack of your system.\n");
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
        id = create_new_impasse(thisAgent, true, thisAgent->symbolManager->soarSymbols.nil_symbol,
                                NIL, NONE_IMPASSE_TYPE,
                                TOP_GOAL_LEVEL);
        thisAgent->top_goal = id;
        thisAgent->bottom_goal = id;
        thisAgent->top_state = thisAgent->top_goal;
        id->id->higher_goal = NIL;
        id->id->lower_goal = NIL;
    }

    id->id->isa_goal = true;
    id->id->operator_slot = make_slot(thisAgent, id, thisAgent->symbolManager->soarSymbols.operator_symbol);
    id->id->allow_bottom_up_chunks = true;

    id->id->rl_info->previous_q = 0;
    id->id->rl_info->reward = 0;
    id->id->rl_info->rho = 1.0;
    id->id->rl_info->gap_age = 0;
    id->id->rl_info->hrl_age = 0;
    thisAgent->memoryManager->allocate_with_pool(MP_rl_et, &(id->id->rl_info->eligibility_traces));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->rl_info->eligibility_traces = new(id->id->rl_info->eligibility_traces) rl_et_map(std::less< production* >(), soar_module::soar_memory_pool_allocator< std::pair< production*, double > >());
#else
    id->id->rl_info->eligibility_traces = new(id->id->rl_info->eligibility_traces) rl_et_map();
#endif
    thisAgent->memoryManager->allocate_with_pool(MP_rl_rule, &(id->id->rl_info->prev_op_rl_rules));
    id->id->rl_info->prev_op_rl_rules = new(id->id->rl_info->prev_op_rl_rules) production_list();

    id->id->epmem_info->last_ol_time = 0;
    id->id->epmem_info->last_cmd_time = 0;
    id->id->epmem_info->last_cmd_count = 0;
    id->id->epmem_info->last_memory = EPMEM_MEMID_NONE;
    thisAgent->memoryManager->allocate_with_pool(MP_epmem_wmes, &(id->id->epmem_info->epmem_wmes));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->epmem_info->epmem_wmes = new(id->id->epmem_info->epmem_wmes) epmem_wme_stack(soar_module::soar_memory_pool_allocator< preference* >(thisAgent));
#else
    id->id->epmem_info->epmem_wmes = new(id->id->epmem_info->epmem_wmes) epmem_wme_stack();
#endif

    id->id->smem_info->last_cmd_time[0] = 0;
    id->id->smem_info->last_cmd_time[1] = 0;
    id->id->smem_info->last_cmd_count[0] = 0;
    id->id->smem_info->last_cmd_count[1] = 0;
    thisAgent->memoryManager->allocate_with_pool(MP_smem_wmes, &(id->id->smem_info->smem_wmes));
#ifdef USE_MEM_POOL_ALLOCATORS
    id->id->smem_info->smem_wmes = new(id->id->smem_info->smem_wmes) preference_list(soar_module::soar_memory_pool_allocator< preference* >(thisAgent));
#else
    id->id->smem_info->smem_wmes = new(id->id->smem_info->smem_wmes) preference_list();
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
        if (w->attr == thisAgent->symbolManager->soarSymbols.impasse_symbol)
        {
            if (w->value == thisAgent->symbolManager->soarSymbols.no_change_symbol)
            {
                return NO_CHANGE_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->symbolManager->soarSymbols.tie_symbol)
            {
                return TIE_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->symbolManager->soarSymbols.constraint_failure_symbol)
            {
                return CONSTRAINT_FAILURE_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->symbolManager->soarSymbols.conflict_symbol)
            {
                return CONFLICT_IMPASSE_TYPE;
            }
            if (w->value == thisAgent->symbolManager->soarSymbols.none_symbol)
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
        if (w->attr == thisAgent->symbolManager->soarSymbols.attribute_symbol)
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
            attribute_of_impasse = thisAgent->symbolManager->soarSymbols.state_symbol;
        }
    }
    else
    {
        /* --- for all other kinds of impasses --- */
        attribute_of_impasse = s->attr;
    }

    /* --- remove wme's for lower slots of this context --- */
    if (attribute_of_impasse == thisAgent->symbolManager->soarSymbols.state_symbol)
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
            if (thisAgent->trace_settings[TRACE_CONSISTENCY_CHANGES_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent, "Removing state %y because of a decision.\n", goal->id->lower_goal);
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

    /* --- no impasse already existed, or an impasse of the wrong type already existed --- */
    for (temp = candidates; temp; temp = temp->next_candidate)
    {
        preference_add_ref(temp);
    }

    if (goal->id->lower_goal)
    {
        if (thisAgent->trace_settings[TRACE_CONSISTENCY_CHANGES_SYSPARAM])
        {
            thisAgent->outputManager->printa_sf(thisAgent, "Removing state %y because it's the wrong type of impasse.\n", goal->id->lower_goal);
        }

        remove_existing_context_and_descendents(thisAgent, goal->id->lower_goal);
    }

    if (!thisAgent->Decider->settings[DECIDER_WAIT_SNC]|| !(impasse_type == NO_CHANGE_IMPASSE_TYPE) || !(attribute_of_impasse == thisAgent->symbolManager->soarSymbols.state_symbol))
    {
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
    thisAgent->memoryManager->init_memory_pool(MP_slot, sizeof(slot), "slot");
    thisAgent->memoryManager->init_memory_pool(MP_wme, sizeof(wme), "wme");
    thisAgent->memoryManager->init_memory_pool(MP_preference, sizeof(preference), "preference");
}

void do_buffered_wm_and_ownership_changes(agent* thisAgent)
{
    do_buffered_acceptable_preference_wme_changes(thisAgent);
    do_buffered_link_changes(thisAgent);
    do_buffered_wm_changes(thisAgent);
    remove_garbage_slots(thisAgent);
}


/* -----------------------------------------------------------------------
 Do Preference Phase

 This routine is called from the top level to run the preference phase.

 Preference phase follows this sequence:

 (1) Productions are fired for new matches.  As productions are fired,
 their instantiations are stored on the list newly_created_instantiations,
 linked via the "next" fields in the instantiation structure.  No
 preferences are actually asserted yet.

 (2) Instantiations are retracted; their preferences are retracted.

 (3) Finally, o-rejects are processed.
 (4) Preferences (except o-rejects) from newly_created_instantiations
 are asserted, and these instantiations are removed from the
 newly_created_instantiations list and moved over to the per-production
 lists of instantiations of that production.

 ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 Assert New Preferences

 This routine scans through newly_created_instantiations, asserting
 each preference generated except for o-rejects.  It also removes
 each instantiation from newly_created_instantiations, linking each
 onto the list of instantiations for that particular production.
 O-rejects are buffered and handled after everything else.

 Note that some instantiations on newly_created_instantiations are not
 in the match set--for the initial instantiations of chunks/justifications,
 if they don't match WM, we have to assert the o-supported preferences
 and throw away the rest.
 ----------------------------------------------------------------------- */

void assert_new_preferences(agent* thisAgent, preference_list& bufdeallo)
{
    instantiation* inst, *next_inst;
    preference* pref, *next_pref;
    preference* o_rejects;

    o_rejects = NIL;

    if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
    {
        printf("\n   in assert_new_preferences:");
        xml_generate_verbose(thisAgent, "in assert_new_preferences:");
    }
    {
        /* Do an initial loop to process o-rejects, then re-loop
         to process normal preferences. */

        for (inst = thisAgent->newly_created_instantiations; inst != NIL; inst =  next_inst)
        {
            next_inst = inst->next;

            for (pref = inst->preferences_generated; pref != NIL; pref = next_pref)
            {
                next_pref = pref->inst_next;
                if ((pref->type == REJECT_PREFERENCE_TYPE) && (pref->o_supported))
                {
                    /* --- o-reject: just put it in the buffer for later --- */
                    pref->next = o_rejects;
                    o_rejects = pref;
                }
            }
        }

        if (o_rejects)
        {
            process_o_rejects_and_deallocate_them(thisAgent, o_rejects, bufdeallo);
        }
    }

    for (inst = thisAgent->newly_created_instantiations; inst != NIL; inst = next_inst)
    {
        next_inst = inst->next;
        if (!inst->in_newly_deleted)
        {
            if (inst->in_ms)
            {
                inst->in_newly_created = false;
                insert_at_head_of_dll(inst->prod->instantiations, inst, next, prev);
            }

            if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "\n      asserting instantiation: %y\n", inst->prod_name);
                char buf[256];
                SNPRINTF(buf, 254, "asserting instantiation: %s", inst->prod_name->to_string(true));
                xml_generate_verbose(thisAgent, buf);
            }

            for (pref = inst->preferences_generated; pref != NIL; pref = next_pref)
            {
                next_pref = pref->inst_next;
                if ((pref->type == REJECT_PREFERENCE_TYPE) && (pref->o_supported))
                {
                    /* No knowledge retrieval necessary in Operand2 */
                }
                else if (inst->in_ms || pref->o_supported)
                {
                    /* --- normal case --- */
                    if (add_preference_to_tm(thisAgent, pref))
                    {
                        /* No knowledge retrieval necessary in Operand2 */
                        if (wma_enabled(thisAgent))
                        {
                            wma_activate_wmes_in_pref(thisAgent, pref);
                        }
                    }
                    else
                    {
                        // NLD: the preference was o-supported, at
                        // the top state, and was asserting an acceptable
                        // preference for a WME that was already
                        // o-supported. hence unnecessary.

                        preference_add_ref(pref);
                        preference_remove_ref(thisAgent, pref);
                    }
                }
                else
                {
                    /* --- inst. is refracted chunk, and pref. is not o-supported:
                 remove the preference --- */

                    /* --- first splice it out of the clones list--otherwise we might
                 accidentally deallocate some clone that happens to have refcount==0
                 just because it hasn't been asserted yet --- */

                    if (pref->next_clone)
                    {
                        pref->next_clone->prev_clone = pref->prev_clone;
                    }
                    if (pref->prev_clone)
                    {
                        pref->prev_clone->next_clone = pref->next_clone;
                    }
                    pref->next_clone = pref->prev_clone = NIL;

                    /* --- now add then remove ref--this should result in deallocation */
                    preference_add_ref(pref);
                    preference_remove_ref(thisAgent, pref);
                }
            }
        }
    }
    if (!thisAgent->Decider->settings[DECIDER_KEEP_TOP_OPREFS] && !thisAgent->newly_deleted_instantiations.empty())
    {
        dprint(DT_DEALLOCATE_INST, "Deallocating %d newly created instantiations that were flagged for deletion before they were asserted.\n", thisAgent->newly_deleted_instantiations.size());
        instantiation* lDeleteInst;
        for (auto iter = thisAgent->newly_deleted_instantiations.begin(); iter != thisAgent->newly_deleted_instantiations.end(); iter++)
        {
            lDeleteInst = *iter;
            lDeleteInst->in_newly_created = false;
            lDeleteInst->in_newly_deleted = false;
            deallocate_instantiation(thisAgent, lDeleteInst);
        }
        thisAgent->newly_deleted_instantiations.clear();
    }
}

/**
 * New waterfall model:
 * Returns true if the function create_instantiation should run for this production.
 * Used to delay firing of matches in the inner preference loop.
 */
bool shouldCreateInstantiation(agent* thisAgent, production* prod,
                               struct token_struct* tok, wme* w)
{
    if (thisAgent->active_level == thisAgent->highest_active_level)
    {
        return true;
    }

    if (prod->type == TEMPLATE_PRODUCTION_TYPE)
    {
        return true;
    }

    // Scan RHS identifiers for their levels, don't fire those at or higher than the change level
    action* a = NIL;
    for (a = prod->action_list; a != NIL; a = a->next)
    {
        if (a->type == FUNCALL_ACTION)
        {
            continue;
        }

        // skip unbound variables
        if (rhs_value_is_unboundvar(a->id))
        {
            continue;
        }

        // try to make a symbol
        Symbol* sym = NIL;
        if (rhs_value_is_symbol(a->id))
        {
            sym = rhs_value_to_symbol(a->id);
        }
        else
        {
            if (rhs_value_is_reteloc(a->id))
            {
                sym = get_symbol_from_rete_loc(rhs_value_to_reteloc_levels_up(a->id), rhs_value_to_reteloc_field_num(a->id), tok, w);
            }
        }
        assert(sym != NIL);

        // check level for legal change
        if (sym->id->level <= thisAgent->change_level)
        {
            if (thisAgent->trace_settings[TRACE_WATERFALL_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent, "*** Waterfall: aborting firing because (%y * *)", sym);
                thisAgent->outputManager->printa_sf(thisAgent,
                      " level %d is on or higher (lower int) than change level %d\n",
                      static_cast<int64_t>(sym->id->level), static_cast<int64_t>(thisAgent->change_level));
            }
            return false;
        }
    }
    return true;
}

void do_preference_phase(agent* thisAgent)
{
    instantiation* inst = 0;

    /* AGR 617/634:  These are 2 bug reports that report the same problem,
     namely that when 2 chunk firings happen in succession, there is an
     extra newline printed out.  The simple fix is to monitor
     get_printer_output_column and see if it's at the beginning of a line
     or not when we're ready to print a newline.  94.11.14 */
    if (thisAgent->trace_settings[TRACE_PHASES_SYSPARAM])
    {
        if (thisAgent->current_phase == APPLY_PHASE)   /* it's always IE for PROPOSE */
        {
            xml_begin_tag(thisAgent, kTagSubphase);
            xml_att_val(thisAgent, kPhase_Name, kSubphaseName_FiringProductions);
            switch (thisAgent->FIRING_TYPE)
            {
                case PE_PRODS:
                    thisAgent->outputManager->printa_sf(thisAgent,
                          "\t--- Firing Productions (PE) For State At Depth %d ---\n",
                          static_cast<int64_t>(thisAgent->active_level)); // SBW 8/4/2008: added active_level
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_PE);
                    break;
                case IE_PRODS:
                    thisAgent->outputManager->printa_sf(thisAgent,
                          "\t--- Firing Productions (IE) For State At Depth %d ---\n",
                          static_cast<int64_t>(thisAgent->active_level)); // SBW 8/4/2008: added active_level
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_IE);
                    break;
            }
            std::string levelString;
            to_string(thisAgent->active_level, levelString);
            xml_att_val(thisAgent, kPhase_LevelNum, levelString.c_str()); // SBW 8/4/2008: active_level for XML output mode
            xml_end_tag(thisAgent, kTagSubphase);
        }
    }

    if (wma_enabled(thisAgent))
    {
        wma_activate_wmes_tested_in_prods(thisAgent);
    }

    /* New waterfall model: */
    // Save previous active level for usage on next elaboration cycle.
    thisAgent->highest_active_level = thisAgent->active_level;
    thisAgent->highest_active_goal = thisAgent->active_goal;

    thisAgent->change_level = thisAgent->highest_active_level;
    thisAgent->next_change_level = thisAgent->highest_active_level;

    // Temporary list to buffer deallocation of some preferences until the inner elaboration loop is over.
    preference_list bufdeallo;

    // inner elaboration cycle
    for (;;)
    {
        thisAgent->change_level = thisAgent->next_change_level;
        if (thisAgent->trace_settings[TRACE_WATERFALL_SYSPARAM])
        {
            thisAgent->outputManager->printa_sf(thisAgent, "\n--- Inner Elaboration Phase, active level %d goal %y ---\n", static_cast<int64_t>(thisAgent->active_level), thisAgent->active_goal);
        }
        dprint(DT_DEALLOCATE_INST, "Clearing newly_created_instantiations...\n");
        thisAgent->newly_created_instantiations = NIL;

        bool assertionsExist = false;
        production* prod = 0;
        struct token_struct* tok = 0;
        wme* w = 0;
        bool once = true;
        while (postpone_assertion(thisAgent, &prod, &tok, &w))
        {
            assertionsExist = true;

            if (prod->type == JUSTIFICATION_PRODUCTION_TYPE)
            {
                consume_last_postponed_assertion(thisAgent);

                // don't fire justifications
                continue;
            }

            if (shouldCreateInstantiation(thisAgent, prod, tok, w))
            {
                once = false;
                consume_last_postponed_assertion(thisAgent);
                create_instantiation(thisAgent, prod, tok, w);
            }
        }

        // New waterfall model: something fired or is pending to fire at this level,
        // so this active level becomes the next change level.
        if (assertionsExist)
        {
            if (thisAgent->active_level > thisAgent->next_change_level)
            {
                thisAgent->next_change_level = thisAgent->active_level;
            }
        }

        // New waterfall model: push unfired matches back on to the assertion lists
        restore_postponed_assertions(thisAgent);

        assert_new_preferences(thisAgent, bufdeallo);

        // Update accounting
        thisAgent->inner_e_cycle_count++;

        if (thisAgent->active_goal == NIL)
        {
            dprint(DT_WATERFALL, " inner elaboration loop doesn't have active goal.\n");
            break;
        }

        if (thisAgent->active_goal->id->lower_goal == NIL)
        {
            dprint(DT_WATERFALL, " inner elaboration loop at bottom goal.\n");
            break;
        }

        if (thisAgent->current_phase == APPLY_PHASE)
        {
            thisAgent->active_goal = highest_active_goal_apply(thisAgent, thisAgent->active_goal->id->lower_goal, true);
        }
        else
        {
            assert(thisAgent->current_phase == PROPOSE_PHASE);
            thisAgent->active_goal = highest_active_goal_propose(thisAgent, thisAgent->active_goal->id->lower_goal, true);
        }

        if (thisAgent->active_goal != NIL)
        {
            thisAgent->active_level = thisAgent->active_goal->id->level;
        }
        else
        {
            dprint(DT_WATERFALL, " inner elaboration loop finished but not at quiescence.\n");
            break;
        }
    } // end inner elaboration loop

    // Deallocate preferences delayed during inner elaboration loop.
    for (preference_list::iterator iter = bufdeallo.begin();
            iter != bufdeallo.end(); ++iter)
    {
        dprint(DT_DEALLOCATE_PREF, "Removing ref for bufdeallo queued o-rejected preference %p (%u) at level %d\n", (*iter), (*iter)->p_id, static_cast<int64_t>((*iter)->level));
        {
                preference_remove_ref(thisAgent, *iter);
        }
    }

    // Restore previous active level
    thisAgent->active_level = thisAgent->highest_active_level;
    thisAgent->active_goal = thisAgent->highest_active_goal;
    /* End new waterfall model */

    while (get_next_retraction(thisAgent, &inst))
    {
        retract_instantiation(thisAgent, inst);
    }

    /*  In Waterfall, if there are nil goal retractions, then we want to
     retract them as well, even though they are not associated with any
     particular goal (because their goal has been deleted). The
     functionality of this separate routine could have been easily
     combined in get_next_retraction but I wanted to highlight the
     distinction between regualr retractions (those that can be
     mapped onto a goal) and nil goal retractions that require a
     special data strucutre (because they don't appear on any goal) */

    if (thisAgent->nil_goal_retractions)
    {
        while (get_next_nil_goal_retraction(thisAgent, &inst))
        {
            retract_instantiation(thisAgent, inst);
        }
    }

}

void do_working_memory_phase(agent* thisAgent)
{

    if (thisAgent->trace_settings[TRACE_PHASES_SYSPARAM])
    {
        if (thisAgent->current_phase == APPLY_PHASE)    /* it's always IE for PROPOSE */
        {
            xml_begin_tag(thisAgent, kTagSubphase);
            xml_att_val(thisAgent, kPhase_Name, kSubphaseName_ChangingWorkingMemory);
            switch (thisAgent->FIRING_TYPE)
            {
                case PE_PRODS:
                    thisAgent->outputManager->printa_sf(thisAgent, "\t--- Change Working Memory (PE) ---\n", 0);
                    xml_att_val(thisAgent, kPhase_FiringType, kPhaseFiringType_PE);
                    break;
                case IE_PRODS:
                    thisAgent->outputManager->printa_sf(thisAgent, "\t--- Change Working Memory (IE) ---\n", 0);
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
    thisAgent->highest_goal_whose_context_changed = NIL;  /* nothing changed                                                                yet */
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
//            dprint(DT_GDS, "UNIQUE DLL:            %y is already in parent list\n", curr_pi->inst->prod_name);
            return;
        }
//        dprint(DT_GDS,  "UNIQUE DLL:            %y\n", curr_pi->inst->prod_name);
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
//    dprint(DT_GDS, "UNIQUE DLL:         added: %y\n", inst->prod_name);
}

void add_wme_to_gds(agent* thisAgent, goal_dependency_set* gds, wme* wme_to_add)
{
    /* Set the correct GDS for this wme (wme's point to their gds) */
    wme_to_add->gds = gds;
    insert_at_head_of_dll(gds->wmes_in_gds, wme_to_add, gds_next, gds_prev);

    if (thisAgent->trace_settings[TRACE_GDS_WMES_SYSPARAM])
    {
        // BADBAD: the XML code makes this all very ugly
        char msgbuf[256];
        memset(msgbuf, 0, 256);
        thisAgent->outputManager->sprinta_sf_cstr(thisAgent, msgbuf, 255, "Adding to GDS for %y: ", wme_to_add->gds->goal);
        thisAgent->outputManager->printa(thisAgent,  msgbuf);

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

        dprint(DT_GDS, "      EXPLORING INSTANTIATION: %y\n%7", curr_pi->inst->prod_name, curr_pi->inst);

        assert(inst->match_goal_level > 1);
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
                pref_for_this_wme = wme_matching_this_cond->preference;

                /* This following was changed to better handle WMEs that change levels since
                 * their instantiations were created.  If there's a preference for the wme at
                 * the instantiation level, I think we want the GDS code to backtrace through
                 * that so that it can pick up any other instantiations that may have other
                 * wme's to add to the GDS.  If it doesn't exist, then it's a superstate WME,
                 * and we should process it normally. */
                preference* clone_for_this_level;

                if (pref_for_this_wme && (pref_for_this_wme->level != inst->match_goal_level))
                {
                    clone_for_this_level = find_clone_for_level(pref_for_this_wme, inst->match_goal_level);
//                    if ((clone_for_this_level && (pref_for_this_wme != clone_for_this_level)))
//                        thisAgent->outputManager->printa_sf(thisAgent, "%e       EGDS:  Pref was not at the inst match goal level %d for wme %w\n"
//                                                                         "              %p (%d %y)\n"
//                                                                         "              %p (%d %y)\n",
//                                                           static_cast<int64_t>(inst->match_goal_level), wme_matching_this_cond,
//                                                           pref_for_this_wme, pref_for_this_wme ? static_cast<int64_t>(pref_for_this_wme->level) : 0, pref_for_this_wme ? pref_for_this_wme->inst->prod_name : NULL,
//                                                           clone_for_this_level, clone_for_this_level ? static_cast<int64_t>(clone_for_this_level->level) : 0, clone_for_this_level ? clone_for_this_level->inst->prod_name : NULL);
                    if (clone_for_this_level)
                        pref_for_this_wme = clone_for_this_level;
                }

                dprint(DT_GDS, "       wme_matching_this_cond (level %d)   : %w\n", static_cast<int64_t>(wme_goal_level), wme_matching_this_cond);
                dprint(DT_GDS, "       - pref_for_this_wme  (level %d (%d)): %p\n",  pref_for_this_wme ? static_cast<int64_t>(pref_for_this_wme->level) : 0,
                                                                                   wme_matching_this_cond->preference ? static_cast<int64_t>(wme_matching_this_cond->preference->level) : 0,
                                                                                   pref_for_this_wme);
                dprint(DT_GDS, "       - inst->match_goal                  : %y (level %d), p\n" , inst->match_goal, static_cast<int64_t>(inst->match_goal_level));


                /* WME is in a supergoal or is architecturally created
                 *
                 * Note:  architectural instantiations for retrievals and impasse items do have prefs,
                 *        and get handled in clause for "wme is local and i-supported")         */

                if ((pref_for_this_wme == NIL) || (wme_goal_level < inst->match_goal_level))
                {

                    dprint(DT_GDS, "%s%s", (!pref_for_this_wme) ? "         WME has no preferences (arch-created)" : "         ",
                                           (wme_goal_level < inst->match_goal_level) ? "WME is in a supergoal\n" : "\n");

                    if (wme_matching_this_cond->gds != NIL)
                    {
                        /* Then we want to check and see if the old GDS value should be changed */
                        if (wme_matching_this_cond->gds->goal == NIL)
                        {
                            dprint(DT_GDS, "  GDS' goal is NIL --> CLEARING GDS FOR REMOVED GOAL FROM WME %w\n", wme_matching_this_cond);
                            /* The goal is NIL: meaning that the goal for the GDS is no longer around */

                            fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds,  wme_matching_this_cond, wme, gds_next, gds_prev);

                            /* Must check for GDS removal every time we take a WME off the GDS wme list */
                            if (!wme_matching_this_cond->gds->wmes_in_gds)
                            {
                                if (wme_matching_this_cond->gds->goal)
                                {
                                    wme_matching_this_cond->gds->goal->id->gds = NIL;
                                }

                                GDI_remove(thisAgent, wme_matching_this_cond->gds);
                                thisAgent->memoryManager->free_with_pool(MP_gds, wme_matching_this_cond->gds);

                            }

                            add_wme_to_gds(thisAgent, inst->match_goal->id->gds, wme_matching_this_cond);

                            dprint(DT_GDS, "       ..... added WME to GDS list of %y....\n", inst->match_goal);
                        }
                        else if (wme_matching_this_cond->gds->goal->id->level > inst->match_goal_level)
                        {
                            /* This WME currently belongs to the GDS of a goal below the current one */
                            /* 1. Take WME off old (current) GDS list
                             * 2. Check to see if old GDS WME list is empty.  If so, remove(free) it.
                             * 3. Add WME to new GDS list
                             * 4. Update WME pointer to new GDS list
                             */
                            dprint(DT_GDS, "  WME ALREADY IN LOWER LEVEL GDS of %y.  MOVING UP.\n", wme_matching_this_cond->gds->goal);
                            fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds,  wme_matching_this_cond, wme, gds_next, gds_prev);
                            if (!wme_matching_this_cond->gds->wmes_in_gds)
                            {
                                dprint(DT_GDS, "       ....removing now empty GDS from %y", wme_matching_this_cond->gds->goal);
                                if (wme_matching_this_cond->gds->goal)
                                {
                                    wme_matching_this_cond->gds->goal->id->gds = NIL;
                                }

                                GDI_remove(thisAgent, wme_matching_this_cond->gds);
                                thisAgent->memoryManager->free_with_pool(MP_gds, wme_matching_this_cond->gds);

                            }
                            add_wme_to_gds(thisAgent, inst->match_goal->id->gds, wme_matching_this_cond);

                            dprint(DT_GDS, "       ....moved to GDS list of %y\n", inst->match_goal);
                            wme_matching_this_cond->gds = inst->match_goal->id->gds;
                        }
                    }
                    else
                    {
                        /* WME should be in the GDS of the current goal if the WME's GDS does not already exist. (i.e., if NIL GDS) */

                        add_wme_to_gds(thisAgent, inst->match_goal->id->gds, wme_matching_this_cond);

                        if (wme_matching_this_cond->gds->wmes_in_gds->gds_prev)
                        {
                            thisAgent->outputManager->printa_sf(thisAgent, "\nDEBUG DEBUG : The new header should never have a prev value.\n");
                        }
                        dprint(DT_GDS, "       ......WME did not have defined GDS.  Now adding to goal [%y].\n", wme_matching_this_cond->gds->goal);
                    } /* end else clause for "if wme_matching_this_cond->gds != NIL" */
                    dprint(DT_GDS, "            Added WME to GDS for goal level %d (%y)\n", static_cast<int64_t>(wme_matching_this_cond->gds->goal->id->level), wme_matching_this_cond->gds->goal);
                } /* end "wme in supergoal or arch-supported" */
                else
                {
                    /* WME must be local. If wme's pref is o-supported, then just ignore it and move to next condition */
                    if (pref_for_this_wme->o_supported == true)
                    {
                        dprint(DT_GDS, "         this wme is local and o-supported\n");
                        continue;
                    }

                    else
                    {
                        /* wme's pref is i-supported, so remember it's instantiation
                         * for later examination */

                        /* this test avoids "backtracing" through the top state */
                        if (inst->match_goal_level == 1)
                        {
                            dprint(DT_GDS, "         don't back up through top state for instantiation %y\n", inst->prod_name);
                            continue;
                        }

                        else   /* (inst->match_goal_level != 1) */
                        {
                            dprint(DT_GDS,  "         this wme is local and i-supported\n");
                            s = find_slot(pref_for_this_wme->id, pref_for_this_wme->attr);
                            if (s == NIL)
                            {
                                /* this must be an arch-wme from a fake instantiation */
                                dprint(DT_GDS, "here's the wme with no slot: %w", pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme_);

                                /* this is the same code as above, just using the differently-named pointer.  it probably should be a subroutine */
                                {
                                    wme* fake_inst_wme_cond;

                                    fake_inst_wme_cond = pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme_;
                                    if (fake_inst_wme_cond->gds != NIL)
                                    {
                                        /* Then we want to check and see if the old GDS value should be changed */
                                        if (fake_inst_wme_cond->gds->goal == NIL)
                                        {
                                            /* The goal is NIL: meaning that the goal for the GDS is no longer around */

                                            fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds, fake_inst_wme_cond, wme, gds_next, gds_prev);

                                            /* We have to check for GDS removal anytime we take a WME off the GDS wme list, not just when a WME
                                             * is removed from memory. */
                                            if (!fake_inst_wme_cond->gds->wmes_in_gds)
                                            {
                                                if (fake_inst_wme_cond->gds->goal)
                                                {
                                                    fake_inst_wme_cond->gds->goal->id->gds = NIL;
                                                }
                                                GDI_remove(thisAgent, fake_inst_wme_cond->gds);
                                                thisAgent->memoryManager->free_with_pool(MP_gds, fake_inst_wme_cond->gds);

                                                dprint(DT_GDS, "  REMOVING GDS FROM MEMORY.");
                                            }

                                            add_wme_to_gds(thisAgent, inst->match_goal->id->gds, fake_inst_wme_cond);

                                            dprint(DT_GDS, "       .....GDS' goal is NIL so switching from old to new GDS list....\n");
                                        }
                                        else if (fake_inst_wme_cond->gds->goal->id->level > inst->match_goal_level)
                                        {
                                            /* If the WME currently belongs to the GDS of a goal below the current one:
                                             * 1. Take WME off old (current) GDS list
                                             * 2. Check to see if old GDS WME list is empty. If so, remove(free) it.
                                             * 3. Add WME to new GDS list
                                             * 4. Update WME pointer to new GDS list */
                                            fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds, fake_inst_wme_cond, wme, gds_next, gds_prev);
                                            if (!fake_inst_wme_cond->gds->wmes_in_gds)
                                            {
                                                if (fake_inst_wme_cond->gds->goal)
                                                {
                                                    fake_inst_wme_cond->gds->goal->id->gds = NIL;
                                                }
                                                GDI_remove(thisAgent, fake_inst_wme_cond->gds);

                                                thisAgent->memoryManager->free_with_pool(MP_gds, fake_inst_wme_cond->gds);
                                                dprint(DT_GDS, "  REMOVING GDS FROM MEMORY.");
                                            }

                                            add_wme_to_gds(thisAgent, inst->match_goal->id->gds, fake_inst_wme_cond);

                                            dprint(DT_GDS, "       .....switching from old to new GDS list....\n");
                                            fake_inst_wme_cond->gds = inst->match_goal->id->gds;
                                        }
                                    }
                                    else
                                    {
                                        /* We know that the WME should be in the GDS of the current goal if the WME's GDS does not already exist. (i.e., if NIL GDS) */

                                        add_wme_to_gds(thisAgent, inst->match_goal->id->gds, fake_inst_wme_cond);

                                        if (fake_inst_wme_cond->gds->wmes_in_gds->gds_prev)
                                        {
                                            thisAgent->outputManager->printa_sf(thisAgent, "\nDEBUG DEBUG : The new header should never have a prev value.\n");
                                        }
                                        dprint(DT_GDS, "       ......WME did not have defined GDS.  Now adding to goal [%y].\n", fake_inst_wme_cond->gds->goal);
                                    }
                                    dprint(DT_GDS, "            Added WME to GDS for goal level %d (%y)\n", static_cast<int64_t>(fake_inst_wme_cond->gds->goal->id->level), fake_inst_wme_cond->gds->goal);
                                }  /* matches { wme *fake_inst_wme_cond  */
                            }
                            else
                            {
                                /* this was the original "local & i-supported" action */
                                for (pref = s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; pref; pref = pref->next)
                                {
                                    dprint(DT_GDS, "           looking at pref for the wme: %p", pref);

                                    /* Check that the value with acceptable pref for the slot is the same as the value for the wme in the condition, since
                                       operators can have acceptable preferences for values other than the WME value.  We dont want to backtrack thru acceptable
                                       prefs for other operators */

                                    if (pref->value == wme_matching_this_cond->value)
                                    {
                                        /* REW BUG: may have to go over all insts regardless of this visited_already flag... */

                                        if (pref->inst->GDS_evaluated_already == false)
                                        {
                                            dprint(DT_GDS, "           adding inst that produced the pref to GDS: %y\n", pref->inst->prod_name);
                                            /* If the preference comes from a lower level inst, then  ignore it.
                                             *   - Preferences from lower levels must come from result  instantiations
                                             *   - We just want to use the justification/chunk instantiations at the  match goal level */
                                             
                                            if (pref->level <= inst->match_goal_level)
                                            {
                                                uniquely_add_to_head_of_dll(thisAgent, pref->inst);
                                            }
                                            else
                                            {
                                                /* This was added to follow up on above comment. This looks for the pref at the current level.
                                                 * If EBC fails to learn a chunk or justification, it's possible that it cannot find a pref
                                                 * for this level.*/
                                                preference* clone_for_this_pref = find_clone_for_level(pref, inst->match_goal_level);
                                                if (clone_for_this_pref)
                                                {
                                                    uniquely_add_to_head_of_dll(thisAgent, pref->inst);
                                                } else {
                                                    dprint(DT_GDS, "           ignoring inst %y because it is at a lower level than the GDS\n", pref->inst->prod_name);
                                                }
                                            }
                                            pref->inst->GDS_evaluated_already = true;
                                        }
                                        else
                                        {
                                            dprint(DT_GDS, "           the inst producing this pref was already explored; skipping it\n");
                                        }

                                    }
                                    else
                                    {
                                        dprint(DT_GDS, "        this inst is for a pref with a differnt value than the condition WME; skipping it\n");
                                    }
                                }  /* for pref = s->pref[ACCEPTABLE_PREF ...*/
                            }
                        }
                    }
                }
            }  /* for (cond = inst->top_of_instantiated_cond ...  *;*/

        /* remove just used instantiation from list */

        dprint(DT_GDS, "      removing instantiation: %y\n", curr_pi->inst->prod_name);

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

        dprint(DT_GDS, "    RECURSING using these parents:\n");
#ifdef DEBUG_GDS
        for (curr_pi = thisAgent->parent_list_head; curr_pi; curr_pi = curr_pi->next)
        {
            dprint(DT_GDS, "      %y\n", curr_pi->inst->prod_name);
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
    if (thisAgent->trace_settings[TRACE_GDS_STATE_REMOVAL_SYSPARAM])
    {
        // BADBAD: the XML code makes this all very ugly
        char msgbuf[256];
        memset(msgbuf, 0, 256);
        thisAgent->outputManager->sprinta_sf_cstr(thisAgent, msgbuf, 255, "Removing state %y because element in GDS changed. WME: ", w->gds->goal);
        thisAgent->outputManager->printa(thisAgent, msgbuf);

        xml_begin_tag(thisAgent, soar_TraceNames::kTagVerbose);
        xml_att_val(thisAgent, soar_TraceNames::kTypeString, msgbuf);
        print_wme(thisAgent, w); // prints XML, too
        xml_end_tag(thisAgent, soar_TraceNames::kTagVerbose);
    }

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    thisAgent->timers_gds.start();
#endif
#endif

    /* This call to GDS_PrintCmd will have to be uncommented later. -ajc */
    //if (thisAgent->outputManager->settings[OM_VERBOSE]) {} //GDS_PrintCmd();

    /* REW: BUG.  I have no idea right now if this is a terrible hack or
    * actually what we want to do.  The idea here is that the context of
    * the immediately higher goal above a retraction should be marked as
    * having its context changed in order that the architecture doesn't
    * look below this level for context changes.  I think it's a hack b/c
    * it seems like there should already be mechanisms for doing this in
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
        /* If nothing has yet changed (highest_ ... = NIL) then set the goal automatically */
        thisAgent->highest_goal_whose_context_changed = w->gds->goal->id->higher_goal;

        // Tell those slots they are changed so that the impasses can be regenerated bug 1011
        for (slot* s = thisAgent->highest_goal_whose_context_changed->id->slots; s != 0; s = s->next)
        {
            if (s->isa_context_slot && !s->changed)
            {
                s->changed = reinterpret_cast<dl_cons*>(1); // use non-zero value to indicate change, see definition of slot::changed
            }
        }
    }

    if (thisAgent->trace_settings[TRACE_GDS_STATE_REMOVAL_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent, "\n    REMOVING GOAL [%y] due to change in GDS WME ", w->gds->goal);
        print_wme(thisAgent, w);
    }

    remove_existing_context_and_descendents(thisAgent, w->gds->goal);

    /* BUG: Need to reset highest_goal here ???*/

    /* usually, we'd call do_buffered_wm_and_ownership_changes() here, but
    * we don't need to because it will be done at the end of the working
    * memory phase; cf. the end of do_working_memory_phase().
    */

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
    thisAgent->timers_gds.stop();
    thisAgent->timers_gds_cpu_time[thisAgent->current_phase].update(thisAgent->timers_gds);
#endif
#endif
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

    thisAgent->memoryManager->allocate_with_pool(MP_gds, &gds);

    gds->goal = goal;
    gds->wmes_in_gds = NIL;
    goal->id->gds = gds;

    GDI_add(thisAgent, gds);

    dprint(DT_GDS, "Created GDS for goal [%y].\n", gds->goal);
}
