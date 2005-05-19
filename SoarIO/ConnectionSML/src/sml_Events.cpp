/////////////////////////////////////////////////////////////////
// Map event ids to and from strings
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2005
//
/////////////////////////////////////////////////////////////////

#include "sml_Events.h"
#include "sml_ClientEvents.h"
#include <assert.h>

using namespace sml ;

/*************************************************************
* @brief	Initialize the mappings in the constructor.
*			We should only do this once ever.
*************************************************************/
Events::Events()
{
	// Kernel events
	RegisterEvent(smlEVENT_BEFORE_SHUTDOWN, "before-shutdown") ;
	RegisterEvent(smlEVENT_BEFORE_RESTART, "before-restart") ;
	RegisterEvent(smlEVENT_AFTER_RESTART, "after-restart") ;
	RegisterEvent(smlEVENT_SYSTEM_START, "system-start") ;
	RegisterEvent(smlEVENT_SYSTEM_STOP, "system-stop") ;
	RegisterEvent(smlEVENT_INTERRUPT_CHECK, "interrupt-check") ;
	RegisterEvent(smlEVENT_BEFORE_RHS_FUNCTION_ADDED, "before-rhs-function-added") ;
	RegisterEvent(smlEVENT_AFTER_RHS_FUNCTION_ADDED, "after-rhs-function-added") ;
	RegisterEvent(smlEVENT_BEFORE_RHS_FUNCTION_REMOVED, "before-rhs-function-removed") ;
	RegisterEvent(smlEVENT_AFTER_RHS_FUNCTION_REMOVED, "after-rhs-function-removed") ;
	RegisterEvent(smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED, "before-rhs-function-executed") ;
	RegisterEvent(smlEVENT_AFTER_RHS_FUNCTION_EXECUTED, "after-rhs-function-executed") ;

	// Run events
	RegisterEvent(smlEVENT_BEFORE_SMALLEST_STEP, "before-smallest-step") ;
	RegisterEvent(smlEVENT_AFTER_SMALLEST_STEP, "after-smallest-step") ;
	RegisterEvent(smlEVENT_BEFORE_ELABORATION_CYCLE, "before-elaboration-cycle") ;
	RegisterEvent(smlEVENT_AFTER_ELABORATION_CYCLE, "after-elaboration-cycle") ;
	RegisterEvent(smlEVENT_BEFORE_PHASE_EXECUTED, "before-phase-executed") ;
	RegisterEvent(smlEVENT_AFTER_PHASE_EXECUTED, "after-phase-executed") ;
	RegisterEvent(smlEVENT_BEFORE_DECISION_CYCLE, "before-decision-cycle") ;
	RegisterEvent(smlEVENT_AFTER_DECISION_CYCLE, "after-decision-cycle") ;
	RegisterEvent(smlEVENT_AFTER_INTERRUPT, "after-interrupt") ;
	RegisterEvent(smlEVENT_BEFORE_RUN_STARTS, "before-run-starts") ;
	RegisterEvent(smlEVENT_AFTER_RUN_ENDS, "after-run-ends") ;
	RegisterEvent(smlEVENT_BEFORE_RUNNING, "before-running") ;
	RegisterEvent(smlEVENT_AFTER_RUNNING, "after-running") ;

	// Production manager
	RegisterEvent(smlEVENT_AFTER_PRODUCTION_ADDED, "after-production-added") ;
	RegisterEvent(smlEVENT_BEFORE_PRODUCTION_REMOVED, "before-production-added") ;
	RegisterEvent(smlEVENT_AFTER_PRODUCTION_FIRED, "after-production-fired") ;
	RegisterEvent(smlEVENT_BEFORE_PRODUCTION_RETRACTED, "before-production-retracted") ;

	// Agent manager
	RegisterEvent(smlEVENT_AFTER_AGENT_CREATED, "after-agent-created") ;
	RegisterEvent(smlEVENT_BEFORE_AGENT_DESTROYED, "before-agent-destroyed") ;
	RegisterEvent(smlEVENT_BEFORE_AGENTS_RUN_STEP, "before-agents-run-step") ;
	RegisterEvent(smlEVENT_BEFORE_AGENT_REINITIALIZED, "before-agent-reinitialized") ;
	RegisterEvent(smlEVENT_AFTER_AGENT_REINITIALIZED, "after-agent-reinitialized") ;

	// Working memory changes
	RegisterEvent(smlEVENT_OUTPUT_PHASE_CALLBACK, "output-phase") ;

	// Raw XML messages
	RegisterEvent(smlEVENT_XML_TRACE_OUTPUT, "xml-trace-output") ;

    // Error and print callbacks
	RegisterEvent(smlEVENT_LOG_ERROR, "log-error") ;
	RegisterEvent(smlEVENT_LOG_WARNING, "log-warning") ;
	RegisterEvent(smlEVENT_LOG_INFO, "log-info") ;
	RegisterEvent(smlEVENT_LOG_DEBUG, "log-debug") ;
	RegisterEvent(smlEVENT_PRINT, "print") ;

	// Rhs user function fired
	RegisterEvent(smlEVENT_RHS_USER_FUNCTION, "rhs-user-function") ;
}

/*************************************************************
* @brief Register a mapping from an event ID to its string form
*************************************************************/
void Events::RegisterEvent(int id, char const* pStr)
{
	// Neither the id nor the name should be in use already
	assert(InternalConvertToEvent(pStr) == smlEVENT_INVALID_EVENT) ;
	assert(InternalConvertToString(id) == NULL) ;

	m_ToStringMap[id] = pStr ;
	m_ToEventMap[pStr] = id ;
}

