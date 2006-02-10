#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  callback.cpp
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
 *   soar_callback_name_to_enum
 *
 * Each agent has a separate callback table.  The table has one entry
 * per callback type and the entry is a pointer to a list.  The list
 * contains installed callbacks, one callback per list cons cell.
 *
 * =======================================================================
 */


#include "callback.h"
#include "agent.h"
#include "init_soar.h"
#include "print.h"

char * soar_callback_names[] = {    /* Must match order of       */
  "none",                           /* SOAR_CALLBACK_TYPE        */
  "system-startup",
  "system-termination",
  "after-init-agent",
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
  "before-propose-phase-cycle"
  "after-propose-phase-cycle"
  "before-apply-phase-cycle"
  "after-apply-phase-cycle"
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
//  "read",				/* kjh CUSP B10 */
//  "record",				/* kjh CUSP B10 */
  /* Nothing corresponds to NUMBER_OF_CALLBACKS */
};


void soar_init_callbacks (soar_callback_agent the_agent)
{
  int ct; // ct was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)

  for (ct = 1; ct < NUMBER_OF_CALLBACKS; ct++)
    {
      ((agent *)the_agent)->soar_callbacks[ct] = (list *) NIL;
    }
}

void soar_add_callback (agent* thisAgent, 
			soar_callback_agent the_agent, 
			SOAR_CALLBACK_TYPE callback_type, 
			soar_callback_fn fn, 
			soar_callback_data data,
			soar_callback_free_fn free_fn,
			soar_callback_id id)
{
  soar_callback * cb;

  cb = (soar_callback *) malloc (sizeof(soar_callback));
  cb->function      = fn;
  cb->data          = data;
  cb->free_function = free_fn;
  cb->id            = savestring(id);
  
  push(thisAgent, cb, ((agent *) the_agent)->soar_callbacks[callback_type]);
}

void soar_callback_data_free_string (soar_callback_data data)
{
  free((char *) data);
}

char * soar_callback_enum_to_name (SOAR_CALLBACK_TYPE i, 
				   Bool monitorable_only)
{
  int limit;

  if (monitorable_only)
    {
      limit = NUMBER_OF_MONITORABLE_CALLBACKS;
    }
  else 
    {
      limit = NUMBER_OF_CALLBACKS;
    }

  if ((0 < i) && (i < limit))
    {
      return soar_callback_names[i];
    }
  return NULL;
}

SOAR_CALLBACK_TYPE soar_callback_name_to_enum (char * name,
					       Bool monitorable_only)
{
  int limit;
  int i;  // i was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)

  if (monitorable_only)
    {
      limit = NUMBER_OF_MONITORABLE_CALLBACKS;
    }
  else 
    {
      limit = NUMBER_OF_CALLBACKS;
    }

  for(i = 1; i < limit; i++)
    {
      if (!strcmp(name, soar_callback_names[i]))
	{
	  return static_cast<SOAR_CALLBACK_TYPE>(i);
	}
    }

  return NO_CALLBACK;
}

Bool soar_exists_callback(soar_callback_agent the_agent,
			  SOAR_CALLBACK_TYPE callback_type)
{
  list * cb_cons;

  cb_cons = ((agent *)the_agent)->soar_callbacks[callback_type];

  if (cb_cons == NULL)
    {
      return FALSE;
    }

  return TRUE;
}

soar_callback * soar_exists_callback_id (soar_callback_agent the_agent,
					 SOAR_CALLBACK_TYPE callback_type,
					 soar_callback_id id)
{
  cons * c;

  for (c = ((agent *)the_agent)->soar_callbacks[callback_type];
       c != NIL;
       c = c->rest)
    {
      soar_callback * cb;

      cb = (soar_callback *) c->first;

      if (!strcmp(cb->id, id))
	{
	  return cb;
	}
    }

  return NULL;
}

void soar_destroy_callback(soar_callback * cb)
{
  if (cb->id)
  {
     free(cb->id);
  }
  if (cb->free_function)
  {
     cb->free_function(cb->data);
  }
  free((void *) cb);
}

// voigtjr: removed inline in an attempt to quell linker error
//inline void soar_invoke_callbacks (agent* thisAgent, 
void soar_invoke_callbacks (agent* thisAgent, 
				soar_callback_agent the_agent, 
			    SOAR_CALLBACK_TYPE callback_type, 
			    soar_call_data call_data)
{
  cons * c;  /* we need this if we loop over multiple callback functions */

/* if no callback is registered, just return */
 if (!((agent *)the_agent)->soar_callbacks[callback_type]) return;

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

  case BEFORE_DECISION_CYCLE_CALLBACK:
  case BEFORE_INPUT_PHASE_CALLBACK:
  case AFTER_INPUT_PHASE_CALLBACK:
    /* for above three: thisAgent->current_phase = INPUT_PHASE */
  case BEFORE_OUTPUT_PHASE_CALLBACK:
  case AFTER_OUTPUT_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = OUTPUT_PHASE */
  case BEFORE_PREFERENCE_PHASE_CALLBACK:
  case AFTER_PREFERENCE_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = PREFERENCE_PHASE soar7 only */
  case BEFORE_WM_PHASE_CALLBACK:
  case AFTER_WM_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = WM_PHASE soar7 only */
  case BEFORE_DECISION_PHASE_CALLBACK:
  case AFTER_DECISION_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = DECISION_PHASE */
  case BEFORE_PROPOSE_PHASE_CALLBACK:
  case AFTER_PROPOSE_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = PROPOSE_PHASE  soar8 only */
  case BEFORE_APPLY_PHASE_CALLBACK:
  case AFTER_APPLY_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = APPLY_PHASE soar8 only */
  case AFTER_DECISION_CYCLE_CALLBACK:
    /* for soar7: thisAgent->current_phase = DECISION_PHASE; for soar8 it's OUTPUT_PHASE */
	    stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	    stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
        start_timer (thisAgent, &thisAgent->start_phase_tv);
        break;
  case INPUT_PHASE_CALLBACK:
       /* Stop the kernel and phase timers for the input function. 
	    *   the output function is done in do_output_phase */
       stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                   &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
       stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
       start_timer (thisAgent, &thisAgent->start_kernel_tv);
       break;
 
  default: break;
  }
#endif

/* REW: end 28.07.96 */


  for (c = ((agent *)the_agent)->soar_callbacks[callback_type];
       c != NIL;
       c = c->rest)
    {
      soar_callback * cb;

      cb = (soar_callback *) c->first;
      cb->function(the_agent, cb->data, call_data);
    }

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
  switch (callback_type) {
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
  case BEFORE_PROPOSE_PHASE_CALLBACK:
  case AFTER_PROPOSE_PHASE_CALLBACK:
  case BEFORE_APPLY_PHASE_CALLBACK:
  case AFTER_APPLY_PHASE_CALLBACK:
  case AFTER_DECISION_CYCLE_CALLBACK:
    /* for soar7: thisAgent->current_phase = DECISION_PHASE; for soar8 it's OUTPUT_PHASE */
       stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
       break;
  case INPUT_PHASE_CALLBACK:
    /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
       stop_timer (thisAgent, &thisAgent->start_kernel_tv, 
                   &thisAgent->input_function_cpu_time);
       start_timer (thisAgent, &thisAgent->start_kernel_tv);
       start_timer (thisAgent, &thisAgent->start_phase_tv); 
       break;
 
  default: break;
  }
#endif

/* REW: end 28.07.96 */

}

void soar_invoke_first_callback (agent* thisAgent, 
				 soar_callback_agent the_agent, 
				 SOAR_CALLBACK_TYPE callback_type, 
				 soar_call_data call_data)
{
  list * head;

  /* if no callback is registered, just return */
  head = ((agent *)the_agent)->soar_callbacks[callback_type];
  if (head == NULL) return;

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
  switch (callback_type) {
  case BEFORE_DECISION_CYCLE_CALLBACK:
  case BEFORE_INPUT_PHASE_CALLBACK:
  case AFTER_INPUT_PHASE_CALLBACK:
    /* for these three: thisAgent->current_phase = INPUT_PHASE */
  case BEFORE_OUTPUT_PHASE_CALLBACK:
  case AFTER_OUTPUT_PHASE_CALLBACK:
    /* for these two: thisAgent->current_phase = OUTPUT_PHASE */
  case BEFORE_PREFERENCE_PHASE_CALLBACK:
  case AFTER_PREFERENCE_PHASE_CALLBACK:
    /* for these two: thisAgent->current_phase = PREFERENCE_PHASE */
  case BEFORE_WM_PHASE_CALLBACK:
  case AFTER_WM_PHASE_CALLBACK:
    /* for these two: thisAgent->current_phase = WM_PHASE */
  case BEFORE_DECISION_PHASE_CALLBACK:
  case AFTER_DECISION_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = DECISION_PHASE */
  case BEFORE_PROPOSE_PHASE_CALLBACK:
  case AFTER_PROPOSE_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = PROPOSE_PHASE  soar8 only */
  case BEFORE_APPLY_PHASE_CALLBACK:
  case AFTER_APPLY_PHASE_CALLBACK:
    /* for above two: thisAgent->current_phase = APPLY_PHASE soar8 only */
  case AFTER_DECISION_CYCLE_CALLBACK:
    /* for soar7: thisAgent->current_phase = DECISION_PHASE; for soar8 it's OUTPUT_PHASE */
	   stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                    &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
	   stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
       start_timer (thisAgent, &thisAgent->start_phase_tv);
       break;
  case INPUT_PHASE_CALLBACK:
       /* Stop the kernel and phase timers for the input function. 
	    *   the output function is done in do_output_phase */
       stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                   &thisAgent->decision_cycle_phase_timers[thisAgent->current_phase]);
       stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
       start_timer (thisAgent, &thisAgent->start_kernel_tv);
       break;
  default: break;
  }
#endif

/* REW: end 28.07.96 */
 
      soar_callback * cb;

      cb = (soar_callback *) head->first;
      cb->function(the_agent, cb->data, call_data);
    

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
  switch (callback_type) {
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
  case BEFORE_PROPOSE_PHASE_CALLBACK:
  case AFTER_PROPOSE_PHASE_CALLBACK:
  case BEFORE_APPLY_PHASE_CALLBACK:
  case AFTER_APPLY_PHASE_CALLBACK:
  case AFTER_DECISION_CYCLE_CALLBACK:
       stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                   &thisAgent->monitors_cpu_time[thisAgent->current_phase]);
       start_timer(thisAgent, &thisAgent->start_kernel_tv);
       start_timer(thisAgent, &thisAgent->start_phase_tv);
       break;
  case INPUT_PHASE_CALLBACK:
    /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
       stop_timer (thisAgent, &thisAgent->start_kernel_tv, 
                   &thisAgent->input_function_cpu_time);
       start_timer (thisAgent, &thisAgent->start_kernel_tv);
       start_timer (thisAgent, &thisAgent->start_phase_tv); 
       break;

  default: break;
  }
#endif

/* REW: end 28.07.96 */

}

void soar_list_all_callbacks (agent* thisAgent,
				  soar_callback_agent the_agent,
			      Bool monitorable_only)
{
  int limit;
  int ct; // ct was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)

  if (monitorable_only)
    {
      limit = NUMBER_OF_MONITORABLE_CALLBACKS;
    }
  else 
    {
      limit = NUMBER_OF_CALLBACKS;
    }

  for (ct = 1; ct < limit; ct++)
    {
      print(thisAgent, "%s: ", soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(ct), FALSE));
      soar_list_all_callbacks_for_event (thisAgent, the_agent, static_cast<SOAR_CALLBACK_TYPE>(ct));
      print(thisAgent, "\n");
    }
}

void soar_list_all_callbacks_for_event (agent* thisAgent, soar_callback_agent the_agent,
					SOAR_CALLBACK_TYPE ct)
{
  cons * c;

  for (c = ((agent *)the_agent)->soar_callbacks[ct]; 
       c != NIL; 
       c = c->rest)
    {
      soar_callback * cb;
      
      cb = (soar_callback *) c->first;

      print(thisAgent, "%s ", cb->id);
    }
}

void soar_pop_callback (agent* thisAgent, 
			soar_callback_agent the_agent, 
			SOAR_CALLBACK_TYPE callback_type)
{
  list * head;
  soar_callback * cb;

  head = ((agent *)the_agent)->soar_callbacks[callback_type];
  
  if (head == NULL)
    {
      print_string(thisAgent, "Attempt to remove non-existant callback.\n");
      return;
    }

  if (   (callback_type == PRINT_CALLBACK)
      && (head->rest == NULL))
    {
      print_string(thisAgent, "Attempt to remove last print callback. Ignored.\n");
      return;
    }

  cb = (soar_callback *) head->first;

  ((agent *)the_agent)->soar_callbacks[callback_type] = head->rest;
  soar_destroy_callback(cb);
  free_cons(thisAgent, head);
}

void soar_push_callback (agent* thisAgent, 
			soar_callback_agent the_agent, 
			SOAR_CALLBACK_TYPE callback_type, 
			soar_callback_fn fn, 
			soar_callback_data data,
			soar_callback_free_fn free_fn)
{
  soar_callback * cb;

  cb = (soar_callback *) malloc (sizeof(soar_callback));
  cb->function      = fn;
  cb->data          = data;
  cb->free_function = free_fn;
  cb->id            = NULL;
  
  push(thisAgent, cb, ((agent *) the_agent)->soar_callbacks[callback_type]);
}

void soar_remove_all_monitorable_callbacks (agent* thisAgent, soar_callback_agent the_agent)
{
  int ct; // ct was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)

  for (ct = 1; ct < NUMBER_OF_MONITORABLE_CALLBACKS; ct++)
    {
      soar_remove_all_callbacks_for_event (thisAgent, the_agent, static_cast<SOAR_CALLBACK_TYPE>(ct));
    }
}

void soar_remove_all_callbacks_for_event (agent* thisAgent, 
					  soar_callback_agent the_agent,
					  SOAR_CALLBACK_TYPE ct)
{
  cons * c;
  list * next;

  next = ((agent *)the_agent)->soar_callbacks[ct];

  for (c = next; c != NIL; c = next)
    {
      soar_callback * cb;
      
      cb = (soar_callback *) c->first;
	  
      next = next->rest;
      soar_destroy_callback(cb);
      free_cons(thisAgent, c);
    }

  ((agent *)the_agent)->soar_callbacks[ct] = NIL;
}

void soar_remove_callback (agent* thisAgent, 
			   soar_callback_agent the_agent, 
			   SOAR_CALLBACK_TYPE callback_type, 
			   soar_callback_id id)
{
  cons * c;
  cons * prev_c = NULL;     /* Initialized to placate gcc -Wall */
  list * head;

  head = ((agent *)the_agent)->soar_callbacks[callback_type];

  for (c = head; c != NIL; c = c->rest)
    {
      soar_callback * cb;

      cb = (soar_callback *) c->first;

      if (!strcmp(cb->id, id))
	{
	  if (c != head)
	    {
	      prev_c->rest = c->rest;
	      soar_destroy_callback(cb);
	      free_cons(thisAgent, c);
	      return;
	    }
	  else
	    {
	      ((agent *)the_agent)->soar_callbacks[callback_type] = head->rest;
	      soar_destroy_callback(cb);
	      free_cons(thisAgent, c);
	      return;
	    }
	}
      prev_c = c;
    }
}

void soar_callback_test_callback (soar_callback_agent the_agent,
				  soar_callback_data data,
				  soar_call_data call_data)
{
  printf("%s test callback executed.\n", (char *) data);
}


void soar_test_all_monitorable_callbacks(agent* thisAgent, soar_callback_agent the_agent)
{
  int i; // i was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)
  static char * test_callback_name = "test";

  for(i = 1; i < NUMBER_OF_MONITORABLE_CALLBACKS; i++)
    {
      soar_add_callback(thisAgent, the_agent, static_cast<SOAR_CALLBACK_TYPE>(i), 
			(soar_callback_fn) soar_callback_test_callback,
			soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(i), TRUE), 
			NULL, test_callback_name);
    }
}

