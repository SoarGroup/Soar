/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_Enumerations.h 
*********************************************************************
* created:	   6/19/2002   15:00
*
* purpose: 
*********************************************************************/

#ifndef GSKI_ENUMERATIONS_H
#define GSKI_ENUMERATIONS_H


   /** Enumeration of all event ids in the system */

      /** These are their types:
	  egSKISystemEventId
	  egSKIRunEventId
	  egSKIProductionEventId 
	  egSKIAgentEventId
	  egSKIWorkingMemoryEventId 
	  egSKIPrintEventId
	  egSKIRhsEventId
	  egSKIGenericEventId
      */

   typedef enum {
      // Kernel events
      gSKIEVENT_BEFORE_SHUTDOWN            = 1,
	  gSKIEVENT_AFTER_CONNECTION,
      gSKIEVENT_AFTER_CONNECTION_LOST,
      gSKIEVENT_BEFORE_RESTART,
      gSKIEVENT_AFTER_RESTART,
	  gSKIEVENT_SYSTEM_START,
	  gSKIEVENT_SYSTEM_STOP,
	  gSKIEVENT_INTERRUPT_CHECK,
	  gSKIEVENT_SYSTEM_PROPERTY_CHANGED,
	  gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED,
      gSKIEVENT_AFTER_RHS_FUNCTION_ADDED,
      gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED,
      gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED,
      gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
      gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED,
      gSKIEVENT_LAST_SYSTEM_EVENT  = gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED,
    } egSKISystemEventId;

    static inline bool IsSystemEventID (int id)
    {
 	   return (id >= gSKIEVENT_BEFORE_SHUTDOWN && 
 		id <= gSKIEVENT_LAST_SYSTEM_EVENT) ;
    }
 
    typedef enum {
      // Agent (Run)Events
      gSKIEVENT_BEFORE_SMALLEST_STEP = gSKIEVENT_LAST_SYSTEM_EVENT + 1,
      gSKIEVENT_AFTER_SMALLEST_STEP,
      gSKIEVENT_BEFORE_ELABORATION_CYCLE,
      gSKIEVENT_AFTER_ELABORATION_CYCLE,
	  /*  tests for phase events depends on this ordering */
      gSKIEVENT_BEFORE_PHASE_EXECUTED,
      gSKIEVENT_BEFORE_INPUT_PHASE,
      gSKIEVENT_BEFORE_PROPOSE_PHASE,
      gSKIEVENT_BEFORE_DECISION_PHASE,
      gSKIEVENT_BEFORE_APPLY_PHASE,
      gSKIEVENT_BEFORE_OUTPUT_PHASE,
      gSKIEVENT_BEFORE_PREFERENCE_PHASE,	// Soar-7 mode only
      gSKIEVENT_BEFORE_WM_PHASE,			// Soar-7 mode only
      gSKIEVENT_AFTER_INPUT_PHASE,
      gSKIEVENT_AFTER_PROPOSE_PHASE,
      gSKIEVENT_AFTER_DECISION_PHASE,
      gSKIEVENT_AFTER_APPLY_PHASE,
      gSKIEVENT_AFTER_OUTPUT_PHASE,
      gSKIEVENT_AFTER_PREFERENCE_PHASE,		// Soar-7 mode only
      gSKIEVENT_AFTER_WM_PHASE,				// Soar-7 mode only
	  gSKIEVENT_AFTER_PHASE_EXECUTED,
	  /* 	  */
      gSKIEVENT_BEFORE_DECISION_CYCLE,
      gSKIEVENT_AFTER_DECISION_CYCLE,
      gSKIEVENT_AFTER_INTERRUPT,
	  gSKIEVENT_BEFORE_RUN_STARTS,		// Before start a run
	  gSKIEVENT_AFTER_RUN_ENDS,			// After run ends for any reason
      gSKIEVENT_BEFORE_RUNNING,			// Before running one increment
      gSKIEVENT_AFTER_RUNNING,			// After running one increment
      gSKIEVENT_LAST_RUN_EVENT = gSKIEVENT_AFTER_RUNNING,
    } egSKIRunEventId;

    static inline bool IsRunEventID (int id)
    {
 	   return (id >= gSKIEVENT_BEFORE_SMALLEST_STEP && 
 		id <= gSKIEVENT_LAST_RUN_EVENT) ;
    }
     static inline bool IsPhaseEventID (int id)
    {
 	   return (id > gSKIEVENT_BEFORE_PHASE_EXECUTED && 
 		id < gSKIEVENT_AFTER_PHASE_EXECUTED) ;
    }
      static inline bool IsBEFOREPhaseEventID (int id)
    {
 	   return (id > gSKIEVENT_BEFORE_PHASE_EXECUTED && 
 		id <= gSKIEVENT_BEFORE_WM_PHASE) ;
    }
     static inline bool IsAFTERPhaseEventID (int id)
    {
 	   return (id >= gSKIEVENT_AFTER_INPUT_PHASE && 
 		id < gSKIEVENT_AFTER_PHASE_EXECUTED) ;
    }


    typedef enum {
      // Production Manager
      gSKIEVENT_AFTER_PRODUCTION_ADDED  = gSKIEVENT_LAST_RUN_EVENT + 1,
      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
      //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
      gSKIEVENT_AFTER_PRODUCTION_FIRED,
      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
      gSKIEVENT_LAST_PRODUCTION_EVENT = gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
    } egSKIProductionEventId;
    
    static inline bool IsProductionEventID (int id)
    {
 	   return (id >= gSKIEVENT_AFTER_PRODUCTION_ADDED && 
 		id <= gSKIEVENT_LAST_PRODUCTION_EVENT ) ;
    }

    typedef enum {
      // Agent manager
      gSKIEVENT_AFTER_AGENT_CREATED = gSKIEVENT_LAST_PRODUCTION_EVENT + 1,
      gSKIEVENT_BEFORE_AGENT_DESTROYED,
	  gSKIEVENT_BEFORE_AGENTS_RUN_STEP,
      gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
      gSKIEVENT_AFTER_AGENT_REINITIALIZED,
      gSKIEVENT_LAST_AGENT_EVENT = gSKIEVENT_AFTER_AGENT_REINITIALIZED,
    } egSKIAgentEventId;
     
    static inline bool IsAgentEventID (int id)
    {
 	   return (id >= gSKIEVENT_AFTER_AGENT_CREATED && 
 		id <= gSKIEVENT_LAST_AGENT_EVENT) ;
    }

    typedef enum {
      // Working memory changes
      gSKIEVENT_OUTPUT_PHASE_CALLBACK = gSKIEVENT_LAST_AGENT_EVENT + 1,
      gSKIEVENT_LAST_WM_EVENT = gSKIEVENT_OUTPUT_PHASE_CALLBACK,
    } egSKIWorkingMemoryEventId;

    static inline bool IsWorkingMemoryEventID (int id)
    {
 	   return (id >= gSKIEVENT_OUTPUT_PHASE_CALLBACK && 
 		id <= gSKIEVENT_LAST_WM_EVENT) ;
    }
 

    typedef enum {
      // Error and print callbacks
      gSKIEVENT_LOG_ERROR =  gSKIEVENT_LAST_WM_EVENT + 1,
	  gSKIEVENT_FIRST_PRINT_EVENT = gSKIEVENT_LOG_ERROR,
      gSKIEVENT_LOG_WARNING,
      gSKIEVENT_LOG_INFO,
      gSKIEVENT_LOG_DEBUG,
	  gSKIEVENT_ECHO,
      gSKIEVENT_PRINT,
      gSKIEVENT_LAST_PRINT_EVENT = gSKIEVENT_PRINT,
    } egSKIPrintEventId;

    static inline bool IsPrintEventID (int id)
    {
 	   return (id >= gSKIEVENT_LOG_ERROR && 
 		id <= gSKIEVENT_LAST_PRINT_EVENT) ;
    }
 
    typedef enum { 
 	 // Used to provide user handler functions for RHS (right hand side) functions
   	 // fired within Soar productions.  This is different from normal events in that
 	 // the handler is executing the function and returning a value, not just being notified
 	 // that something has happened.
 	 gSKIEVENT_RHS_USER_FUNCTION = gSKIEVENT_LAST_PRINT_EVENT + 1,
	 gSKIEVENT_CLIENT_MESSAGE,
	 gSKIEVENT_LAST_RHS_EVENT = gSKIEVENT_CLIENT_MESSAGE,
    } egSKIRhsEventId;
 
    static inline bool IsRhsEventID (int id)
    {
 	   return (id >= gSKIEVENT_RHS_USER_FUNCTION && 
 		id <= gSKIEVENT_LAST_RHS_EVENT) ;
    }

	 typedef enum { 
 	 // This is directly implemented in gSKI, but is not really the same event as the SML side.
	 // On the gSKI side this is used to signal the generation of XML trace events.
	 // On the client side it is used send complete XML objects.
	 // That is, a bunch of gSKI XML trace events get combined into a single XML object and sent
	 //  as a single SML event.
	 // We decided to use the same event number for both sides since the event numbers need to match up
	 //  anyway and these are strongly related events.
 	 gSKIEVENT_XML_TRACE_OUTPUT = gSKIEVENT_LAST_RHS_EVENT + 1,

	 // Used to echo input wmes back to a client (so one client can listen in on additions made by another).
	 gSKIEVENT_XML_INPUT_RECEIVED,

	 gSKIEVENT_LAST_XML_EVENT = gSKIEVENT_XML_INPUT_RECEIVED,
    } egSKIXMLEventId;

	static inline bool IsXMLEventID (int id)
    {
 	   return (id >= gSKIEVENT_XML_TRACE_OUTPUT && 
 		id <= gSKIEVENT_LAST_XML_EVENT) ;
    }

	// Events that can be used by environments to trigger when the world should update
	// Currently not implemented by gSKI, but included for completeness.
	typedef enum {
		gSKIEVENT_AFTER_ALL_OUTPUT_PHASES = gSKIEVENT_LAST_XML_EVENT + 1,	// All agents have completed output phase
		gSKIEVENT_AFTER_ALL_GENERATED_OUTPUT,								// All agents have generated output (since run began)
	    gSKIEVENT_LAST_UPDATE_EVENT = gSKIEVENT_AFTER_ALL_GENERATED_OUTPUT,
	} egSKIUpdateEventId ;

	static inline bool IsUpdateEventID(int id)
	{
		return (id >= gSKIEVENT_AFTER_ALL_OUTPUT_PHASES && id <= gSKIEVENT_LAST_UPDATE_EVENT) ;
	}

	// Events that pass a string as parameter
	typedef enum {
		gSKIEVENT_EDIT_PRODUCTION = gSKIEVENT_AFTER_ALL_GENERATED_OUTPUT + 1,	// Arg is "char const*".
	} egSKIStringEventId ;

	static inline bool IsStringEventID(int id)
	{
		return (id >= gSKIEVENT_EDIT_PRODUCTION && id <= gSKIEVENT_EDIT_PRODUCTION) ;
	}

    typedef enum {
       // Used to indicate an error in some cases
       gSKIEVENT_INVALID_EVENT              = 0,
       // Marker for end of gSKI event list
       // Must always be at the end of the enum
       gSKIEVENT_LAST = gSKIEVENT_EDIT_PRODUCTION + 1
    } egSKIGenericEventId;

   /** End of Event Id enumerations.  **/

   /** Types of threading models 
		@li SINGLE_THREADED  Does not synchronize multi-threaded access.
		@li MULTI_THREADED   Synchronizes multi-threaded access (multiple threads can access at same time)
   */
   typedef enum  { 
      gSKI_SINGLE_THREAD, 
      gSKI_MULTI_THREAD 
   } egSKIThreadingModel;

   /** Types of working memory changes
	   @li ADDED_OUTPUT_COMMAND		A new wme is added to the top level of the output link
	   @li MODIFIED_OUTPUT_COMMAND	A wme within the transitive closure of the output link is added or removed
	   @li REMOVED_OUTPUT_COMMAND	A wme is removed from the top level of the output link
   */
   typedef enum {
	  gSKI_ADDED_OUTPUT_COMMAND,
	  gSKI_MODIFIED_OUTPUT_COMMAND,
	  gSKI_REMOVED_OUTPUT_COMMAND
   } egSKIWorkingMemoryChange ;

   /**
      Types of processes in which the kernel can be housed.  

      Used as a paramter passed to the kernel factory's create method and as a return
        type for IInstanceInfo::GetProcessType.
      
      @li IN_PROCESS:             In the client's process
      @li LOCAL_OUT_OF_PROCESS:   On the same machine, but different process than client
      @li REMOTE_OUT_OF_PROCESS:  On a different machine and different process than client
      @li ANY_OUT_OF_PROCESS:     Either LOCAL_OUT_OF_PROCESS or REMOTE_OUT_OF_PROCESS
      @li ANY_PROCESS:            Any of IN/LOCAL_OUT_OF/REMOTE_OUT_OF PROCESS.
   */
   typedef enum  { 
      gSKI_IN_PROCESS, 
      gSKI_LOCAL_OUT_OF_PROCESS, 
      gSKI_REMOTE_OUT_OF_PROCESS, 
      gSKI_ANY_OUT_OF_PROCESS, 
      gSKI_ANY_PROCESS 
   } egSKIProcessType;

   /**
      Types of debug logging allowed.  
     
      @li LOG_NONE:             No debug logging occurs
      @li LOG_ERRORS:           Only errors are logged
      @li LOG_ALL_EXCEPT_DEBUG: Everything but pure debug messages logged
      @li LOG_ALL:              Everything is logged
   */
   typedef enum  { 
      gSKI_LOG_NONE,
      gSKI_LOG_ERRORS,
      gSKI_LOG_ALL,
      gSKI_LOG_ALL_EXCEPT_DEBUG
   } egSKILogActivityLevel;

   /**
    * Symbol types for the ISymbol interface.
    *
    * Returned from ISymbol's GetType() method, this defines the type
    * of information stored in a Symbol. An ISymbol's type should be
    * checked before any Get methods are called as the return data
    * is undefined if the type is not the same as the one retrieved.
    *
    * @li DOUBLE:    Floating point, double precision number  
    * @li INT:       Integer number
    * @li STRING:    A simple string
    * @li OBJECT:    A Working memory ObjectSoar identifier
    */
   typedef enum {
     gSKI_ANY_SYMBOL = 0xff,
     gSKI_DOUBLE     = 0x01,
     gSKI_INT        = 0x02,
     gSKI_STRING     = 0x04,
     gSKI_OBJECT     = 0x08,
     gSKI_VARIABLE   = 0x10,
     gSKI_INVALID    = 0x20
   } egSKISymbolType;


   
   /** 
    * @brief Definition of the build in working memory object types
    *
    * This type is returned from IWMObject::GetObjectType.  The object
    *  types defined here are the only objects with direct API structure
    *  support in gSKI.
    *
    * @li SIMPLE_OBJECT:   A general working memory object. Any object that
    *                       is not one of the types listed below.
    * @li OPERATOR_OBJECT: An operator.  You can cast an operator object
    *                       to an IOperator object using IWMObject::ToOperator
    * @li STATE_OBJECT:    A working memory state. You can cast a state object
    *                       to an IState object using IWMObject::ToState
    */
   typedef enum {
      gSKI_SIMPLE_OBJECT,
      gSKI_OPERATOR_OBJECT,
      gSKI_STATE_OBJECT
   } egSKIWMObjectType;

   /** 
    * @brief Definition of the types of impasses possible in soar
    *
    * Impasses are roadblocks in decisoin making such as inability
    *  to choose an operator, etc.  They always result in a substate
    *  being created for resolving that impasse.
    *
    * @li STATE_NO_CHANGE: Occurs when no operators are proposed for
    *         a given state.
    * @li OPERATOR_NO_CHANGE: Occurs when an operator remians selected for
    *         multiple decision cycles.  This typically occurs when there
    *         is no knowledge about how to apply the operator.
    * @li OPERATOR_TIE_IMPASSE: Occurs when the operator selection phase
    *         cannot choose a single operator because multiple operators
    *         are equally acceptable.
    * @li OPERATOR_CONFLICT_IMPASSE: Occurs when two or more operators have
    *         conflicting preferences (e.g. each is prefered over the other).
    * @li OPERATOR_CONSTRAINT_FAILURE_IMPASSE: Occurs when there are more than
    *         one required operator, or the same operator is both required
    *         and prohibited.
    */
   typedef enum {
      gSKI_STATE_NO_CHANGE_IMPASSE,
      gSKI_OPERATOR_NO_CHANGE_IMPASSE,
      gSKI_OPERATOR_TIE_IMPASSE,
      gSKI_OPERATOR_CONFLICT_IMPASSE,
      gSKI_OPERATOR_CONSTRAINT_FAILURE_IMPASSE
   } egSKIImpasseType;

   /** 
    * @brief Defines types of unary preferences in soar
    * 
    * Operators (and wmes) must be acceptable or required to be chosen.
    *  The reject and prohibit preference have precidence over accept and
    *  best.  However, require has precidence over reject.  For more information
    *  see the Soar manual.
    *
    * @li ACCEPTABLE: Operator (or wme) can be selected
    * @li REJECT:     Operator (or wme) should not be selected, should be removed
    *                    from consideration.
    * @li BEST:       Operator is the best choice, and should be selected.
    * @li WORST:      Operator is the worst choice, and should be selected ONLY
    *                    if no other operator is acceptable or best.
    * @li REQUIRE:    Operator MUST be selected in order to reach agent's goal.  This
    *                    operator will ALWAYS be selected.
    * @li PROHIBIT:   Operator CANNOT be selected if agant's goal is to be reached.
    *                    This operator will NEVER be selected.
    * @li INDIFFERENT: Operator can be be chosen or not chosen, it doesn't matter.
    *                    (Results in random choice among operators).
    * @li BIN_INDIFFERENT: Binary preference used for partial ordering where two operators
    *                        are considered indiferent to each other (eithe can be chosen).
    *                        (returned for RhsActions only).
    * @li BIN_BETTER:  Binary preference used for partial ordering where one operator
    *                   is better than another (returned for RhsActions only).
    * @li BIN_WORSE:   Binary preference used for partial ordering where one operator
    *                   is worse than another (returned for RhsActions only).
    */
   typedef enum {
      gSKI_ANY_PREF              = 0x0000,
      gSKI_ACCEPTABLE_PREF       = 0x0001,
      gSKI_REJECT_PREF           = 0x0002,
      gSKI_BEST_PREF             = 0x0004,
      gSKI_WORST_PREF            = 0x0008,
      gSKI_REQUIRE_PREF          = 0x0010,
      gSKI_PROHIBIT_PREF         = 0x0020,
      gSKI_INDIFFERENT_PREF      = 0x0040,
      gSKI_BIN_INDIFFERENT_PREF  = 0x0080,
      gSKI_BIN_BETTER_PREF       = 0x0100,
      gSKI_BIN_WORSE_PREF        = 0x0200
   } egSKIPreferenceType;


   
   /** 
   * @brief Definition of the types of decision phases for Soar
   *
   * This enumeration lists all of the decision phase types for the
   *  Soar system.  There are five distinct decision phases in Soar:
   *  Input, Operator Proposal, Operator Selection, Operator Application,
   *  and Output.  In Operator Proposal and Operator Application the
   *  the system executes 1 or more elaboration phases (rule firing phases).
   *
   * @li INPUT_PHASE: Phase during which the input link is updated.
   *       If the client is using input callbacks, the input producers
   *       are called during this phase.
   * @li PROPOSAL_PHASE: Phase during which operators are proposed and
   *       state elaborations occur. This phase consists of one or more
   *       elaboration phases during which all rules that match fire.
   * @li DECISION_PHASE: Phase during which an operator is selected for
   *       application.  During this phase, if the system cannot choose
   *       a unique and new operator, it will drop into a sub-state.
   * @li APPLY_PHASE: Phase during which an operator is applied.  This
   *       phase consists of one or more elaboration phases during which
   *       all applicable operator application and elaboration rules fire.
   *       The application phase may be cut short if some operator application
   *       invalidates the currently selected operator (e.g. changes a part
   *       of memory the operator proposal production tests).
   * @li OUTPUT_PHASE: Phase during which the output routines are executed.
   *       During this phase any registered output consumers are notified
   *       of the agent's commands.
   */
   typedef enum {
      gSKI_INPUT_PHASE,			// NOTE: This enum MUST be kept in synch with smlPhase defined in sml_ClientEvents.h
      gSKI_PROPOSAL_PHASE,
      gSKI_DECISION_PHASE,
      gSKI_APPLY_PHASE,
      gSKI_OUTPUT_PHASE,
	  gSKI_PREFERENCE_PHASE,	// Soar 7 mode only
	  gSKI_WM_PHASE,			// Soar 7 mode only
   } egSKIPhaseType;

   /**
   * @brief Agent run step definitions.
   *
   * This enumeration is used for the agent manager and agent thread group 
   *  run methods to describe how long the agent should run before stopping.
   *
   * @see egSKIDecisionPhaseType
   *
   * @li RUN_SMALLEST_STEP:     Run the smallest step possible before returning.  For
   *                             the INPUT_PHASE, OUTPUT_PHASE and SELECTION_PHASE 
   *                             the decision phase is the smallest step.  For
   *                             PROPOSAL_PHASE and APPLY_PHASE the smallest step
   *                             is an elaboration cycle (a single pass of parallel
   *                             production firings)
   * @li RUN_PHASE:             Run a single decision phase.  A decision phase is one of
   *                             the following phase types: gSKI_INPUT_PHASE, gSKI_PROPOSAL_PHASE,
   *                             gSKI_DECISION_PHASE, gSKI_APPLY_PHASE, and gSKI_OUTPUT_PHASE.
   * @li RUN_ELABORATION_CYCLE:	Run for a single round of parallel production firings.
   *							May be within a single phase or cross multiple phases if no productions
   *							match for those phases.
   * @li RUN_DECISION_CYCLE:    Run a single decision cycle.  A decision cycle is a single iteration
   *                             of all decision phases in sequence.
   * @li RUN_UNTIL_OUTPUT:      Run as many decision cycles as necessary until the agent produces something
   *                             on its output link.
   * @li RUN_FOREVER:           Run and don't stop until Stop is called.
   */
   typedef enum {
      gSKI_RUN_SMALLEST_STEP,
      gSKI_RUN_PHASE,
	  gSKI_RUN_ELABORATION_CYCLE,	// in Soar 7 mode, this is not the same as smallest_step 
      gSKI_RUN_DECISION_CYCLE,
      gSKI_RUN_UNTIL_OUTPUT,
      gSKI_RUN_FOREVER,
      gSKI_NUM_RUN_TYPES
   } egSKIRunType;

   /** 
   * @brief Agent run interleaving definitions
   *
   * Agent interleave definitions are used to tell the system how long
   *   to run each agent before giving run time to the next agent.  Of
   *   course this only applies to multiple agents running in a single
   *   thread group.
   *
   * @li INTERLEAVE_SMALLEST_STEP: Run each agent the shortest time possible
   *      before moving to the next agent.  The amount of time each agent
   *      runs depends on the current decision phase.  For the Soar 8 mode
   *      INPUT, OUTPUT and DECISION phases, the smallest step is the
   *      decision phase.  For PROPOSAL and APPLY phases, the smallest
   *      step is the elaboration phase (a single pass of parallel rule
   *      firings).  
   *      In Soar 7 mode, the smallest step is the same as a PHASE, but
   *	  users should be allowed to interleave by ELABORATION if they want to.
   *      A Soar 7 ELABORATION is once thru input-preference-wm-output, 
   *	  until mini-quiescence.
   * @li INTERLEAVE_PHASE: Run each agent one phase before
   *      moving to the next agent.  A phase is one of
   *      the following phase types: gSKI_INPUT_PHASE, gSKI_PROPOSAL_PHASE,
   *      gSKI_DECISION_PHASE, gSKI_APPLY_PHASE, and gSKI_OUTPUT_PHASE.
   * @li INTERLEAVE_DECISION_CYCLE: Run each agent a full decision cycle
   *      before transfering processing to the next agent.
   * @li INTERLEAVE_OUTPUT: Run each agent until it produces output on the
   *      output link before transfering to the next agent.
   */
   typedef enum {
      gSKI_INTERLEAVE_SMALLEST_STEP,
	  gSKI_INTERLEAVE_ELABORATION_PHASE,
      gSKI_INTERLEAVE_PHASE,
      gSKI_INTERLEAVE_DECISION_CYCLE,
      gSKI_INTERLEAVE_OUTPUT
   } egSKIInterleaveType;


   /** 
    * @brief Definitions for the agent running states
    *
    * @li RUNSTATE_STOPPED - the agent is currently not running at all.
    *           Continue() will do nothing in this state.
    * @li RUNSTATE_INTERRUPTED - the agent was stopped in the middle of
    *           execution.  It is currently not executing.  You may continue
    *           execution by using the Continue() method.
    * @li RUNSTATE_RUNNING - the agent is currently executing.
    * @li RUNSTATE_HALTED  - the agent has completed execution and cannot
    *                           be run again until reinitialized.  This is
    *                           caused by the (halt) rhs function and by
    *                           the IAgent::Halt method or the IAgentManager::HaltAll
    *                           method.
    */
   typedef enum {
      gSKI_RUNSTATE_STOPPED,
      gSKI_RUNSTATE_INTERRUPTED,
      gSKI_RUNSTATE_RUNNING,
      gSKI_RUNSTATE_HALTED
   } egSKIRunState;

   /** 
    * @brief Definition of the ways in which runs can complete
    *
    * @li RUN_ERROR - an error occured and the run could not complete.  
    *     The state of the system depends on the particular error.
    * @li RUN_EXECUTING - if the run is still executing after the method
    *      returns.
    * @li RUN_INTERRUPTED - the run was interrupted in the middle.  This
    *      applies only to running in client threads.  You will need
    *      to call Continue to continue execution.
    * @li RUN_COMPLETED - the run completed normally.  You can call
    *      a Run method again to execute further.
    * @li RUN_COMPLETED_AND_INTERRUPTED - An interrupt was requested, but
    *      the run completed before it actually triggered.  The state
    *      of the system is the same as if the run completed. (you call
    *      a Run method to start running again.)
    */
   typedef enum {
      gSKI_RUN_ERROR,
      gSKI_RUN_EXECUTING,
      gSKI_RUN_INTERRUPTED,
      gSKI_RUN_COMPLETED,
      gSKI_RUN_COMPLETED_AND_INTERRUPTED
   } egSKIRunResult;
   
   /**
   *  Agent stop points definition.
   *
   *  This enumeration defines the safe stop points for an agent.
   *  You cannot stop an agent at points other than those described
   *   by this enumeration. One of these enumerated values is passed
   *   to one of the Stop methods when commanding an agent to stop.
   *
   *  The stop location is always used together with a stop type which tells
   *    the system how to execute the stop (via return or thread sleep).
   *
   *  @see egSKIDecisionPhaseType
   *  @see egSKIStopType
   *
   *  @li STOP_ON_CALLBACK_RETURN:  Stop immediately after the next callback return event.
   *        If Stop(gSKI_STOP_ON_CALLBACK_RETURN) is called from within a 
   *        callback, the agent(s) will stop after that callback returns.  
   *        If it is called outside of a callback, the agent(s) will stop after
   *        the next callback returns.
   *  @li STOP_AFTER_ALL_CALLBACKS_RETURN: Stop after the next set of callbacks
   *       is notified.  If the system is in the middle of notifying listeners
   *       of an event, the stop occurs after the last listener is notified
   *       of that event.
   *  @li STOP_AFTER_CURRENT_PHASE: Stop immediately after completing processing
   *       for the current phase. The phase is either an elaboration phase
   *       or a decision phase, whichever the system is executing when the
   *       stop method is called.
   *  @li STOP_NEXT_ELABORATION_PHASE: Stop at the beginning of the next 
   *       elaboration phase.
   *  @li STOP_NEXT_DECISION_PHASE: Stop at the beginning of the next decision
   *       phase.
   *  @li STOP_NEXT_DECISION_CYCLE: Stop at the end of the next decision cycle.
   */
   typedef enum {
      gSKI_STOP_ON_CALLBACK_RETURN			= 1 << 0,
      gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN	= 1 << 1,
      gSKI_STOP_AFTER_SMALLEST_STEP			= 1 << 2,
      gSKI_STOP_AFTER_PHASE					= 1 << 3,
      gSKI_STOP_AFTER_DECISION_CYCLE		= 1 << 4
   } egSKIStopLocation;

   /** 
   *  @brief Ways you can stop the agents from running.
   *
   *  @see egSKIStopLocation
   *
   *  @li gSKI_STOP_BY_RETURNING: Stop the agent by returning from the Run 
   *       function.  If the agent is running in a threadgroup, this will
   *       effectively kill the agent thread.  This type of stop does not
   *       work for gSKI_STOP_ON_CALLBACK_RETURN and gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN,
   *       stop locations.
   *  @li gSKI_STOP_BY_SUSPENDING: Stop the agent thread by suspending thread  
   *       execution.  If you stop a single threaded application this way, you
   *       effectively stop the entire application.  The thread remains
   *       suspended until Run is called on that thread group again.
   */
   typedef enum {
      gSKI_STOP_BY_RETURNING,
      gSKI_STOP_BY_SUSPENDING
   } egSKIStopType;

   /**
    * Support types for IWME instances.
    *
    * Returned from IWME's GetSupportType() method, and from the
    *  IRhsAction::GetSupportType() method, this defines the
    *  type of support that a WME (or a wme that will be created
    *  by an action) has. Currently the types of support are
    *  as follows:
    *
    * @li gSKI_O_SUPPORT:   Supported by an operator
    * @li gSKI_I_SUPPORT:   Supported by input wmes
    * @li gSKI_UNKOWN_SUPPORT: If the type of support is not
    *                            known (applies only to
    *                            IRhsActions).
    *
    * For a more indepth discussion of WME support in Soar see ???.
    */
   // TODO: Flesh this out with more details and make sure that these are
   // the only types of support
   typedef enum {
      // Note that these are purposely the same values as used by the soar kernel
     gSKI_UNKNOWN_SUPPORT = 0,
     gSKI_O_SUPPORT = 1,
     gSKI_I_SUPPORT = 2    
   } egSKISupportType;

   /** 
   * @brief Definition of all allowed o-support modes in Soar
   *
   * @li gSKI_O_SUPPORT_MODE_0: Mode 0 is the base mode.  O-support is calculated
   *       based on the structure of working memory that is tested and 
   *       modified.  Testing an operator or operator acceptable preference
   *       results in state or operator augmentations being o-supported.
   *       The support computation is very complex (see soar manual).
   * @li gSKI_O_SUPPORT_MODE_1: NOT AVAILABLE THROUGH GSKI (see soar manual)
   * @li gSKI_O_SUPPORT_MODE_2: Mode 2 is the same as mode 0 except that
   *       all support is calculated the production structure, not from
   *       working memory structure.  Augmentations of operators are still
   *       o-supported.
   * @li gSKI_O_SUPPORT_MODE_3: Mode 3 is the same as mode 2 except that
   *       operator elaborations (adding attributes to operators) now get
   *       i-support even though you have to test the operator to 
   *       elaborate an operator.  This is the default mode in Soar 8.
   * @li gSKI_O_SUPPORT_MODE_4: The new default, TODO
   */
   typedef enum {
      gSKI_O_SUPPORT_MODE_0,
      gSKI_O_SUPPORT_MODE_2,
      gSKI_O_SUPPORT_MODE_3,
      gSKI_O_SUPPORT_MODE_4,
   } egSKIOSupportMode;

   /**
    * @brief Definition of action element types
    *
    * @li ACTION_SYMBOL - Action element that is represented by a
    *                      symbol (either a constant or a variable).
    * @li ACTION_FUNCTION - Action element that is represented by a
    *                        RhsFunctionAction that generates the
    *                        symbol for the element at runtime.
    */
   typedef enum {
      gSKI_ACTION_SYMBOL,
      gSKI_ACTION_FUNCTION
   } egSKIActionElementType;

   /**
    * @brief: Definition of the different types of tests.
    *
    * These enumeration names should be pretty self explanatory.
    *
    * @li gSKI_EQUAL
    * @li gSKI_GREATER_THAN
    * @li gSKI_LESS_THAN
    * @li gSKI_GREATER_OR_EQUAL
    * @li gSKI_LESS_THAN_OR_EQUAL
    * @li gSKI_NOT_EQUAL
    */
   typedef enum  {
      gSKI_EQUAL,
      gSKI_GREATER_THAN,
      gSKI_LESS_THAN,
      gSKI_GREATER_OR_EQUAL,
      gSKI_LESS_THAN_OR_EQUAL,
      gSKI_NOT_EQUAL,
      gSKI_DISJUNCTION,
      gSKI_CONJUNCTION,
      gSKI_OTHER,
   } egSKITestType;

   /**
    * @brief Difinition of the different types of productions.
    */
   typedef enum {
      gSKI_CHUNK,
      gSKI_DEFAULT,
      gSKI_JUSTIFICATION,
      gSKI_USER,

      gSKI_NUM_PRODUCTION_TYPES, /// End marker for iteration
   } egSKIProdType;

   /** 
    * @brief Definition of a special value for rhs function parameter number
    *
    * This defines special RHS function parameter number values.
    *
    * @li gSKI_PARAM_NUM_VARIABLE Any number of parameters can be passed into the function
    */
   typedef enum {
      gSKI_PARAM_NUM_VARIABLE = -1
   } egSKIParamNumType;

   typedef enum
   {
      gSKI_USER_SELECT_FIRST,    /// just choose the first candidate item
      gSKI_USER_SELECT_ASK,      /// ask the user
      gSKI_USER_SELECT_RANDOM,   /// pick one at random
      gSKI_USER_SELECT_LAST,     /// choose the last item
   } egSKIUserSelectType;

   typedef enum
   {
      gSKI_NUMERIC_INDIFFERENT_MODE_SUM,  /// do numeric indifference by summing all values asserted by the rules.  Indifferent prefferences with no explicit value are assigned the numeric weight of 0.
      gSKI_NUMERIC_INDIFFERENT_MODE_AVG,  /// do numeric indiffernce by averaging all values asserted by the rules.  Indifferent preferrences with no explicit value are assigned the numeric weight of 50.
   } egSKINumericIndifferentMode;

#endif
