/*************************************************************************
 *
 *  file:  prefmem.c
 *
 * =======================================================================
 *  BUGBUG  need some comments here
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

/* ======================================================================
        Preference Memory routines for Soar 6
   ====================================================================== */

/* Debugging stuff:  #define DEBUG_PREFS to get preference printouts */

/* #define DEBUG_PREFS */

#include "soarkernel.h"


/* ======================================================================

                     Preference Management Routines

====================================================================== */

/* ----------------------------------------------------------------------
   Make_preference() creates a new preference structure of the given type
   with the given id/attribute/value/referent.  (Referent is only used
   for binary preferences.)  The preference is not yet added to preference
   memory, however.
---------------------------------------------------------------------- */

preference *make_preference (byte type, Symbol *id, Symbol *attr,
                             Symbol *value, Symbol *referent) {
  preference *p;

  allocate_with_pool (&current_agent(preference_pool), &p);
  p->type = type;
  p->in_tm = FALSE;
  p->o_supported = FALSE;
  p->on_goal_list = FALSE;
  p->reference_count = 0;
  p->id = id;
  p->attr = attr;
  p->value = value;
  p->referent = referent;
  p->slot = NIL;
  p->next_clone = NIL;
  p->prev_clone = NIL;

#ifdef DEBUG_PREFS
  print ("\nAllocating preference at 0x%8x: ", (unsigned long)p);
  print_preference (p);
#endif

  return p;

  /* BUGBUG check to make sure the pref doesn't have
        value or referent .isa_goal or .isa_impasse; */
}

/* ----------------------------------------------------------------------
   Deallocate_preference() deallocates a given preference.
---------------------------------------------------------------------- */

void deallocate_preference (preference *pref) {

#ifdef DEBUG_PREFS  
  print ("\nDeallocating preference at 0x%8x: ",(unsigned long)pref);
  print_preference (pref);
  if (pref->reference_count != 0) {   /* --- sanity check --- */
    char msg[128];
    strcpy (msg, "prefmem.c: Internal Error: Deallocating preference with ref. count != 0\n");
    abort_with_fatal_error(msg);
  }
#endif

  /* --- remove it from the list of pref's for its match goal --- */
  if (pref->on_goal_list)
    remove_from_dll (pref->inst->match_goal->id.preferences_from_goal,
                     pref, all_of_goal_next, all_of_goal_prev);
  
  /* --- remove it from the list of pref's from that instantiation --- */
  remove_from_dll (pref->inst->preferences_generated, pref,
                   inst_next, inst_prev);
  possibly_deallocate_instantiation (pref->inst);

  /* --- dereference component symbols --- */
  symbol_remove_ref (pref->id);
  symbol_remove_ref (pref->attr);
  symbol_remove_ref (pref->value);
  if (preference_is_binary(pref->type))
    symbol_remove_ref (pref->referent);
  
  /* --- free the memory --- */
  free_with_pool (&current_agent(preference_pool), pref);
}  

/* ----------------------------------------------------------------------
   Possibly_deallocate_preference_and_clones() checks whether a given
   preference and all its clones have reference_count 0, and deallocates
   them all if they do.  It returns TRUE if they were actually
   deallocated, FALSE otherwise.
---------------------------------------------------------------------- */

bool possibly_deallocate_preference_and_clones (preference *pref) {
  preference *clone, *next;
  
  if (pref->reference_count) return FALSE;
  for (clone=pref->next_clone; clone!=NIL; clone=clone->next_clone)
    if (clone->reference_count) return FALSE;
  for (clone=pref->prev_clone; clone!=NIL; clone=clone->prev_clone)
    if (clone->reference_count) return FALSE;

  /* --- deallocate all the clones --- */
  clone = pref->next_clone;
  while (clone) {
    next = clone->next_clone;
    deallocate_preference (clone);
    clone = next;
  }
  clone = pref->prev_clone;
  while (clone) {
    next = clone->prev_clone;
    deallocate_preference (clone);
    clone = next;
  }

  /* --- deallocate pref --- */
  deallocate_preference (pref);

  return TRUE;
}

/* ----------------------------------------------------------------------
   Remove_preference_from_clones() splices a given preference out of the
   list of clones.  If the preference's reference_count is 0, it also
   deallocates it and returns TRUE.  Otherwise it returns FALSE.
---------------------------------------------------------------------- */

bool remove_preference_from_clones (preference *pref) {
  preference *any_clone;
  
  any_clone = NIL;
  if (pref->next_clone) {
    any_clone = pref->next_clone;
    pref->next_clone->prev_clone = pref->prev_clone;
  }
  if (pref->prev_clone) {
    any_clone = pref->prev_clone;
    pref->prev_clone->next_clone = pref->next_clone;
  }
  pref->next_clone = pref->prev_clone = NIL;
  if (any_clone) possibly_deallocate_preference_and_clones (any_clone);
  if (! pref->reference_count) {
    deallocate_preference (pref);
    return TRUE;
  } else {
    return FALSE;
  }
}

/* ------------------------------------------------------------------------
   Add_preference_to_tm() adds a given preference to preference memory (and
   hence temporary memory).
------------------------------------------------------------------------ */

void add_preference_to_tm (preference *pref) {
  slot *s;
  preference *p2;

#ifdef DEBUG_PREFS
  print ("\nAdd preference at 0x%8x:  ",(unsigned long)pref);
  print_preference (pref);
#endif

  s = make_slot (pref->id, pref->attr);
  pref->slot = s;

  insert_at_head_of_dll (s->all_preferences, pref,
                         all_of_slot_next, all_of_slot_prev);

  /* --- add preference to the list (in the right place, according to match
     goal level of the instantiations) for the slot --- */

  if (! s->preferences[pref->type]) {
    /* --- this is the only pref. of its type, just put it at the head --- */
    insert_at_head_of_dll (s->preferences[pref->type], pref, next, prev);
  } else if (s->preferences[pref->type]->inst->match_goal_level >=
             pref->inst->match_goal_level) {
    /* --- it belongs at the head of the list, so put it there --- */
    insert_at_head_of_dll (s->preferences[pref->type], pref, next, prev);
  } else {
    /* --- scan through the pref. list, find the one to insert after --- */
    for (p2=s->preferences[pref->type]; p2->next!=NIL; p2=p2->next)
      if (p2->next->inst->match_goal_level >= pref->inst->match_goal_level)
        break;
    /* --- insert pref after p2 --- */
    pref->next = p2->next;
    pref->prev = p2;
    p2->next = pref;
    if (pref->next) pref->next->prev = pref;
  }

  /* --- other miscellaneous stuff --- */    
  pref->in_tm = TRUE;
  preference_add_ref (pref);
  mark_slot_as_changed (s);

  /* --- update identifier levels --- */
  if (pref->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
    post_link_addition (pref->id, pref->value);
  if (preference_is_binary(pref->type))
    if (pref->referent->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
      post_link_addition (pref->id, pref->referent);
  
  /* --- if acceptable/require pref for context slot, we may need to add a
     wme later --- */
  if ((s->isa_context_slot) &&
      ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
       (pref->type==REQUIRE_PREFERENCE_TYPE)))
    mark_context_slot_as_acceptable_preference_changed (s);
}

/* ------------------------------------------------------------------------
   Remove_preference_from_tm() removes a given preference from PM and TM.
------------------------------------------------------------------------ */

void remove_preference_from_tm (preference *pref) {
  slot *s;
  
  s = pref->slot;

#ifdef DEBUG_PREFS
  print ("\nRemove preference at 0x%8x:  ",(unsigned long)pref);
  print_preference (pref);
#endif

  /* --- remove preference from the list for the slot --- */
  remove_from_dll (s->all_preferences, pref,
                   all_of_slot_next, all_of_slot_prev);
  remove_from_dll (s->preferences[pref->type], pref, next, prev);

  /* --- other miscellaneous stuff --- */    
  pref->in_tm = FALSE;
  pref->slot = NIL;      /* BUGBUG use pref->slot in place of pref->in_tm? */
  mark_slot_as_changed (s);
    
  /* --- if acceptable/require pref for context slot, we may need to remove
     a wme later --- */
  if ((s->isa_context_slot) &&
      ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
       (pref->type==REQUIRE_PREFERENCE_TYPE)))
    mark_context_slot_as_acceptable_preference_changed (s);

  /* --- update identifier levels --- */
  if (pref->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
    post_link_removal (pref->id, pref->value);
  if (preference_is_binary(pref->type))
    if (pref->referent->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
      post_link_removal (pref->id, pref->referent);

  /* --- deallocate it and clones if possible --- */
  preference_remove_ref (pref);
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

void process_o_rejects_and_deallocate_them (preference *o_rejects) {
  preference *pref, *next_pref, *p, *next_p;
  slot *s;

  for (pref=o_rejects; pref!=NIL; pref=pref->next) {
    preference_add_ref (pref);  /* prevents it from being deallocated if it's
                                   a clone of some other pref we're about to
                                   remove */
#ifdef DEBUG_PREFS
  print ("\nO-reject posted at 0x%8x:  ",(unsigned long)pref);
  print_preference (pref);
#endif
  }

  pref = o_rejects;
  while (pref) {
    next_pref = pref->next;
    s = find_slot (pref->id, pref->attr);
    if (s) {
      /* --- remove all pref's in the slot that have the same value --- */
      p = s->all_preferences;
      while (p) {
        next_p = p->all_of_slot_next;
        if (p->value==pref->value)
          remove_preference_from_tm (p);
        p = next_p;
      }
    }
    preference_remove_ref (pref);
    pref = next_pref;
  }
}

