/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ---------------------------------------------------------------------
                               tempmem.h

   Find_slot() looks for an existing slot for a given id/attr pair, and
   returns it if found.  If no such slot exists, it returns NIL.
   Make_slot() looks for an existing slot for a given id/attr pair,
   returns it if found, and otherwise creates a new slot and returns it.

   Mark_slot_as_changed() is called by the preference manager whenever
   the preferences for a slot change.  This updates the list of
   changed_slots and highest_goal_whose_context_changed for use by the
   decider.

   Old slots are garbage collected as follows:  whenever we notice that
   the last preference has been removed from a slot, we call
   mark_slot_for_possible_removal().  We don't deallocate the slot
   right away, because there might still be wmes in it, or we might
   be about to add a new preference to it (through some later action
   of the same production firing, for example).  At the end of the phase, 
   we call remove_garbage_slots(), which scans through each marked slot 
   and garbage collects it if it has no wmes or preferences.
--------------------------------------------------------------------- */

#ifndef TEMPMEM_H
#define TEMPMEM_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef union symbol_union Symbol;
typedef struct slot_struct slot;
typedef struct agent_struct agent;

extern slot *find_slot (Symbol *id, Symbol *attr);
extern slot *make_slot (agent* thisAgent, Symbol *id, Symbol *attr);
extern void mark_slot_as_changed (agent* thisAgent, slot *s);
extern void mark_slot_for_possible_removal (agent* thisAgent, slot *s);
extern void remove_garbage_slots (agent* thisAgent);

#ifdef __cplusplus
}
#endif

#endif
