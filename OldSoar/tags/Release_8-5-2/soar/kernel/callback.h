/**
 * 
 *  \file callback.h
 *
 *                        Soar Callbacks
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
 *
 *
 * $Id$
 *
 *
 * Each agent has a separate callback table.  The table has one entry
 * per callback type and the entry is a pointer to a list.  The list
 * contains installed callbacks, one callback per list cons cell.
 *
 */

 /* Two points about this enumeration: a) The first entry is not */
 /* a valid callback because 0 is used to indicate an invalid or */
 /* missing callback and b) The last entry in the enum list      */
 /* indicates how many enums are defined.                        */

#ifndef CALLBACK_H_INCLUDED     /* Avoid duplicate includes  */
#define CALLBACK_H_INCLUDED     /* excludeFromBuildInfo */

 /* First we define the possible callbacks in an enum.  Then we  */
 /* describe how each one will be called in user code.           */

typedef enum {
    NO_CALLBACK,                /* Used for missing callback */
    SYSTEM_STARTUP_CALLBACK,
    SYSTEM_TERMINATION_CALLBACK,
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
    ASK_CALLBACK,
    WAIT_CALLBACK,
#ifdef ATTENTION_LAPSE
    INIT_LAPSE_DURATION_CALLBACK,
#endif
    /*  READ_CALLBACK,    *//* kjh CUSP B10 */
    /*  RECORD_CALLBACK,  *//* kjh CUSP B10 */
    NUMBER_OF_CALLBACKS         /* Not actually a callback   */
        /* type.  Used to indicate   */
        /* list size and MUST ALWAYS */
        /* BE LAST.                  */
} SOAR_CALLBACK_TYPE;

#define NUMBER_OF_MONITORABLE_CALLBACKS (NUMBER_OF_CALLBACKS - 2)

typedef enum {
    NO_GLOBAL_CALLBACK,
    GLB_CREATE_AGENT,
    GLB_AGENT_CREATED,
    GLB_DESTROY_AGENT,
    NUMBER_OF_GLOBAL_CALLBACKS
} SOAR_GLOBAL_CALLBACK_TYPE;

typedef list *soar_global_callback_array[NUMBER_OF_GLOBAL_CALLBACKS];

/*!

  \enum SOAR_CALLBACK_TYPE

                  Standard (Agent Specific) Callbacks

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

  \par SYSTEM_STARTUP_CALLBACK

    This function is called only once, at system startup time.  It can
    be used to set up any Soar I/O routines and install any interface 
    commands.  This function is called after most of the system has been
    initialized, but before any agent initialization file is loaded. 
    The "call_data" argument is given as NULL.
  
  \par SYSTEM_TERMINATION_CALLBACK

    This function is called only once, just before the system exits back
    to the shell.  The "call_data" parameter is of type "bool" which is 
    TRUE if the system is exiting normally, and FALSE if the exit is 
    happening because some fatal error situation was detected.  Typically,
    this routine should do any final cleanup (closing files, etc.) 
    necessary.

  \par BEFORE_INIT_SOAR_CALLBACK

    This function is called just before any init-soar is done.  (This 
    includes not only the init-soar command, but also excise-task and 
    excise-all, which do an init-soar.)  The "call_data" argument is 
    given as NULL.

  \par AFTER_INIT_SOAR_CALLBACK

    This function is called just after any init-soar is done.  (This 
    includes not only the init-soar command, but also excise-task and 
    excise-all, which do an init-soar.)  The "call_data" argument is 
    given as NULL.

  \par AFTER_HALT_SOAR_CALLBACK

    This function is called after Soar halts; i.e., after the preference
    phase in which the RHS function "halt" is executed.  The "call_data" 
    argument is given as NULL.

  \par BEFORE_SCHEDULE_CYCLE_CALLBACK

    This function is called just before the agent is scheduled.  The 
    "call_data" argument is given as NULL.

  \par AFTER_SCHEDULE_CYCLE_CALLBACK

    This function is called just after the agent is scheduled.  The 
    "call_data" argument is given as NULL.

  \par BEFORE_DECISION_CYCLE_CALLBACK

    This function is called at the start of each decision cycle.  The
    "call_data" argument is given as NULL.

  \par AFTER_DECISION_CYCLE_CALLBACK

    This function is called at the end of each decision cycle.  The
    "call_data" argument is given as NULL.

  \par BEFORE_INPUT_PHASE_CALLBACK

    This function is called at the start of each input phase.  This
    is called even if the input cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  \par INPUT_PHASE_CALLBACK

    This function is called during each input phase.  The "call_data"
    argument contains the input mode currently being used.

  \par AFTER_INPUT_PHASE_CALLBACK

    This function is called at the end of each input phase.  This
    is called even if the input cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  \par BEFORE_PREFERENCE_PHASE_CALLBACK

    This function is called at the start of each preference phase.
    The "call_data" argument is given as NULL.

  \par AFTER_PREFERENCE_PHASE_CALLBACK

    This function is called at the end of each preference phase.
    The "call_data" argument is given as NULL.

  \par BEFORE_WM_PHASE_CALLBACK

    This function is called at the start of each working memory phase.
    The "call_data" argument is given as NULL.

  \par AFTER_WM_PHASE_CALLBACK

    This function is called at the end of each working memory phase.
    The "call_data" argument is given as NULL.

  \par BEFORE_OUTPUT_PHASE_CALLBACK

    This function is called at the start of each Soar output cycle.
    This is called even if the output cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  \par OUTPUT_PHASE_CALLBACK

    This function is called during each Soar output cycle.  The "call_data"
    argument is a structure which contains both the output mode being
    used and the list of output wmes.

  \par AFTER_OUTPUT_PHASE_CALLBACK

    This function is called at the end of each Soar output cycle.
    This is called even if the output cycle is effectively null because 
    there is no top state.  The "call_data" argument is given as NULL.

  \par BEFORE_DECISION_PHASE_CALLBACK

    This function is called at the start of each decision phase.
    The "call_data" argument is given as NULL.

  \par AFTER_DECISION_PHASE_CALLBACK

    This function is called at the end of each decision phase.
    The "call_data" argument is given as NULL.

  \par WM_CHANGES_CALLBACK

    This function is called just before changes are made to working 
    memory.  The pre-callback hook function passed two arguments to
    the corresponding hook function.  However, the "call_data" 
    argument is given as NULL, in this callback.  The wmes_being_added
    and wmes_being_removed can be retrieved from the agent structure
    already being passed.

  \par CREATE_NEW_CONTEXT_CALLBACK

    This function is called after a new goal context is created.  The
    "call_data" argument is a pointer to a "Symbol" which is the new
    goal identifier.  This goal identifier is equal to the agent
    variable bottom_goal.

  \par POP_CONTEXT_STACK_CALLBACK

    This function is called just before the context stack is popped.  
    The "call_data" argument is a pointer to a "Symbol" which is the
    identifier of the goal about to be removed.  This goal identifier
    is equal to the aget variable bottom_goal.  If the stack is popped 
    k levels at once, this routine is called k times in bottom-up order.

  \par CREATE_NEW_ATTRIBUTE_IMPASSE_CALLBACK

    This function is called just after an attribute impasse is created.
    The "call_data" argument is a pointer to a "slot" which is the
    impassed slot.

  \par REMOVE_ATTRIBUTE_IMPASSE_CALLBACK

    This function is called just before an attribute impasse is removed.
    The "call_data" argument is a pointer to a "slot" which is the
    impassed slot.

  \par PRODUCTION_JUST_ADDED_CALLBACK

    This function is called just after a production (including chunks
    and justifications) is added to the system.  The "call_data" argument
    is a pointer to a "production" which is the production just added.

  \par PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK

    This function is called just before a production (including chunks
    and justifications) is excised from the system.  The "call_data" 
    argument is a pointer to a "production" which is the production 
    just about to be removed.

  \par FIRING_CALLBACK

    This function is called after every production firing.  The 
    "call_data" argument is a pointer to an "instantiation" which 
    is the newly created instantiation.

  \par RETRACTION_CALLBACK

    This function is called before every production retraction.  The 
    "call_data" argument is a pointer to an "instantiation" which 
    is the instantiation about to be retracted.

  \par SYSTEM_PARAMETER_CHANGED_CALLBACK

   This function is called after any change to one of the agent system
   parameters (e.g., learn on/off).  See soarkernel.h for a list of these
   system parameters.  The "call_data" argument is an "int" which 
   indicates which system parameter is being changed.  This function 
   should examine the new value of the parameter by looking at the 
   appropriate agent variable.  (For most parameters, this means 
   looking at the sysparams[] array.)

  \par INIT_LAPSE_DURATION_CALLBACK
   
   This function is called upon agent (re-initialization). 
   The "call_data" argument is a "long" which indicates the time
   since the last lapse.  It should be modified by the callback
   to indicate how long of an attention lapse should occur.

*/

/*!

 \enum SOAR_GLOBAL_CALLBACK_TYPE

                Global (Agent Independent) Callbacks
  
  All global callback functions installed by the user must use the
  function soar_add_global_callback.  The type signature of all 
  callback functions is the same as that used for agent-dependent
  callbacks and has the following form: 

    my_callback_fn(agent * a, 
                   soar_callback_data scd, 
		           soar_call_data call_data);

  however, in some situations, the argument "a" may be NULL, whereas 
  this will never be a case in a normal (agent-dependent) callback.
  All other parameter, mock those in a normal callback function.
  Below, arguments are specified for each global callback type:

  
  \par GLB_CREATE_AGENT
 
   This function is called just prior to the creation of a new agent.
   Because this is global becuase it is obviously impossible to register
   a normal callback with an agent which has not yet been created.
   The parameters passed to the callback function are:
		a          --> NULL
		call_data  --> (char *) agent_name

  \par GLB_AGENT_CREATED

   This function is called just after the creation of a new agent.
   The parameters passed to the callback function are:
		a			-->  a pointer to the new agent
		call_data   -->  the same as above, a pointer to the new agent
  
  
  \par GLB_DESTROY_AGENT

   This function is called just before an agent is destroyed.
   The parameters passed to the callback function are:
		a			-->  a pointer to the agent to be destroyed
		call_data   -->  the same as above, a pointer to the
		                    agent to be destroyed

*/

typedef list *soar_callback_array[NUMBER_OF_CALLBACKS];

/**
 * The agent upon which the callback is invoked.
 * Directly castable to either \b agent \b *  or \b psoar_agent
 */
typedef void *soar_callback_agent;

/**
 *  A callback id is also a string 
 */
typedef const char *soar_callback_id;

/**
 *  Data sent to the callback function each time it is invoked.
 *  This data is available at the time the user 
 *  registers the callback, and has nothing to do 
 *  with Soar's internals whatsoever.
 */
typedef void *soar_callback_data;

/**
 *  Data sent to the callback function.  This content
 *  of this data is determined solely by the type of
 *  callback and the internals of Soar 
 */
typedef const void *soar_call_data;

/**
 *  Any callback registered with Soar, must be in
 *  this format.
 */
typedef void (*soar_callback_fn) (soar_callback_agent, soar_callback_data, soar_call_data);

typedef void (*soar_callback_free_fn) (soar_callback_data);

typedef struct callback_struct {
    soar_callback_id id;
    soar_callback_fn function;
    soar_callback_data data;
    soar_callback_free_fn free_function;
} soar_callback;

extern void soar_callback_data_free_string(soar_callback_data);
extern char *soar_callback_enum_to_name(SOAR_CALLBACK_TYPE, bool);
extern void soar_destroy_callback(soar_callback *);
extern bool soar_exists_callback(soar_callback_agent, SOAR_CALLBACK_TYPE);
extern soar_callback *soar_exists_callback_id(soar_callback_agent the_agent,
                                              SOAR_CALLBACK_TYPE callback_type, soar_callback_id id);
extern void soar_init_callbacks(soar_callback_agent);
extern void soar_init_global_callbacks(void);

extern void soar_invoke_callbacks(soar_callback_agent, SOAR_CALLBACK_TYPE, soar_call_data);
extern void soar_invoke_global_callbacks(soar_callback_agent, SOAR_CALLBACK_TYPE, soar_call_data);
extern void soar_invoke_first_callback(soar_callback_agent, SOAR_CALLBACK_TYPE, soar_call_data);
extern bool soar_exists_global_callback(SOAR_GLOBAL_CALLBACK_TYPE);

#endif
