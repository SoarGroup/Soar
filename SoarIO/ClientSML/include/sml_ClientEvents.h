/////////////////////////////////////////////////////////////////
// List of event ids
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// These are the events that an SML agent can listen for and definitions
// for the handlers that will be called.
//
// For now we these IDs should match those in the esmlEventId enumeration
// although later on we may introduce another level of mapping between them.
// We do make some spot checks for this in KernelSML to ensure that the
// mapping doesn't get out of step between here and there.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EVENT_ID_H
#define SML_EVENT_ID_H

namespace sml {

// Forward declaration
class Agent ;
class Kernel ;

/** Enumeration of all event ids in the system */
typedef enum {
    
    // Used to indicate an error in some cases
    smlEVENT_INVALID_EVENT              = 0,

	// BADBAD: It would be better if these event IDs were all different enums
	// so you couldn't accidentally pass the wrong event ID to the wrong handler etc.
	// But gSKI defines them this way and it would be potentially risky to define them differently
	// here on the client side.  Perhaps we'll fix it in gSKI one day and change it here as well.

    // Kernel events
    smlEVENT_BEFORE_SHUTDOWN            = 1,
    smlEVENT_AFTER_CONNECTION_LOST,
    smlEVENT_BEFORE_RESTART,
    smlEVENT_AFTER_RESTART,
    smlEVENT_BEFORE_RHS_FUNCTION_ADDED,
    smlEVENT_AFTER_RHS_FUNCTION_ADDED,
    smlEVENT_BEFORE_RHS_FUNCTION_REMOVED,
    smlEVENT_AFTER_RHS_FUNCTION_REMOVED,
    smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
    smlEVENT_AFTER_RHS_FUNCTION_EXECUTED,

    // Run events
    smlEVENT_BEFORE_SMALLEST_STEP,
    smlEVENT_AFTER_SMALLEST_STEP,
    smlEVENT_BEFORE_ELABORATION_CYCLE,
    smlEVENT_AFTER_ELABORATION_CYCLE,
    smlEVENT_BEFORE_PHASE_EXECUTED,
    smlEVENT_AFTER_PHASE_EXECUTED,
    smlEVENT_BEFORE_DECISION_CYCLE,
    smlEVENT_AFTER_DECISION_CYCLE,
    smlEVENT_AFTER_INTERRUPT,
    smlEVENT_BEFORE_RUNNING,
    smlEVENT_AFTER_RUNNING,

    // Production Manager
    smlEVENT_AFTER_PRODUCTION_ADDED,
    smlEVENT_BEFORE_PRODUCTION_REMOVED,
    //smlEVENT_BEFORE_PRODUCTION_FIRED,
    smlEVENT_AFTER_PRODUCTION_FIRED,
    smlEVENT_BEFORE_PRODUCTION_RETRACTED,

    // Agent manager
    smlEVENT_AFTER_AGENT_CREATED,
    smlEVENT_BEFORE_AGENT_DESTROYED,
    smlEVENT_BEFORE_AGENT_REINITIALIZED,
    smlEVENT_AFTER_AGENT_REINITIALIZED,

	// Working memory changes
	smlEVENT_OUTPUT_PHASE_CALLBACK,

    // Error and print callbacks
    smlEVENT_LOG_ERROR,
    smlEVENT_LOG_WARNING,
    smlEVENT_LOG_INFO,
    smlEVENT_LOG_DEBUG,
    smlEVENT_PRINT,

	// Marker for end of sml event list
	// Must always be at the end of the enum
	smlEVENT_LAST
} smlEventId;

typedef enum {
    sml_INPUT_PHASE,
    sml_PROPOSAL_PHASE,
    sml_SELECTION_PHASE,
    sml_APPLY_PHASE,
    sml_OUTPUT_PHASE,
    sml_DECISION_PHASE,
} smlPhase;

// These typedefs all define types of functions.
// For example: typedef void (*X)(type1 arg1, type2 arg2) means we're defining function "X" to take (type1 arg1, type2 arg2) and return void.
// To provide such a handler define a function with this type and pass its address in to the registration function for the event.

// Handler for Run events.
// Passed back the event ID, the agent and the phase together with whatever user data we registered with the client
typedef void (*RunEventHandler)(smlEventId id, void* pUserData, Agent* pAgent, smlPhase phase);

// Handler for Agent events.
typedef void (*AgentEventHandler)(smlEventId id, void* pUserData, Agent* pAgent) ;

// Handler for Print events.
typedef void (*PrintEventHandler)(smlEventId id, void* pUserData, Agent* pAgent, char const* pMessage) ;

// Handler for Production manager events.
typedef void (*ProductionEventHandler)(smlEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantion) ;

// Handler for System events.
typedef void (*SystemEventHandler)(smlEventId id, void* pUserData, Kernel* pKernel) ;

} ;	// End of namespace

#endif	// Header
