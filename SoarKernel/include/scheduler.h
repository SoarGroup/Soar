/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  scheduler.h
 *
 * =======================================================================

/*  This code is now obsolete, replaced by code in the Tcl interface.

    The schedule_agents function is the entry point to the multi-agent
    soar scheduler.  A round robin scheduling protocol is used for now.

    In a cycle all agents are run for their specified "agent-go" duration
    and the X-window event queue is processed (if applicable).  The allowed
    values for input are some nuber of cycles.  With an input of -1, the
    scheduler continues until all agents are stopped.  This may happen
    in normal agent processing termination or through a user interrupt.
*/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef struct agent_struct agent;

extern long scheduler_cycle_count;
extern void schedule_agents (agent* thisAgent, int cycles);

#ifdef __cplusplus
}
#endif

#endif
