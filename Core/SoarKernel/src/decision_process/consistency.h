/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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
 * ======================================================================= */

#ifndef CONSISTENCY_H
#define CONSISTENCY_H

#include "kernel.h"

/* For information on the consistency check routines */
/* #define DEBUG_CONSISTENCY_CHECK    */

/* For information on aspects of determining the active level */
/* #define DEBUG_DETERMINE_LEVEL_PHASE   */

#define NEW_DECISION         0
#define SAME_LEVEL           1
#define HIGHER_LEVEL         2
#define LOWER_LEVEL          3
#define NIL_GOAL_RETRACTIONS 4

void remove_operator_if_necessary(agent* thisAgent, slot* s, wme* w);
bool decision_consistent_with_current_preferences(agent* thisAgent, Symbol* goal, slot* s);
void remove_current_decision(agent* thisAgent, slot* s);
bool check_context_slot_decisions(agent* thisAgent, goal_stack_level level);

/* REW: begin 08.20.97 */  /* To implement the Waterfall part of Operand2 */
void initialize_consistency_calculations_for_new_decision(agent* thisAgent);
void determine_highest_active_production_level_in_stack_apply(agent* thisAgent);
void determine_highest_active_production_level_in_stack_propose(agent* thisAgent);
bool goal_stack_consistent_through_goal(agent* thisAgent, Symbol* goal);
bool i_activity_at_goal(Symbol* goal);
bool minor_quiescence_at_goal(agent* thisAgent, Symbol* goal);
int active_production_type_at_goal(Symbol* goal);

Symbol* highest_active_goal_propose(agent* thisAgent, Symbol* start_goal, bool noneOk);
Symbol* highest_active_goal_apply(agent* thisAgent, Symbol* start_goal, bool noneOk);

/* REW: end   08.20.97 */

#endif
