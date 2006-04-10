/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                decide.h

   Decide.cpp contains the decider as well as routine for managing working
   memory, preference memory, slots, and the garbage collection of
   disconnected WMEs.

   Whenever a link is added from one identifier to another (i.e.,
   (I37 ^x R26)), we call post_link_addition().  This records the link
   addition and buffers it for later processing.  Similarly, whenever a
   link is removed, we call post_link_removal(), which buffers the
   removal for later processing.  At the end of the phase, we call
   do_buffered_link_changes() to update the goal stack level of all
   identifiers, and garbage collect anything that's now disconnected.

   Whenever some acceptable or require preference for a context slot
   changes, we call mark_context_slot_as_acceptable_preference_changed().
   
   see decide.cpp for more information in the comments.
======================================================================= */

#ifndef DECIDE_H
#define DECIDE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef unsigned char byte;
typedef union symbol_union Symbol;
typedef struct wme_struct wme;
typedef struct slot_struct slot;
typedef struct instantiation_struct instantiation;
typedef struct preference_struct preference;
typedef struct agent_struct agent;

extern void post_link_addition (agent* thisAgent, Symbol *from, Symbol *to);
extern void post_link_removal (agent* thisAgent, Symbol *from, Symbol *to);

extern void mark_context_slot_as_acceptable_preference_changed (agent* thisAgent, slot *s);

void remove_existing_attribute_impasse_for_slot (agent* thisAgent, slot *s);
void post_link_addition (agent* thisAgent, Symbol *from, Symbol *to);
void post_link_removal (agent* thisAgent, Symbol *from, Symbol *to);

/* REW: begin 09.15.96   additions for Soar8 architecture */
void elaborate_gds (agent* thisAgent);
void gds_invalid_so_remove_goal (agent* thisAgent, wme *w);
void free_parent_list(agent* thisAgent);
void uniquely_add_to_head_of_dll(agent* thisAgent, instantiation *inst);
void create_gds_for_goal( agent* thisAgent, Symbol *goal );
extern void remove_operator_if_necessary(agent* thisAgent, slot *s, wme *w);

extern int GDS_PrintCmd (/****ClientData****/ int clientData, 
                         /****Tcl_Interp****/ void * interp,
                         int argc, char *argv[]);
/* REW: end   09.15.96 */


/* ---------------------------------------------------------------------
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
--------------------------------------------------------------------- */

extern void remove_wmes_for_context_slot (agent* thisAgent, slot *s); /* added this prototype -ajc (5/1/02) */
extern void init_decider (agent* thisAgent);
extern void do_buffered_wm_and_ownership_changes (agent* thisAgent);
extern void do_working_memory_phase (agent* thisAgent);
extern void do_decision_phase (agent* thisAgent);
extern void create_top_goal (agent* thisAgent);
extern void clear_goal_stack (agent* thisAgent);
extern void print_lowest_slot_in_context_stack (agent* thisAgent);

/* These prototypes moved here from consistency.cpp -ajc (5/3/02) */
extern void remove_existing_context_and_descendents (agent* thisAgent, Symbol *goal);
extern byte type_of_existing_impasse (agent* thisAgent, Symbol *goal);
extern Symbol *attribute_of_existing_impasse (agent* thisAgent, Symbol *goal);
extern byte run_preference_semantics_for_consistency_check (agent* thisAgent, 
															slot *s, preference **result_candidates);

/* These prototypes moved here from chunk.cpp -ajc (5/3/02) */
extern byte type_of_existing_impasse (agent* thisAgent, Symbol *goal);

#ifdef __cplusplus
}
#endif

#endif
