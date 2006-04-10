/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                                osupport.c

   Calculate_support_for_instantiation_preferences() does run-time o-support
   calculations -- it fills in pref->o_supported in each pref. on the
   instantiation.  Calculate_compile_time_o_support() does the compile-time
   version:  it takes the LHS and RHS, and fills in the a->support field in 
   each RHS action with either UNKNOWN_SUPPORT, O_SUPPORT, or I_SUPPORT.
======================================================================= */

#ifndef OSUPPORT_H
#define OSUPPORT_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef struct instantiation_struct instantiation;
typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct agent_struct agent;

extern void calculate_support_for_instantiation_preferences (agent* thisAgent, instantiation *inst);
extern void calculate_compile_time_o_support (condition *lhs, action *rhs);

extern void dougs_calculate_support_for_instantiation_preferences (agent* thisAgent, instantiation *inst);

#ifdef __cplusplus
}
#endif

#endif

