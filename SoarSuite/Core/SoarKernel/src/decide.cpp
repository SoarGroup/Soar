#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"
#include "soar_rand.h" // provides SoarRand, a better random number generator (see bug 595)

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  decide.cpp
 *
 * =======================================================================
 *  Decider and  Associated Routines for Soar 6
 *
 *  This file contains the decider as well as routine for managing
 *  slots, and the garbage collection of disconnected WMEs.
 * =======================================================================
 */


/* Debugging stuff:  #define DEBUG_LINKS to get links, gc printouts */
/*                   #define DEBUG_SLOTS to get slot printouts */
/* #define DEBUG_LINKS */
/* #define DEBUG_SLOTS */

/* REW: begin 09.15.96 */
/* For low-level, detailed information on the processing of the GDS */
/* #define DEBUG_GDS */
/* For high-level information on the instantiations that created an
 * o-supported element and lead to the elaboration of the GDS */
/* #define DEBUG_GDS_HIGH */
/* REW: end   09.15.96 */

#include "decide.h"
#include "gdatastructs.h"
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
#include "io.h"

/* JC ADDED: This is for event firing in gSKI */
#include "gski_event_system_functions.h"

using namespace xmlTraceNames;

#ifdef NUMERIC_INDIFFERENCE
/* REW: 2003-01-02 Behavior Variability Kernel Experiments */
preference *probabilistically_select(agent* thisAgent, slot * s, preference * candidates);

/* REW: 2003-01-06 A temporary helper function */

void print_candidates(agent* thisAgent, preference * candidates)
{
    preference *cand = 0;
    int max_count = 0;

    for (cand = candidates; cand != NIL; cand = cand->next_candidate) {
        max_count++;
        print(thisAgent, "\n Candidate %d", cand);
        print_with_symbols(thisAgent, "\n    %y %y %y", cand->id, cand->attr, cand->value);
        if (max_count > 10)
            break;
    }
}

/* END: 2003-01-02 Behavior Variability Kernel Experiments */

#endif

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
#define UNARY_PARALLEL_DECIDER_FLAG 8
/* REW: 2003-01-02 Behavior Variability Kernel Experiments 
   A new preference type: unary indifferent + constant (probability) value
*/
#define UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG 9

/* ======================================================================

                  Acceptable Preference WME Routines

   Whenever some acceptable or require preference for a context slot
   changes, we call mark_context_slot_as_acceptable_preference_changed().

   At the end of the phase, do_buffered_acceptable_preference_wme_changes()
   is called to update the acceptable preference wmes.  This should be
   called *before* do_buffered_link_changes() and do_buffered_wm_changes().
====================================================================== */

void mark_context_slot_as_acceptable_preference_changed (agent* thisAgent, slot *s) {
  dl_cons *dc;
  
  if (s->acceptable_preference_changed) return;
  allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
  dc->item = s;
  s->acceptable_preference_changed = dc;
  insert_at_head_of_dll (thisAgent->context_slots_with_changed_acceptable_preferences, dc,
                         next, prev);
}

/* --- This updates the acceptable preference wmes for a single slot. --- */
void do_acceptable_preference_wme_changes_for_slot (agent* thisAgent, slot *s) 
{
  wme *w, *next_w;
  preference *p;

  /* --- first, reset marks to "NOTHING" --- */
  for (w=s->acceptable_preference_wmes; w!=NIL; w=w->next)
    w->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  
  /* --- now mark values for which we WANT a wme as "CANDIDATE" values --- */
  for (p=s->preferences[REQUIRE_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
  for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;

  /* --- remove any existing wme's that aren't CANDIDATEs; mark the
     rest as ALREADY_EXISTING --- */

  w = s->acceptable_preference_wmes;
  while (w) 
  {
    next_w = w->next;
    if (w->value->common.decider_flag==CANDIDATE_DECIDER_FLAG) {
      w->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
      w->value->common.a.decider_wme = w;
      w->preference = NIL;  /* we'll update this later */
    } else {
      remove_from_dll (s->acceptable_preference_wmes, w, next, prev);
/* REW: begin 09.15.96 */
      /* IF we lose an acceptable preference for an operator, then that
         operator comes out of the slot immediately in OPERAND2.
         However, if the lost acceptable preference is not for item
         in the slot, then we don;t need to do anything special until
         mini-quiescence. */
      if (thisAgent->operand2_mode)
         remove_operator_if_necessary(thisAgent, s,w);
/* REW: end   09.15.96 */
      remove_wme_from_wm (thisAgent, w);
    }
    w = next_w;
  }

  /* --- add the necessary wme's that don't ALREADY_EXIST --- */

  for (p=s->preferences[REQUIRE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
     if (p->value->common.decider_flag==ALREADY_EXISTING_WME_DECIDER_FLAG) {
        /* --- found existing wme, so just update its trace --- */
        w = p->value->common.a.decider_wme;
        if (! w->preference) w->preference = p;
     } else {
        w = make_wme (thisAgent, p->id, p->attr, p->value, TRUE);
        insert_at_head_of_dll (s->acceptable_preference_wmes, w, next, prev);
        w->preference = p;
        add_wme_to_wm (thisAgent, w);
        p->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
        p->value->common.a.decider_wme = w;
     }
  }
  for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
     if (p->value->common.decider_flag==ALREADY_EXISTING_WME_DECIDER_FLAG) {
        /* --- found existing wme, so just update its trace --- */
        w = p->value->common.a.decider_wme;
        if (! w->preference) w->preference = p;
     } else {
        w = make_wme (thisAgent, p->id, p->attr, p->value, TRUE);
        insert_at_head_of_dll (s->acceptable_preference_wmes, w, next, prev);
        w->preference = p;
        add_wme_to_wm (thisAgent, w);
        p->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
        p->value->common.a.decider_wme = w;
     }
  }
}

void do_buffered_acceptable_preference_wme_changes (agent* thisAgent) {
  dl_cons *dc;
  slot *s;

  while (thisAgent->context_slots_with_changed_acceptable_preferences) {
    dc = thisAgent->context_slots_with_changed_acceptable_preferences;
    thisAgent->context_slots_with_changed_acceptable_preferences = dc->next;
    s = static_cast<slot_struct *>(dc->item);
    free_with_pool (&thisAgent->dl_cons_pool, dc);
    do_acceptable_preference_wme_changes_for_slot (thisAgent, s);
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

void post_link_addition (agent* thisAgent, Symbol *from, Symbol *to) 
{
   
/* --- don't add links to goals/impasses, except the special one
   (NIL,goal) --- */
   if ((to->id.isa_goal || to->id.isa_impasse) && from) 
      return;
   
   to->id.link_count++;
   
#ifdef DEBUG_LINKS
   if (from)
      print_with_symbols (thisAgent, "\nAdding link from %y to %y", from, to);
   else
      print_with_symbols (thisAgent, "\nAdding special link to %y", to);
   print (" (count=%lu)", to->id.link_count);
#endif
   
   if (!from) 
      return;  /* if adding a special link, we're done */
   
   /* --- if adding link from same level, ignore it --- */
   if (from->id.promotion_level == to->id.promotion_level) 
      return;
   
   /* --- if adding link from lower to higher, mark higher accordingly --- */
   if (from->id.promotion_level > to->id.promotion_level) {
      to->id.could_be_a_link_from_below = TRUE;
      return;
   }
   
   /* --- otherwise buffer it for later --- */
   to->id.promotion_level = from->id.promotion_level;
   symbol_add_ref (to);
   push (thisAgent, to, thisAgent->promoted_ids);
}

/* ----------------------------------------------
   Promote an id and its transitive closure.
---------------------------------------------- */

#define promote_if_needed(thisAgent, sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      promote_id_and_tc(thisAgent, sym,new_level); }
                                    
void promote_id_and_tc (agent* thisAgent, Symbol *id, goal_stack_level new_level) {
  slot *s;
  preference *pref;
  wme *w;
  
  /* --- if it's already that high, or is going to be soon, don't bother -- */
  if (id->id.level <= new_level) return;
  if (id->id.promotion_level < new_level) return;

  /* --- update its level, etc. --- */
  id->id.level = new_level;
  id->id.promotion_level = new_level;
  id->id.could_be_a_link_from_below = TRUE;

  /* --- sanity check --- */
  if (id->id.isa_goal || id->id.isa_impasse) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "decide.c: Internal error: tried to promote a goal or impasse id\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
    /* Note--since we can't promote a goal, we don't have to worry about
       slot->acceptable_preference_wmes below */
  }
  
  /* --- scan through all preferences and wmes for all slots for this id -- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    promote_if_needed (thisAgent, w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
      promote_if_needed (thisAgent, pref->value);
      if (preference_is_binary(pref->type))
        promote_if_needed (thisAgent, pref->referent);
    }
    for (w=s->wmes; w!=NIL; w=w->next)
      promote_if_needed (thisAgent, w->value);
  } /* end of for slots loop */
}

/* ----------------------------------------------
   Do all buffered promotions.
---------------------------------------------- */

void do_promotion (agent* thisAgent) {
  cons *c;
  Symbol *to;

  while (thisAgent->promoted_ids) {
    c = thisAgent->promoted_ids;
    to = static_cast<symbol_union *>(c->first);
    thisAgent->promoted_ids = thisAgent->promoted_ids->rest;
    free_cons (thisAgent, c);
    promote_id_and_tc (thisAgent, to, to->id.promotion_level);
    symbol_remove_ref (thisAgent, to);
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

void post_link_removal (agent* thisAgent, Symbol *from, Symbol *to) 
{
  dl_cons *dc;

  /* --- don't remove links to goals/impasses, except the special one
     (NIL,goal) --- */
  if ((to->id.isa_goal || to->id.isa_impasse) && from) return;

  to->id.link_count--;

#ifdef DEBUG_LINKS
  if (from) {
    print_with_symbols (thisAgent, "\nRemoving link from %y to %y", from, to);
    print (" (%d to %d)", from->id.level, to->id.level);
  } else {
    print_with_symbols (thisAgent, S"\nRemoving special link to %y ", to);
    print (" (%d)", to->id.level);
  }
  print (" (count=%lu)", to->id.link_count);
#endif

  /* --- if a gc is in progress, handle differently --- */
  if (thisAgent->link_update_mode==JUST_UPDATE_COUNT) return;

  if ((thisAgent->link_update_mode==UPDATE_DISCONNECTED_IDS_LIST) &&
      (to->id.link_count==0)) {
    if (to->id.unknown_level) {
      dc = to->id.unknown_level;
      remove_from_dll (thisAgent->ids_with_unknown_level, dc, next, prev);
      insert_at_head_of_dll (thisAgent->disconnected_ids, dc, next, prev);
    } else {
      symbol_add_ref (to);
      allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
      dc->item = to;
      to->id.unknown_level = dc;   
      insert_at_head_of_dll (thisAgent->disconnected_ids, dc, next, prev);
    }
    return;
  }
    
  /* --- if removing a link from a different level, there must be some other
     link at the same level, so we can ignore this change --- */
  if (from && (from->id.level != to->id.level)) return;
  
  if (! to->id.unknown_level) {
    symbol_add_ref (to);
    allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
    dc->item = to;
    to->id.unknown_level = dc;   
    insert_at_head_of_dll (thisAgent->ids_with_unknown_level, dc, next, prev);
  }
}

/* ----------------------------------------------
   Garbage collect an identifier.  This removes
   all wmes, input wmes, and preferences for that
   id from TM.
---------------------------------------------- */

void garbage_collect_id (agent* thisAgent, Symbol *id) 
{
   slot *s;
   preference *pref, *next_pref;
   
   /* JC ADDED: Tell gSKI that an object is being removed from memory */
   /* KJC:  Do we really want this here?  This is garbage collection, not WM operations */
   gSKI_MakeAgentCallback(gSKI_K_EVENT_WMOBJECT_REMOVED, 0, thisAgent, static_cast<void*>(id));

#ifdef DEBUG_LINKS  
   print_with_symbols (thisAgent, "\n*** Garbage collecting id: %y",id);
#endif
   
   /* Note--for goal/impasse id's, this does not remove the impasse wme's.
       This is handled by remove_existing_such-and-such... */
   
   /* --- remove any input wmes from the id --- */
   remove_wme_list_from_wm (thisAgent, id->id.input_wmes);
   id->id.input_wmes = NIL;
   
   for (s = id->id.slots; s != NIL; s = s->next) 
   {
      /* --- remove any existing attribute impasse for the slot --- */
      if (s->impasse_type != NONE_IMPASSE_TYPE)
         remove_existing_attribute_impasse_for_slot (thisAgent, s);
      
      /* --- remove all wme's from the slot --- */
      remove_wme_list_from_wm (thisAgent, s->wmes);
      s->wmes = NIL;
      
      /* --- remove all preferences for the slot --- */
      pref = s->all_preferences;
      while (pref) 
      {
         next_pref = pref->all_of_slot_next;
         remove_preference_from_tm (thisAgent, pref);
         
         /* Note:  the call to remove_preference_from_slot handles the removal
         of acceptable_preference_wmes */
         pref = next_pref;
      }

      mark_slot_for_possible_removal (thisAgent, s);
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

void mark_id_and_tc_as_unknown_level (agent* thisAgent, Symbol *id);

/*#define mark_unknown_level_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      mark_id_and_tc_as_unknown_level(sym); }*/
inline void mark_unknown_level_if_needed(agent* thisAgent, Symbol * sym)
{
  if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
      mark_id_and_tc_as_unknown_level(thisAgent, sym);
}

void mark_id_and_tc_as_unknown_level (agent* thisAgent, Symbol *id) {
  slot *s;
  preference *pref;
  wme *w;
  dl_cons *dc;

  /* --- if id is already marked, do nothing --- */
  if (id->id.tc_num==thisAgent->mark_tc_number) return;
  
  /* --- don't mark anything higher up as disconnected--in order to be higher
     up, it must have a link to it up there --- */
  if (id->id.level < thisAgent->level_at_which_marking_started) return; 

  /* --- mark id, so we won't do it again later --- */
  id->id.tc_num = thisAgent->mark_tc_number;

  /* --- update range of goal stack levels we'll need to walk --- */
  if (id->id.level < thisAgent->highest_level_anything_could_fall_from)
    thisAgent->highest_level_anything_could_fall_from = id->id.level;
  if (id->id.level > thisAgent->lowest_level_anything_could_fall_to)
    thisAgent->lowest_level_anything_could_fall_to = id->id.level;
  if (id->id.could_be_a_link_from_below)
    thisAgent->lowest_level_anything_could_fall_to = LOWEST_POSSIBLE_GOAL_LEVEL;

  /* --- add id to the set of ids with unknown level --- */
  if (! id->id.unknown_level) {
    allocate_with_pool (thisAgent, &thisAgent->dl_cons_pool, &dc);
    dc->item = id;
    id->id.unknown_level = dc;
    insert_at_head_of_dll (thisAgent->ids_with_unknown_level, dc, next, prev);
    symbol_add_ref (id);
  }

  /* -- scan through all preferences and wmes for all slots for this id -- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    mark_unknown_level_if_needed (thisAgent, w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
      mark_unknown_level_if_needed (thisAgent, pref->value);
      if (preference_is_binary(pref->type))
        mark_unknown_level_if_needed (thisAgent, pref->referent);
    }
    if(s->impasse_id) mark_unknown_level_if_needed(thisAgent, s->impasse_id);
    for (w=s->wmes; w!=NIL; w=w->next)
      mark_unknown_level_if_needed (thisAgent, w->value);
  } /* end of for slots loop */
}

/* ----------------------------------------------
   After marking the ids with unknown level,
   we walk various levels of the goal stack,
   higher level to lower level.  If, while doing
   the walk, we encounter an id marked as having
   an unknown level, we update its level and
   remove it from ids_with_unknown_level.
---------------------------------------------- */

void walk_and_update_levels (agent* thisAgent, Symbol *id);
/*#define update_levels_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      if ((sym)->id.tc_num!=thisAgent->walk_tc_number) \
        walk_and_update_levels(sym); }*/
inline void update_levels_if_needed(agent* thisAgent, Symbol * sym)
{
  if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
    if ((sym)->id.tc_num!=thisAgent->walk_tc_number)
      walk_and_update_levels(thisAgent, sym);
}

void walk_and_update_levels (agent* thisAgent, Symbol *id) {
  slot *s;
  preference *pref;
  wme *w;
  dl_cons *dc;

  /* --- mark id so we don't walk it twice --- */
  id->id.tc_num = thisAgent->walk_tc_number;

  /* --- if we already know its level, and it's higher up, then exit --- */
  if ((! id->id.unknown_level) && (id->id.level < thisAgent->walk_level)) return;

  /* --- if we didn't know its level before, we do now --- */
  if (id->id.unknown_level) {
    dc = id->id.unknown_level;
    remove_from_dll (thisAgent->ids_with_unknown_level, dc, next, prev);
    free_with_pool (&thisAgent->dl_cons_pool, dc);
    symbol_remove_ref (thisAgent, id);
    id->id.unknown_level = NIL;
    id->id.level = thisAgent->walk_level;
    id->id.promotion_level = thisAgent->walk_level;
  }
  
  /* -- scan through all preferences and wmes for all slots for this id -- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    update_levels_if_needed (thisAgent, w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
      update_levels_if_needed (thisAgent, pref->value);
      if (preference_is_binary(pref->type))
        update_levels_if_needed (thisAgent, pref->referent);
    }
    if(s->impasse_id) update_levels_if_needed(thisAgent, s->impasse_id);
    for (w=s->wmes; w!=NIL; w=w->next)
      update_levels_if_needed (thisAgent, w->value);
  } /* end of for slots loop */
}

/* ----------------------------------------------
   Do all buffered demotions and gc's.
---------------------------------------------- */

void do_demotion (agent* thisAgent) {
  Symbol *g, *id;
  dl_cons *dc, *next_dc;

  /* --- scan through ids_with_unknown_level, move the ones with link_count==0
   *  over to disconnected_ids --- */
  for (dc=thisAgent->ids_with_unknown_level; dc!=NIL; dc=next_dc) {
    next_dc = dc->next;
    id = static_cast<symbol_union *>(dc->item);
    if (id->id.link_count==0) {
      remove_from_dll (thisAgent->ids_with_unknown_level, dc, next, prev);
      insert_at_head_of_dll (thisAgent->disconnected_ids, dc, next, prev);
    }
  }

  /* --- keep garbage collecting ids until nothing left to gc --- */
  thisAgent->link_update_mode = UPDATE_DISCONNECTED_IDS_LIST;
  while (thisAgent->disconnected_ids) {
    dc = thisAgent->disconnected_ids;
    thisAgent->disconnected_ids = thisAgent->disconnected_ids->next;
    id = static_cast<symbol_union *>(dc->item);
    free_with_pool (&thisAgent->dl_cons_pool, dc);
    garbage_collect_id (thisAgent, id);
    symbol_remove_ref (thisAgent, id);
  }
  thisAgent->link_update_mode = UPDATE_LINKS_NORMALLY;
 
  /* --- if nothing's left with an unknown level, we're done --- */
  if (! thisAgent->ids_with_unknown_level) return;

  /* --- do the mark --- */
  thisAgent->highest_level_anything_could_fall_from =
                LOWEST_POSSIBLE_GOAL_LEVEL;
  thisAgent->lowest_level_anything_could_fall_to = -1;
  thisAgent->mark_tc_number = get_new_tc_number(thisAgent);
  for (dc=thisAgent->ids_with_unknown_level; dc!=NIL; dc=dc->next) {
    id = static_cast<symbol_union *>(dc->item);
    thisAgent->level_at_which_marking_started = id->id.level;
    mark_id_and_tc_as_unknown_level (thisAgent, id);
  }

  /* --- do the walk --- */
  g = thisAgent->top_goal;
  while (TRUE) {
    if (!g) break;
    if (g->id.level > thisAgent->lowest_level_anything_could_fall_to) break;
    if (g->id.level >= thisAgent->highest_level_anything_could_fall_from) {
      thisAgent->walk_level = g->id.level;
      thisAgent->walk_tc_number = get_new_tc_number(thisAgent);
      walk_and_update_levels (thisAgent, g);
    }
    g = g->id.lower_goal;
  }

  /* --- GC anything left with an unknown level after the walk --- */
  thisAgent->link_update_mode = JUST_UPDATE_COUNT;
  while (thisAgent->ids_with_unknown_level) {
    dc = thisAgent->ids_with_unknown_level;
    thisAgent->ids_with_unknown_level =
                  thisAgent->ids_with_unknown_level->next;
    id = static_cast<symbol_union *>(dc->item);
    free_with_pool (&thisAgent->dl_cons_pool, dc);
    id->id.unknown_level = NIL;    /* AGR 640:  GAP set to NIL because */
                                   /* symbol may still have pointers to it */
    garbage_collect_id (thisAgent, id);
    symbol_remove_ref (thisAgent, id);
  }
  thisAgent->link_update_mode = UPDATE_LINKS_NORMALLY;
}

/* ------------------------------------------------------------------
                       Do Buffered Link Changes

   This routine does all the buffered link (ownership) chages, updating
   the goal stack level on all identifiers and garbage collecting
   disconnected wmes.
------------------------------------------------------------------ */

void do_buffered_link_changes (agent* thisAgent) {

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  struct timeval saved_start_tv;
#endif
#endif

  /* --- if no promotions or demotions are buffered, do nothing --- */
  if (! (thisAgent->promoted_ids ||
         thisAgent->ids_with_unknown_level ||
         thisAgent->disconnected_ids)) return;

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS  
  start_timer (&saved_start_tv);
#endif
#endif
  do_promotion (thisAgent);  
  do_demotion (thisAgent);
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  stop_timer (&saved_start_tv, &thisAgent->ownership_cpu_time[thisAgent->current_phase]);
#endif
#endif
}

/* **************************************************************************

                         Preference Semantics 

   Run_preference_semantics (slot *s, preference **result_candidates) examines
   the preferences for a given slot, and returns an impasse type for the
   slot.  The argument "result_candidates" is set to a list of candidate
   values for the slot--if the returned impasse type is NONE_IMPASSE_TYPE,
   this is the set of winners; otherwise it is the set of tied, conflicted,
   or constraint-failured values.  This list of values is a list of preferences
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
   preference.  This case is very obsure and unlikely to come up, but it
   could easily cause a core dump or worse.
   
   Require_preference_semantics() is a helper function for
   run_preference_semantics() that is used when there is at least one
   require preference for the slot.
************************************************************************** */

byte require_preference_semantics (slot *s, preference **result_candidates) {
  preference *p;
  preference *candidates;
  Symbol *value;
  
  /* --- collect set of required items into candidates list --- */
  for (p=s->preferences[REQUIRE_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  candidates = NIL;
  for (p=s->preferences[REQUIRE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
    if (p->value->common.decider_flag == NOTHING_DECIDER_FLAG) {
      p->next_candidate = candidates;
      candidates = p;
      /* --- unmark it, in order to prevent it from being added twice --- */
      p->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
    }
  }
  *result_candidates = candidates;
  
  /* --- if more than one required item, we have a constraint failure --- */
  if (candidates->next_candidate) return CONSTRAINT_FAILURE_IMPASSE_TYPE;
  
  /* --- just one require, check for require-prohibit impasse --- */
  value = candidates->value;
  for (p=s->preferences[PROHIBIT_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    if (p->value == value) return CONSTRAINT_FAILURE_IMPASSE_TYPE;
  
  /* --- the lone require is the winner --- */
  return NONE_IMPASSE_TYPE;
}

byte run_preference_semantics (agent* thisAgent, slot *s, preference **result_candidates) 
{
  preference *p, *p2, *cand, *prev_cand;
  Bool match_found, not_all_indifferent, not_all_parallel;
  preference *candidates;

  /* --- if the slot has no preferences at all, things are trivial --- */
  if (!s->all_preferences) 
  {
    if (! s->isa_context_slot) mark_slot_for_possible_removal (thisAgent, s);
    *result_candidates = NIL;
    return NONE_IMPASSE_TYPE;
  }
  
  /* === Requires === */
  if (s->preferences[REQUIRE_PREFERENCE_TYPE]) {
    return require_preference_semantics (s, result_candidates);
  }
    
  /* === Acceptables, Prohibits, Rejects === */

  /* --- mark everything that's acceptable, then unmark the prohibited
         and rejected items --- */
  for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
  for (p=s->preferences[PROHIBIT_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  for (p=s->preferences[REJECT_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = NOTHING_DECIDER_FLAG;

  /* --- now scan through acceptables and build the list of candidates --- */
  candidates = NIL;
  for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
    if (p->value->common.decider_flag == CANDIDATE_DECIDER_FLAG) {
      p->next_candidate = candidates;
      candidates = p;
      /* --- unmark it, in order to prevent it from being added twice --- */
      p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    }
  }

  /* === Handling of attribute_preferences_mode 2 === */
  if ( ( (thisAgent->attribute_preferences_mode==2) ||
			(thisAgent->operand2_mode ==TRUE) )  &&
		 (! s->isa_context_slot)) {
    *result_candidates = candidates;
    return NONE_IMPASSE_TYPE;
  }
       
  /* === If there are only 0 or 1 candidates, we're done === */
  if ((!candidates) || (! candidates->next_candidate)) {
    *result_candidates = candidates;
    return NONE_IMPASSE_TYPE;
  }

  /* === Better/Worse === */
  if (s->preferences[BETTER_PREFERENCE_TYPE] ||
      s->preferences[WORSE_PREFERENCE_TYPE]) {
    Symbol *j, *k;

    /* -------------------- Algorithm to find conflicted set: 
      conflicted = {}
      for each (j > k):
        if j is (candidate or conflicted)
           and k is (candidate or conflicted)
           and at least one of j,k is a candidate
          then if (k > j) or (j < k) then
            conflicted += j, if not already true
            conflicted += k, if not already true
            candidate -= j, if not already true
            candidate -= k, if not already true
      for each (j < k):
        if j is (candidate or conflicted)
           and k is (candidate or conflicted)
           and at least one of j,k is a candidate
           then if (k < j)
             then
                conflicted += j, if not already true
                conflicted += k, if not already true
                candidate -= j, if not already true
                candidate -= k, if not already true
      ----------------------- */
    
    for (p=s->preferences[BETTER_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
      p->referent->common.decider_flag = NOTHING_DECIDER_FLAG;
    }
    for (p=s->preferences[WORSE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
      p->referent->common.decider_flag = NOTHING_DECIDER_FLAG;
    }
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
      cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
    }
    for (p=s->preferences[BETTER_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      j = p->value;
      k = p->referent;
      if (j==k) continue;
      if (j->common.decider_flag && k->common.decider_flag) {
        if(k->common.decider_flag != CONFLICTED_DECIDER_FLAG)
          k->common.decider_flag = FORMER_CANDIDATE_DECIDER_FLAG;
        if ((j->common.decider_flag!=CONFLICTED_DECIDER_FLAG) ||
            (k->common.decider_flag!=CONFLICTED_DECIDER_FLAG)) {
          for (p2=s->preferences[BETTER_PREFERENCE_TYPE]; p2; p2=p2->next)
            if ((p2->value==k)&&(p2->referent==j)) {
              j->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              k->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              break;
            }
          for (p2=s->preferences[WORSE_PREFERENCE_TYPE]; p2; p2=p2->next)
            if ((p2->value==j)&&(p2->referent==k)) {
              j->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              k->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              break;
            }
        }
      }
    }
    for (p=s->preferences[WORSE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      j = p->value;
      k = p->referent;
      if (j==k) continue;
      if (j->common.decider_flag && k->common.decider_flag) {
        if(j->common.decider_flag != CONFLICTED_DECIDER_FLAG)
          j->common.decider_flag = FORMER_CANDIDATE_DECIDER_FLAG;
        if ((j->common.decider_flag!=CONFLICTED_DECIDER_FLAG) ||
            (k->common.decider_flag!=CONFLICTED_DECIDER_FLAG)) {
          for (p2=s->preferences[WORSE_PREFERENCE_TYPE]; p2; p2=p2->next)
            if ((p2->value==k)&&(p2->referent==j)) {
              j->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              k->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              break;
            }
        }
      }
    }
    
    /* --- now scan through candidates list, look for conflicted stuff --- */
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      if (cand->value->common.decider_flag==CONFLICTED_DECIDER_FLAG) break;
    if (cand) {
      /* --- collect conflicted candidates into new candidates list --- */
      prev_cand = NIL;
      cand = candidates;
      while (cand) {
        if (cand->value->common.decider_flag != CONFLICTED_DECIDER_FLAG) {
          if (prev_cand)
            prev_cand->next_candidate = cand->next_candidate;
          else
            candidates = cand->next_candidate;
        } else {
          prev_cand = cand;
        }
        cand = cand->next_candidate;
      }
      *result_candidates = candidates;
      return CONFLICT_IMPASSE_TYPE;
    }
    /* --- no conflicts found, remove former_candidates from candidates --- */
    prev_cand = NIL;
    cand = candidates;
    while (cand) {
      if (cand->value->common.decider_flag == FORMER_CANDIDATE_DECIDER_FLAG) {
        if (prev_cand)
          prev_cand->next_candidate = cand->next_candidate;
        else
          candidates = cand->next_candidate;
      } else {
        prev_cand = cand;
      }
      cand = cand->next_candidate;
    }
  }
  
  /* === Bests === */
  if (s->preferences[BEST_PREFERENCE_TYPE]) {
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    for (p=s->preferences[BEST_PREFERENCE_TYPE]; p!=NIL; p=p->next)
      p->value->common.decider_flag = BEST_DECIDER_FLAG;
    prev_cand = NIL;
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      if (cand->value->common.decider_flag == BEST_DECIDER_FLAG) {
        if (prev_cand)
          prev_cand->next_candidate = cand;
        else
          candidates = cand;
        prev_cand = cand;
      }
    if (prev_cand) prev_cand->next_candidate = NIL;
  }
  
  /* === Worsts === */
  if (s->preferences[WORST_PREFERENCE_TYPE]) {
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    for (p=s->preferences[WORST_PREFERENCE_TYPE]; p!=NIL; p=p->next)
      p->value->common.decider_flag = WORST_DECIDER_FLAG;
    prev_cand = NIL;
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      if (cand->value->common.decider_flag != WORST_DECIDER_FLAG) {
        if (prev_cand)
          prev_cand->next_candidate = cand;
        else
          candidates = cand;
        prev_cand = cand;
      }
    if (prev_cand) prev_cand->next_candidate = NIL;
  }
  
  /* === If there are only 0 or 1 candidates, we're done === */
  if ((!candidates) || (! candidates->next_candidate)) {
    *result_candidates = candidates;
    return NONE_IMPASSE_TYPE;
  }

  /* === Indifferents === */
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
    cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  for (p=s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]; p; p=p->next)
    p->value->common.decider_flag = UNARY_INDIFFERENT_DECIDER_FLAG;

	 #ifdef NUMERIC_INDIFFERENCE
    /* REW: 2003-01-02 Behavior Variability Kernel Experiments
     We want to treat some binary indifferent prefs as unary indifferents,
     the second pref is really an int representing a probability value.
     So we identify these preferences here.
  */
	for (p=s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p; p=p->next)
    if((p->referent->fc.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE) || 
	   (p->referent->fc.common_symbol_info.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE))
     
      p->value->common.decider_flag = UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG;
  
  /* END: 2003-01-02 Behavior Variability Kernel Experiments  */

	#endif

  not_all_indifferent = FALSE;
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
    /* --- if cand is unary indifferent, it's fine --- */
    if (cand->value->common.decider_flag==UNARY_INDIFFERENT_DECIDER_FLAG)
      continue;
    
	#ifdef NUMERIC_INDIFFERENCE
	else if ( cand->value->common.decider_flag==UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG )
	  continue;
	#endif

    /* --- check whether cand is binary indifferent to each other one --- */
    for (p=candidates; p!=NIL; p=p->next_candidate) {
      if (p==cand) continue;
      match_found = FALSE;
      for (p2=s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p2!=NIL;
           p2=p2->next)
        if ( ((p2->value==cand->value)&&(p2->referent==p->value)) ||
             ((p2->value==p->value)&&(p2->referent==cand->value)) ) {
          match_found = TRUE;
          break;
        }
      if (!match_found) {
        not_all_indifferent = TRUE;
        break;
      }
    } /* end of for p loop */
    if (not_all_indifferent) break;
  } /* end of for cand loop */

  if (! not_all_indifferent) {
    /* --- items all indifferent, so just pick one of them to return --- */
    /* RBD 4/13/95 Removed code that looked for an existing value already in
       working memory for this slot, and returned it if found.  This was
       apparently an attempt to "stabilize" working memory if attribute
       preferences kept changing, but it ended up getting in the way, esp.
       with mutually indifferent operators were used with user-select
       random. */
    /* --- choose according to user-select --- */
    switch (thisAgent->sysparams[USER_SELECT_MODE_SYSPARAM]) {
    case USER_SELECT_FIRST:
      *result_candidates = candidates;
      break;
/* AGR 615 begin */
    case USER_SELECT_LAST:
      /* The test to see if candidates is NIL is done just before the
	 indifferent preferences processing begins.  The only place
	 between there and here that candidates is changed is immediately
	 followed by a return statement, so we can assume here that
	 candidates is not NIL.  AGR 94.11.09 */
      for (cand = candidates; cand->next_candidate != NIL; cand = cand->next_candidate);
      *result_candidates = cand;
      break;
/* AGR 615 end */
    case USER_SELECT_ASK: {
      int num_candidates, chosen_num;
      num_candidates = 0;
      print (thisAgent, "\nPlease choose one of the following:\n");
      for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
        num_candidates++;
        print (thisAgent, "  %d:  ", num_candidates);
        print_object_trace (thisAgent, cand->value);
        print (thisAgent, "\n");
      }
/* AGR 615 begin */
      print(thisAgent, "Or choose one of the following to change the user-select mode\n");
      print(thisAgent, "to something else:  %d (first), %d (last), %d (random)\n",
	     num_candidates+=1, num_candidates+=1, num_candidates+=1);
/* AGR 615 end */
      while (TRUE) {
        char ch;
//#ifdef _WINDOWS
//		char buff[256],msg[256];
//		snprintf(msg,256,"Enter selection 1-%d",num_candidates);
//		msg[255] = 0; /* ensure null termination */
//
//		get_line_from_window(msg,buff,255);
//		sscanf(msg,"%d",num_candidates);
//#else
	//  char buf[256]; /* kjh(CUSP-B10) */
        print (thisAgent, "Enter selection (1-%d): ", num_candidates);
        chosen_num = -1;
        scanf (" %d", &chosen_num);
        do { ch=getchar(); } while ((ch!='\n') && (ch!=EOF_AS_CHAR));

	if (ch==EOF_AS_CHAR) clearerr(stdin); /* Soar-Bugs #103, TMH */
	
     /* kjh(CUSP-B10) BEGIN*/
     /* Soar_Read(thisAgent, buf, 256);
	          sscanf(buf,"%d",&chosen_num); */
     /* kjh(CUSP-B10) END*/

//#endif
        if ((chosen_num>=1) && (chosen_num<=num_candidates)) break;
        print (thisAgent, "You must enter a number between 1 and %d\n", num_candidates);
      }
      if (thisAgent->logging_to_file) {
        char temp[50];
        snprintf (temp,50, "%d\n", chosen_num);
		temp[49] = 0; /* ensure null termination */

        print_string_to_log_file_only (thisAgent, temp);
      }
/* AGR 615 begin */
      switch (num_candidates - chosen_num) {
      case 2:
	set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
	print (thisAgent, "User-select mode changed to:  first\n");
	*result_candidates = candidates;
	break;
      case 1:
	set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
	print (thisAgent, "User-select mode changed to:  last\n");
	for (cand = candidates; cand->next_candidate != NIL; cand = cand->next_candidate);
	*result_candidates = cand;
	break;
      case 0:
	set_sysparam (thisAgent, USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
	print (thisAgent, "User-select mode changed to:  random\n");
	
	    /* RPM 12/05 replacing calls to rand() with calls to SoarRand; see bug 595 */
        //chosen_num = rand() % (num_candidates-3); // generates an integer in [0,num_candidates-3)
	    chosen_num = SoarRandInt(num_candidates-4); // generates an integer in [0,num_candidates-4]

	cand = candidates;
	while (chosen_num) { cand=cand->next_candidate; chosen_num--; }
	*result_candidates = cand;
	break;
      default:
	cand = candidates;
	while (chosen_num>1) { cand=cand->next_candidate; chosen_num--; }
	*result_candidates = cand;
      }
/* AGR 615 end */
      break;
    }
    case USER_SELECT_RANDOM: {

#ifdef NUMERIC_INDIFFERENCE
                /* REW: 2003-01-02 Behavior Variability Kernel Experiments */
                cand = probabilistically_select(thisAgent, s, candidates);
                if (!cand) {
                    *result_candidates = candidates;
                    return TIE_IMPASSE_TYPE;
                }
                *result_candidates = cand;
                break;
#else
      int num_candidates, chosen_num;
      num_candidates = 0;
      for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
        num_candidates++;

	  /* RPM 12/05 replacing calls to rand() with calls to SoarRand; see bug 595 */
      //chosen_num = rand() % num_candidates;
	  chosen_num = SoarRand.randInt(num_candidates-1);

      cand = candidates;
      while (chosen_num) { cand=cand->next_candidate; chosen_num--; }
      *result_candidates = cand;
      break;
#endif
    }
    default:
      { char msg[BUFFER_MSG_SIZE];
      snprintf(msg, BUFFER_MSG_SIZE, "decide.c: Error: bad value of user_select_mode: %d\n",
	      (int)thisAgent->sysparams[USER_SELECT_MODE_SYSPARAM]);
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error(thisAgent, msg);
      }
    }
    (*result_candidates)->next_candidate = NIL;
    return NONE_IMPASSE_TYPE;
  }
  
  /* --- items not all indifferent; for context slots this gives a tie --- */
  if (s->isa_context_slot) {
    *result_candidates = candidates;
    return TIE_IMPASSE_TYPE;
  }

  /* === Parallels === */
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
    cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  for (p=s->preferences[UNARY_PARALLEL_PREFERENCE_TYPE]; p; p=p->next)
    p->value->common.decider_flag = UNARY_PARALLEL_DECIDER_FLAG;
  not_all_parallel = FALSE;
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
    /* --- if cand is unary parallel, it's fine --- */
    if (cand->value->common.decider_flag==UNARY_PARALLEL_DECIDER_FLAG)
      continue;
    /* --- check whether cand is binary parallel to each other candidate --- */
    for (p=candidates; p!=NIL; p=p->next_candidate) {
      if (p==cand) continue;
      match_found = FALSE;
      for (p2=s->preferences[BINARY_PARALLEL_PREFERENCE_TYPE]; p2!=NIL;
           p2=p2->next)
        if ( ((p2->value==cand->value)&&(p2->referent==p->value)) ||
             ((p2->value==p->value)&&(p2->referent==cand->value)) ) {
          match_found = TRUE;
          break;
        }
      if (!match_found) {
        not_all_parallel = TRUE;
        break;
      }
    } /* end of for p loop */
    if (not_all_parallel) break;
  } /* end of for cand loop */

  *result_candidates = candidates;

  if (! not_all_parallel) {
    /* --- items are all parallel, so return them all --- */
    return NONE_IMPASSE_TYPE;
  }

  /* --- otherwise we have a tie --- */
  return TIE_IMPASSE_TYPE;
}


byte run_preference_semantics_for_consistency_check (agent* thisAgent, slot *s, preference **result_candidates) {
  preference *p, *p2, *cand, *prev_cand;
  Bool match_found, not_all_indifferent, not_all_parallel;
  preference *candidates;

  /* printf("\n       Checking the preference semantics for inconsistencies....\n"); */
  /* --- if the slot has no preferences at all, things are trivial --- */
  if (! s->all_preferences) {
    if (! s->isa_context_slot) mark_slot_for_possible_removal (thisAgent, s);
    *result_candidates = NIL;
    return NONE_IMPASSE_TYPE;
  }
  
  /* === Requires === */
  if (s->preferences[REQUIRE_PREFERENCE_TYPE]) {
    return require_preference_semantics (s, result_candidates);
  }
    
  /* === Acceptables, Prohibits, Rejects === */

  /* --- mark everything that's acceptable, then unmark the prohibited
         and rejected items --- */
  for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
  for (p=s->preferences[PROHIBIT_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  for (p=s->preferences[REJECT_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    p->value->common.decider_flag = NOTHING_DECIDER_FLAG;

  /* --- now scan through acceptables and build the list of candidates --- */
  candidates = NIL;
  for (p=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
    if (p->value->common.decider_flag == CANDIDATE_DECIDER_FLAG) {
      p->next_candidate = candidates;
      candidates = p;
      /* --- unmark it, in order to prevent it from being added twice --- */
      p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    }
  }

  /* === Handling of attribute_preferences_mode 2 === */
  if (( (thisAgent->attribute_preferences_mode==2) ||
		  (thisAgent->operand2_mode == TRUE) ) &&
      (! s->isa_context_slot)) {
    *result_candidates = candidates;
    return NONE_IMPASSE_TYPE;
  }
       
  /* === If there are only 0 or 1 candidates, we're done === */
  if ((!candidates) || (! candidates->next_candidate)) {
    *result_candidates = candidates;
    return NONE_IMPASSE_TYPE;
  }

  /* === Better/Worse === */
  if (s->preferences[BETTER_PREFERENCE_TYPE] ||
      s->preferences[WORSE_PREFERENCE_TYPE]) {
    Symbol *j, *k;

    /* -------------------- Algorithm to find conflicted set: 
      conflicted = {}
      for each (j > k):
        if j is (candidate or conflicted)
           and k is (candidate or conflicted)
           and at least one of j,k is a candidate
          then if (k > j) or (j < k) then
            conflicted += j, if not already true
            conflicted += k, if not already true
            candidate -= j, if not already true
            candidate -= k, if not already true
      for each (j < k):
        if j is (candidate or conflicted)
           and k is (candidate or conflicted)
           and at least one of j,k is a candidate
           then if (k < j)
             then
                conflicted += j, if not already true
                conflicted += k, if not already true
                candidate -= j, if not already true
                candidate -= k, if not already true
      ----------------------- */
    
    for (p=s->preferences[BETTER_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
      p->referent->common.decider_flag = NOTHING_DECIDER_FLAG;
    }
    for (p=s->preferences[WORSE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      p->value->common.decider_flag = NOTHING_DECIDER_FLAG;
      p->referent->common.decider_flag = NOTHING_DECIDER_FLAG;
    }
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
      cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
    }
    for (p=s->preferences[BETTER_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      j = p->value;
      k = p->referent;
      if (j==k) continue;
      if (j->common.decider_flag && k->common.decider_flag) {
        if(k->common.decider_flag != CONFLICTED_DECIDER_FLAG)
          k->common.decider_flag = FORMER_CANDIDATE_DECIDER_FLAG;
        if ((j->common.decider_flag!=CONFLICTED_DECIDER_FLAG) ||
            (k->common.decider_flag!=CONFLICTED_DECIDER_FLAG)) {
          for (p2=s->preferences[BETTER_PREFERENCE_TYPE]; p2; p2=p2->next)
            if ((p2->value==k)&&(p2->referent==j)) {
              j->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              k->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              break;
            }
          for (p2=s->preferences[WORSE_PREFERENCE_TYPE]; p2; p2=p2->next)
            if ((p2->value==j)&&(p2->referent==k)) {
              j->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              k->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              break;
            }
        }
      }
    }
    for (p=s->preferences[WORSE_PREFERENCE_TYPE]; p!=NIL; p=p->next) {
      j = p->value;
      k = p->referent;
      if (j==k) continue;
      if (j->common.decider_flag && k->common.decider_flag) {
        if(j->common.decider_flag != CONFLICTED_DECIDER_FLAG)
          j->common.decider_flag = FORMER_CANDIDATE_DECIDER_FLAG;
        if ((j->common.decider_flag!=CONFLICTED_DECIDER_FLAG) ||
            (k->common.decider_flag!=CONFLICTED_DECIDER_FLAG)) {
          for (p2=s->preferences[WORSE_PREFERENCE_TYPE]; p2; p2=p2->next)
            if ((p2->value==k)&&(p2->referent==j)) {
              j->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              k->common.decider_flag = CONFLICTED_DECIDER_FLAG;
              break;
            }
        }
      }
    }
    
    /* --- now scan through candidates list, look for conflicted stuff --- */
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      if (cand->value->common.decider_flag==CONFLICTED_DECIDER_FLAG) break;
    if (cand) {
      /* --- collect conflicted candidates into new candidates list --- */
      prev_cand = NIL;
      cand = candidates;
      while (cand) {
        if (cand->value->common.decider_flag != CONFLICTED_DECIDER_FLAG) {
          if (prev_cand)
            prev_cand->next_candidate = cand->next_candidate;
          else
            candidates = cand->next_candidate;
        } else {
          prev_cand = cand;
        }
        cand = cand->next_candidate;
      }
      *result_candidates = candidates;
      return CONFLICT_IMPASSE_TYPE;
    }
    /* --- no conflicts found, remove former_candidates from candidates --- */
    prev_cand = NIL;
    cand = candidates;
    while (cand) {
      if (cand->value->common.decider_flag == FORMER_CANDIDATE_DECIDER_FLAG) {
        if (prev_cand)
          prev_cand->next_candidate = cand->next_candidate;
        else
          candidates = cand->next_candidate;
      } else {
        prev_cand = cand;
      }
      cand = cand->next_candidate;
    }
  }
  
  /* === Bests === */
  if (s->preferences[BEST_PREFERENCE_TYPE]) {
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    for (p=s->preferences[BEST_PREFERENCE_TYPE]; p!=NIL; p=p->next)
      p->value->common.decider_flag = BEST_DECIDER_FLAG;
    prev_cand = NIL;
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      if (cand->value->common.decider_flag == BEST_DECIDER_FLAG) {
        if (prev_cand)
          prev_cand->next_candidate = cand;
        else
          candidates = cand;
        prev_cand = cand;
      }
    if (prev_cand) prev_cand->next_candidate = NIL;
  }
  
  /* === Worsts === */
  if (s->preferences[WORST_PREFERENCE_TYPE]) {
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    for (p=s->preferences[WORST_PREFERENCE_TYPE]; p!=NIL; p=p->next)
      p->value->common.decider_flag = WORST_DECIDER_FLAG;
    prev_cand = NIL;
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      if (cand->value->common.decider_flag != WORST_DECIDER_FLAG) {
        if (prev_cand)
          prev_cand->next_candidate = cand;
        else
          candidates = cand;
        prev_cand = cand;
      }
    if (prev_cand) prev_cand->next_candidate = NIL;
  }
  
  /* === If there are only 0 or 1 candidates, we're done === */
  if ((!candidates) || (! candidates->next_candidate)) {
    *result_candidates = candidates;
    return NONE_IMPASSE_TYPE;
  }

  /* === Indifferents === */
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
    cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  for (p=s->preferences[UNARY_INDIFFERENT_PREFERENCE_TYPE]; p; p=p->next)
    p->value->common.decider_flag = UNARY_INDIFFERENT_DECIDER_FLAG;

#ifdef NUMERIC_INDIFFERENCE
  /* REW: 2003-01-26 Behavior Variability Kernel Experiments
     We want to treat some binary indifferent prefs as unary indifferents,
     the second pref is really an int representing a probability value.
     So we identify these preferences here.
	 -- want to guarantee decision is not interrupted by a new indiff pref
  */
  for (p=s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p; p=p->next)
    if( (p->referent->fc.common_symbol_info.symbol_type == INT_CONSTANT_SYMBOL_TYPE) ||
				(p->referent->fc.common_symbol_info.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE))
       
      p->value->common.decider_flag = UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG;
  /* END: 2003-01-02 Behavior Variability Kernel Experiments  */
#endif

  not_all_indifferent = FALSE;
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
    /* --- if cand is unary indifferent, it's fine --- */
    if (cand->value->common.decider_flag==UNARY_INDIFFERENT_DECIDER_FLAG)
      continue;

#ifdef NUMERIC_INDIFFERENCE
		else if ( cand->value->common.decider_flag==UNARY_INDIFFERENT_CONSTANT_DECIDER_FLAG )  {
      /* print("\n Ignoring this candidate because it has a constant value for the second pref"); */
      continue;
		}
#endif

	/* --- check whether cand is binary indifferent to each other one --- */
    for (p=candidates; p!=NIL; p=p->next_candidate) {
      if (p==cand) continue;
      match_found = FALSE;
      for (p2=s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; p2!=NIL;
           p2=p2->next)
        if ( ((p2->value==cand->value)&&(p2->referent==p->value)) ||
             ((p2->value==p->value)&&(p2->referent==cand->value)) ) {
          match_found = TRUE;
          break;
        }
      if (!match_found) {
        not_all_indifferent = TRUE;
        break;
      }
    } /* end of for p loop */
    if (not_all_indifferent) break;
  } /* end of for cand loop */

  if (! not_all_indifferent) {
    /* --- items all indifferent, so just pick one of them to return --- */
    /* RBD 4/13/95 Removed code that looked for an existing value already in
       working memory for this slot, and returned it if found.  This was
       apparently an attempt to "stabilize" working memory if attribute
       preferences kept changing, but it ended up getting in the way, esp.
       with mutually indifferent operators were used with user-select
       random. */
    /* --- choose according to user-select ---  */

    /* REW: begin 09.15.96 */
    /* We don't care about the User Select mode in Operand2 for the
     * consistency check.   All we need to do is return the impasse type
     * (None because all preferences are indifferent) and return all the
     * candidates (rather than just one) because we don;t want to commit
     ourselves to single candidate at this point.
     */

    *result_candidates = candidates;


    /* We want the whole list of candidates, not just the one that would
     * be chosen FIRST (ie, the head of result_candidates is also first),
     * so we comment the next line.  */
    /* (*result_candidates)->next_candidate = NIL; */
    return NONE_IMPASSE_TYPE;

  }
  
  /* --- items not all indifferent; for context slots this gives a tie --- */
  if (s->isa_context_slot) {
    *result_candidates = candidates;
    return TIE_IMPASSE_TYPE;
  }

  /* === Parallels === */
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
    cand->value->common.decider_flag = NOTHING_DECIDER_FLAG;
  for (p=s->preferences[UNARY_PARALLEL_PREFERENCE_TYPE]; p; p=p->next)
    p->value->common.decider_flag = UNARY_PARALLEL_DECIDER_FLAG;
  not_all_parallel = FALSE;
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
    /* --- if cand is unary parallel, it's fine --- */
    if (cand->value->common.decider_flag==UNARY_PARALLEL_DECIDER_FLAG)
      continue;
    /* --- check whether cand is binary parallel to each other candidate --- */
    for (p=candidates; p!=NIL; p=p->next_candidate) {
      if (p==cand) continue;
      match_found = FALSE;
      for (p2=s->preferences[BINARY_PARALLEL_PREFERENCE_TYPE]; p2!=NIL;
           p2=p2->next)
        if ( ((p2->value==cand->value)&&(p2->referent==p->value)) ||
             ((p2->value==p->value)&&(p2->referent==cand->value)) ) {
          match_found = TRUE;
          break;
        }
      if (!match_found) {
        not_all_parallel = TRUE;
        break;
      }
    } /* end of for p loop */
    if (not_all_parallel) break;
  } /* end of for cand loop */

  *result_candidates = candidates;

  if (! not_all_parallel) {
    /* --- items are all parallel, so return them all --- */
    return NONE_IMPASSE_TYPE;
  }

  /* --- otherwise we have a tie --- */
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

void add_impasse_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, preference *p) {
  wme *w;
  
  w = make_wme (thisAgent, id, attr, value, FALSE);
  insert_at_head_of_dll (id->id.impasse_wmes, w, next, prev);
  w->preference = p;
  add_wme_to_wm (thisAgent, w);
}

/* ------------------------------------------------------------------
                         Create New Impasse
  
   This creates a new impasse, returning its identifier.  The caller is
   responsible for filling in either id->isa_impasse or id->isa_goal,
   and all the extra stuff for goal identifiers.
------------------------------------------------------------------ */

Symbol *create_new_impasse (agent* thisAgent, Bool isa_goal, Symbol *object, Symbol *attr,
                            byte impasse_type, goal_stack_level level) {
  Symbol *id;

  id = make_new_identifier (thisAgent, (char)(isa_goal ? 'S' : 'I'), level);
  post_link_addition (thisAgent, NIL, id);  /* add the special link */

  add_impasse_wme (thisAgent, id, thisAgent->type_symbol, isa_goal ? thisAgent->state_symbol : thisAgent->impasse_symbol,
                   NIL);

  if (isa_goal)
    add_impasse_wme (thisAgent, id, thisAgent->superstate_symbol, object, NIL);
  else
    add_impasse_wme (thisAgent, id, thisAgent->object_symbol, object, NIL);

  if (attr) add_impasse_wme (thisAgent, id, thisAgent->attribute_symbol, attr, NIL);
  
  switch (impasse_type) {
  case NONE_IMPASSE_TYPE:
    break;    /* this happens only when creating the top goal */
  case CONSTRAINT_FAILURE_IMPASSE_TYPE:
    add_impasse_wme (thisAgent, id, thisAgent->impasse_symbol, thisAgent->constraint_failure_symbol, NIL);
    add_impasse_wme (thisAgent, id, thisAgent->choices_symbol, thisAgent->none_symbol, NIL);
    break;
  case CONFLICT_IMPASSE_TYPE:
    add_impasse_wme (thisAgent, id, thisAgent->impasse_symbol, thisAgent->conflict_symbol, NIL);
    add_impasse_wme (thisAgent, id, thisAgent->choices_symbol, thisAgent->multiple_symbol, NIL);
    break;
  case TIE_IMPASSE_TYPE:
    add_impasse_wme (thisAgent, id, thisAgent->impasse_symbol, thisAgent->tie_symbol, NIL);
    add_impasse_wme (thisAgent, id, thisAgent->choices_symbol, thisAgent->multiple_symbol, NIL);
    break;
  case NO_CHANGE_IMPASSE_TYPE:
    add_impasse_wme (thisAgent, id, thisAgent->impasse_symbol, thisAgent->no_change_symbol, NIL);
    add_impasse_wme (thisAgent, id, thisAgent->choices_symbol, thisAgent->none_symbol, NIL);
    break;
  }
  return id;
}

/* ------------------------------------------------------------------
               Create/Remove Attribute Impasse for Slot
  
   These routines create and remove an attribute impasse for a given
   slot.
------------------------------------------------------------------ */

void create_new_attribute_impasse_for_slot (agent* thisAgent, slot *s, byte impasse_type) {
  Symbol *id;
  
  s->impasse_type = impasse_type;
  id = create_new_impasse (thisAgent, FALSE, s->id, s->attr, impasse_type,
                           ATTRIBUTE_IMPASSE_LEVEL);
  s->impasse_id = id;
  id->id.isa_impasse = TRUE;

  soar_invoke_callbacks(thisAgent, thisAgent, 
                       CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK, 
                       (soar_call_data) s);
}

void remove_existing_attribute_impasse_for_slot (agent* thisAgent, slot *s) {
  Symbol *id;

  soar_invoke_callbacks(thisAgent, thisAgent, 
                       REMOVE_ATTRIBUTE_IMPASSE_CALLBACK, 
                       (soar_call_data) s);

  id = s->impasse_id;
  s->impasse_id = NIL;
  s->impasse_type = NONE_IMPASSE_TYPE;
  remove_wme_list_from_wm (thisAgent, id->id.impasse_wmes);
  id->id.impasse_wmes = NIL;
  post_link_removal (thisAgent, NIL, id);  /* remove the special link */
  symbol_remove_ref (thisAgent, id);
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

preference *make_fake_preference_for_goal_item (agent* thisAgent,
												Symbol *goal,
                                                preference *cand) {
  slot *s;
  wme *ap_wme;
  instantiation *inst;
  preference *pref;
  condition *cond;

  /* --- find the acceptable preference wme we want to backtrace to --- */
  s = cand->slot;
  for (ap_wme=s->acceptable_preference_wmes; ap_wme!=NIL; ap_wme=ap_wme->next)
    if (ap_wme->value==cand->value) break;
  if (!ap_wme) {
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg,
	    "decide.c: Internal error: couldn't find acceptable pref wme\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }
  /* --- make the fake preference --- */
  /* kjc:  here's where we changed REQUIRE to ACCEPTABLE */
  pref = make_preference (thisAgent, ACCEPTABLE_PREFERENCE_TYPE, goal, thisAgent->item_symbol,
                          cand->value, NIL);
  symbol_add_ref (pref->id);
  symbol_add_ref (pref->attr);
  symbol_add_ref (pref->value);
  insert_at_head_of_dll (goal->id.preferences_from_goal, pref,
                         all_of_goal_next, all_of_goal_prev);
  pref->on_goal_list = TRUE;
  preference_add_ref (pref);
  /* --- make the fake instantiation --- */
  allocate_with_pool (thisAgent, &thisAgent->instantiation_pool, &inst);
  pref->inst = inst;
  pref->inst_next = pref->inst_prev = NIL;
  inst->preferences_generated = pref;
  inst->prod = NIL;
  inst->next = inst->prev = NIL;
  inst->rete_token = NIL;
  inst->rete_wme = NIL;
  inst->match_goal = goal;
  inst->match_goal_level = goal->id.level;
  inst->okay_to_variablize = TRUE;
  inst->backtrace_number = 0;
  inst->in_ms = FALSE;
  /* --- make the fake condition --- */
  allocate_with_pool (thisAgent, &thisAgent->condition_pool, &cond);
  cond->type = POSITIVE_CONDITION;
  cond->next = cond->prev = NIL;
  inst->top_of_instantiated_conditions = cond;
  inst->bottom_of_instantiated_conditions = cond;
  inst->nots = NIL;
  cond->data.tests.id_test = make_equality_test (ap_wme->id);
  cond->data.tests.attr_test = make_equality_test (ap_wme->attr);
  cond->data.tests.value_test = make_equality_test (ap_wme->value);
  cond->test_for_acceptable_preference = TRUE;
  cond->bt.wme_ = ap_wme;
  #ifdef DO_TOP_LEVEL_REF_CTS
  wme_add_ref (ap_wme);
  #else 
  if (inst->match_goal_level > TOP_GOAL_LEVEL) wme_add_ref (ap_wme);
  #endif
  cond->bt.level = ap_wme->id->id.level;
  cond->bt.trace = NIL;
  cond->bt.prohibits = NIL;
  /* --- return the fake preference --- */
  return pref;
}

void remove_fake_preference_for_goal_item (agent* thisAgent, preference *pref) {
  preference_remove_ref (thisAgent, pref); /* everything else happens automatically */
}

/* ------------------------------------------------------------------
                       Update Impasse Items
  
   This routine updates the set of ^item wmes on a goal or attribute
   impasse.  It takes the identifier of the goal/impasse, and a list
   of preferences (linked via the "next_candidate" field) for the new
   set of items that should be there.
------------------------------------------------------------------ */

void update_impasse_items (agent* thisAgent, Symbol *id, preference *items) {
  wme *w, *next_w;
  preference *cand;
  preference *bt_pref;

  /* --- reset flags on existing items to "NOTHING" --- */
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
    if (w->attr==thisAgent->item_symbol)
      w->value->common.decider_flag = NOTHING_DECIDER_FLAG;

  /* --- mark set of desired items as "CANDIDATEs" --- */
  for (cand=items; cand!=NIL; cand=cand->next_candidate)
    cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;

  /* --- for each existing item:  if it's supposed to be there still, then
     mark it "ALREADY_EXISTING"; otherwise remove it --- */
  w = id->id.impasse_wmes;
  while (w) {
    next_w = w->next;
    if (w->attr==thisAgent->item_symbol) {
      if (w->value->common.decider_flag==CANDIDATE_DECIDER_FLAG) {
        w->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
        w->value->common.a.decider_wme = w; /* so we can update the pref later */
      } else {
        remove_from_dll (id->id.impasse_wmes, w, next, prev);
        if (id->id.isa_goal)
          remove_fake_preference_for_goal_item (thisAgent, w->preference);
        remove_wme_from_wm (thisAgent, w);
      }
    }
    w = next_w;
  }

  /* --- for each desired item:  if it doesn't ALREADY_EXIST, add it --- */
  for (cand=items; cand!=NIL; cand=cand->next_candidate) {
    if (id->id.isa_goal)
      bt_pref = make_fake_preference_for_goal_item (thisAgent, id, cand);
    else
      bt_pref = cand;
    if (cand->value->common.decider_flag==ALREADY_EXISTING_WME_DECIDER_FLAG) {
      if (id->id.isa_goal) remove_fake_preference_for_goal_item
        (thisAgent, cand->value->common.a.decider_wme->preference);
      cand->value->common.a.decider_wme->preference = bt_pref;
    } else {
      add_impasse_wme (thisAgent, id, thisAgent->item_symbol, cand->value, bt_pref);
    }
  }
}

/* ------------------------------------------------------------------
                       Decide Non Context Slot
  
   This routine decides a given slot, which must be a non-context
   slot.  It calls run_preference_semantics() on the slot, then
   updates the wmes and/or impasse for the slot accordingly.
------------------------------------------------------------------ */

void decide_non_context_slot (agent* thisAgent, slot *s) 
{
  byte impasse_type;
  wme *w, *next_w;
  preference *candidates, *cand, *pref;
  
  impasse_type = run_preference_semantics (thisAgent, s, &candidates);
  
  if (impasse_type==NONE_IMPASSE_TYPE) 
  {
     /* --- no impasse, so remove any existing one and update the wmes --- */
     if (s->impasse_type != NONE_IMPASSE_TYPE)
        remove_existing_attribute_impasse_for_slot (thisAgent, s);
     
     /* --- reset marks on existing wme values to "NOTHING" --- */
     for (w=s->wmes; w!=NIL; w=w->next)
        w->value->common.decider_flag = NOTHING_DECIDER_FLAG;
     
     /* --- set marks on desired values to "CANDIDATES" --- */
     for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
        cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
     
        /* --- for each existing wme, if we want it there, mark it as
     ALREADY_EXISTING; otherwise remove it --- */
     w = s->wmes;
     while (w) 
     {
        next_w = w->next;
        if (w->value->common.decider_flag == CANDIDATE_DECIDER_FLAG) 
        {
           w->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
           w->value->common.a.decider_wme = w; /* so we can set the pref later */
        } 
        else 
        {
           remove_from_dll (s->wmes, w, next, prev);
           /* REW: begin 09.15.96 */
           if (thisAgent->operand2_mode)
           {
              if (w->gds) 
              {
                 if (w->gds->goal != NIL)
                 {
                    /* If the goal pointer is non-NIL, then goal is in the stack */
                    if (thisAgent->soar_verbose_flag || thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM]) 
                    {
                       print(thisAgent, "\nRemoving state S%d because element in GDS changed.", w->gds->goal->id.level);
                       print(thisAgent, " WME: "); 

                       char buf[256];
                       snprintf(buf, 254, "Removing state S%d because element in GDS changed.", w->gds->goal->id.level);
                       gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagVerbose);
	                   gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kTypeString, buf);
                       print_wme(thisAgent, w);
                       gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagVerbose);
                    }
                    gds_invalid_so_remove_goal(thisAgent, w);
                 }
              }
           }
           /* REW: end   09.15.96 */
           remove_wme_from_wm (thisAgent, w);
        }
        w = next_w;
     }  /* end while (W) */
     
     /* --- for each desired value, if it's not already there, add it --- */
     for (cand=candidates; cand!=NIL; cand=cand->next_candidate) 
     {
        if (cand->value->common.decider_flag==ALREADY_EXISTING_WME_DECIDER_FLAG)
        {
           /* REW: begin 11.22.97 */ 
           /* print(thisAgent, "\n This WME was marked as already existing...."); print_wme(cand->value->common.a.decider_wme); */
           
           /* REW: end   11.22.97 */ 
           cand->value->common.a.decider_wme->preference = cand;
        } 
        else 
        {
           w = make_wme (thisAgent, cand->id, cand->attr, cand->value, FALSE);
           insert_at_head_of_dll (s->wmes, w, next, prev);
           w->preference = cand;
           
           /* REW: begin 09.15.96 */
           if (thisAgent->operand2_mode)
           {
           /* Whenever we add a WME to WM, we also want to check and see if
           this new WME is o-supported.  If so, then we want to add the
           supergoal dependencies of the new, o-supported element to the
           goal in which the element was created (as long as the o_supported
           element was not created in the top state -- the top goal has
              no gds).  */
              
              /* REW: begin 11.25.96 */ 
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
              start_timer(thisAgent, &thisAgent->start_gds_tv);
#endif 
#endif
              /* REW: end   11.25.96 */ 
              
              thisAgent->parent_list_head = NIL;
              
              /* If the working memory element being added is going to have
              o_supported preferences and the instantion that created it
              is not in the top_level_goal (where there is no GDS), then
              loop over the preferences for this WME and determine which
              WMEs should be added to the goal's GDS (the goal here being the
              goal to which the added memory is attached). */
              
              if ((w->preference->o_supported == TRUE) &&
                 (w->preference->inst->match_goal_level != 1)) {
                 
                 if (w->preference->inst->match_goal->id.gds == NIL) {
                 /* If there is no GDS yet for this goal,
                    * then we need to create one */
                    if (w->preference->inst->match_goal_level ==
                       w->preference->id->id.level) {
                       
                       create_gds_for_goal( thisAgent, w->preference->inst->match_goal );
                       
                       /* REW: BUG When chunks and result instantiations both create
                       * preferences for the same WME, then we only want to create
                       * the GDS for the highest goal.  Right now I ensure that we
                       * elaborate the correct GDS with the tests in the loop just
                       * below this code, but the GDS creation above assumes that
                       * the chunk will be first on the GDS list.  This order
                       * appears to be always true, although I am not 100% certain
                       * (I think it occurs this way because the chunk is
                       * necessarily added to the instantiaton list after the
                       * original instantiation and lists get built such older items
                       * appear further from the head of the list) . If not true,
                       * then we need to keep track of any GDS's that get created
                       * here to remove them later if we find a higher match goal
                       * for the WME. For now, the program just exits in this
                       * situation; otherwise, we would build a GDS for the wrong
                       * level and never elaborate it (resulting in a memory
                       * leak). 
                       */
                    } else {
                       char msg[256];
                       strncpy(msg,"**** Wanted to create a GDS for a WME level different from the instantiation level.....Big problems....exiting....****\n\n",256);
					   msg[255] = 0; /* ensure null termination */
                       abort_with_fatal_error(thisAgent, msg);
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
                 
                 for (pref=w->preference; pref!=NIL; pref=pref->next) {
#ifdef DEBUG_GDS_HIGH
                    print(thisAgent, thisAgent, "\n\n   "); print_preference(pref);
                    print(thisAgent, "   Goal level of preference: %d\n",
                       pref->id->id.level);
#endif
                    
                    if (pref->inst->GDS_evaluated_already == FALSE) {
#ifdef DEBUG_GDS_HIGH
                       print_with_symbols(thisAgent, "   Match goal lev of instantiation %y ",
                          pref->inst->prod->name);
                       print(thisAgent, "is %d\n", pref->inst->match_goal_level);
#endif
                       if (pref->inst->match_goal_level > pref->id->id.level) {
#ifdef DEBUG_GDS_HIGH
                          print_with_symbols(thisAgent, "        %y  is simply the instantiation that led to a chunk.\n        Not adding it the current instantiations.\n", pref->inst->prod->name);
#endif
                          
                       } else {
#ifdef DEBUG_GDS_HIGH
                          print_with_symbols(thisAgent, "\n   Adding %y to list of parent instantiations\n", pref->inst->prod->name); 
#endif
                          uniquely_add_to_head_of_dll(thisAgent, pref->inst);
                          pref->inst->GDS_evaluated_already = TRUE;
                       }
                    }  /* end if GDS_evaluated_already is FALSE */
#ifdef DEBUG_GDS_HIGH
                    else
                       print_with_symbols(thisAgent, "\n    Instantiation %y was already explored; skipping it\n", pref->inst->prod->name);
#endif
                    
                 }  /* end of forloop over preferences for this wme */
                 
                 
#ifdef DEBUG_GDS_HIGH
                 print(thisAgent, "\n    CALLING ELABORATE GDS....\n");
#endif 
                 elaborate_gds(thisAgent);
                 
                 /* technically, the list should be empty at this point ??? */
                 
                 free_parent_list(thisAgent); 
#ifdef DEBUG_GDS_HIGH
                 print(thisAgent, "    FINISHED ELABORATING GDS.\n\n");
#endif
              }  /* end if w->preference->o_supported == TRUE ... */
              
              
              /* REW: begin 11.25.96 */ 
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
              stop_timer(thisAgent, &thisAgent->start_gds_tv, 
                 &thisAgent->gds_cpu_time[thisAgent->current_phase]);
#endif
#endif
              /* REW: end   11.25.96 */ 
              
            }  /* end if thisAgent->OPERAND2_MODE ... */
               /* REW: end   09.15.96 */
   
            add_wme_to_wm (thisAgent, w);
         }
      }
      
      return;
   } /* end of if impasse type == NONE */

   /* --- impasse type != NONE --- */
   if (s->wmes) 
   {  
      /* --- remove any existing wmes --- */
      remove_wme_list_from_wm (thisAgent, s->wmes); 
      s->wmes = NIL;
   }

   /* --- create and/or update impasse structure --- */
   if (s->impasse_type != NONE_IMPASSE_TYPE) 
   {
      if (s->impasse_type != impasse_type) 
      {
         remove_existing_attribute_impasse_for_slot (thisAgent, s);
         create_new_attribute_impasse_for_slot (thisAgent, s, impasse_type);
      }
      update_impasse_items (thisAgent, s->impasse_id, candidates);
   } 
   else 
   {
      create_new_attribute_impasse_for_slot (thisAgent, s, impasse_type);
      update_impasse_items (thisAgent, s->impasse_id, candidates);
   }
}

/* ------------------------------------------------------------------
                      Decide Non Context Slots
  
   This routine iterates through all changed non-context slots, and
   decides each one.
------------------------------------------------------------------ */

void decide_non_context_slots (agent* thisAgent) {
  dl_cons *dc;
  slot *s;

  while (thisAgent->changed_slots) 
  {
    dc = thisAgent->changed_slots;
    thisAgent->changed_slots = thisAgent->changed_slots->next;
    s = static_cast<slot_struct *>(dc->item);
    decide_non_context_slot (thisAgent, s);
    s->changed = NIL;
    free_with_pool (&thisAgent->dl_cons_pool, dc);
  }
}

/* ------------------------------------------------------------------
                      Context Slot Is Decidable
  
   This returns TRUE iff the given slot (which must be a context slot)
   is decidable.  A context slot is decidable if:
     - it has an installed value in WM and there is a reconsider
       preference for that value, or
     - it has no installed value but does have changed preferences
------------------------------------------------------------------ */

Bool context_slot_is_decidable (slot *s) 
{
   Symbol *v;
   preference *p;
   
   if (!s->wmes) 
      return (s->changed != NIL);
   
   v = s->wmes->value;
   for (p = s->preferences[RECONSIDER_PREFERENCE_TYPE]; p != NIL; p = p->next)
   {
      if (v == p->value) 
         return TRUE;
   }
   
   return FALSE;
}

/* ------------------------------------------------------------------
                      Remove WMEs For Context Slot
  
   This removes the wmes (there can only be 0 or 1 of them) for the
   given context slot.
------------------------------------------------------------------ */

void remove_wmes_for_context_slot (agent* thisAgent, slot *s) {
  wme *w;
  
  if (!s->wmes) return;
  /* Note that we only need to handle one wme--context slots never have
     more than one wme in them */
  w = s->wmes;
  preference_remove_ref (thisAgent, w->preference);
  remove_wme_from_wm (thisAgent, w);
  s->wmes = NIL;
}

/* ------------------------------------------------------------------
                 Remove Existing Context And Descendents
  
   This routine truncates the goal stack by removing the given goal
   and all its subgoals.  (If the given goal is the top goal, the
   entire context stack is removed.)
------------------------------------------------------------------ */

void remove_existing_context_and_descendents (agent* thisAgent, Symbol *goal) {
  preference *p;

  ms_change *head, *tail;  /* REW:   08.20.97 */

  /* --- remove descendents of this goal --- */
  if (goal->id.lower_goal)
    remove_existing_context_and_descendents (thisAgent, goal->id.lower_goal);

  /* --- invoke callback routine --- */
  soar_invoke_callbacks(thisAgent, thisAgent, 
                       POP_CONTEXT_STACK_CALLBACK, 
                       (soar_call_data) goal);

  /* JC ADDED: Tell gSKI that we have removed a subgoal */
  gSKI_MakeAgentCallback(gSKI_K_EVENT_SUBSTATE_DESTROYED, 0, thisAgent, static_cast<void*>(goal));

  /* --- disconnect this goal from the goal stack --- */
  if (goal == thisAgent->top_goal) {
    thisAgent->top_goal = NIL;
    thisAgent->bottom_goal = NIL;
  } else {
    thisAgent->bottom_goal = goal->id.higher_goal;
    thisAgent->bottom_goal->id.lower_goal = NIL;
  }

  /* --- remove any preferences supported by this goal --- */
#ifdef DO_TOP_LEVEL_REF_CTS
  while (goal->id.preferences_from_goal) {
    p = goal->id.preferences_from_goal;
    remove_from_dll (goal->id.preferences_from_goal, p,
                     all_of_goal_next, all_of_goal_prev);
    p->on_goal_list = FALSE;
    if (! remove_preference_from_clones (thisAgent, p))
      if (p->in_tm) remove_preference_from_tm (thisAgent, p);
  }
#else   
  /* KJC Aug 05: this seems to cure a potential for exceeding callstack 
   * when popping soar's goal stack and not doing DO_TOP_LEVEL_REF_CTS 
   * Probably should make this change for all cases, but needs testing.  */
  /* Prefs are added to head of dll, so try removing from tail */
  if (goal->id.preferences_from_goal) {
	  p = goal->id.preferences_from_goal;
	  while (p->all_of_goal_next) p = p->all_of_goal_next;
	  while (p) {
		  preference* p_next = p->all_of_goal_prev; // RPM 10/06 we need to save this because p may be freed by the end of the loop
		  remove_from_dll (goal->id.preferences_from_goal, p,
                     all_of_goal_next, all_of_goal_prev);
		  p->on_goal_list = FALSE;
		  if (! remove_preference_from_clones (thisAgent, p))
			  if (p->in_tm) remove_preference_from_tm (thisAgent, p);
		  p = p_next;
	  }
  }
#endif
  /* --- remove wmes for this goal, and garbage collect --- */
  remove_wmes_for_context_slot (thisAgent, goal->id.operator_slot);
  update_impasse_items (thisAgent, goal, NIL); /* causes items & fake pref's to go away */
  remove_wme_list_from_wm (thisAgent, goal->id.impasse_wmes);
  goal->id.impasse_wmes = NIL;
  /* REW: begin   09.15.96 */
  /* If there was a GDS for this goal, we want to set the pointer for the
     goal to NIL to indicate it no longer exists.  
     BUG: We probably also need to make certain that the GDS doesn't need
     to be free'd here as well. */
  if (goal->id.gds != NIL) goal->id.gds->goal = NIL;
  /* REW: end   09.15.96 */

  /* REW: begin 08.20.97 */

  /* If we remove a goal WME, then we have to transfer any already existing
     retractions to the nil-goal list on the current agent.  We should be
     able to do this more efficiently but the most obvious way (below) still
     requires scanning over the whole list (to set the goal pointer of each
     msc to NIL); therefore this solution should be acceptably efficient. */

  if (goal->id.ms_retractions) { /* There's something on the retraction list */
      
    head = goal->id.ms_retractions;
    tail = head;
    
    /* find the tail of this list */
    while (tail->next_in_level) {
      tail->goal = NIL;  /* force the goal to be NIL */
      tail = tail->next_in_level;
    }
    tail->goal = NIL; 

    if (thisAgent->nil_goal_retractions) {
      /* There are already retractions on the list */
      
      /* Append this list to front of NIL goal list */
      thisAgent->nil_goal_retractions->prev_in_level = tail;
      tail->next_in_level = thisAgent->nil_goal_retractions;
      thisAgent->nil_goal_retractions = head;
      
    } else { /* If no retractions, make this list the NIL goal list */
      thisAgent->nil_goal_retractions = head;
    }
  }

  /* REW: BUG
   * Tentative assertions can exist for removed goals.  However, it looks
   * like the removal forces a tentative retraction, which then leads to
   * the deletion of the tentative assertion.  However, I have not tested
   * such cases exhaustively -- I would guess that some processing may be
   * necessary for the assertions here at some point?
   */

  /* REW: end   08.20.97 */

  post_link_removal (thisAgent, NIL, goal);  /* remove the special link */
  symbol_remove_ref (thisAgent, goal);
}

/* ------------------------------------------------------------------
                         Create New Context
  
   This routine creates a new goal context (becoming the new bottom
   goal) below the current bottom goal.  If there is no current
   bottom goal, this routine creates a new goal and makes it both
   the top and bottom goal.
------------------------------------------------------------------ */

void create_new_context (agent* thisAgent, Symbol *attr_of_impasse, byte impasse_type) 
{
  Symbol *id;
  
  if (thisAgent->bottom_goal) 
  {
     /* Creating a sub-goal (or substate) */
    id = create_new_impasse (thisAgent, TRUE, thisAgent->bottom_goal,	                 
	     	                    attr_of_impasse, impasse_type,
                             (goal_stack_level) (thisAgent->bottom_goal->id.level + 1));
    id->id.higher_goal = thisAgent->bottom_goal;
    thisAgent->bottom_goal->id.lower_goal = id;
    thisAgent->bottom_goal = id;
    add_impasse_wme (thisAgent, id, thisAgent->quiescence_symbol,
		             thisAgent->t_symbol, NIL);
	if ((NO_CHANGE_IMPASSE_TYPE == impasse_type) && 
		(thisAgent->sysparams[MAX_GOAL_DEPTH] < thisAgent->bottom_goal->id.level )) 
	{
 		// appear to be SNC'ing deep in goalstack, so interrupt and warn user
		// KJC note: we actually halt, because there is no interrupt function in SoarKernel
		// in the gSKI Agent code, if system_halted, MAX_GOAL_DEPTH is checked and if exceeded
		// then the interrupt is generated and system_halted is set to FALSE so the user can recover.
		print(thisAgent, "\nGoal stack depth exceeded %d on a no-change impasse.\n",thisAgent->sysparams[MAX_GOAL_DEPTH]);
		print(thisAgent, "Soar appears to be in an infinite loop.  \nContinuing to subgoal may cause Soar to \nexceed the program stack of your system.\n");
		GenerateWarningXML(thisAgent, "\nGoal stack depth exceeded on a no-change impasse.\n");
		GenerateWarningXML(thisAgent, "Soar appears to be in an infinite loop.  \nContinuing to subgoal may cause Soar to \nexceed the program stack of your system.\n");
		thisAgent->stop_soar = TRUE;
		thisAgent->system_halted = TRUE;
		thisAgent->reason_for_stopping = "Max Goal Depth exceeded.";
	}
  } 
  else 
  {
     /* Creating the top state */ 
     id = create_new_impasse (thisAgent, TRUE, thisAgent->nil_symbol,
               			     NIL, NONE_IMPASSE_TYPE,
                             TOP_GOAL_LEVEL);
    thisAgent->top_goal = id;
    thisAgent->bottom_goal = id;
    thisAgent->top_state = thisAgent->top_goal;
    id->id.higher_goal = NIL;
    id->id.lower_goal = NIL;
  }

  id->id.isa_goal = TRUE;
  id->id.operator_slot = make_slot (thisAgent, id, thisAgent->operator_symbol);
  id->id.allow_bottom_up_chunks = TRUE;

  /* --- invoke callback routine --- */
  soar_invoke_callbacks(thisAgent, thisAgent, 
                       CREATE_NEW_CONTEXT_CALLBACK, 
                       (soar_call_data) id);

   /* JC ADDED: Tell gSKI we have a new object in general (there are three places this can occur). */
   gSKI_MakeAgentCallbackWMObjectAdded(thisAgent, NIL, NIL, id);

   /* JC ADDED: Tell gSKI that a substate was created */
   gSKI_MakeAgentCallback(gSKI_K_EVENT_SUBSTATE_CREATED, 1, thisAgent, static_cast<void*>(id));
}

/* ------------------------------------------------------------------
              Type and Attribute of Existing Impasse
  
   Given a goal, these routines return the type and attribute,
   respectively, of the impasse just below that goal context.  It
   does so by looking at the impasse wmes for the next lower goal
   in the goal stack.
------------------------------------------------------------------ */

byte type_of_existing_impasse (agent* thisAgent, Symbol *goal) {
  wme *w;
  char msg[BUFFER_MSG_SIZE];

  if (! goal->id.lower_goal) return NONE_IMPASSE_TYPE;
  for (w=goal->id.lower_goal->id.impasse_wmes; w!=NIL; w=w->next)
    if (w->attr==thisAgent->impasse_symbol) {
      if (w->value==thisAgent->no_change_symbol)
	return NO_CHANGE_IMPASSE_TYPE;
      if (w->value==thisAgent->tie_symbol)
	return TIE_IMPASSE_TYPE;
      if (w->value==thisAgent->constraint_failure_symbol)
        return CONSTRAINT_FAILURE_IMPASSE_TYPE;
      if (w->value==thisAgent->conflict_symbol)
	return CONFLICT_IMPASSE_TYPE;
      if (w->value==thisAgent->none_symbol)
	return NONE_IMPASSE_TYPE;
      strncpy (msg,"decide.c: Internal error: bad type of existing impasse.\n", BUFFER_MSG_SIZE);
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error(thisAgent, msg);
    }
  strncpy (msg,"decide.c: Internal error: couldn't find type of existing impasse.\n", BUFFER_MSG_SIZE);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  return 0; /* unreachable, but without it, gcc -Wall warns here */
}

Symbol *attribute_of_existing_impasse (agent* thisAgent, Symbol *goal) {
  wme *w;

  if (! goal->id.lower_goal) return NIL;
  for (w=goal->id.lower_goal->id.impasse_wmes; w!=NIL; w=w->next)
    if (w->attr==thisAgent->attribute_symbol) return w->value;
  { char msg[BUFFER_MSG_SIZE];
  strncpy (msg, "decide.c: Internal error: couldn't find attribute of existing impasse.\n", BUFFER_MSG_SIZE);
  msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
  abort_with_fatal_error(thisAgent, msg);
  }
  return NIL; /* unreachable, but without it, gcc -Wall warns here */
}

/* ------------------------------------------------------------------
                       Decide Context Slot

   This decides the given context slot.  It normally returns TRUE,
   but returns FALSE if the ONLY change as a result of the decision
   procedure was a change in the set of ^item's on the impasse below
   the given slot.
------------------------------------------------------------------ */

Bool decide_context_slot (agent* thisAgent, Symbol *goal, slot *s) 
{
   byte impasse_type;
   Symbol *attribute_of_impasse;
   wme *w;
   preference *candidates;
   preference *temp;
   
   if (!context_slot_is_decidable(s)) 
   {
      /* --- the only time we decide a slot that's not "decidable" is when it's
             the last slot in the entire context stack, in which case we have a
             no-change impasse there --- */
      impasse_type = NO_CHANGE_IMPASSE_TYPE;
      candidates = NIL; /* we don't want any impasse ^item's later */
   } 
   else 
   {
      /* --- the slot is decidable, so run preference semantics on it --- */
      impasse_type = run_preference_semantics (thisAgent, s, &candidates);
      remove_wmes_for_context_slot (thisAgent, s); /* must remove old wme before adding
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
            strncpy (msg,"decide.c: Internal error: more than one winner for context slot\n", BUFFER_MSG_SIZE);
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
      remove_wmes_for_context_slot (thisAgent, goal->id.operator_slot);
   }
   
   
   /* --- if we have a winner, remove any existing impasse and install the
            new value for the current slot --- */
   if (impasse_type == NONE_IMPASSE_TYPE) 
   {
      for(temp = candidates; temp; temp = temp->next_candidate)
         preference_add_ref(temp);
   
      if (goal->id.lower_goal)
         remove_existing_context_and_descendents (thisAgent, goal->id.lower_goal);
      
      w = make_wme (thisAgent, s->id, s->attr, candidates->value, FALSE);
      insert_at_head_of_dll (s->wmes, w, next, prev);
      w->preference = candidates;
      preference_add_ref (w->preference);

      /* JC Adding an operator to working memory in the current state */
      add_wme_to_wm (thisAgent, w);
      
      for(temp = candidates; temp; temp = temp->next_candidate)
         preference_remove_ref(thisAgent, temp);
      
      /* JC ADDED: Notify gSKI of an operator selection  */
      gSKI_MakeAgentCallback(gSKI_K_EVENT_OPERATOR_SELECTED, 1, thisAgent, 
                             static_cast<void*>(w));
      
      return TRUE;
   } 
   
   /* --- no winner; if an impasse of the right type already existed, just
   update the ^item set on it --- */
   if ((impasse_type == type_of_existing_impasse(thisAgent, goal)) &&
      (attribute_of_impasse == attribute_of_existing_impasse(thisAgent, goal))) 
   {
      update_impasse_items (thisAgent, goal->id.lower_goal, candidates);
      return FALSE;
   }
   
   /* --- no impasse already existed, or an impasse of the wrong type
   already existed --- */
   for(temp = candidates; temp; temp = temp->next_candidate)
      preference_add_ref(temp);
   
   if (goal->id.lower_goal)
      remove_existing_context_and_descendents (thisAgent, goal->id.lower_goal);
   
   /* REW: begin 10.24.97 */
   if (thisAgent->operand2_mode && thisAgent->waitsnc &&
      (impasse_type == NO_CHANGE_IMPASSE_TYPE) &&
      (attribute_of_impasse == thisAgent->state_symbol)) 
   {
      thisAgent->waitsnc_detect = TRUE; 
   } 
   else 
   {
      /* REW: end     10.24.97 */
      create_new_context (thisAgent, attribute_of_impasse, impasse_type);
      update_impasse_items (thisAgent, goal->id.lower_goal, candidates);
   }
   
   for(temp = candidates; temp; temp = temp->next_candidate)
      preference_remove_ref(thisAgent, temp);
   
   return TRUE;
}

/* ------------------------------------------------------------------
                       Decide Context Slots

   This scans down the goal stack and runs the decision procedure on
   the appropriate context slots.
------------------------------------------------------------------ */

void decide_context_slots (agent* thisAgent) 
{
   Symbol *goal;
   slot *s;
   
   if (thisAgent->highest_goal_whose_context_changed) 
   {
      goal = thisAgent->highest_goal_whose_context_changed;
   }
   else
      /* no context changed, so jump right to the bottom */
      goal = thisAgent->bottom_goal;
   
   s = goal->id.operator_slot;
   
   /* --- loop down context stack --- */
   while (TRUE) 
   {
      /* --- find next slot to decide --- */
      while (TRUE) 
      {
         if (context_slot_is_decidable(s)) 
            break;
         
         if ((s == goal->id.operator_slot) || (! s->wmes)) 
         {
            /* --- no more slots to look at for this goal; have we reached
            the last slot in whole stack? --- */
            if (! goal->id.lower_goal) 
               break;
            
            /* --- no, go down one level --- */
            goal = goal->id.lower_goal;
            s = goal->id.operator_slot;
         }
      } /* end of while (TRUE) find next slot to decide */
      
      /* --- now go and decide that slot --- */
      if (decide_context_slot (thisAgent, goal, s)) 
         break;
      
   } /* end of while (TRUE) loop down context stack */
   thisAgent->highest_goal_whose_context_changed = NIL;
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

void init_decider (agent* thisAgent) 
{
  init_memory_pool (thisAgent, &thisAgent->slot_pool, sizeof(slot), "slot");
  init_memory_pool (thisAgent, &thisAgent->wme_pool, sizeof(wme), "wme");
  init_memory_pool (thisAgent, &thisAgent->preference_pool,
		    sizeof(preference), "preference");
}

void do_buffered_wm_and_ownership_changes (agent* thisAgent) 
{
  do_buffered_acceptable_preference_wme_changes(thisAgent);
  do_buffered_link_changes(thisAgent);
  do_buffered_wm_changes(thisAgent);
  remove_garbage_slots(thisAgent);
}

void do_working_memory_phase (agent* thisAgent) {
 
   if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
      if (thisAgent->operand2_mode == TRUE) {		  
		  if (thisAgent->current_phase == APPLY_PHASE) {  /* it's always IE for PROPOSE */
			  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagSubphase);
			  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kPhase_Name, kSubphaseName_ChangingWorkingMemory);
			  switch (thisAgent->FIRING_TYPE) {
                  case PE_PRODS:
					  print (thisAgent, "\t--- Change Working Memory (PE) ---\n",0);
					  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kPhase_FiringType, kPhaseFiringType_PE);
					  break;      
				  case IE_PRODS:	
					  print (thisAgent, "\t--- Change Working Memory (IE) ---\n",0);
					  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kPhase_FiringType, kPhaseFiringType_IE);
					  break;
			  }
			  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagSubphase);
		  }
      }
      else
		  // the XML for this is generated in this function
		  print_phase (thisAgent, "\n--- Working Memory Phase ---\n",0);
   }
   
   decide_non_context_slots(thisAgent);
   do_buffered_wm_and_ownership_changes(thisAgent);

   if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
     if (! thisAgent->operand2_mode) {
 	  print_phase (thisAgent, "\n--- END Working Memory Phase ---\n",1);
	 }
  }

}

void do_decision_phase (agent* thisAgent) 
{
   /* phase printing moved to init_soar: do_one_top_level_phase */

   decide_context_slots (thisAgent);
   do_buffered_wm_and_ownership_changes(thisAgent);

  /*
   * Bob provided a solution to fix WME's hanging around unsupported
   * for an elaboration cycle.
   */
   decide_non_context_slots(thisAgent);
   do_buffered_wm_and_ownership_changes(thisAgent);
}  

void create_top_goal (agent* thisAgent) 
{
   create_new_context (thisAgent, NIL, NONE_IMPASSE_TYPE);
   thisAgent->highest_goal_whose_context_changed = NIL;  /* nothing changed yet */
   do_buffered_wm_and_ownership_changes(thisAgent);
}

void clear_goal_stack (agent* thisAgent) 
{
   if (!thisAgent->top_goal) 
      return;
   
   remove_existing_context_and_descendents (thisAgent, thisAgent->top_goal);
   thisAgent->highest_goal_whose_context_changed = NIL;  /* nothing changed                                                                yet */
   do_buffered_wm_and_ownership_changes(thisAgent);
   thisAgent->top_state = NIL;
   thisAgent->active_goal = NIL;
   do_input_cycle(thisAgent);  /* tell input functions that the top state is gone */
   do_output_cycle(thisAgent); /* tell output functions that output commands are gone */
}
  
void print_lowest_slot_in_context_stack (agent* thisAgent) {

  /* REW: begin 10.24.97 */
  /* This doesn't work yet so for now just print the last selection */
  /*  if (thisAgent->operand2_mode && 
   *   thisAgent->waitsnc &&
   *   thisAgent->waitsnc_detect) {
   * thisAgent->waitsnc_detect = FALSE;
   * print_stack_trace (thisAgent->wait_symbol,
   *                    thisAgent->bottom_goal, FOR_OPERATORS_TF, TRUE);
   * print(thisAgent, "\n waiting"); 
   * return;
   *  }
   */
  /* REW: end   10.24.97 */

  if (thisAgent->bottom_goal->id.operator_slot->wmes)
    print_stack_trace (thisAgent, thisAgent->bottom_goal->id.operator_slot->wmes->value,
                       thisAgent->bottom_goal, FOR_OPERATORS_TF, TRUE);


  /* RCHONG: begin 10.11 */
  /*
  this coded is needed just so that when an ONC is created in OPERAND
  (i.e. if the previous goal's operator slot is not empty), it's stack
  trace line doesn't get a number.  this is done because in OPERAND,
  ONCs are detected for "free".
  */

  else {

    /* REW: begin 09.15.96 */
    if (thisAgent->operand2_mode == FALSE) 
       print_stack_trace (thisAgent, thisAgent->bottom_goal,
			  thisAgent->bottom_goal, FOR_STATES_TF,TRUE);
    /* REW: end   09.15.96 */

    else {
       if (thisAgent->d_cycle_count == 0)
          print_stack_trace (thisAgent, thisAgent->bottom_goal,
			     thisAgent->bottom_goal, FOR_STATES_TF,TRUE);
       else {
          if (thisAgent->bottom_goal->id.higher_goal && 
              thisAgent->bottom_goal->id.higher_goal->id.operator_slot->wmes) {
             print_stack_trace (thisAgent, thisAgent->bottom_goal,
				thisAgent->bottom_goal,
				FOR_STATES_TF,TRUE);
	  }
          else {
             print_stack_trace (thisAgent, thisAgent->bottom_goal,
				thisAgent->bottom_goal,
				FOR_STATES_TF,TRUE);
	  }
       }
    }
  }

  /* RCHONG: end 10.11 */

}




/* REW: begin 09.15.96 */

void uniquely_add_to_head_of_dll(agent* thisAgent, instantiation *inst)
{

  parent_inst *new_pi, *curr_pi;
   
  /* print(thisAgent, "UNIQUE DLL:         scanning parent list...\n"); */

  for (curr_pi = thisAgent->parent_list_head;
       curr_pi;
       curr_pi = curr_pi->next) {
     if (curr_pi->inst == inst) {
        #ifdef DEBUG_GDS
        print_with_symbols(thisAgent, "UNIQUE DLL:            %y is already in parent list\n",curr_pi->inst->prod->name);
        #endif
        return;
     }
     #ifdef DEBUG_GDS
         print_with_symbols(thisAgent, "UNIQUE DLL:            %y\n",curr_pi->inst->prod->name); 
     #endif
  } /* end for loop */

  new_pi = (parent_inst *) malloc(sizeof(parent_inst));
  new_pi->next = NIL;
  new_pi->prev = NIL;
  new_pi->inst = inst;

  new_pi->next = thisAgent->parent_list_head;

  if (thisAgent->parent_list_head != NIL)
     thisAgent->parent_list_head->prev = new_pi;

  thisAgent->parent_list_head = new_pi;
  #ifdef DEBUG_GDS
   print_with_symbols(thisAgent, "UNIQUE DLL:         added: %y\n",inst->prod->name); 
  #endif
}

/* JC ADDED:  Added this function to make one place for wme's being added to
 *   the GDS.  Callback for wme added to GDS is made here.
 */
void add_wme_to_gds(agent* agentPtr, goal_dependency_set* gds, wme* wme_to_add)
{
   /* Set the correct GDS for this wme (wme's point to their gds) */
   wme_to_add->gds = gds;
   insert_at_head_of_dll(gds->wmes_in_gds, wme_to_add, gds_next, gds_prev);
                
   if (agentPtr->soar_verbose_flag || agentPtr->sysparams[TRACE_WM_CHANGES_SYSPARAM]) 
   {                    
	   print(agentPtr, "Adding to GDS for S%d", wme_to_add->gds->goal->id.level);    
	   print(agentPtr, " WME: "); 
	   char buf[256];
	   snprintf(buf, 254, "Adding to GDS for S%d", wme_to_add->gds->goal->id.level);
	   gSKI_MakeAgentCallbackXML(agentPtr, kFunctionBeginTag, kTagVerbose);
	   gSKI_MakeAgentCallbackXML(agentPtr, kFunctionAddAttribute, kTypeString, buf);
	   print_wme(agentPtr, wme_to_add);
	   gSKI_MakeAgentCallbackXML(agentPtr, kFunctionEndTag, kTagVerbose);               
   }
 
   /* Callback gSKI (AFTER) */
   gSKI_MakeAgentCallback(gSKI_K_EVENT_GDS_WME_ADDED, 1, 
                          agentPtr, static_cast<void*>(wme_to_add));
}

/*
========================

========================
*/
void elaborate_gds (agent* thisAgent) {

   wme *wme_matching_this_cond;
   goal_stack_level  wme_goal_level;
   preference *pref_for_this_wme, *pref;
   condition *cond;
   parent_inst *curr_pi, *temp_pi;
   slot *s;
   instantiation *inst;

   for (curr_pi=thisAgent->parent_list_head; curr_pi; curr_pi=temp_pi) {

      inst = curr_pi->inst;

#ifdef DEBUG_GDS
      print_with_symbols(thisAgent, "\n      EXPLORING INSTANTIATION: %y\n",curr_pi->inst->prod->name);
      print(thisAgent, "      ");
      print_instantiation_with_wmes( thisAgent, curr_pi->inst , TIMETAG_WME_TRACE, -1);
#endif

      for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next) 
      {

         if (cond->type != POSITIVE_CONDITION) 
            continue;

         /* We'll deal with negative instantiations after we get the
         * positive ones figured out */

         wme_matching_this_cond = cond->bt.wme_;
         wme_goal_level         = cond->bt.level;
         pref_for_this_wme      = wme_matching_this_cond->preference;

#ifdef DEBUG_GDS
         print(thisAgent, "\n       wme_matching_this_cond at goal_level = %d : ",
            wme_goal_level);
         print_wme(thisAgent, wme_matching_this_cond); 

         if (pref_for_this_wme) {
            print(thisAgent, "       pref_for_this_wme                        : ");
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
               print(thisAgent, "         this wme has no preferences (it's an arch-created wme)\n");
            }
            else if (wme_goal_level < inst->match_goal_level) 
            {
               print(thisAgent, "         this wme is in the supergoal\n");
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
                     free_memory(thisAgent, wme_matching_this_cond->gds,
                        MISCELLANEOUS_MEM_USAGE);
#ifdef DEBUG_GDS
                     print(thisAgent, "\n  REMOVING GDS FROM MEMORY.");
#endif
                  }

                  /* JC ADDED: Separate adding wme to GDS as a function */
                  add_wme_to_gds(thisAgent, inst->match_goal->id.gds, wme_matching_this_cond);

                  //                  wme_matching_this_cond->gds = inst->match_goal->id.gds;
                  //                  insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds, 
                  //                     wme_matching_this_cond, gds_next,
                  //                     gds_prev);
#ifdef DEBUG_GDS
                  print(thisAgent, "\n       .....GDS' goal is NIL so switching from old to new GDS list....\n"); 
#endif

               } 
               else if (wme_matching_this_cond->gds->goal->id.level >
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
                     print(thisAgent, "\n\n\n HELLO! HELLO! The inst->match_goal_level is 1");

                  fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds, \
                     wme_matching_this_cond, wme,
                     gds_next, gds_prev);
                  if (!wme_matching_this_cond->gds->wmes_in_gds) {
                     free_memory(thisAgent, wme_matching_this_cond->gds,
                        MISCELLANEOUS_MEM_USAGE);
#ifdef DEBUG_GDS
                     print(thisAgent, "\n  REMOVING GDS FROM MEMORY.");
#endif
                  } 
                  /* JC ADDED: Separate adding wme to GDS as a function */
                  add_wme_to_gds(thisAgent, inst->match_goal->id.gds, wme_matching_this_cond);

                  //                  wme_matching_this_cond->gds = inst->match_goal->id.gds;
                  //                  insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
                  //                     wme_matching_this_cond, gds_next,
                  //                     gds_prev);
#ifdef DEBUG_GDS
                  print(thisAgent, "\n       ....switching from old to new GDS list....\n");
#endif
                  wme_matching_this_cond->gds = inst->match_goal->id.gds;
               }
            } 
            else 
            {
               /* We know that the WME should be in the GDS of the current
               * goal if the WME's GDS does not already exist.
               * (i.e., if NIL GDS) */

               /* JC ADDED: Separate adding wme to GDS as a function */
               add_wme_to_gds(thisAgent, inst->match_goal->id.gds, wme_matching_this_cond);

               //               wme_matching_this_cond->gds = inst->match_goal->id.gds;
               //               insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
               //                  wme_matching_this_cond, gds_next, gds_prev);

               if (wme_matching_this_cond->gds->wmes_in_gds->gds_prev)
                  print(thisAgent, "\nDEBUG DEBUG : The new header should never have a prev value.\n");
#ifdef DEBUG_GDS
               print_with_symbols(thisAgent, "\n       ......WME did not have defined GDS.  Now adding to goal [%y].\n", wme_matching_this_cond->gds->goal); 
#endif
            } /* end else clause for "if wme_matching_this_cond->gds != NIL" */


#ifdef DEBUG_GDS
            print(thisAgent, "            Added WME to GDS for goal = %d",
               wme_matching_this_cond->gds->goal->id.level);
            print_with_symbols(thisAgent, " [%y]\n", wme_matching_this_cond->gds->goal);  
#endif
         } /* end "wme in supergoal or arch-supported" */
         else 
         {
            /* wme must be local */

            /* if wme's pref is o-supported, then just ignore it and
            * move to next condition */
            if (pref_for_this_wme->o_supported == TRUE) {
#ifdef DEBUG_GDS
               print(thisAgent, "         this wme is local and o-supported\n");
#endif
               continue;
            }

            else {
               /* wme's pref is i-supported, so remember it's instantiation
               * for later examination */

               /* this test avoids "backtracing" through the top state */
               if (inst->match_goal_level == 1) {
#ifdef DEBUG_GDS
                  print(thisAgent, "         don't back up through top state\n");  
                  if (inst->prod)
                     if (inst->prod->name)
                        print_with_symbols(thisAgent, "         don't back up through top state for instantiation %y\n", inst->prod->name);
#endif
                  continue;
               }

               else { /* (inst->match_goal_level != 1) */
#ifdef DEBUG_GDS
                  print(thisAgent, "         this wme is local and i-supported\n"); 
#endif
                  s = find_slot (pref_for_this_wme->id, pref_for_this_wme->attr);
                  if (s == NIL) 
                  {
                     /* this must be an arch-wme from a fake instantiation */

#ifdef DEBUG_GDS
                     print(thisAgent, "here's the wme with no slot:\t");
                     print_wme(thisAgent, pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme_);
#endif

                     /* this is the same code as above, just using the 
                     * differently-named pointer.  it probably should
                     * be a subroutine */
                     {
                        wme *fake_inst_wme_cond;

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
                                 free_memory(thisAgent, fake_inst_wme_cond->gds, MISCELLANEOUS_MEM_USAGE);
#ifdef DEBUG_GDS
                                 print(thisAgent, "\n  REMOVING GDS FROM MEMORY.");
#endif
                              }

                              /* JC ADDED: Separate adding wme to GDS as a function */
                              add_wme_to_gds(thisAgent, inst->match_goal->id.gds, fake_inst_wme_cond);

                              //                                 fake_inst_wme_cond->gds = inst->match_goal->id.gds;
                              //                                 insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds, 
                              //                                                       fake_inst_wme_cond, gds_next, gds_prev);
#ifdef DEBUG_GDS
                              print(thisAgent, "\n       .....GDS' goal is NIL so switching from old to new GDS list....\n"); 
#endif
                           } 
                           else if (fake_inst_wme_cond->gds->goal->id.level > inst->match_goal_level) 
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
                                 print(thisAgent, "\n\n\n\n\n HELLO! HELLO! The inst->match_goal_level is 1");

                              fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds, \
                                 fake_inst_wme_cond, wme,
                                 gds_next, gds_prev);
                              if (!fake_inst_wme_cond->gds->wmes_in_gds) 
                              {
                                 free_memory(thisAgent, fake_inst_wme_cond->gds,
                                    MISCELLANEOUS_MEM_USAGE);
#ifdef DEBUG_GDS
                                 print(thisAgent, "\n  REMOVING GDS FROM MEMORY.");
#endif
                              }

                              /* JC ADDED: Separate adding wme to GDS as a function */
                              add_wme_to_gds(thisAgent, inst->match_goal->id.gds, fake_inst_wme_cond);

                              //                                 fake_inst_wme_cond->gds = inst->match_goal->id.gds;
                              //                                 insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds,
                              //                                    fake_inst_wme_cond, gds_next,
                              //                                    gds_prev);
#ifdef DEBUG_GDS
                              print(thisAgent, "\n       .....switching from old to new GDS list....\n");
#endif
                              fake_inst_wme_cond->gds = inst->match_goal->id.gds;
                           }
                        } 
                        else 
                        {
                           /* We know that the WME should be in the GDS of
                           * the current goal if the WME's GDS does not
                           * already exist. (i.e., if NIL GDS) */

                           /* JC ADDED: Separate adding wme to GDS as a function */
                           add_wme_to_gds(thisAgent, inst->match_goal->id.gds, fake_inst_wme_cond);

                           //                              fake_inst_wme_cond->gds = inst->match_goal->id.gds;
                           //                              insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds,
                           //                                                    fake_inst_wme_cond,
                           //                                                    gds_next, gds_prev);

                           if (fake_inst_wme_cond->gds->wmes_in_gds->gds_prev)
                              print(thisAgent, "\nDEBUG DEBUG : The new header should never have a prev value.\n");
#ifdef DEBUG_GDS
                           print_with_symbols(thisAgent, "\n       ......WME did not have defined GDS.  Now adding to goal [%y].\n", fake_inst_wme_cond->gds->goal); 
#endif
                        }
#ifdef DEBUG_GDS
                        print(thisAgent, "            Added WME to GDS for goal = %d", fake_inst_wme_cond->gds->goal->id.level);
                        print_with_symbols(thisAgent, " [%y]\n",
                           fake_inst_wme_cond->gds->goal);  
#endif
                     }  /* matches { wme *fake_inst_wme_cond  */
                  } 
                  else 
                  {
                     /* this was the original "local & i-supported" action */
                     for (pref=s->preferences[ACCEPTABLE_PREFERENCE_TYPE]; 
                           pref; pref=pref->next) 
                     {

#ifdef DEBUG_GDS
                        print(thisAgent, "           looking at pref for the wme: ");
                        print_preference(thisAgent, pref); 
#endif


                        /* REW: 2004-05-27: Bug fix
                           We must check that the value with acceptable pref for the slot
                           is the same as the value for the wme in the condition, since
                           operators can have acceptable preferences for values other than
                           the WME value.  We dont want to backtrack thru acceptable prefs
                           for other operators */
                
                        if (pref->value == wme_matching_this_cond->value) {
                           

                        /* REW BUG: may have to go over all insts regardless
                        * of this visited_already flag... */

                        if (pref->inst->GDS_evaluated_already == FALSE) 
                        {

#ifdef DEBUG_GDS	      
                           print_with_symbols(thisAgent, "\n           adding inst that produced the pref to GDS: %y\n",pref->inst->prod->name); 
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
                           pref->inst->GDS_evaluated_already = TRUE;
                           ////////////////////////////////////////////////////// 
                           } 
#ifdef DEBUG_GDS
                           else 
                           {
                              print_with_symbols(thisAgent, "\n           ignoring inst %y because it is at a lower level than the GDS\n",pref->inst->prod->name);
                              pref->inst->GDS_evaluated_already = TRUE;
                           }
#endif
                           /* REW: 2003-12-07 */

                           //////////////////////////////////////////////////////
                        }
#ifdef DEBUG_GDS
                        else 
                        {
                           print(thisAgent, "           the inst producing this pref was already explored; skipping it\n"); 
                           }
#endif

                           }
#ifdef DEBUG_GDS
                        else
                        {
                           print("        this inst is for a pref with a differnt value than the condition WME; skippint it\n");
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
         curr_pi->next->prev = curr_pi->prev;

      if (curr_pi->prev != NIL) 
         curr_pi->prev->next = curr_pi->next;

      if (thisAgent->parent_list_head == curr_pi)
         thisAgent->parent_list_head = curr_pi->next;

      temp_pi = curr_pi->next;
      free(curr_pi);

   } /* end of "for (curr_pi = thisAgent->parent_list_head ... */


   if (thisAgent->parent_list_head != NIL) 
   {

#ifdef DEBUG_GDS
      print(thisAgent, "\n    RECURSING using these parents:\n");
      for (curr_pi = thisAgent->parent_list_head;
         curr_pi;
         curr_pi = curr_pi->next) {
            print_with_symbols(thisAgent, "      %y\n",curr_pi->inst->prod->name);
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

void gds_invalid_so_remove_goal (agent* thisAgent, wme *w) {

   /* JC ADDED: Tell gSKI that the goals stack is about to be blown away */
   gSKI_MakeAgentCallback(gSKI_K_EVENT_GDS_VIOLATED, 0, thisAgent, static_cast<void*>(w));

  /* REW: begin 11.25.96 */ 
  #ifndef NO_TIMING_STUFF
  #ifdef DETAILED_TIMING_STATS
  start_timer(thisAgent, &thisAgent->start_gds_tv);
  #endif
  #endif
  /* REW: end   11.25.96 */ 

  /* This call to GDS_PrintCmd will have to be uncommented later. -ajc */
  if (thisAgent->soar_verbose_flag) {} //GDS_PrintCmd();

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

   if (thisAgent->highest_goal_whose_context_changed) {
      if (thisAgent->highest_goal_whose_context_changed->id.level >=
          w->gds->goal->id.level) {
        thisAgent->highest_goal_whose_context_changed =
	  w->gds->goal->id.higher_goal;
      }
   } else {
     /* If nothing has yet changed (highest_ ... = NIL) then set
      * the goal automatically */
     thisAgent->highest_goal_whose_context_changed =
       w->gds->goal->id.higher_goal; 
   }

   if (thisAgent->sysparams[TRACE_OPERAND2_REMOVALS_SYSPARAM]) {
     print_with_symbols(thisAgent, "\n    REMOVING GOAL [%y] due to change in GDS WME ",
			w->gds->goal);
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
  stop_timer(thisAgent, &thisAgent->start_gds_tv, 
             &thisAgent->gds_cpu_time[thisAgent->current_phase]);
  #endif
  #endif
  /* REW: end   11.25.96 */ 
}


void free_parent_list(agent* thisAgent)
{
  parent_inst *curr_pi;

  for (curr_pi = thisAgent->parent_list_head;
       curr_pi;
       curr_pi = curr_pi->next)
     free(curr_pi);

  thisAgent->parent_list_head = NIL;
}

void create_gds_for_goal( agent* thisAgent, Symbol *goal){
   goal_dependency_set *gds;

   gds = static_cast<gds_struct *>(allocate_memory(thisAgent, sizeof(goal_dependency_set), 
												   MISCELLANEOUS_MEM_USAGE));
   gds->goal = goal;
   gds->wmes_in_gds = NIL;
   goal->id.gds = gds;
   #ifdef DEBUG_GDS
     print_with_symbols(thisAgent, "\nCreated GDS for goal [%y].\n", gds->goal);
   #endif
}

#ifdef NUMERIC_INDIFFERENCE

/* REW: 2003-01-06 */
/* This a helper function that sets the decider flag to candidate for
   all the items on the candidate list and initializes the counters 
   that will track the total probability distributions to zero.

   It's okay to muck with the 
   decider flags here because this will be called after the decision
   has been determined to be a choice among indifferent candidates.

   Note: the slot is only needed for debugging/data verification

   Note: slot parameter removed by voigtjr to quell compiler warnings
*/

/* SAN: 2003-10-30 */
/* Revised - this function also initializes candidate's value to default value for
   appropriate numeric-indifferent-mode 
*/

/*void initialize_indifferent_candidates_for_probability_selection(slot *s, preference *candidates)*/
void initialize_indifferent_candidates_for_probability_selection(preference * candidates)
{
    preference *cand = 0;

    for (cand = candidates; cand != NIL; cand = cand->next_candidate) {
        /* print_with_symbols("\nInitializing candidate %y",cand->value); 
         */
        cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
        cand->total_preferences_for_candidate = 0;
		cand->sum_of_probability = 0;
         
    }
}

/*unsigned int count_candidates(slot *s, preference *candidates)*/
unsigned int count_candidates(preference * candidates)
{
    unsigned int numCandidates = 0;
    preference *cand = 0;

    /*
       Count up the number of candidates
       REW: 2003-01-06
       I'm assuming that all of the candidates have unary or 
       unary+value (binary) indifferent preferences at this point.
       So we loop over the candidates list and count the number of
       elements in the list.
     */

    for (cand = candidates; cand != NIL; cand = cand->next_candidate)
        numCandidates++;

    return numCandidates;
}

/* Below is the Temperature, used to keep summed indifferent
   preferences within a reasonable range, since they will be used as
	 an exponent to the number 10, and must be stored in a 64 bit double
*/
#define TEMPERATURE 25.0

preference *probabilistically_select(agent* thisAgent, slot * s, preference * candidates)
{
    preference *cand = 0;
    preference *pref = 0;
    double total_probability = 0;
    unsigned int numCandidates = 0;
    double selectedProbability = 0;
    double currentSumOfValues = 0;
//    static int initialized_rand = 0;
    //unsigned long rn = 0;
	double rn = 0;
	double default_ni;

    /* initialized, but not referenced, so i commented them out:
       preference*    selectedCandidate=0;
       unsigned int   currentCandidate=0; */

    assert(s != 0);
    assert(candidates != 0);

// RPM 12/05 The seed shouldn't be set here (see bug 593) and we aren't using srand and rand anymore (see bug 595)
//    if (!initialized_rand) {
//        srand((unsigned) time(NULL));
//        initialized_rand = 1;
//    }

    /*
       print("\nCandidates at top of probabilistically_select"); 
       print_candidates(candidates);  
     */

    /* s param uneccesary, commented out to quell compiler warning.  see comment
       block above the following function definitions
       initialize_indifferent_candidates_for_probability_selection(s, candidates);
       numCandidates = count_candidates(s,candidates); */

    initialize_indifferent_candidates_for_probability_selection(candidates);
    numCandidates = count_candidates(candidates);

    /*
       print("\n numCandidates = %d", numCandidates);
       print("\nCandidates before unary indifferent loop");
       print_candidates(candidates); 
     */


	switch (thisAgent->numeric_indifferent_mode) {
	case NUMERIC_INDIFFERENT_MODE_AVG:
		default_ni = 50;
		break;

	case NUMERIC_INDIFFERENT_MODE_SUM:
	default:
		default_ni = 0;
		break;
	}
    /*
       BUGBUGBUG 
       Next some error checking here to ensure that the binary preference
       is indeed really an indifferent+value preference....
       someday.

       also, precision loss (long to float conversion) here:
       value = (float)pref->referent->ic.value;
     */

    for (pref = s->preferences[BINARY_INDIFFERENT_PREFERENCE_TYPE]; pref != NIL; pref = pref->next) {
        /*print_with_symbols("\nPreference for %y", pref->value); */
        float value;
        if (pref->referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
            value = pref->referent->fc.value;
        } else if (pref->referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
            value = (float) pref->referent->ic.value;
        } else
            continue;
        for (cand = candidates; cand != NIL; cand = cand->next_candidate) {
            /*print_with_symbols("\nConsidering candidate %y", cand->value); */

            if (cand->value == pref->value) {
                cand->total_preferences_for_candidate += 1;
                cand->sum_of_probability += value;
            }
        }
    }

	for (cand = candidates; cand != NIL; cand = cand->next_candidate) {
		if (cand->total_preferences_for_candidate == 0) {
			cand->sum_of_probability = default_ni;
			cand->total_preferences_for_candidate = 1;
		}
	}

    if (thisAgent->numeric_indifferent_mode == NUMERIC_INDIFFERENT_MODE_SUM) {

        total_probability = 0.0;
        for (cand = candidates; cand != NIL; cand = cand->next_candidate) {

            if (thisAgent->sysparams[TRACE_INDIFFERENT_SYSPARAM]){
				print_with_symbols(thisAgent, "\n Candidate %y:  ", cand->value);
		           print(thisAgent, "Value (Sum) = %f", exp(cand->sum_of_probability / TEMPERATURE));
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagCandidate);
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kCandidateName, symbol_to_string (thisAgent, cand->value, true, 0, 0));
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kCandidateType, kCandidateTypeSum);
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kCandidateValue, exp(cand->sum_of_probability / TEMPERATURE));
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagCandidate);
			}     
            /*  Total Probability represents the range of values, we expect
             *  the use of negative valued preferences, so its possible the
             *  sum is negative, here that means a fractional probability
             */
            total_probability += exp(cand->sum_of_probability / TEMPERATURE);
            /* print("\n   Total (Sum) Probability = %f", total_probability ); */
        }

        /* Now select the candidate */

        //print(thisAgent, "\n");

		/* RPM 12/05 replacing calls to rand() with calls to SoarRand; see bug 595 */
		//rn = rand();
		rn = SoarRand(); // generates a number in [0,1]
        //selectedProbability = ((double) rn / (double) RAND_MAX) * total_probability;
		selectedProbability = rn * total_probability;
        currentSumOfValues = 0;

        for (cand = candidates; cand != NIL; cand = cand->next_candidate) {

            currentSumOfValues += exp(cand->sum_of_probability / TEMPERATURE);

            if (selectedProbability <= currentSumOfValues) {
                /* 
                   print_with_symbols("\n    Returning (Sum) candidate %y", cand->value); 
                 */

                return cand;
            }
        }

    } else if (thisAgent->numeric_indifferent_mode == NUMERIC_INDIFFERENT_MODE_AVG) {

        total_probability = 0.0;
        for (cand = candidates; cand != NIL; cand = cand->next_candidate) {

            if (thisAgent->sysparams[TRACE_INDIFFERENT_SYSPARAM]) {
				print_with_symbols(thisAgent, "\n Candidate %y:  ", cand->value);  
		           print(thisAgent, "Value (Avg) = %f", fabs(cand->sum_of_probability / cand->total_preferences_for_candidate));
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagCandidate);
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kCandidateName, symbol_to_string (thisAgent, cand->value, true, 0, 0));
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kCandidateType, kCandidateTypeAvg);
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kCandidateValue, fabs(cand->sum_of_probability / cand->total_preferences_for_candidate));
               gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagCandidate);
			}    
            /* Total probability represents the range of values that
             * we'll map into for selection.  Here we don't expect the use
             * of negative values, so we'll warn when we see one.
             */

            total_probability += fabs(cand->sum_of_probability / cand->total_preferences_for_candidate);

            if (cand->sum_of_probability < 0.0) {
                print_with_symbols
                    (thisAgent, "WARNING: Candidate %y has a negative value, which is unexpected with 'numeric-indifferent-mode -avg'",
                     cand->value);
				growable_string gs = make_blank_growable_string(thisAgent);
				add_to_growable_string(thisAgent, &gs, "WARNING: Candidate ");
				add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, cand->value, true, 0, 0));
				add_to_growable_string(thisAgent, &gs, " has a negative value, which is unexpected with 'numeric-indifferent-mode -avg'");
				GenerateWarningXML(thisAgent, text_of_growable_string(gs));
				free_growable_string(thisAgent, gs);
            }
            /* print("\n   Total (Avg) Probability = %f", total_probability ); */
        }

        /* Now select the candidate */

		//print(thisAgent, "\n");

		/* RPM 12/05 replacing calls to rand() with calls to SoarRand; see bug 595 */
        //rn = rand();
		rn = SoarRand(); 
        selectedProbability = rn * total_probability;
        currentSumOfValues = 0;

        for (cand = candidates; cand != NIL; cand = cand->next_candidate) {

            currentSumOfValues += fabs(cand->sum_of_probability / cand->total_preferences_for_candidate);

            if (selectedProbability <= currentSumOfValues) {
                /*
                   print_with_symbols("\n    Returning (Avg) candidate %y", cand->value); 
                 */

                return cand;
            }
        }

    } else {
        print(thisAgent, "\nERROR: Invalid Numeric Indifferent Mode!\n");
    }

    print(thisAgent, "\nERROR: Probability Selection failed. This should never happen.\n");
    return NIL;

}

#endif
