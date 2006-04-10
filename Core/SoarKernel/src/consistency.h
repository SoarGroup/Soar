/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  consistency.h
 *
 * =======================================================================
 *
 * Source code for Operand2/Waterfall specific functions in the kernel.
 *
 * =======================================================================
 *
 * Revision history:
 * 
 * 05 May 97: Created for version 2.0 of Operand2
 * REW
 *
 * 20 Aug 97: Version 2.1 of Operand2/Waterfall
 * Reimplemented the Waterfall functions with a more efficient algorithm
 * REW
 *
 */

#ifndef CONSISTENCY_H
#define CONSISTENCY_H

#include "gdatastructs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef signed short goal_stack_level;
typedef struct slot_struct slot;
typedef struct wme_struct wme;
typedef struct agent_struct agent;
typedef struct ms_change_struct ms_change;
typedef union symbol_union Symbol;

/* For information on the consistency check routines */
/* #define DEBUG_CONSISTENCY_CHECK    */

/* For information on aspects of determining the active level */
/* #define DEBUG_DETERMINE_LEVEL_PHASE   */

/* REW: begin 08.20.97 */
#define NEW_DECISION         0
#define SAME_LEVEL           1
#define HIGHER_LEVEL         2
#define LOWER_LEVEL          3
#define NIL_GOAL_RETRACTIONS 4
/* REW: end   08.20.97 */

/* REW: begin 09.15.96 */

/* These prototypes moved to decide.h -ajc (5/3/02) */

//extern void remove_wmes_for_context_slot (agent* thisAgent, slot *s);
//extern void remove_existing_context_and_descendents (Symbol *goal);
//extern byte type_of_existing_impasse (Symbol *goal);
//extern Symbol *attribute_of_existing_impasse (Symbol *goal);
//extern byte run_preference_semantics_for_consistency_check (slot *s, preference **result_candidates);

void remove_operator_if_necessary(agent* thisAgent, slot *s, wme *w);
Bool decision_consistent_with_current_preferences(agent* thisAgent, Symbol *goal, slot *s);
void remove_current_decision(agent* thisAgent, slot *s);
Bool check_context_slot_decisions (agent* thisAgent, goal_stack_level level);
/* REW: end   09.15.96 */

/* REW: begin 08.20.97 */  /* To implement the Waterfall part of Operand2 */
extern void print_assertion(ms_change *msc);
extern void print_retraction(ms_change *msc);
void initialize_consistency_calculations_for_new_decision(agent* thisAgent);
void determine_highest_active_production_level_in_stack_apply(agent* thisAgent);
void determine_highest_active_production_level_in_stack_propose(agent* thisAgent);
Bool goal_stack_consistent_through_goal(agent* thisAgent, Symbol *goal);
Bool i_activity_at_goal(Symbol *goal);
Bool minor_quiescence_at_goal(agent* thisAgent, Symbol *goal);
int active_production_type_at_goal(Symbol *goal);
Symbol * highest_active_goal_propose();
Symbol * highest_active_goal_apply(agent* thisAgent);
/* REW: end   08.20.97 */

#ifdef __cplusplus
}
#endif

#endif
