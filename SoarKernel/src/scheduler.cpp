/*************************************************************************
 *
 *  file:  scheduler.cpp
 *
 * =======================================================================
 *  This code is now obsolete, replaced by code in the Tcl interface.
 *
 *  The schedule_agents function is the entry point to the multi-agent
 *  soar scheduler.  A round robin scheduling protocol is used for now.
 *
 *  In a cycle all agents are run for their specified "agent-go" duration
 *  and the X-window event queue is processed (if applicable).  The allowed
 *  values for input are some nuber of cycles.  With an input of -1, the
 *  scheduler continues until all agents are stopped.  This may happen
 *  in normal agent processing termination or through a user interrupt.
 * =======================================================================
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

#include "scheduler.h"
#include "kernel.h"
#include "mem.h"
#include "agent.h"
#include "print.h"
#include "interface.h"
#include "kernel_struct.h"

long scheduler_cycle_count = 0;

void
schedule_agents (Kernel* thisKernel, agent* thisAgent, int cycles) 
{
  Bool agent_to_run = TRUE;
  cons  * c; 
  agent * the_agent; 
  agent * prev_agent;
  int cycle_count = 0;

  prev_agent = thisAgent;

  for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
    the_agent = (agent *) c->first;
    the_agent->stop_soar = FALSE;
  }

  while (agent_to_run) {

    if (cycle_count == cycles) break;
    cycle_count++;

    agent_to_run = FALSE;

    for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {

      thisAgent = (agent *) c->first;

     if (!thisAgent->stop_soar) {
       agent_to_run = TRUE;

#ifndef USE_X_DISPLAY 
        print(thisAgent, "\nSelecting agent %s", thisAgent->name);
#endif

#ifndef NO_CALLBACKS /* kjc 1/00 */
       soar_invoke_callbacks(thisAgent, thisAgent, 
			     BEFORE_SCHEDULE_CYCLE_CALLBACK,
			     (soar_call_data) TRUE);
#endif

       //old_execute_go_selection(thisAgent);

#ifndef NO_CALLBACKS /* kjc 1/00 */
	   soar_invoke_callbacks(thisAgent, thisAgent, 
			     AFTER_SCHEDULE_CYCLE_CALLBACK,
			     (soar_call_data) TRUE);
#endif

#ifdef USE_X_DISPLAY
        if (thisAgent->monitor) 
          {
	    refresh_monitor_window(thisAgent);
	  }
#endif
     }
    
    }

#ifdef USE_X_DISPLAY
      handle_soar_x_events();
#endif

  }

  thisAgent = prev_agent;

}

