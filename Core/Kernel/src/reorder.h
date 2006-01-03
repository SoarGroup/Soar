/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                reorder.h
   Need to add comments here
======================================================================= */

#ifndef REORDER_H
#define REORDER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef unsigned long tc_number;
typedef struct cons_struct cons;
typedef struct agent_struct agent;
typedef struct action_struct action;
typedef struct condition_struct condition;
typedef cons list;

extern Bool reorder_action_list (agent* thisAgent, action **action_list, tc_number lhs_tc);
extern Bool reorder_lhs (agent* thisAgent, condition **lhs_top, 
						 condition **lhs_bottom, Bool reorder_nccs);
extern void init_reorderer (agent* thisAgent);

/* this prototype moved here from osupport.cpp -ajc (5/3/02) */
extern list *collect_root_variables(agent* thisAgent, condition *, tc_number, Bool);

#ifdef __cplusplus
}
#endif

#endif


