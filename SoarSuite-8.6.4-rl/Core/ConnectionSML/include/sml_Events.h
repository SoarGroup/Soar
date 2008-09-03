/////////////////////////////////////////////////////////////////
// List of event ids
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// These are the events that an SML agent can listen for
//
// ==========================================
// READ THESE NOTES BEFORE CHANGING THIS FILE:
// ==========================================
//
// There are several constraints on the IDs in these event enums.
//
// First, each value must be a unique integer.
//     That's to say the value of the enums in smlSystemEventId
//     must not intersect with any of the values in smlRunEventId (e.g.)
//     This is generally achieved by making the first value of one enum
//     equal the last value of the previous enum + 1.
//     (e.g. smlEVENT_BEFORE_SMALLEST_STEP = smlEVENT_LAST_SYSTEM_EVENT + 1)
//
// Second, if you change the list of events in an enum, make sure that the
//    related test function (e.g. IsSystemEventID()) is still valid.
//
// Third, the values of these IDs currently are required to match the values
//    in the gSKIEventId enumeration in the kernel.
//    This way there's no need to map to and from ID values, making debugging
//    easier.
//    If we wish to break this requirement later all that's needed is to add
//    a mapping function on the kernel side.
//    There are some run time ASSERTs included in the kernel to help check
//    that the mapping has not been thrown off somehow, but we basically
//    need to be careful when changing either.
//
// Fourth, when you add a new event you need to add a string form for it
//		   to the sml::Events class (defined in ConnectionSML\sml_Events.cpp)
//
/////////////////////////////////////////////////////////////////

#ifndef SML_EVENT_ID_H
#define SML_EVENT_ID_H

//#include "sml_ClientEvents.h"

#include <map>
#include <string>
#include <assert.h>

namespace sml {

typedef enum {
    smlEVENT_BEFORE_SHUTDOWN            = 1,
	smlEVENT_AFTER_CONNECTION,
    smlEVENT_AFTER_CONNECTION_LOST,
    smlEVENT_BEFORE_RESTART,
    smlEVENT_AFTER_RESTART,
	smlEVENT_SYSTEM_START,
	smlEVENT_SYSTEM_STOP,
	smlEVENT_INTERRUPT_CHECK,					// Chance for client to interrupt a run (designed to be low bandwidth)
	smlEVENT_SYSTEM_PROPERTY_CHANGED,			// A sysparam or other value has been changed 
    smlEVENT_BEFORE_RHS_FUNCTION_ADDED,
    smlEVENT_AFTER_RHS_FUNCTION_ADDED,
    smlEVENT_BEFORE_RHS_FUNCTION_REMOVED,
    smlEVENT_AFTER_RHS_FUNCTION_REMOVED,
    smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
	smlEVENT_AFTER_RHS_FUNCTION_EXECUTED,
	smlEVENT_LAST_SYSTEM_EVENT = smlEVENT_AFTER_RHS_FUNCTION_EXECUTED
} smlSystemEventId ;

typedef enum {
    smlEVENT_BEFORE_SMALLEST_STEP = smlEVENT_LAST_SYSTEM_EVENT + 1,
    smlEVENT_AFTER_SMALLEST_STEP,
    smlEVENT_BEFORE_ELABORATION_CYCLE,
    smlEVENT_AFTER_ELABORATION_CYCLE,
  /*  inline tests depend on this ordering... */
    smlEVENT_BEFORE_PHASE_EXECUTED,
    smlEVENT_BEFORE_INPUT_PHASE,
    smlEVENT_BEFORE_PROPOSE_PHASE,
    smlEVENT_BEFORE_DECISION_PHASE,
    smlEVENT_BEFORE_APPLY_PHASE,
    smlEVENT_BEFORE_OUTPUT_PHASE,
    smlEVENT_BEFORE_PREFERENCE_PHASE,	// Soar-7 mode only
    smlEVENT_BEFORE_WM_PHASE,			// Soar-7 mode only
	smlEVENT_AFTER_INPUT_PHASE,
    smlEVENT_AFTER_PROPOSE_PHASE,
    smlEVENT_AFTER_DECISION_PHASE,
    smlEVENT_AFTER_APPLY_PHASE,
    smlEVENT_AFTER_OUTPUT_PHASE,
    smlEVENT_AFTER_PREFERENCE_PHASE,	// Soar-7 mode only
    smlEVENT_AFTER_WM_PHASE,			// Soar-7 mode only
    smlEVENT_AFTER_PHASE_EXECUTED,
	/* */
    smlEVENT_BEFORE_DECISION_CYCLE,
    smlEVENT_AFTER_DECISION_CYCLE,
	smlEVENT_MAX_MEMORY_USAGE_EXCEEDED,
    smlEVENT_AFTER_INTERRUPT,
	smlEVENT_AFTER_HALTED,
	smlEVENT_BEFORE_RUN_STARTS,			// Before start of a run
	smlEVENT_AFTER_RUN_ENDS,			// After run ends for any reason
    smlEVENT_BEFORE_RUNNING,			// Before running one step (phase)
    smlEVENT_AFTER_RUNNING,				// After running one step (phase)
	smlEVENT_LAST_RUN_EVENT = smlEVENT_AFTER_RUNNING
} smlRunEventId ;

typedef enum {
    // Production Manager
    smlEVENT_AFTER_PRODUCTION_ADDED = smlEVENT_LAST_RUN_EVENT + 1,
    smlEVENT_BEFORE_PRODUCTION_REMOVED,
    //smlEVENT_BEFORE_PRODUCTION_FIRED,
    smlEVENT_AFTER_PRODUCTION_FIRED,
    smlEVENT_BEFORE_PRODUCTION_RETRACTED,
	smlEVENT_LAST_PRODUCTION_EVENT = smlEVENT_BEFORE_PRODUCTION_RETRACTED
} smlProductionEventId ;

typedef enum {
	// Agent manager
    smlEVENT_AFTER_AGENT_CREATED = smlEVENT_LAST_PRODUCTION_EVENT + 1,
    smlEVENT_BEFORE_AGENT_DESTROYED,
	smlEVENT_BEFORE_AGENTS_RUN_STEP,
    smlEVENT_BEFORE_AGENT_REINITIALIZED,
    smlEVENT_AFTER_AGENT_REINITIALIZED,
	smlEVENT_LAST_AGENT_EVENT = smlEVENT_AFTER_AGENT_REINITIALIZED
} smlAgentEventId ;

typedef enum {
	// Working memory changes
	smlEVENT_OUTPUT_PHASE_CALLBACK = smlEVENT_LAST_AGENT_EVENT + 1,
	smlEVENT_LAST_WM_EVENT = smlEVENT_OUTPUT_PHASE_CALLBACK
} smlWorkingMemoryEventId ;

typedef enum {
    // Error and print callbacks
	smlEVENT_LOG_ERROR = smlEVENT_LAST_WM_EVENT + 1,
    smlEVENT_LOG_WARNING,
    smlEVENT_LOG_INFO,
    smlEVENT_LOG_DEBUG,
	smlEVENT_ECHO,
    smlEVENT_PRINT,
	smlEVENT_LAST_PRINT_EVENT = smlEVENT_PRINT
} smlPrintEventId ;

typedef enum {
	// Used to provide user handler functions for RHS (right hand side) functions
	// fired within Soar productions.  This is different from normal events in that
	// the handler is executing the function and returning a value, not just being notified
	// that something has happened.
	smlEVENT_RHS_USER_FUNCTION = smlEVENT_LAST_PRINT_EVENT + 1,
	smlEVENT_FILTER,				// This event can be used to filter (modify) command lines before they are processed by the kernel
	smlEVENT_CLIENT_MESSAGE,		// A generic message from one client to another (not really involving Soar/kernel directly)
	smlEVENT_LAST_RHS_EVENT = smlEVENT_CLIENT_MESSAGE
} smlRhsEventId ;

typedef enum {
	smlEVENT_XML_TRACE_OUTPUT = smlEVENT_LAST_RHS_EVENT + 1,
	smlEVENT_XML_INPUT_RECEIVED,		// Echo event for input wmes added by a client (so others can listen in)
	smlEVENT_LAST_XML_EVENT = smlEVENT_XML_INPUT_RECEIVED
} smlXMLEventId ;

// Events that can be used by environments to trigger when the world should update
typedef enum {
	smlEVENT_AFTER_ALL_OUTPUT_PHASES = smlEVENT_LAST_XML_EVENT + 1,	// All agents have completed output phase
	smlEVENT_AFTER_ALL_GENERATED_OUTPUT,						// All agents have generated output (since run began)
	smlEVENT_LAST_UPDATE_EVENT = smlEVENT_AFTER_ALL_GENERATED_OUTPUT
} smlUpdateEventId ;

// Events that pass a string as an argument
typedef enum {
	smlEVENT_EDIT_PRODUCTION = smlEVENT_LAST_UPDATE_EVENT + 1,	// Arg is "char const*" -- the name of the production to edit
	smlEVENT_LOAD_LIBRARY,
	smlEVENT_LAST_STRING_EVENT = smlEVENT_LOAD_LIBRARY,
} smlStringEventId ;

typedef enum {
    // Used to indicate an error in some cases
    smlEVENT_INVALID_EVENT              = 0,

	// Marker for end of sml event list
	// Must always be at the end of the enum
	smlEVENT_LAST = smlEVENT_LAST_STRING_EVENT + 1
} smlGenericEventId ;

static inline bool IsStringEventID(int id)
{
	return (id >= smlEVENT_EDIT_PRODUCTION && id <= smlEVENT_LAST_STRING_EVENT) ;
}

static inline bool IsSystemEventID(int id)
{
	return (id >= smlEVENT_BEFORE_SHUTDOWN && id <= smlEVENT_LAST_SYSTEM_EVENT) ;
}

static inline bool IsRunEventID(int id)
{
	return (id >= smlEVENT_BEFORE_SMALLEST_STEP && id <= smlEVENT_LAST_RUN_EVENT) ;
}
static inline bool IsPhaseEventID (int id)
{   
	return (id > smlEVENT_BEFORE_PHASE_EXECUTED && id < smlEVENT_AFTER_PHASE_EXECUTED) ;
}
static inline bool IsBEFOREPhaseEventID (int id)
{
 	return (id > smlEVENT_BEFORE_PHASE_EXECUTED && id <= smlEVENT_BEFORE_WM_PHASE) ;
}
static inline bool IsAFTERPhaseEventID (int id)
{
 	return (id >= smlEVENT_AFTER_INPUT_PHASE && id < smlEVENT_AFTER_PHASE_EXECUTED) ;
}

static inline bool IsProductionEventID(int id)
{
	return (id >= smlEVENT_AFTER_PRODUCTION_ADDED && id <= smlEVENT_LAST_PRODUCTION_EVENT) ;
}

static inline bool IsAgentEventID(int id)
{
	return (id >= smlEVENT_AFTER_AGENT_CREATED && id <= smlEVENT_LAST_AGENT_EVENT) ;
}

static inline bool IsWorkingMemoryEventID(int id)
{
	return (id >= smlEVENT_OUTPUT_PHASE_CALLBACK && id <= smlEVENT_LAST_WM_EVENT) ;
}

static inline bool IsPrintEventID(int id)
{
	return (id >= smlEVENT_LOG_ERROR && id <= smlEVENT_LAST_PRINT_EVENT) ;
}

static inline bool IsRhsEventID(int id)
{
	return (id >= smlEVENT_RHS_USER_FUNCTION && id <= smlEVENT_LAST_RHS_EVENT) ;
}

static inline bool IsXMLEventID(int id)
{
	return (id >= smlEVENT_XML_TRACE_OUTPUT && id <= smlEVENT_XML_INPUT_RECEIVED) ;
}

static inline bool IsUpdateEventID(int id)
{
	return (id >= smlEVENT_AFTER_ALL_OUTPUT_PHASES && id <= smlEVENT_LAST_UPDATE_EVENT) ;
}

typedef enum {
    sml_INPUT_PHASE,		// NOTE: This enum MUST be kept in synch with egSKIPhaseType defined in gSKI_Enumerations.h
    sml_PROPOSAL_PHASE,
    sml_DECISION_PHASE,
    sml_APPLY_PHASE,
    sml_OUTPUT_PHASE,
	sml_PREFERENCE_PHASE,	// Soar 7 mode only
	sml_WM_PHASE			// Soar 7 mode only
} smlPhase;

typedef enum
{
	sml_ELABORATION,
	sml_PHASE,
	sml_DECISION,
	sml_UNTIL_OUTPUT
} smlRunStepSize ;

typedef enum
{
	sml_INTERLEAVE_ELABORATION,
	sml_INTERLEAVE_PHASE,
	sml_INTERLEAVE_DECISION,
	sml_INTERLEAVE_UNTIL_OUTPUT
} smlInterleaveStepSize ;

typedef enum
{
	sml_NONE				=  0,		// No special flags set
	sml_RUN_SELF			=  1 << 0,	// User included --self flag when running agent
	sml_RUN_ALL				=  1 << 1,	// User ran all agents
	sml_UPDATE_WORLD		=  1 << 2,	// User explicitly requested world to update
	sml_DONT_UPDATE_WORLD	=  1 << 3	// User explicitly requested world to not update
} smlRunFlags ;

typedef enum {
    sml_RUN_ERROR,
    sml_RUN_EXECUTING,
    sml_RUN_INTERRUPTED,
    sml_RUN_COMPLETED,
    sml_RUN_COMPLETED_AND_INTERRUPTED	// Stop was requested but run completed before agent was interrupted.
} smlRunResult;

typedef enum {
  sml_RUNSTATE_STOPPED,
  sml_RUNSTATE_INTERRUPTED,
  sml_RUNSTATE_RUNNING,
  sml_RUNSTATE_HALTED
} smlRunState;

/////////////////////////////////////////////////////////////////
// Map event ids to and from strings
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2005
//
/////////////////////////////////////////////////////////////////

// This class is used to convert to and from the string form of these IDs
class Events
{
protected:
	// We keep two maps, one for each direction
	typedef std::map<int, std::string> ToStringMap ;
	typedef std::map<std::string, int> ToEventMap  ;

	ToStringMap m_ToStringMap ;
	ToEventMap  m_ToEventMap ;

	void		RegisterEvent(int id, char const* pStr) ;

	/*************************************************************
	* @brief Convert from a string version of an event to the int (enum) version.
	*		 Returns smlEVENT_INVALID_EVENT (== 0) if the string is not recognized.
	*************************************************************/
	inline int InternalConvertToEvent(char const* pStr)
	{
		Events::ToEventMap::iterator mapIter = m_ToEventMap.find(pStr);

		if (mapIter == m_ToEventMap.end())
		{
			return smlEVENT_INVALID_EVENT ;
		}

		return mapIter->second ;
	}

	/*************************************************************
	* @brief Convert from int version of an event to the string form.
	*		 Returns NULL if the id is not recognized.
	*************************************************************/
	inline char const* InternalConvertToString(int id)
	{
		Events::ToStringMap::iterator mapIter = m_ToStringMap.find(id);

		if (mapIter == m_ToStringMap.end())
		{
			return NULL ;
		}

		return mapIter->second.c_str() ;
	}

public:
	// Construction is expensive, so do this once and then use the methods repeatedly
	Events() ;

	// Methods to convert event IDs from int to string values and back.
	// We do this because the list of IDs will likely change from one version of Soar to another
	// but we'd like the list of names to remain largely constant (and just be added to).
	// This will allow clients built with an earlier version to work with later Soar kernels.
	// As a side effect it makes the XML much more human readable as you get event "init-soar" not event 23.

	/*************************************************************
	* @brief Convert from a string version of an event to the int (enum) version.
	*		 Returns smlEVENT_INVALID_EVENT (== 0) if the string is not recognized.
	*************************************************************/
	inline int ConvertToEvent(char const* pStr)
	{
		return InternalConvertToEvent(pStr) ;
	}

	/*************************************************************
	* @brief Convert from int version of an event to the string form.
	*		 Returns NULL if the id is not recognized.
	*************************************************************/
	inline char const* ConvertToString(int id)
	{
		char const* pName = InternalConvertToString(id) ;

		// This event ID needs to be added to the list registered in the constructor
		assert(pName != NULL) ;

		return pName ;
	}
} ;

} ;	// End of namespace

#endif	// Header
