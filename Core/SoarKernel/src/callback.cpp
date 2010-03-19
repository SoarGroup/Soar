#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include <stdlib.h>

#include "callback.h"
#include "agent.h"
#include "init_soar.h"
#include "print.h"
#include "utilities.h"

const char * soar_callback_names[] = 
{
  stringify(NO_CALLBACK),
  stringify(AFTER_INIT_AGENT_CALLBACK),
  stringify(BEFORE_INIT_SOAR_CALLBACK),
  stringify(AFTER_INIT_SOAR_CALLBACK),
  stringify(AFTER_HALT_SOAR_CALLBACK),
  stringify(BEFORE_ELABORATION_CALLBACK),
  stringify(AFTER_ELABORATION_CALLBACK),
  stringify(BEFORE_DECISION_CYCLE_CALLBACK),
  stringify(AFTER_DECISION_CYCLE_CALLBACK),
  stringify(BEFORE_INPUT_PHASE_CALLBACK),
  stringify(INPUT_PHASE_CALLBACK),
  stringify(AFTER_INPUT_PHASE_CALLBACK),
  stringify(BEFORE_PREFERENCE_PHASE_CALLBACK),
  stringify(AFTER_PREFERENCE_PHASE_CALLBACK),
  stringify(BEFORE_WM_PHASE_CALLBACK),
  stringify(AFTER_WM_PHASE_CALLBACK),
  stringify(BEFORE_OUTPUT_PHASE_CALLBACK),
  stringify(OUTPUT_PHASE_CALLBACK),
  stringify(AFTER_OUTPUT_PHASE_CALLBACK),
  stringify(BEFORE_DECISION_PHASE_CALLBACK),
  stringify(AFTER_DECISION_PHASE_CALLBACK),
  stringify(BEFORE_PROPOSE_PHASE_CALLBACK),
  stringify(AFTER_PROPOSE_PHASE_CALLBACK),
  stringify(BEFORE_APPLY_PHASE_CALLBACK),
  stringify(AFTER_APPLY_PHASE_CALLBACK),
  stringify(WM_CHANGES_CALLBACK),
  stringify(CREATE_NEW_CONTEXT_CALLBACK),
  stringify(POP_CONTEXT_STACK_CALLBACK),
  stringify(CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK),
  stringify(REMOVE_ATTRIBUTE_IMPASSE_CALLBACK),
  stringify(PRODUCTION_JUST_ADDED_CALLBACK),
  stringify(PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK),
  stringify(AFTER_INTERRUPT_CALLBACK),
  stringify(AFTER_HALTED_CALLBACK),
  stringify(BEFORE_RUN_STARTS_CALLBACK),
  stringify(AFTER_RUN_ENDS_CALLBACK),
  stringify(BEFORE_RUNNING_CALLBACK),
  stringify(AFTER_RUNNING_CALLBACK),
  stringify(FIRING_CALLBACK),
  stringify(RETRACTION_CALLBACK),
  stringify(SYSTEM_PARAMETER_CHANGED_CALLBACK),
  stringify(MAX_MEMORY_USAGE_CALLBACK),
  stringify(XML_GENERATION_CALLBACK),
  stringify(PRINT_CALLBACK),
  stringify(LOG_CALLBACK),
  stringify(INPUT_WME_GARBAGE_COLLECTED_CALLBACK),
  //NUMBER_OF_CALLBACKS
};

std::map<SOAR_CALLBACK_TYPE, soar_timer_accumulator> callback_timers;

void soar_init_callbacks (agent* the_agent)
{
	int ct; // ct was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)

	for (ct = 1; ct < NUMBER_OF_CALLBACKS; ct++)
	{
		the_agent->soar_callbacks[ct] = NIL;
	}
}

int callback_count(agent* the_agent, SOAR_CALLBACK_TYPE callback_type)
{
	int count = 0;
	for (cons * c = the_agent->soar_callbacks[callback_type];
		c != NIL;
		c = c->rest)
	{
		++count;
	}

	return count;
}

#include <iostream>
void soar_add_callback (agent* thisAgent, 
						SOAR_CALLBACK_TYPE callback_type, 
						soar_callback_fn fn, 
						soar_callback_event_id eventid,
						soar_callback_data data,
						soar_callback_free_fn free_fn,
						soar_callback_id id)
{

	soar_callback * cb;

	cb = new soar_callback;
	cb->function      = fn;
	cb->data          = data;
	cb->eventid		= eventid ;
	cb->free_function = free_fn;
	cb->id            = id;

	push(thisAgent, cb, thisAgent->soar_callbacks[callback_type]);

	//std::cout << soar_callback_enum_to_name(callback_type, false)
	//	<< " (" << callback_count(thisAgent, callback_type) << ")";
	//if (callback_timers.find(callback_type) == callback_timers.end())
	//{
	//	callback_timers[callback_type] = soar_timer_accumulator();
	//	std::cout << " created timer";
	//}
	//std::cout << std::endl;
}

//void soar_print_detailed_callback_stats()
//{
//	for (std::map<SOAR_CALLBACK_TYPE, soar_timer_accumulator>::iterator iter = callback_timers.begin(); 
//		iter != callback_timers.end(); ++iter)
//	{
//		std::cout << soar_callback_enum_to_name(iter->first, false) << ": " 
//			<< iter->second.get_sec() << std::endl;
//	}
//}

void soar_callback_data_free_string (soar_callback_data data)
{
  free(data);
}

const char * soar_callback_enum_to_name (SOAR_CALLBACK_TYPE i, 
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

Bool soar_exists_callback(agent* the_agent,
			  SOAR_CALLBACK_TYPE callback_type)
{
  list * cb_cons;

  cb_cons = the_agent->soar_callbacks[callback_type];

  if (cb_cons == NULL)
    {
      return FALSE;
    }

  return TRUE;
}

soar_callback * soar_exists_callback_id (agent* the_agent,
										 SOAR_CALLBACK_TYPE callback_type,
										 soar_callback_id id)
{
	cons * c;

	for (c = the_agent->soar_callbacks[callback_type];
		c != NIL;
		c = c->rest)
	{
		soar_callback * cb;

		cb = static_cast< soar_callback* >(c->first);

		if (cb->id == id)
		{
			return cb;
		}
	}

	return NULL;
}

void soar_destroy_callback(soar_callback * cb)
{
  if (cb->free_function)
  {
     cb->free_function(cb->data);
  }
  delete cb;
}

// voigtjr: removed inline in an attempt to quell linker error
//inline void soar_invoke_callbacks (agent* thisAgent, 
void soar_invoke_callbacks (agent* thisAgent, 
			    SOAR_CALLBACK_TYPE callback_type, 
			    soar_call_data call_data)
{
  cons * c;  /* we need this if we loop over multiple callback functions */

/* if no callback is registered, just return */
 if (!thisAgent->soar_callbacks[callback_type]) return;

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
	    thisAgent->timers_phase.stop();
		thisAgent->timers_kernel.stop();
		thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
	    thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
		thisAgent->timers_decision_cycle.update(thisAgent->timers_phase);
		thisAgent->timers_phase.start();
        break;
  case INPUT_PHASE_CALLBACK:
       /* Stop the kernel and phase timers for the input function. 
	    *   the output function is done in do_output_phase */
	   thisAgent->timers_phase.stop();
       thisAgent->timers_kernel.stop();
       thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
	   thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
	   thisAgent->timers_decision_cycle.update(thisAgent->timers_phase);
	   thisAgent->timers_kernel.start();
       break;
 
  default: break;
  }
#endif

/* REW: end 28.07.96 */


  for (c = thisAgent->soar_callbacks[callback_type];
       c != NIL;
       c = c->rest)
    {
      soar_callback * cb;

      cb = static_cast< soar_callback* >(c->first);
	  cb->function(thisAgent, cb->eventid, cb->data, call_data);
    }

/* REW: begin 28.07.96 */

#ifndef NO_TIMING_STUFF
  switch (callback_type) {
  case BEFORE_ELABORATION_CALLBACK:
  case AFTER_ELABORATION_CALLBACK:
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
	   thisAgent->timers_phase.stop();
	   thisAgent->timers_monitors_cpu_time[thisAgent->current_phase].update(thisAgent->timers_phase);
	   callback_timers[callback_type].update(thisAgent->timers_phase);
	   thisAgent->timers_kernel.start();
	   thisAgent->timers_phase.start();
       break;
  case INPUT_PHASE_CALLBACK:
    /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
       thisAgent->timers_kernel.stop();
       thisAgent->timers_input_function_cpu_time.update(thisAgent->timers_kernel);
	   callback_timers[callback_type].update(thisAgent->timers_kernel);
	   thisAgent->timers_kernel.start();
	   thisAgent->timers_phase.start();
       break;
 
  default: break;
  }
#endif

/* REW: end 28.07.96 */

}

void soar_invoke_first_callback (agent* thisAgent, 
				 SOAR_CALLBACK_TYPE callback_type, 
				 soar_call_data call_data)
{
  list * head;

  /* if no callback is registered, just return */
  head = thisAgent->soar_callbacks[callback_type];
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
	   thisAgent->timers_phase.stop();
	   thisAgent->timers_kernel.stop();
	   thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
	   thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
	   thisAgent->timers_decision_cycle.update(thisAgent->timers_phase);
	   thisAgent->timers_phase.start();
       break;
  case INPUT_PHASE_CALLBACK:
       /* Stop the kernel and phase timers for the input function. 
	    *   the output function is done in do_output_phase */
	   thisAgent->timers_phase.stop();
	   thisAgent->timers_kernel.stop();
	   thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
	   thisAgent->timers_decision_cycle_phase[thisAgent->current_phase].update(thisAgent->timers_phase);
	   thisAgent->timers_decision_cycle.update(thisAgent->timers_phase);
	   thisAgent->timers_kernel.start();
       break;
  default: break;
  }
#endif

/* REW: end 28.07.96 */
 
      soar_callback * cb;

      cb = static_cast< soar_callback* >(head->first);
	  cb->function(thisAgent, cb->eventid, cb->data, call_data);
    

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
	   thisAgent->timers_phase.stop();
	   thisAgent->timers_monitors_cpu_time[thisAgent->current_phase].update(thisAgent->timers_phase);
	   callback_timers[callback_type].update(thisAgent->timers_phase);
	   thisAgent->timers_kernel.start();
	   thisAgent->timers_phase.start();
       break;
  case INPUT_PHASE_CALLBACK:
    /* Stop input_function_cpu_time timer.  Restart kernel and phase timers */
	   thisAgent->timers_kernel.stop();
	   thisAgent->timers_input_function_cpu_time.update(thisAgent->timers_kernel);
	   callback_timers[callback_type].update(thisAgent->timers_kernel);
	   thisAgent->timers_kernel.start();
	   thisAgent->timers_phase.start();
       break;

  default: break;
  }
#endif

/* REW: end 28.07.96 */

}

void soar_list_all_callbacks (agent* thisAgent,
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
      soar_list_all_callbacks_for_event (thisAgent, static_cast<SOAR_CALLBACK_TYPE>(ct));
      print(thisAgent, "\n");
    }
}

void soar_list_all_callbacks_for_event (agent* thisAgent, 
					SOAR_CALLBACK_TYPE ct)
{
  cons * c;

  for (c = thisAgent->soar_callbacks[ct]; 
       c != NIL; 
       c = c->rest)
    {
      soar_callback * cb;
      
      cb = static_cast< soar_callback* >(c->first);

	  print(thisAgent, "%s ", cb->id.c_str());
    }
}

void soar_pop_callback (agent* thisAgent, 
		SOAR_CALLBACK_TYPE callback_type)
{
  list * head;
  soar_callback * cb;

  head = thisAgent->soar_callbacks[callback_type];
  
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

  cb = static_cast< soar_callback* >(head->first);

  thisAgent->soar_callbacks[callback_type] = head->rest;
  soar_destroy_callback(cb);
  free_cons(thisAgent, head);
}

void soar_push_callback (agent* thisAgent, 
			SOAR_CALLBACK_TYPE callback_type, 
			soar_callback_fn fn,
			soar_callback_event_id eventid,
			soar_callback_data data,
			soar_callback_free_fn free_fn)
{
  soar_callback * cb;

  cb = new soar_callback;
  cb->function      = fn;
  cb->data          = data;
  cb->eventid		= eventid ;
  cb->free_function = free_fn;
  //cb->id initialized to empty string
  
  push(thisAgent, cb, thisAgent->soar_callbacks[callback_type]);
}

void soar_remove_all_monitorable_callbacks (agent* thisAgent)
{
  int ct; // ct was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)

  for (ct = 1; ct < NUMBER_OF_MONITORABLE_CALLBACKS; ct++)
    {
      soar_remove_all_callbacks_for_event (thisAgent, static_cast<SOAR_CALLBACK_TYPE>(ct));
    }
}

void soar_remove_all_callbacks_for_event (agent* thisAgent, 
					  SOAR_CALLBACK_TYPE ct)
{
  cons * c;
  list * next;

  next = thisAgent->soar_callbacks[ct];

  for (c = next; c != NIL; c = next)
    {
      soar_callback * cb;
      
      cb = static_cast< soar_callback* >(c->first);
	  
      next = next->rest;
      soar_destroy_callback(cb);
      free_cons(thisAgent, c);
    }

  thisAgent->soar_callbacks[ct] = NIL;
}

void soar_remove_callback (agent* thisAgent, 
			   SOAR_CALLBACK_TYPE callback_type, 
			   soar_callback_id id)
{
  cons * c;
  cons * prev_c = NULL;     /* Initialized to placate gcc -Wall */
  list * head;

  head = thisAgent->soar_callbacks[callback_type];

  for (c = head; c != NIL; c = c->rest)
    {
      soar_callback * cb;

      cb = static_cast< soar_callback* >(c->first);

      if (cb->id == id)
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
	      thisAgent->soar_callbacks[callback_type] = head->rest;
	      soar_destroy_callback(cb);
	      free_cons(thisAgent, c);
	      return;
	    }
	}
      prev_c = c;
    }
}

void soar_callback_test_callback (agent* /*the_agent*/,
				  soar_callback_data data,
				  soar_call_data /*call_data*/)
{
  printf("%s test callback executed.\n", static_cast<char *>(data));
}


void soar_test_all_monitorable_callbacks(agent* thisAgent)
{
  int i; // i was originally of type SOAR_CALLBACK_TYPE, changed for c++ compatibility (5/1/02)
  static const char * test_callback_name = "test";

  for(i = 1; i < NUMBER_OF_MONITORABLE_CALLBACKS; i++)
    {
      soar_add_callback(thisAgent, static_cast<SOAR_CALLBACK_TYPE>(i), 
			reinterpret_cast<soar_callback_fn>(soar_callback_test_callback), i,
			static_cast<void*>(const_cast<char*>(soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(i), TRUE))), 
			NULL, test_callback_name);
    }
}

