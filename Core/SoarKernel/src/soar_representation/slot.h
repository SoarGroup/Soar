/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* ---------------------------------------------------------------------
                               slot.h

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

#include "kernel.h"
#include "stl_typedefs.h"

typedef struct slot_struct
{
    struct slot_struct* next, *prev;  /* dll of slots for this id */
    Symbol* id;                       /* id, attr of the slot */
    Symbol* attr;
    wme* wmes;                        /* dll of wmes in the slot */
    wme* acceptable_preference_wmes;  /* dll of acceptable pref. wmes */
    preference* all_preferences;      /* dll of all pref's in the slot */
    preference* preferences[NUM_PREFERENCE_TYPES]; /* dlls for each type */
    cons* OSK_prefs;                  /* list of OSK prefs to backtrace through */
    Symbol* impasse_id;               /* NIL if slot is not impassed */
    bool isa_context_slot;
    byte impasse_type;
    bool marked_for_possible_removal;
    dl_cons* changed;   /* for non-context slots: points to the corresponding
                         dl_cons in changed_slots;  for context slots: just
                         zero/nonzero flag indicating slot changed */
    dl_cons* acceptable_preference_changed; /* for context slots: either zero,
                                             or points to dl_cons if the slot
                                             has changed + or ! pref's */

    wma_sym_reference_map* wma_val_references;

} slot;

extern slot* find_slot(Symbol* id, Symbol* attr);
extern slot* make_slot(agent* thisAgent, Symbol* id, Symbol* attr);
extern void mark_slot_as_changed(agent* thisAgent, slot* s);
extern void mark_slot_for_possible_removal(agent* thisAgent, slot* s);
extern void remove_garbage_slots(agent* thisAgent);
extern void clear_OSK_prefs(agent* thisAgent, slot* s);

#endif
