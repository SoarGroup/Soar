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

/** Enumeration of all event ids in the system */
typedef enum {
    
    // Used to indicate an error in some cases
    smlEVENT_INVALID_EVENT              = 0,

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

// Handler for Run events.
// Passed back the event ID, the agent and the phase
typedef void (*RunEventHandler)(smlEventId id, Agent* pAgent, smlPhase phase);

} ;	// End of namespace

#endif	// Header
