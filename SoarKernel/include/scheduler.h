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
/* =======================================================================
 *
 * Copyright (c) 1995-1999 Carnegie Mellon University,
 *                         The Regents of the University of Michigan,
 *                         University of Southern California/Information
 *                         Sciences Institute.  All rights reserved.
 *
 * The Soar consortium proclaims this software is in the public domain, and
 * is made available AS IS.  Carnegie Mellon University, The University of 
 * Michigan, and The University of Southern California/Information Sciences 
 * Institute make no warranties about the software or its performance,
 * implied or otherwise.
 * =======================================================================
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
