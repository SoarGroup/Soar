/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*
 *=======================================================================
 *
 * File: callback.h
 *
 * Description: This file contains the callback facility header decls.
 * 
 * Exported data types:
 *   SOAR_CALLBACK_TYPE
 *   soar_callback_data
 *   soar_call_data
 *   soar_callback_fn
 *   
 * Exported functions:
 *   soar_add_callback
 *   soar_invoke_callbacks
 *   soar_remove_callback
 *
 * Each agent has a separate callback table.  The table has one entry
 * per callback type and the entry is a pointer to a list.  The list
 * contains installed callbacks, one callback per list cons cell.
 *
 * =======================================================================
 */


 /* Two points about this enumeration: a) The first entry is not */
 /* a valid callback because 0 is used to indicate an invalid or */
 /* missing callback and b) The last entry in the enum list      */
 /* indicates how many enums are defined.                        */

#ifndef CALLBACK_H_INCLUDED         /* Avoid duplicate includes  */
#define CALLBACK_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

 /* First we define the possible callbacks in an enum.  Then we  */
 /* describe how each one will be called in user code.           */

typedef enum {
  NO_CALLBACK,                      /* Used for missing callback */
  SYSTEM_STARTUP_CALLBACK,
  SYSTEM_TERMINATION_CALLBACK,
  AFTER_INIT_AGENT_CALLBACK,
  BEFORE_INIT_SOAR_CALLBACK,
  AFTER_INIT_SOAR_CALLBACK,
  AFTER_HALT_SOAR_CALLBACK,
  BEFORE_SCHEDULE_CYCLE_CALLBACK,
  AFTER_SCHEDULE_CYCLE_CALLBACK,
  BEFORE_DECISION_CYCLE_CALLBACK,
  AFTER_DECISION_CYCLE_CALLBACK,
  BEFORE_INPUT_PHASE_CALLBACK,
  INPUT_PHASE_CALLBACK,
  AFTER_INPUT_PHASE_CALLBACK,
  BEFORE_PREFERENCE_PHASE_CALLBACK,
  AFTER_PREFERENCE_PHASE_CALLBACK,
  BEFORE_WM_PHASE_CALLBACK,
  AFTER_WM_PHASE_CALLBACK,
  BEFORE_OUTPUT_PHASE_CALLBACK,
  OUTPUT_PHASE_CALLBACK,
  AFTER_OUTPUT_PHASE_CALLBACK,
  BEFORE_DECISION_PHASE_CALLBACK,
  AFTER_DECISION_PHASE_CALLBACK,
  BEFORE_PROPOSE_PHASE_CALLBACK,
  AFTER_PROPOSE_PHASE_CALLBACK,
  BEFORE_APPLY_PHASE_CALLBACK,
  AFTER_APPLY_PHASE_CALLBACK,
  WM_CHANGES_CALLBACK,
  CREATE_NEW_CONTEXT_CALLBACK,
  POP_CONTEXT_STACK_CALLBACK,
  CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK,
  REMOVE_ATTRIBUTE_IMPASSE_CALLBACK,
  PRODUCTION_JUST_ADDED_CALLBACK,
  PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK,
  FIRING_CALLBACK,
  RETRACTION_CALLBACK,
  SYSTEM_PARAMETER_CHANGED_CALLBACK,
  PRINT_CALLBACK,
  LOG_CALLBACK,
/*  READ_CALLBACK,					kjh CUSP B10 */
/*  RECORD_CALLBACK,					kjh CUSP B10 */
  NUMBER_OF_CALLBACKS               /* Not actually a callback   */
                                    /* type.  Used to indicate   */
                                    /* list size and MUST ALWAYS */
                                    /* BE LAST.                  */
} SOAR_CALLBACK_TYPE;

#define NUMBER_OF_MONITORABLE_CALLBACKS (NUMBER_OF_CALLBACKS - 2)

/* --------------------------------------------------------------------

  All callback functions installed by the user must use the function
  soar_add_callback.  The type signature of all callback functions is
  the same and has the following form: 

    my_callback_fn(agent * a, 
                   soar_callback_data scd, 
		   soar_call_data call_data);

  where "a" is a pointer to the agent structure for the agent in which
  the callback was invoked, "scd" is the data structure given in the 
  installation when soar_add_callback was called, and "call_data" is 
  the data relevant to this particular call.  To use the "call_data"
  parameter, it should first be cast to the appropriate type, which
  is mentioned below.  Since only the "call_data" argument varies for
  all callbacks, only that argument will be mentioned in the following.

  SYSTEM_STARTUP_CALLBACK

    This function is called only once, at system startup time.  It can
    be used to set up any Soar I/O routines and install any interface 
    commands.  This function is called after most of the system has been
    initialized, but before any agent initialization file is loaded. 
    The "call_data" argument is given as NULL.
  
  SYSTEM_TERMINATION_CALLBACK

    This function is called only once, just before the system exits back
    to the shell.  The "call_data" parameter is of type "Bool" which is 
    TRUE if the system is exiting normally, and FALSE if the exit is 
    happening because some fatal error situation was detected.  Typically,
    this routine should do any final cleanup (closing files, etc.) 
    necessary.

  AFTER_INIT_AGENT_CALLBACK

    This function is called only once per each agent after the agent
    has been initialized.  This function is called after most of the 
    agent data structures have been initialized, but before the agent
    initialization file is loaded.  The "call_data" argument is given 
    as NULL.

  BEFORE_INIT_SOAR_CALLBACK

    This function is called just before any init-soar is done.  (This 
    includes not only the init-soar command, but also excise-task and 
    excise-all, which do an init-soar.)  The "call_data" argument is 
    given as NULL.

  AFTER_INIT_SOAR_CALLBACK

    This function is called just after any init-soar is done.  (This 
    includes not only the init-soar command, but also excise-task and 
    excise-all, which do an init-soar.)  The "call_data" argument is 
    given as NULL.

  AFTER_HALT_SOAR_CALLBACK

    This function is called after Soar halts; i.e., after the preference
    phase in which the RHS function "halt" is executed.  The "call_data" 
    argument is given as NULL.

  BEFORE_SCHEDULE_CYCLE_CALLBACK

    This function is called just before the agent is scheduled.  The 
    "call_data" argument is given as NULL.

  AFTER_SCHEDULE_CYCLE_CALLBACK

    This function is called just after the agent is scheduled.  The 
    "call_data" argument is given as NULL.

  BEFORE_DECISION_CYCLE_CALLBACK

    This function is called at the start of each decision cycle.  The
    "call_data" argument is given as NULL.

  AFTER_DECISION_CYCLE_CALLBACK

    This function is called at the end of each decision cycle.  The
    "call_data" argument is given as NULL.

  BEFORE_INPUT_PHASE_CALLBACK

    This function is called at the start of each input phase.  This
    is called even if the input cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  INPUT_PHASE_CALLBACK

    This function is called during each input phase.  The "call_data"
    argument contains the input mode currently being used.

  AFTER_INPUT_PHASE_CALLBACK

    This function is called at the end of each input phase.  This
    is called even if the input cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  BEFORE_PREFERENCE_PHASE_CALLBACK

    This function is called at the start of each preference phase.
    The "call_data" argument is given as NULL.

  AFTER_PREFERENCE_PHASE_CALLBACK

    This function is called at the end of each preference phase.
    The "call_data" argument is given as NULL.

  BEFORE_WM_PHASE_CALLBACK

    This function is called at the start of each working memory phase.
    The "call_data" argument is given as NULL.

  AFTER_WM_PHASE_CALLBACK

    This function is called at the end of each working memory phase.
    The "call_data" argument is given as NULL.

  BEFORE_OUTPUT_PHASE_CALLBACK

    This function is called at the start of each Soar output cycle.
    This is called even if the output cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  OUTPUT_PHASE_CALLBACK

    This function is called during each Soar output cycle.  The "call_data"
    argument is a structure which contains both the output mode being
    used and the list of output wmes.

  AFTER_OUTPUT_PHASE_CALLBACK

    This function is called at the end of each Soar output cycle.
    This is called even if the output cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  BEFORE_DECISION_PHASE_CALLBACK

    This function is called at the start of each decision phase.
    The "call_data" argument is given as NULL.

  AFTER_DECISION_PHASE_CALLBACK

    This function is called at the end of each decision phase.
    The "call_data" argument is given as NULL.

  WM_CHANGES_CALLBACK

    This function is called just before changes are made to working 
    memory.  The pre-callback hook function passed two arguments to
    the corresponding hook function.  However, the "call_data" 
    argument is given as NULL, in this callback.  The wmes_being_added
    and wmes_being_removed can be retrieved from the agent structure
    already being passed.

  CREATE_NEW_CONTEXT_CALLBACK

    This function is called after a new goal context is created.  The
    "call_data" argument is a pointer to a "Symbol" which is the new
    goal identifier.  This goal identifier is equal to the agent
    variable bottom_goal.

  POP_CONTEXT_STACK_CALLBACK

    This function is called just before the context stack is popped.  
    The "call_data" argument is a pointer to a "Symbol" which is the
    identifier of the goal about to be removed.  This goal identifier
    is equal to the aget variable bottom_goal.  If the stack is popped 
    k levels at once, this routine is called k times in bottom-up order.

  CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK

    This function is called just after an attribute impasse is created.
    The "call_data" argument is a pointer to a "slot" which is the
    impassed slot.

  REMOVE_ATTRIBUTE_IMPASSE_CALLBACK

    This function is called just before an attribute impasse is removed.
    The "call_data" argument is a pointer to a "slot" which is the
    impassed slot.

  PRODUCTION_JUST_ADDED_CALLBACK

    This function is called just after a production (including chunks
    and justifications) is added to the system.  The "call_data" argument
    is a pointer to a "production" which is the production just added.

  PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK

    This function is called just before a production (including chunks
    and justifications) is excised from the system.  The "call_data" 
    argument is a pointer to a "production" which is the production 
    just about to be removed.

  FIRING_CALLBACK

    This function is called after every production firing.  The 
    "call_data" argument is a pointer to an "instantiation" which 
    is the newly created instantiation.

  RETRACTION_CALLBACK

    This function is called before every production retraction.  The 
    "call_data" argument is a pointer to an "instantiation" which 
    is the instantiation about to be retracted.

  SYSTEM_PARAMETER_CHANGED_CALLBACK

   This function is called after any change to one of the agent system
   parameters (e.g., learn on/off).  See soarkernel.h for a list of these
   system parameters.  The "call_data" argument is an "int" which 
   indicates which system parameter is being changed.  This function 
   should examine the new value of the parameter by looking at the 
   appropriate agent variable.  (For most parameters, this means 
   looking at the sysparams[] array.)

-------------------------------------------------------------------- */

#ifndef CALLBACK_H
#define CALLBACK_H

//typedef list * soar_callback_array[NUMBER_OF_CALLBACKS];
typedef char Bool;
typedef char * soar_callback_id;
typedef void * soar_callback_agent;
typedef void * soar_callback_data;
typedef void * soar_call_data;
typedef void (*soar_callback_fn)(soar_callback_agent, 
				 soar_callback_data, 
				 soar_call_data);
typedef void (*soar_callback_free_fn)(soar_callback_data);
typedef struct cons_struct cons;
typedef cons list;
typedef struct agent_struct agent;

typedef struct callback_struct 
{
  soar_callback_id      id;
  soar_callback_fn      function;
  soar_callback_data    data;
  soar_callback_free_fn free_function;
} soar_callback;

extern void soar_add_callback (agent* thisAgent, 
			       soar_callback_agent, 
			       SOAR_CALLBACK_TYPE, 
			       soar_callback_fn, 
			       soar_callback_data,
			       soar_callback_free_fn, 
			       soar_callback_id);
extern void soar_callback_data_free_string (soar_callback_data);
extern char * soar_callback_enum_to_name (SOAR_CALLBACK_TYPE, Bool);
extern SOAR_CALLBACK_TYPE soar_callback_name_to_enum (char *, Bool);
extern void soar_destroy_callback(soar_callback *);
extern Bool soar_exists_callback (soar_callback_agent, SOAR_CALLBACK_TYPE);
extern soar_callback * soar_exists_callback_id (soar_callback_agent the_agent,
  					      SOAR_CALLBACK_TYPE callback_type,
						soar_callback_id id);
extern void soar_init_callbacks (soar_callback_agent);
extern void soar_invoke_callbacks (agent* thisAgent, 
				   soar_callback_agent, 
				   SOAR_CALLBACK_TYPE, 
				   soar_call_data);
extern void soar_invoke_first_callback (agent* thisAgent, 
					soar_callback_agent, 
					SOAR_CALLBACK_TYPE, 
					soar_call_data);
extern void soar_list_all_callbacks (soar_callback_agent, 
				     Bool);
extern void soar_list_all_callbacks_for_event (agent* thisAgent, soar_callback_agent, 
					       SOAR_CALLBACK_TYPE);
extern void soar_pop_callback (soar_callback_agent the_agent, 
			       SOAR_CALLBACK_TYPE callback_type);
extern void soar_push_callback (soar_callback_agent the_agent, 
				SOAR_CALLBACK_TYPE callback_type, 
				soar_callback_fn fn, 
				soar_callback_data data,
				soar_callback_free_fn free_fn);
extern void soar_remove_all_monitorable_callbacks (agent* thisAgent, 
						   soar_callback_agent);
extern void soar_remove_all_callbacks_for_event (agent* thisAgent, 
						 soar_callback_agent, 
						 SOAR_CALLBACK_TYPE);
extern void soar_remove_callback (agent* thisAgent, 
				  soar_callback_agent,
				  SOAR_CALLBACK_TYPE, 
				  soar_callback_id);
extern void soar_test_all_monitorable_callbacks(soar_callback_agent);
#endif

#ifdef __cplusplus
}
#endif

#endif
