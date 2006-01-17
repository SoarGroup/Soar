/*
 * =======================================================================
 *  File:  soarScheduler.c
 *
 * Uses a round-robin to run all soar agents.
 *
 * =======================================================================
 *
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


#include "soar.h"
#include "soarCommands.h"
#include "soarCommandUtils.h"
#include "soarScheduler.h"

long scheduler_cycle_count = 0;

Soar_ScheduledInterp Soar_BeforeSchedule = {NULL, NULL};
Soar_ScheduledInterp Soar_AfterSchedule = {NULL, NULL};

int
schedule_agents (Tcl_Interp * interp, int cycles) 
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

  while (TRUE) {

    if (cycle_count == cycles) break;
    cycle_count++;

    agent_to_run = FALSE;
    for(c = all_soar_agents; c != NIL; c = c->rest) {
      soar_agent = (agent *) c->first;
      if (!(soar_agent->stop_soar)) {
	agent_to_run = TRUE;
      }
    }

    if (!agent_to_run) break;

    if (Soar_BeforeSchedule.interp != NULL) {
      int code;

      code = Tcl_GlobalEval(Soar_BeforeSchedule.interp, 
			    Soar_BeforeSchedule.script);

      if (code != TCL_OK)
	{
	  interp->result = Soar_BeforeSchedule.interp->result;
	  return TCL_ERROR;
	}
    }

    for(c = all_soar_agents; c != NIL; c = c->rest) {
      soar_agent = (agent *) c->first;

      Soar_SelectGlobalInterpByInterp(soar_agent->interpreter);

     if (!current_agent(stop_soar)) {

       #ifndef NO_CALLBACKS
       soar_invoke_callbacks(soar_agent, 
			     BEFORE_SCHEDULE_CYCLE_CALLBACK,
			     (soar_call_data) TRUE);
       #endif

       execute_go_selection(soar_agent,
			    soar_agent->go_number,
			    soar_agent->go_type,
			    soar_agent->go_slot_attr,
			    soar_agent->go_slot_level);

#ifndef NO_CALLBACKS
       soar_invoke_callbacks(soar_agent, 
			     AFTER_SCHEDULE_CYCLE_CALLBACK,
			     (soar_call_data) TRUE);
#endif
     } /* if */
    } /* for */

    if (Soar_AfterSchedule.interp != NULL) {
      int code;

      code = Tcl_GlobalEval(Soar_AfterSchedule.interp, 
			    Soar_AfterSchedule.script);

      if (code != TCL_OK)
	{
	  interp->result = Soar_AfterSchedule.interp->result;
	  return TCL_ERROR;
	}
    }


  } /* while */

  soar_agent = prev_agent;

  Soar_SelectGlobalInterpByInterp(soar_agent->interpreter);

  return TCL_OK;
}

