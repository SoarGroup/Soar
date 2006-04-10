/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                recmem.h

   Init_firer() and init_chunker() should be called at startup time, to
   do initialization.

   Do_preference_phase() runs the entire preference phase.  This is called
   from the top-level control in main.c.

   Possibly_deallocate_instantiation() checks whether an instantiation
   can be deallocated yet, and does so if possible.  This is used whenever
   the (implicit) reference count on the instantiation decreases.
======================================================================= */

#ifndef RECMEM_H
#define RECMEM_H


#include "instantiations.h"

/* TEMPORARY HACK (Ideally this should be doable through
   the external kernel interface but for now using a 
   couple of global STL lists to get this information
   from the rhs function to this prefference adding code)*/
extern wme* glbDeepCopyWMEs;   

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef signed short goal_stack_level;
typedef struct agent_struct agent;
typedef struct preference_struct preference;
typedef struct instantiation_struct instantiation;
typedef struct wme_struct wme;


extern void init_firer (agent* thisAgent);
extern void do_preference_phase (agent* thisAgent);

/* RBD Definitely need more comments here */
extern preference *find_clone_for_level(preference *p, goal_stack_level level);
extern void fill_in_new_instantiation_stuff (agent* thisAgent, instantiation *inst,
                                      Bool need_to_do_support_calculations);

/* mvp 5-17-94 */
extern void build_prohibits_list (agent* thisAgent, instantiation *inst);

extern void deallocate_instantiation (agent* thisAgent, instantiation *inst);

#ifdef USE_MACROS
#define possibly_deallocate_instantiation(thisAgent, inst) { \
  if ((! (inst)->preferences_generated) && \
      (! (inst)->in_ms)) \
    deallocate_instantiation (thisAgent, inst); }
 
#else
inline void possibly_deallocate_instantiation(agent* thisAgent, instantiation * inst)
{
  if ((! (inst)->preferences_generated) &&
      (! (inst)->in_ms))
    deallocate_instantiation (thisAgent, inst);
}
#endif /* USE_MACROS */


#ifdef __cplusplus
}
#endif

#endif
