/*************************************************************************
 *
 *  file:  agent.c
 *
 * =======================================================================
 *  Initialization for the agent structure.  Also the cleanup routine
 *  when an agent is destroyed.  These routines are usually replaced
 *  by the same-named routines in the Tcl interface file soarAgent.c
 *  The versions in this file are used only when not linking in Tcl.
 *  HOWEVER, this code should be maintained, and the agent structure
 *  must be kept up to date.
 * =======================================================================
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
/* ===================================================================
                     Agent-related functions
   =================================================================== */

#include "soarkernel.h"
#include "scheduler.h"
#include "sysdep.h"
#include "rhsfun_examples.h"

agent *soar_agent;

list *all_soar_agents = NIL;

int agent_counter = -1;
int agent_count = -1;

char *soar_version_string;

extern int soar_agent_ids[];

/*
  Try to assign a unique and previously unassigned id.  If none are
   available, assign a unique, but previously assigned id.
*/
int next_available_agent_id()
{
    int i;

    for (i = 0; i < MAX_SIMULTANEOUS_AGENTS; i++) {
        if (soar_agent_ids[i] == UNTOUCHED) {
            soar_agent_ids[i] = ALLOCATED;
            return i;
        }
    }
    for (i = 0; i < MAX_SIMULTANEOUS_AGENTS; i++) {
        if (soar_agent_ids[i] == TOUCHED) {
            soar_agent_ids[i] = ALLOCATED;
            return i;
        }
    }
    {
        char msg[MESSAGE_SIZE];
        snprintf(msg, MESSAGE_SIZE, "agent.c: Error: Too many simultaneous agents (> %d\n", MAX_SIMULTANEOUS_AGENTS);
        msg[MESSAGE_SIZE - 1] = 0;      /* snprintf doesn't set last char to null if output is truncated */

        abort_with_fatal_error(msg);
    }
    return -1;                  /* To placate compilier */
}

#ifdef ATTENTION_LAPSE
/* RMJ;
   When doing attentional lapsing, we need a function that determines
   when (and for how long) attentional lapses should occur.  This
   will normally be provided as a user-defined TCL procedure.
*/

long init_lapse_duration(TIMER_VALUE * tv)
{
    int ret;
    long time_since_last_lapse;
    char buf[128];

    start_timer(current_real_time);
    timersub(current_real_time, tv, current_real_time);
    time_since_last_lapse = (long) (1000 * timer_value(tv));

    if (soar_exists_callback(soar_agent, INIT_LAPSE_DURATION_CALLBACK)) {

        /* SW 11.07.00
         *
         *  Modified this to use the generic soar interface, as opposed
         *  to being Tcl specific.  This requires a new callback
         *  in particular, here the callback receives the value 
         *  time_since_last_lapse, and must RESET this value to
         *  the appropriate number
         */
        soar_invoke_callback(soar_agent, INIT_LAPSE_DURATION_CALLBACK, (void *) &time_since_last_lapse);

        return time_since_last_lapse;
    }
    return 0;
}

#endif

/* ===================================================================
   
                           Initialization Function

=================================================================== */

void init_soar_agent(void)
{

    /* --- initialize everything --- */
    init_memory_utilities();
    init_symbol_tables();
    create_predefined_symbols();
    init_production_utilities();
    init_built_in_rhs_functions();
    /*add_bot_rhs_functions (soar_agent); */
    add_bot_rhs_functions();
    init_rete();
    init_lexer();
    init_firer();
    init_decider();
    init_soar_io();
    init_chunker();
    init_sysparams();
    init_tracing();
    init_explain();             /* AGR 564 */
#ifdef REAL_TIME_BEHAVIOR
    /* RMJ */
    init_real_time();
#endif
#ifdef ATTENTION_LAPSE
    /* RMJ */
    init_attention_lapse();
#endif

    /* --- add default object trace formats --- */
    add_trace_format(FALSE, FOR_ANYTHING_TF, NIL, "%id %ifdef[(%v[name])]");
    add_trace_format(FALSE, FOR_STATES_TF, NIL, "%id %ifdef[(%v[attribute] %v[impasse])]");
    {
        Symbol *evaluate_object_sym;
        evaluate_object_sym = make_sym_constant("evaluate-object");
        add_trace_format(FALSE, FOR_OPERATORS_TF, evaluate_object_sym, "%id (evaluate-object %o[object])");
        symbol_remove_ref(evaluate_object_sym);
    }
    /* --- add default stack trace formats --- */
    add_trace_format(TRUE, FOR_ANYTHING_TF, NIL, "%right[6,%dc]: %rsd[   ]==>S: %id %ifdef[(%v[name])]");
    add_trace_format(TRUE, FOR_STATES_TF, NIL, "%right[6,%dc]: %rsd[   ]==>S: %cs");
    add_trace_format(TRUE, FOR_OPERATORS_TF, NIL, "%right[6,%dc]: %rsd[   ]   O: %co");
    reset_statistics();
}
