/*************************************************************************
 *
 *  file:  decide.c
 *
 * =======================================================================
 *  Decider and  Associated Routines for Soar 6
 *
 *  This file contains the decider as well as routine for managing
 *  slots, and the garbage collection of disconnected WMEs.
 * =======================================================================
 *
 * Copyright 1995-2003 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
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

#include "soarkernel.h"


void remove_existing_attribute_impasse_for_slot (slot *s);
void post_link_addition (Symbol *from, Symbol *to);
void post_link_removal (Symbol *from, Symbol *to);

/* REW: begin 09.15.96   additions for Soar8 architecture */
void elaborate_gds (void);
void gds_invalid_so_remove_goal (wme *w);
void free_parent_list(void);
void uniquely_add_to_head_of_dll(instantiation *inst);
void create_gds_for_goal( Symbol *goal );
extern void remove_operator_if_necessary(slot *s, wme *w);
extern GDS_PrintCmd();
/* REW: end   09.15.96 */

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

/* ======================================================================

                  Acceptable Preference WME Routines

   Whenever some acceptable or require preference for a context slot
   changes, we call mark_context_slot_as_acceptable_preference_changed().

   At the end of the phase, do_buffered_acceptable_preference_wme_changes()
   is called to update the acceptable preference wmes.  This should be
   called *before* do_buffered_link_changes() and do_buffered_wm_changes().
====================================================================== */

void mark_context_slot_as_acceptable_preference_changed (slot *s) {
  dl_cons *dc;
  
  if (s->acceptable_preference_changed) return;
  allocate_with_pool (&current_agent(dl_cons_pool), &dc);
  dc->item = s;
  s->acceptable_preference_changed = dc;
  insert_at_head_of_dll (current_agent(context_slots_with_changed_acceptable_preferences), dc,
                         next, prev);
}

/* --- This updates the acceptable preference wmes for a single slot. --- */
void do_acceptable_preference_wme_changes_for_slot (slot *s) {
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
  while (w) {
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
      if (current_agent(operand2_mode))
         remove_operator_if_necessary(s,w);
/* REW: end   09.15.96 */
      remove_wme_from_wm (w);
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
      w = make_wme (p->id, p->attr, p->value, TRUE);
      insert_at_head_of_dll (s->acceptable_preference_wmes, w, next, prev);
      w->preference = p;
      add_wme_to_wm (w);
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
      w = make_wme (p->id, p->attr, p->value, TRUE);
      insert_at_head_of_dll (s->acceptable_preference_wmes, w, next, prev);
      w->preference = p;
      add_wme_to_wm (w);
      p->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
      p->value->common.a.decider_wme = w;
    }
  }
}

void do_buffered_acceptable_preference_wme_changes (void) {
  dl_cons *dc;
  slot *s;

  while (current_agent(context_slots_with_changed_acceptable_preferences)) {
    dc = current_agent(context_slots_with_changed_acceptable_preferences);
    current_agent(context_slots_with_changed_acceptable_preferences) = dc->next;
    s = dc->item;
    free_with_pool (&current_agent(dl_cons_pool), dc);
    do_acceptable_preference_wme_changes_for_slot (s);
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

void post_link_addition (Symbol *from, Symbol *to) {

  /* --- don't add links to goals/impasses, except the special one
     (NIL,goal) --- */
  if ((to->id.isa_goal || to->id.isa_impasse) && from) return;

  to->id.link_count++;

#ifdef DEBUG_LINKS
  if (from)
    print_with_symbols ("\nAdding link from %y to %y", from, to);
  else
    print_with_symbols ("\nAdding special link to %y", to);
  print (" (count=%lu)", to->id.link_count);
#endif

  if (!from) return;  /* if adding a special link, we're done */
  
  /* --- if adding link from same level, ignore it --- */
  if (from->id.promotion_level == to->id.promotion_level) return;

  /* --- if adding link from lower to higher, mark higher accordingly --- */
  if (from->id.promotion_level > to->id.promotion_level) {
    to->id.could_be_a_link_from_below = TRUE;
    return;
  }

  /* --- otherwise buffer it for later --- */
  to->id.promotion_level = from->id.promotion_level;
  symbol_add_ref (to);
  push (to, current_agent(promoted_ids));
}

/* ----------------------------------------------
   Promote an id and its transitive closure.
---------------------------------------------- */

#define promote_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      promote_id_and_tc(sym,new_level); }
                                    
void promote_id_and_tc (Symbol *id, goal_stack_level new_level) {
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
    char msg[128];
    strcpy (msg, "decide.c: Internal error: tried to promote a goal or impasse id\n");
    abort_with_fatal_error(msg);
    /* Note--since we can't promote a goal, we don't have to worry about
       slot->acceptable_preference_wmes below */
  }
  
  /* --- scan through all preferences and wmes for all slots for this id -- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    promote_if_needed (w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
      promote_if_needed (pref->value);
      if (preference_is_binary(pref->type))
        promote_if_needed (pref->referent);
    }
    for (w=s->wmes; w!=NIL; w=w->next)
      promote_if_needed (w->value);
  } /* end of for slots loop */
}

/* ----------------------------------------------
   Do all buffered promotions.
---------------------------------------------- */

void do_promotion (void) {
  cons *c;
  Symbol *to;

  while (current_agent(promoted_ids)) {
    c = current_agent(promoted_ids);
    to = c->first;
    current_agent(promoted_ids) = current_agent(promoted_ids)->rest;
    free_cons (c);
    promote_id_and_tc (to, to->id.promotion_level);
    symbol_remove_ref (to);
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

void post_link_removal (Symbol *from, Symbol *to) {
  dl_cons *dc;

  /* --- don't remove links to goals/impasses, except the special one
     (NIL,goal) --- */
  if ((to->id.isa_goal || to->id.isa_impasse) && from) return;

  to->id.link_count--;

#ifdef DEBUG_LINKS
  if (from) {
    print_with_symbols ("\nRemoving link from %y to %y", from, to);
    print (" (%d to %d)", from->id.level, to->id.level);
  } else {
    print_with_symbols ("\nRemoving special link to %y ", to);
    print (" (%d)", to->id.level);
  }
  print (" (count=%lu)", to->id.link_count);
#endif

  /* --- if a gc is in progress, handle differently --- */
  if (current_agent(link_update_mode)==JUST_UPDATE_COUNT) return;

  if ((current_agent(link_update_mode)==UPDATE_DISCONNECTED_IDS_LIST) &&
      (to->id.link_count==0)) {
    if (to->id.unknown_level) {
      dc = to->id.unknown_level;
      remove_from_dll (current_agent(ids_with_unknown_level), dc, next, prev);
      insert_at_head_of_dll (current_agent(disconnected_ids), dc, next, prev);
    } else {
      symbol_add_ref (to);
      allocate_with_pool (&current_agent(dl_cons_pool), &dc);
      dc->item = to;
      to->id.unknown_level = dc;   
      insert_at_head_of_dll (current_agent(disconnected_ids), dc, next, prev);
    }
    return;
  }
    
  /* --- if removing a link from a different level, there must be some other
     link at the same level, so we can ignore this change --- */
  if (from && (from->id.level != to->id.level)) return;
  
  if (! to->id.unknown_level) {
    symbol_add_ref (to);
    allocate_with_pool (&current_agent(dl_cons_pool), &dc);
    dc->item = to;
    to->id.unknown_level = dc;   
    insert_at_head_of_dll (current_agent(ids_with_unknown_level), dc, next, prev);
  }
}

/* ----------------------------------------------
   Garbage collect an identifier.  This removes
   all wmes, input wmes, and preferences for that
   id from TM.
---------------------------------------------- */

void garbage_collect_id (Symbol *id) {
  slot *s;
  preference *pref, *next_pref;

#ifdef DEBUG_LINKS  
  print_with_symbols ("\n*** Garbage collecting id: %y",id);
#endif

  /* Note--for goal/impasse id's, this does not remove the impasse wme's.
     This is handled by remove_existing_such-and-such... */

  /* --- remove any input wmes from the id --- */
  remove_wme_list_from_wm (id->id.input_wmes);
  id->id.input_wmes = NIL;

  for (s=id->id.slots; s!=NIL; s=s->next) {
    /* --- remove any existing attribute impasse for the slot --- */
    if (s->impasse_type!=NONE_IMPASSE_TYPE)
      remove_existing_attribute_impasse_for_slot (s);
    /* --- remove all wme's from the slot --- */
    remove_wme_list_from_wm (s->wmes);
    s->wmes = NIL;
    /* --- remove all preferences for the slot --- */
    pref = s->all_preferences;
    while (pref) {
      next_pref = pref->all_of_slot_next;
      remove_preference_from_tm (pref);
      /* Note:  the call to remove_preference_from_slot handles the removal
         of acceptable_preference_wmes */
      pref = next_pref;
    }
    mark_slot_for_possible_removal (s);
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

#define mark_unknown_level_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      mark_id_and_tc_as_unknown_level(sym); }

void mark_id_and_tc_as_unknown_level (Symbol *id) {
  slot *s;
  preference *pref;
  wme *w;
  dl_cons *dc;

  /* --- if id is already marked, do nothing --- */
  if (id->id.tc_num==current_agent(mark_tc_number)) return;
  
  /* --- don't mark anything higher up as disconnected--in order to be higher
     up, it must have a link to it up there --- */
  if (id->id.level < current_agent(level_at_which_marking_started)) return; 

  /* --- mark id, so we won't do it again later --- */
  id->id.tc_num = current_agent(mark_tc_number);

  /* --- update range of goal stack levels we'll need to walk --- */
  if (id->id.level < current_agent(highest_level_anything_could_fall_from))
    current_agent(highest_level_anything_could_fall_from) = id->id.level;
  if (id->id.level > current_agent(lowest_level_anything_could_fall_to))
    current_agent(lowest_level_anything_could_fall_to) = id->id.level;
  if (id->id.could_be_a_link_from_below)
    current_agent(lowest_level_anything_could_fall_to) = LOWEST_POSSIBLE_GOAL_LEVEL;

  /* --- add id to the set of ids with unknown level --- */
  if (! id->id.unknown_level) {
    allocate_with_pool (&current_agent(dl_cons_pool), &dc);
    dc->item = id;
    id->id.unknown_level = dc;
    insert_at_head_of_dll (current_agent(ids_with_unknown_level), dc, next, prev);
    symbol_add_ref (id);
  }

  /* -- scan through all preferences and wmes for all slots for this id -- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    mark_unknown_level_if_needed (w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
      mark_unknown_level_if_needed (pref->value);
      if (preference_is_binary(pref->type))
        mark_unknown_level_if_needed (pref->referent);
    }
    if(s->impasse_id) mark_unknown_level_if_needed(s->impasse_id);
    for (w=s->wmes; w!=NIL; w=w->next)
      mark_unknown_level_if_needed (w->value);
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

#define update_levels_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      if ((sym)->id.tc_num!=current_agent(walk_tc_number)) \
        walk_and_update_levels(sym); }

void walk_and_update_levels (Symbol *id) {
  slot *s;
  preference *pref;
  wme *w;
  dl_cons *dc;

  /* --- mark id so we don't walk it twice --- */
  id->id.tc_num = current_agent(walk_tc_number);

  /* --- if we already know its level, and it's higher up, then exit --- */
  if ((! id->id.unknown_level) && (id->id.level < current_agent(walk_level))) return;

  /* --- if we didn't know its level before, we do now --- */
  if (id->id.unknown_level) {
    dc = id->id.unknown_level;
    remove_from_dll (current_agent(ids_with_unknown_level), dc, next, prev);
    free_with_pool (&current_agent(dl_cons_pool), dc);
    symbol_remove_ref (id);
    id->id.unknown_level = NIL;
    id->id.level = current_agent(walk_level);
    id->id.promotion_level = current_agent(walk_level);
  }
  
  /* -- scan through all preferences and wmes for all slots for this id -- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    update_levels_if_needed (w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
      update_levels_if_needed (pref->value);
      if (preference_is_binary(pref->type))
        update_levels_if_needed (pref->referent);
    }
    if(s->impasse_id) update_levels_if_needed(s->impasse_id);
    for (w=s->wmes; w!=NIL; w=w->next)
      update_levels_if_needed (w->value);
  } /* end of for slots loop */
}

/* ----------------------------------------------
   Do all buffered demotions and gc's.
---------------------------------------------- */

void do_demotion (void) {
  Symbol *g, *id;
  dl_cons *dc, *next_dc;

  /* --- scan through ids_with_unknown_level, move the ones with link_count==0
   *  over to disconnected_ids --- */
  for (dc=current_agent(ids_with_unknown_level); dc!=NIL; dc=next_dc) {
    next_dc = dc->next;
    id = dc->item;
    if (id->id.link_count==0) {
      remove_from_dll (current_agent(ids_with_unknown_level), dc, next, prev);
      insert_at_head_of_dll (current_agent(disconnected_ids), dc, next, prev);
    }
  }

  /* --- keep garbage collecting ids until nothing left to gc --- */
  current_agent(link_update_mode) = UPDATE_DISCONNECTED_IDS_LIST;
  while (current_agent(disconnected_ids)) {
    dc = current_agent(disconnected_ids);
    current_agent(disconnected_ids) = current_agent(disconnected_ids)->next;
    id = dc->item;
    free_with_pool (&current_agent(dl_cons_pool), dc);
    garbage_collect_id (id);
    symbol_remove_ref (id);
  }
  current_agent(link_update_mode) = UPDATE_LINKS_NORMALLY;
 
  /* --- if nothing's left with an unknown level, we're done --- */
  if (! current_agent(ids_with_unknown_level)) return;

  /* --- do the mark --- */
  current_agent(highest_level_anything_could_fall_from) =
                LOWEST_POSSIBLE_GOAL_LEVEL;
  current_agent(lowest_level_anything_could_fall_to) = -1;
  current_agent(mark_tc_number) = get_new_tc_number();
  for (dc=current_agent(ids_with_unknown_level); dc!=NIL; dc=dc->next) {
    id = dc->item;
    current_agent(level_at_which_marking_started) = id->id.level;
    mark_id_and_tc_as_unknown_level (id);
  }

  /* --- do the walk --- */
  g = current_agent(top_goal);
  while (TRUE) {
    if (!g) break;
    if (g->id.level > current_agent(lowest_level_anything_could_fall_to)) break;
    if (g->id.level >= current_agent(highest_level_anything_could_fall_from)) {
      current_agent(walk_level) = g->id.level;
      current_agent(walk_tc_number) = get_new_tc_number();
      walk_and_update_levels (g);
    }
    g = g->id.lower_goal;
  }

  /* --- GC anything left with an unknown level after the walk --- */
  current_agent(link_update_mode) = JUST_UPDATE_COUNT;
  while (current_agent(ids_with_unknown_level)) {
    dc = current_agent(ids_with_unknown_level);
    current_agent(ids_with_unknown_level) =
                  current_agent(ids_with_unknown_level)->next;
    id = dc->item;
    free_with_pool (&current_agent(dl_cons_pool), dc);
    id->id.unknown_level = NIL;    /* AGR 640:  GAP set to NIL because */
                                   /* symbol may still have pointers to it */
    garbage_collect_id (id);
    symbol_remove_ref (id);
  }
  current_agent(link_update_mode) = UPDATE_LINKS_NORMALLY;
}

/* ------------------------------------------------------------------
                       Do Buffered Link Changes

   This routine does all the buffered link (ownership) chages, updating
   the goal stack level on all identifiers and garbage collecting
   disconnected wmes.
------------------------------------------------------------------ */

void do_buffered_link_changes (void) {

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  struct timeval saved_start_tv;
#endif
#endif

  /* --- if no promotions or demotions are buffered, do nothing --- */
  if (! (current_agent(promoted_ids) ||
         current_agent(ids_with_unknown_level) ||
         current_agent(disconnected_ids))) return;

#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS  
  start_timer (&saved_start_tv);
#endif
#endif
  do_promotion ();  
  do_demotion ();
#ifndef NO_TIMING_STUFF
#ifdef DETAILED_TIMING_STATS
  stop_timer (&saved_start_tv, &current_agent(ownership_cpu_time[current_agent(current_phase)]));
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

byte run_preference_semantics (slot *s, preference **result_candidates) {
  preference *p, *p2, *cand, *prev_cand;
  bool match_found, not_all_indifferent, not_all_parallel;
  preference *candidates;

  /* --- if the slot has no preferences at all, things are trivial --- */
  if (! s->all_preferences) {
    if (! s->isa_context_slot) mark_slot_for_possible_removal (s);
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
  if ( ( (current_agent(attribute_preferences_mode)==2) ||
			(current_agent(operand2_mode) ==TRUE) )  &&
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
  not_all_indifferent = FALSE;
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
    /* --- if cand is unary indifferent, it's fine --- */
    if (cand->value->common.decider_flag==UNARY_INDIFFERENT_DECIDER_FLAG)
      continue;
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
    switch (current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM]) {
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
      print ("\nPlease choose one of the following:\n");
      for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
        num_candidates++;
        print ("  %d:  ", num_candidates);
        print_object_trace (cand->value);
        print ("\n");
      }
/* AGR 615 begin */
      print("Or choose one of the following to change the user-select mode\n");
      print("to something else:  %d (first), %d (last), %d (random)\n",
	     num_candidates+=1, num_candidates+=1, num_candidates+=1);
/* AGR 615 end */
      while (TRUE) {
        char ch;
#ifdef _WINDOWS
		char buff[256],msg[256];
		sprintf(msg,"Enter selection 1-%d",num_candidates);
		get_line_from_window(msg,buff,255);
		sscanf(msg,"%d",num_candidates);
#else
	/*  char buf[256]; /* kjh(CUSP-B10) */
        print ("Enter selection (1-%d): ", num_candidates);
        chosen_num = -1;
        scanf (" %d", &chosen_num);
        do { ch=getchar(); } while ((ch!='\n') && (ch!=EOF_AS_CHAR));

	if (ch==EOF_AS_CHAR) clearerr(stdin); /* Soar-Bugs #103, TMH */
	
     /* kjh(CUSP-B10) BEGIN*/
     /* Soar_Read(soar_agent, buf, 256);
	          sscanf(buf,"%d",&chosen_num);
     /* kjh(CUSP-B10) END*/

#endif
        if ((chosen_num>=1) && (chosen_num<=num_candidates)) break;
        print ("You must enter a number between 1 and %d\n", num_candidates);
      }
      if (current_agent(logging_to_file)) {
        char temp[50];
        sprintf (temp, "%d\n", chosen_num);
        print_string_to_log_file_only (temp);
      }
/* AGR 615 begin */
      switch (num_candidates - chosen_num) {
      case 2:
	set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_FIRST);
	print ("User-select mode changed to:  first\n");
	*result_candidates = candidates;
	break;
      case 1:
	set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_LAST);
	print ("User-select mode changed to:  last\n");
	for (cand = candidates; cand->next_candidate != NIL; cand = cand->next_candidate);
	*result_candidates = cand;
	break;
      case 0:
	set_sysparam (USER_SELECT_MODE_SYSPARAM, USER_SELECT_RANDOM);
	print ("User-select mode changed to:  random\n");
#if defined(THINK_C) || defined(__hpux) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)
	chosen_num = rand() % (num_candidates-3);
#else
	chosen_num = random() % (num_candidates-3);
#endif
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
      int num_candidates, chosen_num;
      num_candidates = 0;
      for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
        num_candidates++;
#if defined(THINK_C) || defined(__hpux) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)
      chosen_num = rand() % num_candidates;
#else
      chosen_num = random() % num_candidates;
#endif
      cand = candidates;
      while (chosen_num) { cand=cand->next_candidate; chosen_num--; }
      *result_candidates = cand;
      break;
    }
    default:
      { char msg[128];
      sprintf(msg, "decide.c: Error: bad value of user_select_mode: %d\n",
	      current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM]);
      abort_with_fatal_error(msg);
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


byte run_preference_semantics_for_consistency_check (slot *s, preference **result_candidates) {
  preference *p, *p2, *cand, *prev_cand;
  bool match_found, not_all_indifferent, not_all_parallel;
  preference *candidates;

  /* printf("\n       Checking the preference semantics for inconsistencies....\n"); */
  /* --- if the slot has no preferences at all, things are trivial --- */
  if (! s->all_preferences) {
    if (! s->isa_context_slot) mark_slot_for_possible_removal (s);
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
  if (( (current_agent(attribute_preferences_mode)==2) ||
		  (current_agent(operand2_mode) == TRUE) ) &&
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
  not_all_indifferent = FALSE;
  for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
    /* --- if cand is unary indifferent, it's fine --- */
    if (cand->value->common.decider_flag==UNARY_INDIFFERENT_DECIDER_FLAG)
      continue;
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

void add_impasse_wme (Symbol *id, Symbol *attr, Symbol *value, preference *p) {
  wme *w;
  
  w = make_wme (id, attr, value, FALSE);
  insert_at_head_of_dll (id->id.impasse_wmes, w, next, prev);
  w->preference = p;
  add_wme_to_wm (w);
}

/* ------------------------------------------------------------------
                         Create New Impasse
  
   This creates a new impasse, returning its identifier.  The caller is
   responsible for filling in either id->isa_impasse or id->isa_goal,
   and all the extra stuff for goal identifiers.
------------------------------------------------------------------ */

Symbol *create_new_impasse (bool isa_goal, Symbol *object, Symbol *attr,
                            byte impasse_type, goal_stack_level level) {
  Symbol *id;

  id = make_new_identifier ((char)(isa_goal ? 'S' : 'I'), level);
  post_link_addition (NIL, id);  /* add the special link */

  add_impasse_wme (id, current_agent(type_symbol), isa_goal ? current_agent(state_symbol) : current_agent(impasse_symbol),
                   NIL);

  if (isa_goal)
    add_impasse_wme (id, current_agent(superstate_symbol), object, NIL);
  else
    add_impasse_wme (id, current_agent(object_symbol), object, NIL);

  if (attr) add_impasse_wme (id, current_agent(attribute_symbol), attr, NIL);
  
  switch (impasse_type) {
  case NONE_IMPASSE_TYPE:
    break;    /* this happens only when creating the top goal */
  case CONSTRAINT_FAILURE_IMPASSE_TYPE:
    add_impasse_wme (id, current_agent(impasse_symbol), current_agent(constraint_failure_symbol), NIL);
    add_impasse_wme (id, current_agent(choices_symbol), current_agent(none_symbol), NIL);
    break;
  case CONFLICT_IMPASSE_TYPE:
    add_impasse_wme (id, current_agent(impasse_symbol), current_agent(conflict_symbol), NIL);
    add_impasse_wme (id, current_agent(choices_symbol), current_agent(multiple_symbol), NIL);
    break;
  case TIE_IMPASSE_TYPE:
    add_impasse_wme (id, current_agent(impasse_symbol), current_agent(tie_symbol), NIL);
    add_impasse_wme (id, current_agent(choices_symbol), current_agent(multiple_symbol), NIL);
    break;
  case NO_CHANGE_IMPASSE_TYPE:
    add_impasse_wme (id, current_agent(impasse_symbol), current_agent(no_change_symbol), NIL);
    add_impasse_wme (id, current_agent(choices_symbol), current_agent(none_symbol), NIL);
    break;
  }
  return id;
}

/* ------------------------------------------------------------------
               Create/Remove Attribute Impasse for Slot
  
   These routines create and remove an attribute impasse for a given
   slot.
------------------------------------------------------------------ */

void create_new_attribute_impasse_for_slot (slot *s, byte impasse_type) {
  Symbol *id;
  
  s->impasse_type = impasse_type;
  id = create_new_impasse (FALSE, s->id, s->attr, impasse_type,
                           ATTRIBUTE_IMPASSE_LEVEL);
  s->impasse_id = id;
  id->id.isa_impasse = TRUE;

#ifndef NO_CALLBACKS
  soar_invoke_callbacks(soar_agent, 
                       CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK, 
                       (soar_call_data) s);
#endif
}

void remove_existing_attribute_impasse_for_slot (slot *s) {
  Symbol *id;

#ifndef NO_CALLBACKS
  soar_invoke_callbacks(soar_agent, 
                       REMOVE_ATTRIBUTE_IMPASSE_CALLBACK, 
                       (soar_call_data) s);
#endif

  id = s->impasse_id;
  s->impasse_id = NIL;
  s->impasse_type = NONE_IMPASSE_TYPE;
  remove_wme_list_from_wm (id->id.impasse_wmes);
  id->id.impasse_wmes = NIL;
  post_link_removal (NIL, id);  /* remove the special link */
  symbol_remove_ref (id);
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
   to the fake preference.  *** for Soar 7.3, we changed the fake
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

preference *make_fake_preference_for_goal_item (Symbol *goal,
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
    char msg[128];
    strcpy (msg,
	    "decide.c: Internal error: couldn't find acceptable pref wme\n");
    abort_with_fatal_error(msg);
  }
  /* --- make the fake preference --- */
  /* kjc:  here's where we changed REQUIRE to ACCEPTABLE */
  pref = make_preference (ACCEPTABLE_PREFERENCE_TYPE, goal, current_agent(item_symbol),
                          cand->value, NIL);
  symbol_add_ref (pref->id);
  symbol_add_ref (pref->attr);
  symbol_add_ref (pref->value);
  insert_at_head_of_dll (goal->id.preferences_from_goal, pref,
                         all_of_goal_next, all_of_goal_prev);
  pref->on_goal_list = TRUE;
  preference_add_ref (pref);
  /* --- make the fake instantiation --- */
  allocate_with_pool (&current_agent(instantiation_pool), &inst);
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
  allocate_with_pool (&current_agent(condition_pool), &cond);
  cond->type = POSITIVE_CONDITION;
  cond->next = cond->prev = NIL;
  inst->top_of_instantiated_conditions = cond;
  inst->bottom_of_instantiated_conditions = cond;
  inst->nots = NIL;
  cond->data.tests.id_test = make_equality_test (ap_wme->id);
  cond->data.tests.attr_test = make_equality_test (ap_wme->attr);
  cond->data.tests.value_test = make_equality_test (ap_wme->value);
  cond->test_for_acceptable_preference = TRUE;
  cond->bt.wme = ap_wme;
  wme_add_ref (ap_wme);
  cond->bt.level = ap_wme->id->id.level;
  cond->bt.trace = NIL;
  cond->bt.prohibits = NIL;
  /* --- return the fake preference --- */
  return pref;
}

void remove_fake_preference_for_goal_item (preference *pref) {
  preference_remove_ref (pref); /* everything else happens automatically */
}

/* ------------------------------------------------------------------
                       Update Impasse Items
  
   This routine updates the set of ^item wmes on a goal or attribute
   impasse.  It takes the identifier of the goal/impasse, and a list
   of preferences (linked via the "next_candidate" field) for the new
   set of items that should be there.
------------------------------------------------------------------ */

void update_impasse_items (Symbol *id, preference *items) {
  wme *w, *next_w;
  preference *cand;
  preference *bt_pref;

  /* --- reset flags on existing items to "NOTHING" --- */
  for (w=id->id.impasse_wmes; w!=NIL; w=w->next)
    if (w->attr==current_agent(item_symbol))
      w->value->common.decider_flag = NOTHING_DECIDER_FLAG;

  /* --- mark set of desired items as "CANDIDATEs" --- */
  for (cand=items; cand!=NIL; cand=cand->next_candidate)
    cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;

  /* --- for each existing item:  if it's supposed to be there still, then
     mark it "ALREADY_EXISTING"; otherwise remove it --- */
  w = id->id.impasse_wmes;
  while (w) {
    next_w = w->next;
    if (w->attr==current_agent(item_symbol)) {
      if (w->value->common.decider_flag==CANDIDATE_DECIDER_FLAG) {
        w->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
        w->value->common.a.decider_wme = w; /* so we can update the pref later */
      } else {
        remove_from_dll (id->id.impasse_wmes, w, next, prev);
        if (id->id.isa_goal)
          remove_fake_preference_for_goal_item (w->preference);
        remove_wme_from_wm (w);
      }
    }
    w = next_w;
  }

  /* --- for each desired item:  if it doesn't ALREADY_EXIST, add it --- */
  for (cand=items; cand!=NIL; cand=cand->next_candidate) {
    if (id->id.isa_goal)
      bt_pref = make_fake_preference_for_goal_item (id, cand);
    else
      bt_pref = cand;
    if (cand->value->common.decider_flag==ALREADY_EXISTING_WME_DECIDER_FLAG) {
      if (id->id.isa_goal) remove_fake_preference_for_goal_item
        (cand->value->common.a.decider_wme->preference);
      cand->value->common.a.decider_wme->preference = bt_pref;
    } else {
      add_impasse_wme (id, current_agent(item_symbol), cand->value, bt_pref);
    }
  }
}

/* ------------------------------------------------------------------
                       Decide Non Context Slot
  
   This routine decides a given slot, which must be a non-context
   slot.  It calls run_preference_semantics() on the slot, then
   updates the wmes and/or impasse for the slot accordingly.
------------------------------------------------------------------ */

void decide_non_context_slot (slot *s) {
  byte impasse_type;
  wme *w, *next_w;
  preference *candidates, *cand, *pref;
  
  impasse_type = run_preference_semantics (s, &candidates);
  
  if (impasse_type==NONE_IMPASSE_TYPE) {
    /* --- no impasse, so remove any existing one and update the wmes --- */
    if (s->impasse_type!=NONE_IMPASSE_TYPE)
      remove_existing_attribute_impasse_for_slot (s);
    /* --- reset marks on existing wme values to "NOTHING" --- */
    for (w=s->wmes; w!=NIL; w=w->next)
      w->value->common.decider_flag = NOTHING_DECIDER_FLAG;
    /* --- set marks on desired values to "CANDIDATES" --- */
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate)
      cand->value->common.decider_flag = CANDIDATE_DECIDER_FLAG;
    /* --- for each existing wme, if we want it there, mark it as
       ALREADY_EXISTING; otherwise remove it --- */
    w = s->wmes;
    while (w) {
      next_w = w->next;
      if (w->value->common.decider_flag==CANDIDATE_DECIDER_FLAG) {
        w->value->common.decider_flag = ALREADY_EXISTING_WME_DECIDER_FLAG;
        w->value->common.a.decider_wme = w; /* so we can set the pref later */
      } else {
        remove_from_dll (s->wmes, w, next, prev);
	/* REW: begin 09.15.96 */
        if (current_agent(operand2_mode)){
          if (w->gds) {
    	    if (w->gds->goal != NIL){
	      /* If the goal pointer is non-NIL, then goal is in the stack */
	      if (current_agent(soar_verbose_flag)) {
		print("\n          Removing goal %d because element in GDS changed.", w->gds->goal->id.level); 
		print(" WME: "); 
		print_wme(w); }
	      gds_invalid_so_remove_goal(w);
	    }
	  }
	}
	/* REW: end   09.15.96 */
        remove_wme_from_wm (w);
      }
      w = next_w;
    }  /* end while (W) */

    /* --- for each desired value, if it's not already there, add it --- */
    for (cand=candidates; cand!=NIL; cand=cand->next_candidate) {
      if (cand->value->common.decider_flag==ALREADY_EXISTING_WME_DECIDER_FLAG){
	/* REW: begin 11.22.97 */ 
	/* print("\n This WME was marked as already existing...."); print_wme(cand->value->common.a.decider_wme); */
	
	/* REW: end   11.22.97 */ 
        cand->value->common.a.decider_wme->preference = cand;
      } else {
        w = make_wme (cand->id, cand->attr, cand->value, FALSE);
        insert_at_head_of_dll (s->wmes, w, next, prev);
        w->preference = cand;

	/* REW: begin 09.15.96 */
        if (current_agent(operand2_mode)){
	/* Whenever we add a WME to WM, we also want to check and see if
           this new WME is o-supported.  If so, then we want to add the
           supergoal dependencies of the new, o-supported element to the
           goal in which the element was created (as long as the o_supported
           element was not created in the top state -- the top goal has
           no gds).  */

	  /* REW: begin 11.25.96 */ 
      #ifndef NO_TIMING_STUFF
      #ifdef DETAILED_TIMING_STATS
	  start_timer(&current_agent(start_gds_tv));
      #endif 
      #endif
	  /* REW: end   11.25.96 */ 
            
	  current_agent(parent_list_head) = NIL;

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

		 create_gds_for_goal( w->preference->inst->match_goal );

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
		 strcpy(msg,"**** Wanted to create a GDS for a WME level different from the instantiation level.....Big problems....exiting....****\n\n");
		 abort_with_fatal_error(msg);
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
		  print("\n\n   "); print_preference(pref);
		  print("   Goal level of preference: %d\n",
			pref->id->id.level);
		#endif

		if (pref->inst->GDS_evaluated_already == FALSE) {
                  #ifdef DEBUG_GDS_HIGH
		   print_with_symbols("   Match goal lev of instantiation %y ",
				      pref->inst->prod->name);
		   print("is %d\n", pref->inst->match_goal_level);
                  #endif
		  if (pref->inst->match_goal_level > pref->id->id.level) {
                    #ifdef DEBUG_GDS_HIGH
		    print_with_symbols("        %y  is simply the instantiation that led to a chunk.\n        Not adding it the current instantiations.\n", pref->inst->prod->name);
                    #endif
		    
		   } else {
                    #ifdef DEBUG_GDS_HIGH
		     print_with_symbols("\n   Adding %y to list of parent instantiations\n", pref->inst->prod->name); 
                    #endif
		    uniquely_add_to_head_of_dll(pref->inst);
		    pref->inst->GDS_evaluated_already = TRUE;
		   }
		}  /* end if GDS_evaluated_already is FALSE */
                #ifdef DEBUG_GDS_HIGH
		  else
		    print_with_symbols("\n    Instantiation %y was already explored; skipping it\n", pref->inst->prod->name);
                #endif

	      }  /* end of forloop over preferences for this wme */

	      
              #ifdef DEBUG_GDS_HIGH
                print("\n    CALLING ELABORATE GDS....\n");
              #endif 
              elaborate_gds();

              /* technically, the list should be empty at this point ??? */
  
              free_parent_list(); 
              #ifdef DEBUG_GDS_HIGH
	        print("    FINISHED ELABORATING GDS.\n\n");
              #endif
	  }  /* end if w->preference->o_supported == TRUE ... */


	  /* REW: begin 11.25.96 */ 
      #ifndef NO_TIMING_STUFF
      #ifdef DETAILED_TIMING_STATS
	  stop_timer(&current_agent(start_gds_tv), 
		   &current_agent(gds_cpu_time[current_agent(current_phase)]));
      #endif
      #endif
	  /* REW: end   11.25.96 */ 

	}  /* end if current_agent(OPERAND2_MODE) ... */
	/* REW: end   09.15.96 */

        add_wme_to_wm (w);
      }
    }
    return;
  } /* end of if impasse type == NONE */

  /* --- impasse type != NONE --- */
  if (s->wmes) {  /* --- remove any existing wmes --- */
    remove_wme_list_from_wm (s->wmes); 
    s->wmes = NIL;
  }
  /* --- create and/or update impasse structure --- */
  if (s->impasse_type!=NONE_IMPASSE_TYPE) {
    if (s->impasse_type != impasse_type) {
      remove_existing_attribute_impasse_for_slot (s);
      create_new_attribute_impasse_for_slot (s, impasse_type);
    }
    update_impasse_items (s->impasse_id, candidates);
  } else {
    create_new_attribute_impasse_for_slot (s, impasse_type);
    update_impasse_items (s->impasse_id, candidates);
  }
}

/* ------------------------------------------------------------------
                      Decide Non Context Slots
  
   This routine iterates through all changed non-context slots, and
   decides each one.
------------------------------------------------------------------ */

void decide_non_context_slots (void) {
  dl_cons *dc;
  slot *s;

  while (current_agent(changed_slots)) {
    dc = current_agent(changed_slots);
    current_agent(changed_slots) = current_agent(changed_slots)->next;
    s = dc->item;
    decide_non_context_slot (s);
    s->changed = NIL;
    free_with_pool (&current_agent(dl_cons_pool), dc);
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

bool context_slot_is_decidable (slot *s) {
  Symbol *v;
  preference *p;
  
  if (! s->wmes) return (s->changed != NIL);
  v = s->wmes->value;
  for (p=s->preferences[RECONSIDER_PREFERENCE_TYPE]; p!=NIL; p=p->next)
    if (v==p->value) return TRUE;
  return FALSE;
}

/* ------------------------------------------------------------------
                      Remove WMEs For Context Slot
  
   This removes the wmes (there can only be 0 or 1 of them) for the
   given context slot.
------------------------------------------------------------------ */

void remove_wmes_for_context_slot (slot *s) {
  wme *w;
  
  if (!s->wmes) return;
  /* Note that we only need to handle one wme--context slots never have
     more than one wme in them */
  w = s->wmes;
  preference_remove_ref (w->preference);
  remove_wme_from_wm (w);
  s->wmes = NIL;
}

/* ------------------------------------------------------------------
                 Remove Existing Context And Descendents
  
   This routine truncates the goal stack by removing the given goal
   and all its subgoals.  (If the given goal is the top goal, the
   entire context stack is removed.)
------------------------------------------------------------------ */

void remove_existing_context_and_descendents (Symbol *goal) {
  preference *p;

  ms_change *head, *tail;  /* REW:   08.20.97 */

  /* --- remove descendents of this goal --- */
  if (goal->id.lower_goal)
    remove_existing_context_and_descendents (goal->id.lower_goal);

  /* --- invoke callback routine --- */
#ifndef NO_CALLBACKS
  soar_invoke_callbacks(soar_agent, 
                       POP_CONTEXT_STACK_CALLBACK, 
                       (soar_call_data) goal);
#endif

  /* --- disconnect this goal from the goal stack --- */
  if (goal == current_agent(top_goal)) {
    current_agent(top_goal) = NIL;
    current_agent(bottom_goal) = NIL;
  } else {
    current_agent(bottom_goal) = goal->id.higher_goal;
    current_agent(bottom_goal)->id.lower_goal = NIL;
  }

  /* --- remove any preferences supported by this goal --- */
  while (goal->id.preferences_from_goal) {
    p = goal->id.preferences_from_goal;
    remove_from_dll (goal->id.preferences_from_goal, p,
                     all_of_goal_next, all_of_goal_prev);
    p->on_goal_list = FALSE;
    if (! remove_preference_from_clones (p))
      if (p->in_tm) remove_preference_from_tm (p);
  }
  
  /* --- remove wmes for this goal, and garbage collect --- */
  remove_wmes_for_context_slot (goal->id.operator_slot);
  update_impasse_items (goal, NIL); /* causes items & fake pref's to go away */
  remove_wme_list_from_wm (goal->id.impasse_wmes);
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

    if (current_agent(nil_goal_retractions)) {
      /* There are already retractions on the list */
      
      /* Append this list to front of NIL goal list */
      current_agent(nil_goal_retractions)->prev_in_level = tail;
      tail->next_in_level = current_agent(nil_goal_retractions);
      current_agent(nil_goal_retractions) = head;
      
    } else { /* If no retractions, make this list the NIL goal list */
      current_agent(nil_goal_retractions) = head;
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

  post_link_removal (NIL, goal);  /* remove the special link */
  symbol_remove_ref (goal);
}

/* ------------------------------------------------------------------
                         Create New Context
  
   This routine creates a new goal context (becoming the new bottom
   goal) below the current bottom goal.  If there is no current
   bottom goal, this routine creates a new goal and makes it both
   the top and bottom goal.
------------------------------------------------------------------ */

void create_new_context (Symbol *attr_of_impasse, byte impasse_type) {
  Symbol *id;
  
  if (current_agent(bottom_goal)) {
    id = create_new_impasse (TRUE, current_agent(bottom_goal),	                 
	     	attr_of_impasse, impasse_type,
            (goal_stack_level) (current_agent(bottom_goal)->id.level + 1));
    id->id.higher_goal = current_agent(bottom_goal);
    current_agent(bottom_goal)->id.lower_goal = id;
    current_agent(bottom_goal) = id;
    add_impasse_wme (id, current_agent(quiescence_symbol),
		     current_agent(t_symbol), NIL);
  } else {
    id = create_new_impasse (TRUE, current_agent(nil_symbol),
			     NIL, NONE_IMPASSE_TYPE,
                             TOP_GOAL_LEVEL);
    current_agent(top_goal) = id;
    current_agent(bottom_goal) = id;
    current_agent(top_state) = current_agent(top_goal);
    id->id.higher_goal = NIL;
    id->id.lower_goal = NIL;
  }
  id->id.isa_goal = TRUE;
  id->id.operator_slot = make_slot (id, current_agent(operator_symbol));
  id->id.allow_bottom_up_chunks = TRUE;

  /* --- invoke callback routine --- */
#ifndef NO_CALLBACKS
  soar_invoke_callbacks(soar_agent, 
                       CREATE_NEW_CONTEXT_CALLBACK, 
                       (soar_call_data) id);
#endif
}

/* ------------------------------------------------------------------
              Type and Attribute of Existing Impasse
  
   Given a goal, these routines return the type and attribute,
   respectively, of the impasse just below that goal context.  It
   does so by looking at the impasse wmes for the next lower goal
   in the goal stack.
------------------------------------------------------------------ */

byte type_of_existing_impasse (Symbol *goal) {
  wme *w;
  char msg[128];

  if (! goal->id.lower_goal) return NONE_IMPASSE_TYPE;
  for (w=goal->id.lower_goal->id.impasse_wmes; w!=NIL; w=w->next)
    if (w->attr==current_agent(impasse_symbol)) {
      if (w->value==current_agent(no_change_symbol))
	return NO_CHANGE_IMPASSE_TYPE;
      if (w->value==current_agent(tie_symbol))
	return TIE_IMPASSE_TYPE;
      if (w->value==current_agent(constraint_failure_symbol))
        return CONSTRAINT_FAILURE_IMPASSE_TYPE;
      if (w->value==current_agent(conflict_symbol))
	return CONFLICT_IMPASSE_TYPE;
      if (w->value==current_agent(none_symbol))
	return NONE_IMPASSE_TYPE;
      strcpy (msg,"decide.c: Internal error: bad type of existing impasse.\n");
      abort_with_fatal_error(msg);
    }
  strcpy (msg,"decide.c: Internal error: couldn't find type of existing impasse.\n");
  abort_with_fatal_error(msg);
  return 0; /* unreachable, but without it, gcc -Wall warns here */
}

Symbol *attribute_of_existing_impasse (Symbol *goal) {
  wme *w;

  if (! goal->id.lower_goal) return NIL;
  for (w=goal->id.lower_goal->id.impasse_wmes; w!=NIL; w=w->next)
    if (w->attr==current_agent(attribute_symbol)) return w->value;
  { char msg[128];
  strcpy (msg, "decide.c: Internal error: couldn't find attribute of existing impasse.\n");
  abort_with_fatal_error(msg);
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

bool decide_context_slot (Symbol *goal, slot *s) {
  byte impasse_type;
  Symbol *attribute_of_impasse;
  wme *w;
  preference *candidates;
  preference *temp;
  
  if (! context_slot_is_decidable(s)) {
    /* --- the only time we decide a slot that's not "decidable" is when it's
       the last slot in the entire context stack, in which case we have a
       no-change impasse there --- */
    impasse_type = NO_CHANGE_IMPASSE_TYPE;
    candidates = NIL; /* we don't want any impasse ^item's later */
  } else {
    /* --- the slot is decidable, so run preference semantics on it --- */
    impasse_type = run_preference_semantics (s, &candidates);
    remove_wmes_for_context_slot (s); /* must remove old wme before adding
                                         the new one (if any) */
    if (impasse_type==NONE_IMPASSE_TYPE) {
      if (! candidates) {
        /* --- no winner ==> no-change impasse on the previous slot --- */
        impasse_type = NO_CHANGE_IMPASSE_TYPE;
      } else if (candidates->next_candidate) {
        /* --- more than one winner ==> internal error --- */
	char msg[128];
        strcpy (msg,"decide.c: Internal error: more than one winner for context slot\n");
        abort_with_fatal_error(msg);
      }
    }
  }  /* end if !context_slot_is_decidable  */

  /* --- mark the slot as not changed --- */
  s->changed = NIL;

  /* --- determine the attribute of the impasse (if there is no impasse,
   * this doesn't matter) --- */
  if (impasse_type==NO_CHANGE_IMPASSE_TYPE) {
    if (s->wmes) {
      attribute_of_impasse = s->attr;
    } else {
      attribute_of_impasse = current_agent(state_symbol);
    }
  } else {
    /* --- for all other kinds of impasses --- */
    attribute_of_impasse = s->attr;
  }
  
  /* --- remove wme's for lower slots of this context --- */
  if (attribute_of_impasse==current_agent(state_symbol)) {
    remove_wmes_for_context_slot (goal->id.operator_slot);
  }


  /* --- if we have a winner, remove any existing impasse and install the
     new value for the current slot --- */
  if (impasse_type==NONE_IMPASSE_TYPE) {
    for(temp = candidates; temp; temp = temp->next_candidate)
      preference_add_ref(temp);
    if (goal->id.lower_goal)
      remove_existing_context_and_descendents (goal->id.lower_goal);
    w = make_wme (s->id, s->attr, candidates->value, FALSE);
    insert_at_head_of_dll (s->wmes, w, next, prev);
    w->preference = candidates;
    preference_add_ref (w->preference);
    add_wme_to_wm (w);
    for(temp = candidates; temp; temp = temp->next_candidate)
      preference_remove_ref(temp);
    return TRUE;
  } 
    
  /* --- no winner; if an impasse of the right type already existed, just
     update the ^item set on it --- */
  if ((impasse_type == type_of_existing_impasse(goal)) &&
      (attribute_of_impasse == attribute_of_existing_impasse(goal))) {
    update_impasse_items (goal->id.lower_goal, candidates);
    return FALSE;
  }

  /* --- no impasse already existed, or an impasse of the wrong type
     already existed --- */
  for(temp = candidates; temp; temp = temp->next_candidate)
    preference_add_ref(temp);
  if (goal->id.lower_goal)
    remove_existing_context_and_descendents (goal->id.lower_goal);

  /* REW: begin 10.24.97 */
  if (current_agent(operand2_mode) && current_agent(waitsnc) &&
      (impasse_type == NO_CHANGE_IMPASSE_TYPE) &&
      (attribute_of_impasse == current_agent(state_symbol))) {
    current_agent(waitsnc_detect)                     = TRUE; 
  } else {
  /* REW: end     10.24.97 */
    create_new_context (attribute_of_impasse, impasse_type);
    update_impasse_items (goal->id.lower_goal, candidates);
  }

  for(temp = candidates; temp; temp = temp->next_candidate)
    preference_remove_ref(temp);
  return TRUE;
}

/* ------------------------------------------------------------------
                       Decide Context Slots

   This scans down the goal stack and runs the decision procedure on
   the appropriate context slots.
------------------------------------------------------------------ */

void decide_context_slots (void) {
  Symbol *goal;
  slot *s;


  if (current_agent(highest_goal_whose_context_changed)) {
    goal = current_agent(highest_goal_whose_context_changed);
  }
  else
    /* no context changed, so jump right to the bottom */
    goal = current_agent(bottom_goal);

  s = goal->id.operator_slot;

  /* --- loop down context stack --- */
  while (TRUE) {
    /* --- find next slot to decide --- */
    while (TRUE) {
      if (context_slot_is_decidable(s)) break;

      if ((s==goal->id.operator_slot) || (! s->wmes)) {
        /* --- no more slots to look at for this goal; have we reached
           the last slot in whole stack? --- */
        if (! goal->id.lower_goal) break;
        /* --- no, go down one level --- */
        goal = goal->id.lower_goal;
	s = goal->id.operator_slot;
      }
    } /* end of while (TRUE) find next slot to decide */

    /* --- now go and decide that slot --- */
    if (decide_context_slot (goal, s)) break;

  } /* end of while (TRUE) loop down context stack */
  current_agent(highest_goal_whose_context_changed) = NIL;
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

void init_decider (void) {
  init_memory_pool (&current_agent(slot_pool), sizeof(slot), "slot");
  init_memory_pool (&current_agent(wme_pool), sizeof(wme), "wme");
  init_memory_pool (&current_agent(preference_pool),
		    sizeof(preference), "preference");
}

void do_buffered_wm_and_ownership_changes (void) {
  do_buffered_acceptable_preference_wme_changes();
  do_buffered_link_changes();
  do_buffered_wm_changes();
  remove_garbage_slots();
}

void do_working_memory_phase (void) {

  if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM]) {
     if (current_agent(operand2_mode) == TRUE) {
	switch (current_agent(FIRING_TYPE)) {
	   case PE_PRODS:
              print ("\t--- Change Working Memory (PE) ---\n");
	      break;
	   case IE_PRODS:
              print ("\t--- Change Working Memory (IE) ---\n");
	      break;
	}
     }
     else
        print ("\n--- Working Memory Phase ---\n");
  }

  decide_non_context_slots();
  do_buffered_wm_and_ownership_changes();
}

void do_decision_phase (void) {
  if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM])
    print ("\n--- Decision Phase ---\n");
  decide_context_slots ();
  do_buffered_wm_and_ownership_changes();
  /*
   * Bob provided a solution to fix WME's hanging around unsupported
   * for an elaboration cycle.
   */
  decide_non_context_slots();
  do_buffered_wm_and_ownership_changes();
}  

void create_top_goal (void) {
  create_new_context (NIL, NONE_IMPASSE_TYPE);
  current_agent(highest_goal_whose_context_changed) = NIL;  /* nothing changed yet */
  do_buffered_wm_and_ownership_changes();
}

void clear_goal_stack (void) {
  if (!current_agent(top_goal)) return;
  remove_existing_context_and_descendents (current_agent(top_goal));
  current_agent(highest_goal_whose_context_changed) = NIL;  /* nothing changed                                                                yet */
  do_buffered_wm_and_ownership_changes();
  current_agent(top_state) = NIL;
  do_input_cycle();  /* tell input functions that the top state is gone */
  do_output_cycle(); /* tell output functions that output commands are gone */
}
  
void print_lowest_slot_in_context_stack (void) {

  /* REW: begin 10.24.97 */
  /* This doesn't work yet so for now just print the last selection */
  /*  if (current_agent(operand2_mode) && 
   *   current_agent(waitsnc) &&
   *   current_agent(waitsnc_detect)) {
   * current_agent(waitsnc_detect) = FALSE;
   * print_stack_trace (current_agent(wait_symbol),
   *                    current_agent(bottom_goal), FOR_OPERATORS_TF, TRUE);
   * print("\n waiting"); 
   * return;
   *  }
   */
  /* REW: end   10.24.97 */

  if (current_agent(bottom_goal)->id.operator_slot->wmes)
    print_stack_trace (current_agent(bottom_goal)->id.operator_slot->wmes->value,
                       current_agent(bottom_goal), FOR_OPERATORS_TF, TRUE);


  /* RCHONG: begin 10.11 */
  /*
  this coded is needed just so that when an ONC is created in OPERAND
  (i.e. if the previous goal's operator slot is not empty), it's stack
  trace line doesn't get a number.  this is done because in OPERAND,
  ONCs are detected for "free".
  */

  else {

    /* REW: begin 09.15.96 */
    if (current_agent(operand2_mode) == FALSE) 
       print_stack_trace (current_agent(bottom_goal),
			  current_agent(bottom_goal), FOR_STATES_TF,TRUE);
    /* REW: end   09.15.96 */

    else {
       if (current_agent(d_cycle_count) == 0)
          print_stack_trace (current_agent(bottom_goal),
			     current_agent(bottom_goal), FOR_STATES_TF,TRUE);
       else {
          if (current_agent(bottom_goal)->id.higher_goal->id.operator_slot->wmes) {
             print_stack_trace (current_agent(bottom_goal),
				current_agent(bottom_goal),
				FOR_STATES_TF,FALSE);
	  }
          else {
             print_stack_trace (current_agent(bottom_goal),
				current_agent(bottom_goal),
				FOR_STATES_TF,TRUE);
	  }
       }
    }
  }

  /* RCHONG: end 10.11 */

}




/* REW: begin 09.15.96 */

void uniquely_add_to_head_of_dll(instantiation *inst)
{

  parent_inst *new_pi, *curr_pi;
   
  /* print("UNIQUE DLL:         scanning parent list...\n"); */

  for (curr_pi = current_agent(parent_list_head);
       curr_pi;
       curr_pi = curr_pi->next) {
     if (curr_pi->inst == inst) {
        #ifdef DEBUG_GDS
        print_with_symbols("UNIQUE DLL:            %y is already in parent list\n",curr_pi->inst->prod->name);
        #endif
        return;
     }
     #ifdef DEBUG_GDS
         print_with_symbols("UNIQUE DLL:            %y\n",curr_pi->inst->prod->name); 
     #endif
  } /* end for loop */

  new_pi = (parent_inst *) malloc(sizeof(parent_inst));
  new_pi->next = NIL;
  new_pi->prev = NIL;
  new_pi->inst = inst;

  new_pi->next = current_agent(parent_list_head);

  if (current_agent(parent_list_head) != NIL)
     current_agent(parent_list_head)->prev = new_pi;

  current_agent(parent_list_head) = new_pi;
  #ifdef DEBUG_GDS
   print_with_symbols("UNIQUE DLL:         added: %y\n",inst->prod->name); 
  #endif
}

void elaborate_gds () {

wme *wme_matching_this_cond;
goal_stack_level  wme_goal_level;
preference *pref_for_this_wme, *pref;
condition *cond;
parent_inst *curr_pi, *temp_pi;
slot *s;
instantiation *inst;

  for (curr_pi=current_agent(parent_list_head); curr_pi; curr_pi=temp_pi) {

     inst = curr_pi->inst;

     #ifdef DEBUG_GDS
          print_with_symbols("\n      EXPLORING INSTANTIATION: %y\n",curr_pi->inst->prod->name);
          print("      ");
	  print_instantiation_with_wmes( curr_pi->inst , TIMETAG_WME_TRACE);
     #endif

     for (cond=inst->top_of_instantiated_conditions;
	  cond!=NIL;
	  cond=cond->next) {

	if (cond->type != POSITIVE_CONDITION) continue;
	/* We'll deal with negative instantiations after we get the
	 * positive ones figured out */

	wme_matching_this_cond = cond->bt.wme;
	wme_goal_level         = cond->bt.level;
	pref_for_this_wme      = wme_matching_this_cond->preference;

        #ifdef DEBUG_GDS
	 print("\n       wme_matching_this_cond at goal_level = %d : ",
	       wme_goal_level);
	 print_wme(wme_matching_this_cond); 

	 if (pref_for_this_wme) {
	   print("       pref_for_this_wme                        : ");
	   print_preference(pref_for_this_wme);
	 } 
        #endif


        /* WME is in a supergoal or is arch-supported WME
	 *  (except for fake instantiations, which do have prefs, so
	 *  they get handled under "wme is local and i-supported")
	 */
	if ((pref_for_this_wme == NIL) ||
	    (wme_goal_level < inst->match_goal_level)) {

          #ifdef DEBUG_GDS
	    if (pref_for_this_wme == NIL) {
	       print("         this wme has no preferences (it's an arch-created wme)\n");
	    }
	    else if (wme_goal_level < inst->match_goal_level) {
	       print("         this wme is in the supergoal\n");
	    }
	    print_with_symbols("inst->match_goal [%y]\n" , inst->match_goal);  
          #endif

	  if (wme_matching_this_cond->gds != NIL){
	    /* Then we want to check and see if the old GDS value
	     * should be changed */
	    if (wme_matching_this_cond->gds->goal == NIL) {
	      /* The goal is NIL: meaning that the goal for the GDS
	       * is no longer around */
	       fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds, \
                                    wme_matching_this_cond, wme,
				    gds_next, gds_prev);

	       /* We have to check for GDS removal anytime we take a
		* WME off the GDS wme list, not just when a WME is
		* removed from memory. */
               if (!wme_matching_this_cond->gds->wmes_in_gds) {
		 free_memory(wme_matching_this_cond->gds,
			     MISCELLANEOUS_MEM_USAGE);
                 #ifdef DEBUG_GDS
                   print("\n  REMOVING GDS FROM MEMORY.");
                 #endif
	       }
               wme_matching_this_cond->gds = inst->match_goal->id.gds;
               insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds, 
                                     wme_matching_this_cond, gds_next,
				     gds_prev);
               #ifdef DEBUG_GDS
	         print("\n       .....GDS' goal is NIL so switching from old to new GDS list....\n"); 
               #endif
	    
	    } else if (wme_matching_this_cond->gds->goal->id.level >
		       inst->match_goal_level) {
	      /* if the WME currently belongs to the GDS of a goal below
	       * the current one */
	       /* 1. Take WME off old (current) GDS list 
                * 2. Check to see if old GDS WME list is empty.  If so,
		*         remove(free) it.
		* 3. Add WME to new GDS list
		* 4. Update WME pointer to new GDS list
		*/
	       if (inst->match_goal_level == 1) 
		 print("\n\n\n HELLO! HELLO! The inst->match_goal_level is 1");

	       fast_remove_from_dll(wme_matching_this_cond->gds->wmes_in_gds, \
                                    wme_matching_this_cond, wme,
				    gds_next, gds_prev);
               if (!wme_matching_this_cond->gds->wmes_in_gds) {
		 free_memory(wme_matching_this_cond->gds,
			     MISCELLANEOUS_MEM_USAGE);
                 #ifdef DEBUG_GDS
                   print("\n  REMOVING GDS FROM MEMORY.");
                 #endif
	       } 
               wme_matching_this_cond->gds = inst->match_goal->id.gds;
               insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
				     wme_matching_this_cond, gds_next,
				     gds_prev);
               #ifdef DEBUG_GDS
	        print("\n       ....switching from old to new GDS list....\n");
               #endif
	       wme_matching_this_cond->gds = inst->match_goal->id.gds;
	    }
	  } else {
	    /* We know that the WME should be in the GDS of the current
	     * goal if the WME's GDS does not already exist.
	     * (i.e., if NIL GDS) */
	    wme_matching_this_cond->gds = inst->match_goal->id.gds;

	    insert_at_head_of_dll(wme_matching_this_cond->gds->wmes_in_gds,
				  wme_matching_this_cond, gds_next, gds_prev);
	    if (wme_matching_this_cond->gds->wmes_in_gds->gds_prev)
	      print("\nDEBUG DEBUG : The new header should never have a prev value.\n");
            #ifdef DEBUG_GDS
              print_with_symbols("\n       ......WME did not have defined GDS.  Now adding to goal [%y].\n", wme_matching_this_cond->gds->goal); 
            #endif
	  } /* end else clause for "if wme_matching_this_cond->gds != NIL" */


          #ifdef DEBUG_GDS
	    print("            Added WME to GDS for goal = %d",
		  wme_matching_this_cond->gds->goal->id.level);
	    print_with_symbols(" [%y]\n", wme_matching_this_cond->gds->goal);  
          #endif
        } /* end "wme in supergoal or arch-supported" */

	else {
	  /* wme must be local */

	  /* if wme's pref is o-supported, then just ignore it and
	   * move to next condition */
	  if (pref_for_this_wme->o_supported == TRUE) {
             #ifdef DEBUG_GDS
	      print("         this wme is local and o-supported\n");
             #endif
             continue;
	  }

	  else {
	    /* wme's pref is i-supported, so remember it's instantiation
	     * for later examination */

	      /* this test avoids "backtracing" through the top state */
	      if (inst->match_goal_level == 1) {
                #ifdef DEBUG_GDS
		  print("         don't back up through top state\n");  
		  if (inst->prod)
		    if (inst->prod->name)
		       print_with_symbols("         don't back up through top state for instantiation %y\n", inst->prod->name);
                #endif
		continue;
	      }

	      else { /* (inst->match_goal_level != 1) */
                #ifdef DEBUG_GDS
		 print("         this wme is local and i-supported\n"); 
                #endif
                s = find_slot (pref_for_this_wme->id, pref_for_this_wme->attr);
		if (s == NIL) {
		   /* this must be an arch-wme from a fake instantiation */
		   
                   #ifdef DEBUG_GDS
		     print("here's the wme with no slot:\t");
		     print_wme(pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme);
                   #endif

		   /* this is the same code as above, just using the 
		    * differently-named pointer.  it probably should
		    * be a subroutine */
		   {
		     wme *fake_inst_wme_cond;

		     fake_inst_wme_cond =
		       pref_for_this_wme->inst->top_of_instantiated_conditions->bt.wme;
		     if (fake_inst_wme_cond->gds != NIL){
		       /* Then we want to check and see if the old GDS
			* value should be changed */
		       if (fake_inst_wme_cond->gds->goal == NIL) {
			 /* The goal is NIL: meaning that the goal for
			  * the GDS is no longer around */

			 fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds,
					      fake_inst_wme_cond, wme,
					      gds_next, gds_prev);

			 /* We have to check for GDS removal anytime we take
			  * a WME off the GDS wme list, not just when a WME
			  *is removed from memory. */
			 if (!fake_inst_wme_cond->gds->wmes_in_gds) {
			   free_memory(fake_inst_wme_cond->gds,
				       MISCELLANEOUS_MEM_USAGE);
                           #ifdef DEBUG_GDS
			     print("\n  REMOVING GDS FROM MEMORY.");
                           #endif
			 }
	       
			 fake_inst_wme_cond->gds = inst->match_goal->id.gds;
			 insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds, 
					       fake_inst_wme_cond, gds_next, gds_prev);
                         #ifdef DEBUG_GDS
			 print("\n       .....GDS' goal is NIL so switching from old to new GDS list....\n"); 
                         #endif
		       } else if (fake_inst_wme_cond->gds->goal->id.level >
				  inst->match_goal_level) {
			 /* if the WME currently belongs to the GDS of a
			  *goal below the current one */
			 /* 1. Take WME off old (current) GDS list 
			  * 2. Check to see if old GDS WME list is empty.
			  *    If so, remove(free) it.
			  * 3. Add WME to new GDS list
			  * 4. Update WME pointer to new GDS list
			  */
			 if (inst->match_goal_level == 1) 
			   print("\n\n\n\n\n HELLO! HELLO! The inst->match_goal_level is 1");

			 fast_remove_from_dll(fake_inst_wme_cond->gds->wmes_in_gds, \
					      fake_inst_wme_cond, wme,
					      gds_next, gds_prev);
			 if (!fake_inst_wme_cond->gds->wmes_in_gds) {
			   free_memory(fake_inst_wme_cond->gds,
				       MISCELLANEOUS_MEM_USAGE);
                           #ifdef DEBUG_GDS
 			    print("\n  REMOVING GDS FROM MEMORY.");
                           #endif
			 }
	       
			 fake_inst_wme_cond->gds = inst->match_goal->id.gds;
			 insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds, \
					       fake_inst_wme_cond, gds_next,
					       gds_prev);
                         #ifdef DEBUG_GDS
			   print("\n       .....switching from old to new GDS list....\n");
                         #endif
			   fake_inst_wme_cond->gds = inst->match_goal->id.gds;
		       }
		     } else {
		       /* We know that the WME should be in the GDS of
			* the current goal if the WME's GDS does not
			* already exist. (i.e., if NIL GDS) */
		       fake_inst_wme_cond->gds = inst->match_goal->id.gds;

		       insert_at_head_of_dll(fake_inst_wme_cond->gds->wmes_in_gds,
					     fake_inst_wme_cond,
					     gds_next, gds_prev);
		       if (fake_inst_wme_cond->gds->wmes_in_gds->gds_prev)
			 print("\nDEBUG DEBUG : The new header should never have a prev value.\n");
                       #ifdef DEBUG_GDS
 		         print_with_symbols("\n       ......WME did not have defined GDS.  Now adding to goal [%y].\n", fake_inst_wme_cond->gds->goal); 
                       #endif
		     }
                     #ifdef DEBUG_GDS
		       print("            Added WME to GDS for goal = %d", fake_inst_wme_cond->gds->goal->id.level);
		       print_with_symbols(" [%y]\n",
					  fake_inst_wme_cond->gds->goal);  
                     #endif
		   }  /* matches { wme *fake_inst_wme_cond  */
		} else {
		  /* this was the original "local & i-supported" action */
	     
		  for (pref=s->preferences[ACCEPTABLE_PREFERENCE_TYPE];
		       pref;
		       pref=pref->next) {

                    #ifdef DEBUG_GDS
		       print("           looking at pref for the wme: ");
		       print_preference(pref); 
                    #endif

                    /* REW BUG: may have to go over all insts regardless
		     * of this visited_already flag... */

		    if (pref->inst->GDS_evaluated_already == FALSE) {
	
                       #ifdef DEBUG_GDS	      
		         print_with_symbols("\n           adding inst that produced the pref to GDS: %y\n",pref->inst->prod->name); 
                       #endif

		       uniquely_add_to_head_of_dll(pref->inst);
		       pref->inst->GDS_evaluated_already = TRUE;
                    }

                    else {
                      #ifdef DEBUG_GDS
		        print("           the inst producing this pref was already explored; skipping it\n"); 
                      #endif
		    }

		  }  /* for pref = s->pref[ACCEPTABLE_PREF ...*/
		}
	      }
	  }
        }
	
     }  /* for (cond = inst->top_of_instantiated_cond ...  *;'/


    /* remove just used instantiation from list */

    #ifdef DEBUG_GDS
     print_with_symbols("\n      removing instantiation: %y\n",
			curr_pi->inst->prod->name); 
    #endif
  
    if (curr_pi->next != NIL) curr_pi->next->prev = curr_pi->prev;
    if (curr_pi->prev != NIL) curr_pi->prev->next = curr_pi->next;

    if (current_agent(parent_list_head) == curr_pi)
      current_agent(parent_list_head) = curr_pi->next;
 
    temp_pi = curr_pi->next;
    free(curr_pi);

  } /* end of "for (curr_pi = current_agent(parent_list_head) ... */


  if (current_agent(parent_list_head) != NIL) {
    #ifdef DEBUG_GDS
     print("\n    RECURSING using these parents:\n");
     for (curr_pi = current_agent(parent_list_head);
	  curr_pi;
	  curr_pi = curr_pi->next) {
       print_with_symbols("      %y\n",curr_pi->inst->prod->name);
     } 
    #endif

     /* recursively explore the parents of all the instantiations */

     elaborate_gds();

     /* free the parent instantiation list.  technically, the list
      * should be empty at this point ??? */
     free_parent_list(); 
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

void gds_invalid_so_remove_goal (wme *w) {

  /* REW: begin 11.25.96 */ 
  #ifndef NO_TIMING_STUFF
  #ifdef DETAILED_TIMING_STATS
  start_timer(&current_agent(start_gds_tv));
  #endif
  #endif
  /* REW: end   11.25.96 */ 

  if (current_agent(soar_verbose_flag)) GDS_PrintCmd();

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

   if (current_agent(highest_goal_whose_context_changed)) {
      if (current_agent(highest_goal_whose_context_changed)->id.level >=
          w->gds->goal->id.level) {
        current_agent(highest_goal_whose_context_changed) =
	  w->gds->goal->id.higher_goal;
      }
   } else {
     /* If nothing has yet changed (highest_ ... = NIL) then set
      * the goal automatically */
     current_agent(highest_goal_whose_context_changed) =
       w->gds->goal->id.higher_goal; 
   }

   if (current_agent(sysparams)[TRACE_OPERAND2_REMOVALS_SYSPARAM]) {
     print_with_symbols("\n    REMOVING GOAL [%y] due to change in GDS WME ",
			w->gds->goal);
     print_wme(w);
   }
   remove_existing_context_and_descendents(w->gds->goal);
   /* BUG: Need to reset highest_goal here ???*/

   /* usually, we'd call do_buffered_wm_and_ownership_changes() here, but
    * we don't need to because it will be done at the end of the working
    * memory phase; cf. the end of do_working_memory_phase().
    */

  /* REW: begin 11.25.96 */ 
  #ifndef NO_TIMING_STUFF
  #ifdef DETAILED_TIMING_STATS
  stop_timer(&current_agent(start_gds_tv), 
             &current_agent(gds_cpu_time[current_agent(current_phase)]));
  #endif
  #endif
  /* REW: end   11.25.96 */ 
}


void free_parent_list()
{
  parent_inst *curr_pi;

  for (curr_pi = current_agent(parent_list_head);
       curr_pi;
       curr_pi = curr_pi->next)
     free(curr_pi);

  current_agent(parent_list_head) = NIL;
}

void create_gds_for_goal( Symbol *goal){
   goal_dependency_set *gds;

   gds = allocate_memory(sizeof(goal_dependency_set), MISCELLANEOUS_MEM_USAGE);
   gds->goal = goal;
   gds->wmes_in_gds = NIL;
   goal->id.gds = gds;
   #ifdef DEBUG_GDS
     print_with_symbols("\nCreated GDS for goal [%y].\n", gds->goal);
   #endif
}


