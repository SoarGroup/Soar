/*************************************************************************
 *
 *  file:  scheduler.c
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
 * Copyright 1995-2003 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */

#include "soarkernel.h"

long scheduler_cycle_count = 0;

void
schedule_agents (int cycles) 
{
  bool agent_to_run = TRUE;
  cons  * c; 
  agent * the_agent; 
  agent * prev_agent;
  int cycle_count = 0;

  prev_agent = soar_agent;

  for(c = all_soar_agents; c != NIL; c = c->rest) {
    the_agent = (agent *) c->first;
    the_agent->stop_soar = FALSE;
  }

  while (agent_to_run) {

    if (cycle_count == cycles) break;
    cycle_count++;

    agent_to_run = FALSE;

    for(c = all_soar_agents; c != NIL; c = c->rest) {
      soar_agent = (agent *) c->first;

     if (!current_agent(stop_soar)) {
       agent_to_run = TRUE;

#ifndef USE_X_DISPLAY 
        print("\nSelecting agent %s", soar_agent->name);
#endif

#ifndef NO_CALLBACKS /* kjc 1/00 */
       soar_invoke_callbacks(soar_agent, 
			     BEFORE_SCHEDULE_CYCLE_CALLBACK,
			     (soar_call_data) TRUE);
#endif

       old_execute_go_selection();

#ifndef NO_CALLBACKS /* kjc 1/00 */
	   soar_invoke_callbacks(soar_agent, 
			     AFTER_SCHEDULE_CYCLE_CALLBACK,
			     (soar_call_data) TRUE);
#endif

#ifdef USE_X_DISPLAY
        if (soar_agent->monitor) 
          {
	    refresh_monitor_window(soar_agent);
	  }
#endif
     }
    
    }

#ifdef USE_X_DISPLAY
      handle_soar_x_events();
#endif

  }

  soar_agent = prev_agent;

}

