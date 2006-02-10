#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  prefmem.cpp
 *
 * =======================================================================
 *  NOTE:  need some comments here
 * =======================================================================
 */

/* ======================================================================
        Preference Memory routines for Soar 6
   ====================================================================== */

/* Debugging stuff:  #define DEBUG_PREFS to get preference printouts */

/* #define DEBUG_PREFS */

#include "mem.h"
#include "kernel.h"
#include "agent.h"
#include "gdatastructs.h"
#include "instantiations.h"
#include "symtab.h"
#include "recmem.h"
#include "tempmem.h"
#include "decide.h"
#include "prefmem.h"
#include "print.h"

/* JC ADDED: For telling gski about events */
#include "gski_event_system_functions.h"

char * preference_name[] =
{ "acceptable",
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
  "worse" };

/*                     Preference Management Routines

====================================================================== */

/* ----------------------------------------------------------------------
   Make_preference() creates a new preference structure of the given type
   with the given id/attribute/value/referent.  (Referent is only used
   for binary preferences.)  The preference is not yet added to preference
   memory, however.
---------------------------------------------------------------------- */

preference *make_preference (agent* thisAgent, byte type, Symbol *id, Symbol *attr,
                             Symbol *value, Symbol *referent) {
  preference *p;

  allocate_with_pool (thisAgent, &thisAgent->preference_pool, &p);
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
  print (thisAgent, "\nAllocating preference at 0x%8x: ", (unsigned long)p);
  print_preference (thisAgent, p);
#endif

  return p;

  /* BUGBUG check to make sure the pref doesn't have
        value or referent .isa_goal or .isa_impasse; */
}

/* ----------------------------------------------------------------------
   Deallocate_preference() deallocates a given preference.
---------------------------------------------------------------------- */

void deallocate_preference (agent* thisAgent, preference *pref) {

#ifdef DEBUG_PREFS  
  print (thisAgent, "\nDeallocating preference at 0x%8x: ",(unsigned long)pref);
  print_preference (thisAgent, pref);
  if (pref->reference_count != 0) {   /* --- sanity check --- */
    char msg[BUFFER_MSG_SIZE];
    strncpy (msg, "prefmem.c: Internal Error: Deallocating preference with ref. count != 0\n", BUFFER_MSG_SIZE);
    msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
    abort_with_fatal_error(thisAgent, msg);
  }
#endif

  /* --- remove it from the list of pref's for its match goal --- */
  if (pref->on_goal_list)
    remove_from_dll (pref->inst->match_goal->id.preferences_from_goal,
                     pref, all_of_goal_next, all_of_goal_prev);
  
  /* --- remove it from the list of pref's from that instantiation --- */
  remove_from_dll (pref->inst->preferences_generated, pref,
                   inst_next, inst_prev);
  possibly_deallocate_instantiation (thisAgent, pref->inst);

  /* --- dereference component symbols --- */
  symbol_remove_ref (thisAgent, pref->id);
  symbol_remove_ref (thisAgent, pref->attr);
  symbol_remove_ref (thisAgent, pref->value);
  if (preference_is_binary(pref->type))
    symbol_remove_ref (thisAgent, pref->referent);
  
  /* --- free the memory --- */
  free_with_pool (&thisAgent->preference_pool, pref);
}  

/* ----------------------------------------------------------------------
   Possibly_deallocate_preference_and_clones() checks whether a given
   preference and all its clones have reference_count 0, and deallocates
   them all if they do.  It returns TRUE if they were actually
   deallocated, FALSE otherwise.
---------------------------------------------------------------------- */

Bool possibly_deallocate_preference_and_clones (agent* thisAgent, preference *pref) {
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
    deallocate_preference (thisAgent, clone);
    clone = next;
  }
  clone = pref->prev_clone;
  while (clone) {
    next = clone->prev_clone;
    deallocate_preference (thisAgent, clone);
    clone = next;
  }

  /* --- deallocate pref --- */
  deallocate_preference (thisAgent, pref);

  return TRUE;
}

/* ----------------------------------------------------------------------
   Remove_preference_from_clones() splices a given preference out of the
   list of clones.  If the preference's reference_count is 0, it also
   deallocates it and returns TRUE.  Otherwise it returns FALSE.
---------------------------------------------------------------------- */

Bool remove_preference_from_clones (agent* thisAgent, preference *pref) {
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
  if (any_clone) possibly_deallocate_preference_and_clones (thisAgent, any_clone);
  if (! pref->reference_count) {
    deallocate_preference (thisAgent, pref);
    return TRUE;
  } else {
    return FALSE;
  }
}

/* ------------------------------------------------------------------------
   Add_preference_to_tm() adds a given preference to preference memory (and
   hence temporary memory).
------------------------------------------------------------------------ */

void add_preference_to_tm (agent* thisAgent, preference *pref) 
{
   slot *s;
   preference *p2;

#ifdef DEBUG_PREFS
   print (thisAgent, "\nAdd preference at 0x%8x:  ",(unsigned long)pref);
   print_preference (thisAgent, pref);
#endif
   
   /* JC: This will retrieve the slot for pref->id if it already exists */
   s = make_slot (thisAgent, pref->id, pref->attr);
   pref->slot = s;
   
   insert_at_head_of_dll (s->all_preferences, pref,
                          all_of_slot_next, all_of_slot_prev);
   
   /* --- add preference to the list (in the right place, according to match
          goal level of the instantiations) for the slot --- */
   
   if (!s->preferences[pref->type]) 
   {
      /* --- this is the only pref. of its type, just put it at the head --- */
      insert_at_head_of_dll (s->preferences[pref->type], pref, next, prev);
   } 
   else if (s->preferences[pref->type]->inst->match_goal_level >= pref->inst->match_goal_level) 
   {
      /* --- it belongs at the head of the list, so put it there --- */
      insert_at_head_of_dll (s->preferences[pref->type], pref, next, prev);
   } 
   else 
   {
      /* --- scan through the pref. list, find the one to insert after --- */
      for (p2 = s->preferences[pref->type]; p2->next != NIL; p2 = p2->next)
      {  
         if (p2->next->inst->match_goal_level >= pref->inst->match_goal_level)
            break;
      }
      
      /* --- insert pref after p2 --- */
      pref->next = p2->next;
      pref->prev = p2;
      p2->next = pref;
      if (pref->next) 
         pref->next->prev = pref;
   }
   
   /* --- other miscellaneous stuff --- */    
   pref->in_tm = TRUE;
   preference_add_ref (pref);
   
   mark_slot_as_changed (thisAgent, s);
   
   /* --- update identifier levels --- */
   if (pref->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
   {
      post_link_addition (thisAgent, pref->id, pref->value);

      /* JC ADDED: Tell about a new object if we have an acceptable or required preference */
      if((pref->value->id.link_count == 1) && 
         ((pref->type == ACCEPTABLE_PREFERENCE_TYPE) || (pref->type == REQUIRE_PREFERENCE_TYPE)))
      {
         gSKI_MakeAgentCallbackWMObjectAdded(thisAgent, pref->value, pref->attr, pref->id);
      }
   }
   
   if (preference_is_binary(pref->type))
   {
      if (pref->referent->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
         post_link_addition (thisAgent, pref->id, pref->referent);
   }
   
   /* --- if acceptable/require pref for context slot, we may need to add a
   wme later --- */
   if ((s->isa_context_slot) &&
      ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
      (pref->type==REQUIRE_PREFERENCE_TYPE)))
   {
      mark_context_slot_as_acceptable_preference_changed (thisAgent, s);
   }
   
   /* JC ADDED: notify gSKI of both preference additions and operator proposals */
   if (s->isa_context_slot && (s->attr == thisAgent->operator_symbol))
   {
      /* Tell gSKI we have an operator preference */
      gSKI_MakeAgentCallback(gSKI_K_EVENT_OPERATOR_PREF_ADDED, 1, thisAgent, static_cast<void*>(pref));

      /* If it is acceptable, tell gSKI that we have an operator proposed */
      if((pref->type == ACCEPTABLE_PREFERENCE_TYPE) || (pref->type == REQUIRE_PREFERENCE_TYPE))
         gSKI_MakeAgentCallback(gSKI_K_EVENT_OPERATOR_PROPOSED, 1, thisAgent, static_cast<void*>(pref));
   }
}

/* ------------------------------------------------------------------------
   Remove_preference_from_tm() removes a given preference from PM and TM.
------------------------------------------------------------------------ */

void remove_preference_from_tm (agent* thisAgent, preference *pref) {
  slot *s;
  
  s = pref->slot;

#ifdef DEBUG_PREFS
  print (thisAgent, "\nRemove preference at 0x%8x:  ",(unsigned long)pref);
  print_preference (thisAgent, pref);
#endif

  /* --- remove preference from the list for the slot --- */
  remove_from_dll (s->all_preferences, pref,
                   all_of_slot_next, all_of_slot_prev);
  remove_from_dll (s->preferences[pref->type], pref, next, prev);

  /* --- other miscellaneous stuff --- */    
  pref->in_tm = FALSE;
  pref->slot = NIL;      /* BUG shouldn't we use pref->slot in place of pref->in_tm? */
  mark_slot_as_changed (thisAgent, s);
    
  /* --- if acceptable/require pref for context slot, we may need to remove
     a wme later --- */
  if ((s->isa_context_slot) &&
      ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
       (pref->type==REQUIRE_PREFERENCE_TYPE)))
    mark_context_slot_as_acceptable_preference_changed (thisAgent, s);

  /* --- update identifier levels --- */
  if (pref->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
    post_link_removal (thisAgent, pref->id, pref->value);
  if (preference_is_binary(pref->type))
    if (pref->referent->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
      post_link_removal (thisAgent, pref->id, pref->referent);

  /* --- deallocate it and clones if possible --- */
  preference_remove_ref (thisAgent, pref);
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

void process_o_rejects_and_deallocate_them (agent* thisAgent, preference *o_rejects) 
{
  preference *pref, *next_pref, *p, *next_p;
  slot *s;

  for (pref=o_rejects; pref!=NIL; pref=pref->next) {
    preference_add_ref (pref);  /* prevents it from being deallocated if it's
                                   a clone of some other pref we're about to
                                   remove */
#ifdef DEBUG_PREFS
  print (thisAgent, "\nO-reject posted at 0x%8x:  ",(unsigned long)pref);
  print_preference (thisAgent, pref);
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
          remove_preference_from_tm (thisAgent, p);
        p = next_p;
      }
    }
    preference_remove_ref (thisAgent, pref);
    pref = next_pref;
  }
}

