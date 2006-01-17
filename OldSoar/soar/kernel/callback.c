/*************************************************************************
 *
 *  file:  callback.c
 *
 * =======================================================================
 *
 * Description: This file contains the callback facility processing.  
 * 
 * Exported functions:
 *   soar_add_callback
 *   soar_invoke_callbacks
 *   soar_remove_callback
 *
 *   soar_callback_data_free_string
 *   soar_callback_enum_to_name
 *
 * Each agent has a separate callback table.  The table has one entry
 * per callback type and the entry is a pointer to a list.  The list
 * contains installed callbacks, one callback per list cons cell.
 *
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

#include "soarkernel.h"
#include "soar_core_api.h"
#include "callback.h"

extern soar_global_callback_array soar_global_callbacks;
extern unsigned long soar_global_callback_error;

char *soar_callback_names[] = { /* Must match order of       */
    "none",                     /* SOAR_CALLBACK_TYPE        */
    "system-startup",
    "system-termination",
    "before-init-soar",
    "after-init-soar",
    "after-halt-soar",
    "before-schedule-cycle",
    "after-schedule-cycle",
    "before-decision-cycle",
    "after-decision-cycle",
    "before-input-phase",
    "input-phase-cycle",
    "after-input-phase",
    "before-preference-phase-cycle",
    "after-preference-phase-cycle",
    "before-wm-phase-cycle",
    "after-wm-phase-cycle",
    "before-output-phase",
    "output-phase",
    "after-output-phase",
    "before-decision-phase-cycle",
    "after-decision-phase-cycle",
    "wm-changes",
    "create-new-context",
    "pop-context-stack",
    "create-new-attribute-impasse",
    "remove-attribute-impasse",
    "production-just-added",
    "production-just-about-to-be-excised",
    "firing",
    "retraction",
    "system-parameter-changed",
    "print",
    "log",
    "ask"
#ifdef ATTENTION_LAPSE
        "init-lapse-duration"
#endif
        /*  "read",   *//* kjh CUSP B10 */
        /*  "record", *//* kjh CUSP B10 */
        /* Nothing corresponds to NUMBER_OF_CALLBACKS */
};

void soar_init_callbacks(soar_callback_agent the_agent)
{
    SOAR_CALLBACK_TYPE ct;

    for (ct = 1; ct < NUMBER_OF_CALLBACKS; ct++) {
        ((agent *) the_agent)->soar_callbacks[ct] = (list *) NIL;
    }
    ((agent *) the_agent)->callback_error = 0;
}

void soar_init_global_callbacks(void)
{
    SOAR_GLOBAL_CALLBACK_TYPE gct;

    for (gct = 1; gct < NUMBER_OF_GLOBAL_CALLBACKS; gct++) {
        soar_global_callbacks[gct] = (list *) NIL;
    }
}

void soar_callback_data_free_string(soar_callback_data data)
{
    free((char *) data);
}

char *soar_callback_enum_to_name(SOAR_CALLBACK_TYPE i, bool monitorable_only)
{
    int limit;

    if (monitorable_only) {
        limit = NUMBER_OF_MONITORABLE_CALLBACKS;
    } else {
        limit = NUMBER_OF_CALLBACKS;
    }

    if ((0 < i) && (i < limit)) {
        return soar_callback_names[i];
    }
    return NULL;
}

bool soar_exists_callback(soar_callback_agent the_agent, SOAR_CALLBACK_TYPE callback_type)
{
    list *cb_cons;

    if ((agent *) the_agent == NULL)
        return FALSE;

    cb_cons = ((agent *) the_agent)->soar_callbacks[callback_type];

    if (cb_cons == NULL) {
        return FALSE;
    }

    return TRUE;
}

soar_callback *soar_exists_callback_id(soar_callback_agent the_agent,
                                       SOAR_CALLBACK_TYPE callback_type, soar_callback_id id)
{
    cons *c;

    if ((agent *) the_agent == NULL)
        return FALSE;

    for (c = ((agent *) the_agent)->soar_callbacks[callback_type]; c != NIL; c = c->rest) {
        soar_callback *cb;

        cb = (soar_callback *) c->first;

        if (!strcmp(cb->id, id)) {
            return cb;
        }
    }

    return NULL;
}

void soar_destroy_callback(soar_callback * cb)
{
    if (cb->id) {
        free((char *) cb->id);
    }
    if (cb->free_function) {
        cb->free_function(cb->data);
    }
    free((void *) cb);
}

void soar_invoke_callbacks(soar_callback_agent the_agent, SOAR_CALLBACK_TYPE callback_type, soar_call_data call_data)
{
    cons *c;

/* REW: begin 28.07.96 */
    /* We want to stop the Soar kernel timers whenever a callback is initiated and
       keep track of how much time the callbacks take cumulatively. This 
       switch doesn't include every pre-defined callback -- however, it should 
       provide a good "ballpark" estimate because it is focused on all those
       that occur doing do_one_top_level_phase in init_soar.c.

       Note that this case will only be compiled if NO_TIMING_STUFF is   not 
       defined.  So, if you are worried about the performance costs of this case,
       you can always get rid of it by not including the timing code. */

#ifndef NO_TIMING_STUFF
    switch (callback_type) {
        /* This case is necssary to make sure we are in one of the decision cycle
           monitors when the routine is invoked.  If so, then we want to turn off
           the current timers and turn on the appropriate monitor timers.  The 
           'appropriate' timer is determined by the current phase.  */

#ifndef FEW_CALLBACKS

    case BEFORE_DECISION_CYCLE_CALLBACK:
    case BEFORE_INPUT_PHASE_CALLBACK:
    case AFTER_INPUT_PHASE_CALLBACK:
        /* for these three: current_agent(current_phase) = INPUT_PHASE */
    case BEFORE_OUTPUT_PHASE_CALLBACK:
    case AFTER_OUTPUT_PHASE_CALLBACK:
        /* for these two: current_agent(current_phase) = OUTPUT_PHASE */
    case BEFORE_PREFERENCE_PHASE_CALLBACK:
    case AFTER_PREFERENCE_PHASE_CALLBACK:
        /* for these two: current_agent(current_phase) = PREFERENCE_PHASE */
    case BEFORE_WM_PHASE_CALLBACK:
    case AFTER_WM_PHASE_CALLBACK:
        /* for these two: current_agent(current_phase) = WM_PHASE */
    case BEFORE_DECISION_PHASE_CALLBACK:
    case AFTER_DECISION_PHASE_CALLBACK:
#endif

#ifndef NO_ADC_CALLBACK
    case AFTER_DECISION_CYCLE_CALLBACK:

#ifndef KERNEL_TIME_ONLY
        /* for these three: current_agent(current_phase) = DECISION_PHASE */

        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif

        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
        break;

#endif

#ifndef NO_IO_CALLBACKS
    case INPUT_PHASE_CALLBACK:
        /* Stop the kernel and phase timers for the input function */

#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif

        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
        start_timer(&current_agent(start_kernel_tv));
        break;
#endif
    default:
        break;

    }

#endif

/* REW: end 28.07.96 */
    for (c = ((agent *) the_agent)->soar_callbacks[callback_type]; c != NIL; c = c->rest) {
        soar_callback *cb;

        cb = (soar_callback *) c->first;
        cb->function(the_agent, cb->data, call_data);
    }

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
    switch (callback_type) {

#ifndef FEW_CALLBACKS

    case BEFORE_DECISION_CYCLE_CALLBACK:
    case BEFORE_INPUT_PHASE_CALLBACK:
    case AFTER_INPUT_PHASE_CALLBACK:
    case BEFORE_OUTPUT_PHASE_CALLBACK:
    case AFTER_OUTPUT_PHASE_CALLBACK:
    case BEFORE_PREFERENCE_PHASE_CALLBACK:
    case AFTER_PREFERENCE_PHASE_CALLBACK:
    case BEFORE_WM_PHASE_CALLBACK:
    case AFTER_WM_PHASE_CALLBACK:
    case BEFORE_DECISION_PHASE_CALLBACK:
    case AFTER_DECISION_PHASE_CALLBACK:
#endif

#ifndef NO_ADC_CALLBACK

    case AFTER_DECISION_CYCLE_CALLBACK:

#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv), &current_agent(monitors_cpu_time[current_agent(current_phase)]));
#endif

        start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
        break;

#endif

#ifndef NO_IO_CALLBACKS
    case INPUT_PHASE_CALLBACK:
        /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
        stop_timer(&current_agent(start_kernel_tv), &current_agent(input_function_cpu_time));
        start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
        break;
#endif
    default:
        break;

    }

#endif

/* REW: end 28.07.96 */

}

void soar_invoke_first_callback(soar_callback_agent the_agent,
                                SOAR_CALLBACK_TYPE callback_type, soar_call_data call_data)
{
    list *head;

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
    switch (callback_type) {

#ifndef FEW_CALLBACKS

    case BEFORE_DECISION_CYCLE_CALLBACK:
    case BEFORE_INPUT_PHASE_CALLBACK:
    case AFTER_INPUT_PHASE_CALLBACK:
        /* for these three: current_agent(current_phase) = INPUT_PHASE */
    case BEFORE_OUTPUT_PHASE_CALLBACK:
    case AFTER_OUTPUT_PHASE_CALLBACK:
        /* for these two: current_agent(current_phase) = OUTPUT_PHASE */
    case BEFORE_PREFERENCE_PHASE_CALLBACK:
    case AFTER_PREFERENCE_PHASE_CALLBACK:
        /* for these two: current_agent(current_phase) = PREFERENCE_PHASE */
    case BEFORE_WM_PHASE_CALLBACK:
    case AFTER_WM_PHASE_CALLBACK:
        /* for these two: current_agent(current_phase) = WM_PHASE */
    case BEFORE_DECISION_PHASE_CALLBACK:
    case AFTER_DECISION_PHASE_CALLBACK:

#endif

#ifndef NO_ADC_CALLBACK
    case AFTER_DECISION_CYCLE_CALLBACK:
        /* for these three: current_agent(current_phase) = DECISION_PHASE */

#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
        break;
#endif

#ifndef NO_IO_CALLBACKS
    case INPUT_PHASE_CALLBACK:
        /* Stop the kernel and phase timers for the input function */

#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif

        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
        start_timer(&current_agent(start_kernel_tv));
        break;
#endif
    default:
        break;

    }

#endif
/* REW: end 28.07.96 */

    head = ((agent *) the_agent)->soar_callbacks[callback_type];

    if (head != NULL) {
        soar_callback *cb;

        cb = (soar_callback *) head->first;
        cb->function(the_agent, cb->data, call_data);
    }

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
    switch (callback_type) {

#ifndef FEW_CALLBACKS

    case BEFORE_DECISION_CYCLE_CALLBACK:
    case BEFORE_INPUT_PHASE_CALLBACK:
    case AFTER_INPUT_PHASE_CALLBACK:
    case BEFORE_OUTPUT_PHASE_CALLBACK:
    case AFTER_OUTPUT_PHASE_CALLBACK:
    case BEFORE_PREFERENCE_PHASE_CALLBACK:
    case AFTER_PREFERENCE_PHASE_CALLBACK:
    case BEFORE_WM_PHASE_CALLBACK:
    case AFTER_WM_PHASE_CALLBACK:
    case BEFORE_DECISION_PHASE_CALLBACK:
    case AFTER_DECISION_PHASE_CALLBACK:

#endif
#ifndef NO_ADC_CALLBACK
    case AFTER_DECISION_CYCLE_CALLBACK:
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv), &current_agent(monitors_cpu_time[current_agent(current_phase)]));
#endif
        start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
        break;
#endif

#ifndef NO_IO_CALLBACKS
    case INPUT_PHASE_CALLBACK:
        /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
        stop_timer(&current_agent(start_kernel_tv), &current_agent(input_function_cpu_time));
        start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
        break;
#endif
    default:
        break;

    }

#endif

/* REW: end 28.07.96 */

}

bool soar_exists_global_callback(SOAR_GLOBAL_CALLBACK_TYPE callback_type)
{
    list *cb_cons;

    cb_cons = soar_global_callbacks[callback_type];

    if (cb_cons == NULL) {
        return FALSE;
    }

    return TRUE;
}

void soar_invoke_global_callbacks(soar_callback_agent a, SOAR_CALLBACK_TYPE callback_type, soar_call_data call_data)
{
    cons *c;

    /* So far, there's no need to mess with the timers, becuase
     * we have only implemented function which get called at 
     * agent creation and destruction
     */

    for (c = soar_global_callbacks[callback_type]; c != NIL; c = c->rest) {
        soar_callback *cb;

        cb = (soar_callback *) c->first;
        cb->function(a, cb->data, call_data);
    }

}
