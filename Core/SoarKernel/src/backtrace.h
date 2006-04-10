/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* =======================================================================
                               backtrace.h
======================================================================= */

/*
   For each production which is backtraced through, keep the name of the
   production, which condition was being traced (from the RHS of this
   production firing) and then the lists of grounds, potentials, locals
   and negateds generated during the backtrace.
   At the moment I'm not guaranteeing that each condition appears in the
   correct list -- this is because elements move between lists after their
   initial positioning.
*/

#ifndef BACKTRACE_H
#define BACKTRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef struct condition_struct condition;
typedef struct instantiation_struct instantiation;
typedef signed short goal_stack_level;
typedef struct agent_struct agent;

#define BUFFER_PROD_NAME_SIZE 256
typedef struct backtrace_struct {
   int result;                    /* 1 when this is a result of the chunk */
   condition *trace_cond;         /* The (local) condition being traced */
   char   prod_name[BUFFER_PROD_NAME_SIZE];         /* The production's name */
   condition *grounds;            /* The list of conds for the LHS of chunk */
   condition *potentials;         /* The list of conds which aren't linked */
   condition *locals;             /* Conds in the subgoal -- need to BT */
   condition *negated;            /* Negated conditions (sub/super) */
   struct backtrace_struct *next_backtrace; /* Pointer to next in this list */
} backtrace_str;

/* RBD Note: more comments here */
extern void trace_locals (agent* thisAgent, goal_stack_level grounds_level);
extern void trace_grounded_potentials (agent* thisAgent);
extern Bool trace_ungrounded_potentials (agent* thisAgent, goal_stack_level grounds_level);
extern void backtrace_through_instantiation (agent* thisAgent, 
                                             instantiation *inst,
                                             goal_stack_level grounds_level,
                                             condition *trace_cond,
                                             int indent);

#ifdef __cplusplus
}
#endif

#endif
