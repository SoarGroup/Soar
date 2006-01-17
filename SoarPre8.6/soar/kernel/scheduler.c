/*
 * =======================================================================
 *  File:  soarScheduler.c
 *
 * Uses a round-robin to run all soar agents.
 *
 * =======================================================================
 *
 *
 * Copyright 1995-2004 Carnegie Mellon University,
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
#include "scheduler.h"

void run_all_agents(long go_number, enum go_type_enum go_type, Symbol * go_slot_attr, goal_stack_level go_slot_level)
{
    bool agent_to_run = TRUE;
    cons *c;
    agent *the_agent;
    agent *prev_agent;
    long cycle_count = 0;

    prev_agent = soar_agent;
    c = all_soar_agents;
    if (!c->rest) {

        run_current_agent(go_number, go_type, go_slot_attr, go_slot_level);
    }

    else {

        for (c = all_soar_agents; c != NIL; c = c->rest) {
            the_agent = (agent *) c->first;
            the_agent->stop_soar = FALSE;
        }

        for (;;) {

            if (cycle_count == go_number)
                break;
            cycle_count++;

            agent_to_run = FALSE;
            for (c = all_soar_agents; c != NIL; c = c->rest) {
                soar_agent = (agent *) c->first;
                if (!(soar_agent->stop_soar)) {
                    agent_to_run = TRUE;
                }
            }

            if (!agent_to_run)
                break;

            for (c = all_soar_agents; c != NIL; c = c->rest) {
                soar_agent = (agent *) c->first;

                /*
                 *  I have put a callback in to do just this.  But I haven't
                 *  really figured out what functionality it provides....
                 *  This statement was only defined with the USE_TCL flag
                 *  082099 SW
                 */
                /*
                   Soar_SelectGlobalInterpByInterp(soar_agent->interpreter);
                 */

                if (!current_agent(stop_soar)) {

                    run_current_agent(1, go_type, go_slot_attr, go_slot_level);

                }               /* if */
            }                   /* for */

        }                       /* while */

        soar_agent = prev_agent;

        /* 
         *  There is no analog for this, however, I don't really see
         *  what this does either... (possible bug)
         *  This block was only defined in with USE_TCL flag
         *  082099 SW
         *  SWBUG possible bug for when using Tcl
         */
        /*  Soar_SelectGlobalInterpByInterp(soar_agent->interpreter); */
    }

}

void run_current_agent(long go_number, enum go_type_enum go_type, Symbol * go_slot_attr, goal_stack_level go_slot_level)
{

    soar_invoke_callbacks(soar_agent, BEFORE_SCHEDULE_CYCLE_CALLBACK, (soar_call_data) TRUE);

    switch (go_type) {
    case GO_PHASE:
        run_for_n_phases(go_number);
        break;
    case GO_ELABORATION:
        run_for_n_elaboration_cycles(go_number);
        break;
    case GO_DECISION:
        run_for_n_decision_cycles(go_number);
        break;
    case GO_STATE:
        run_for_n_selections_of_slot(go_number, soar_agent->state_symbol);
        break;
    case GO_OPERATOR:
        run_for_n_selections_of_slot(go_number, soar_agent->operator_symbol);
        break;
    case GO_SLOT:
        run_for_n_selections_of_slot_at_level(go_number, go_slot_attr, go_slot_level);
        break;
    case GO_OUTPUT:
        run_for_n_modifications_of_output(go_number);
        break;
    }

    soar_invoke_callbacks(soar_agent, AFTER_SCHEDULE_CYCLE_CALLBACK, (soar_call_data) TRUE);

}
