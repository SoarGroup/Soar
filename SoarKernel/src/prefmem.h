/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ---------------------------------------------------------------------
                     Preference Management Routines

   Make_preference() creates a new preference structure of the given type
   with the given id/attribute/value/referent.  (Referent is only used
   for binary preferences.)  The preference is not yet added to preference
   memory, however.

   Preference_add_ref() and preference_remove_ref() are macros for
   incrementing and decrementing the reference count on a preference.
   
   Possibly_deallocate_preference_and_clones() checks whether a given
   preference and all its clones have reference_count 0, and deallocates
   them all if they do; it returns TRUE if they were actually deallocated,
   FALSE otherwise.   Deallocate_preference() deallocates a given
   preference.  These routines should normally be invoked only via the
   preference_remove_ref() macro.

   Add_preference_to_tm() adds a given preference to preference memory (and
   hence temporary memory).  Remove_preference_from_tm() removes a given
   preference from PM and TM.

   Process_o_rejects_and_deallocate_them() handles the processing of
   o-supported reject preferences.  This routine is called from the firer
   and passed a list of all the o-rejects generated in the current
   preference phase (the list is linked via the "next" fields on the
   preference structures).  This routine removes all preferences for
   matching values from TM, and deallocates the o-reject preferences when
   done.
--------------------------------------------------------------------- */
  
#ifndef PREFMEM_H
#define PREFMEM_H

#include "gdatastructs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef unsigned char byte;
typedef struct agent_struct agent;
typedef struct preference_struct preference;
typedef union symbol_union Symbol;

extern preference *make_preference (agent* thisAgent, byte type, Symbol *id, Symbol *attr,
                                    Symbol *value, Symbol *referent);

extern Bool possibly_deallocate_preference_and_clones (agent* thisAgent, preference *pref);

#ifdef USE_MACROS

#define preference_add_ref(p) { (p)->reference_count++; }
#define preference_remove_ref(thisAgent, p) { \
  (p)->reference_count--; \
  if ((p)->reference_count == 0) \
    possibly_deallocate_preference_and_clones(thisAgent, p); }

#else

inline void preference_add_ref(preference * p)
{
  (p)->reference_count++;
}

inline void preference_remove_ref(agent* thisAgent, preference * p)
{
  (p)->reference_count--;
  if ((p)->reference_count == 0)
    possibly_deallocate_preference_and_clones(thisAgent, p);
}

#endif /* USE_MACROS */

extern void deallocate_preference (agent* thisAgent, preference *pref);

extern void add_preference_to_tm (agent* thisAgent, preference *pref);
extern void remove_preference_from_tm (agent* thisAgent, preference *pref);
extern void process_o_rejects_and_deallocate_them (agent* thisAgent, 
												   preference *o_rejects);

#ifdef __cplusplus
}
#endif

#endif
