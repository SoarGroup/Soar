#include <portability.h>

/////////////////////////////////////////////////////////////////
// KernelCallback class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2007
//
// This class handles the interface between SML events and
// kernel callbacks.
//
/////////////////////////////////////////////////////////////////

#include "sml_KernelCallback.h"

#include "sml_Utils.h"
#include "sml_AgentSML.h"

#include "KernelHeaders.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace sml ;

void KernelCallback::KernelCallbackStatic(agent* pAgent, int eventID, void* pData, void* pCallData)
{
	KernelCallback* pThis = static_cast<KernelCallback*>(pData) ;

	// Make sure everything matches up correctly.
	(void)pAgent; // silences warning in release mode
	assert(pThis->m_pCallbackAgentSML->GetSoarAgent() == pAgent) ;

	// Make the callback to the non-static method 
	pThis->OnKernelEvent(eventID, pThis->m_pCallbackAgentSML, pCallData) ;
}

// Not returning this as SOAR_CALLBACK_TYPE as that would expose the kernel headers through our headers.
// Want to keep kernel types internal to the implementation files so just using an int.
int KernelCallback::InternalGetCallbackFromEventID(int eventID)
{
	switch (eventID)
	{
	case smlEVENT_AFTER_PRODUCTION_ADDED:		return PRODUCTION_JUST_ADDED_CALLBACK ;
	case smlEVENT_BEFORE_PRODUCTION_REMOVED:	return PRODUCTION_JUST_ABOUT_TO_BE_EXCISED_CALLBACK ;
	case smlEVENT_AFTER_PRODUCTION_FIRED:		return FIRING_CALLBACK ;
	case smlEVENT_BEFORE_PRODUCTION_RETRACTED:	return RETRACTION_CALLBACK ;

	case smlEVENT_BEFORE_SMALLEST_STEP:			return BEFORE_ELABORATION_CALLBACK ;	// Elaboration cycle is the smallest step
	case smlEVENT_AFTER_SMALLEST_STEP:			return AFTER_ELABORATION_CALLBACK ;		// Elaboration cycle is the smallest step
	case smlEVENT_BEFORE_ELABORATION_CYCLE:		return BEFORE_ELABORATION_CALLBACK ;
	case smlEVENT_AFTER_ELABORATION_CYCLE:		return AFTER_ELABORATION_CALLBACK ;
	case smlEVENT_BEFORE_INPUT_PHASE:			return BEFORE_INPUT_PHASE_CALLBACK ;
	case smlEVENT_AFTER_INPUT_PHASE:			return AFTER_INPUT_PHASE_CALLBACK ;
	case smlEVENT_BEFORE_PROPOSE_PHASE:			return BEFORE_PROPOSE_PHASE_CALLBACK ;
	case smlEVENT_AFTER_PROPOSE_PHASE:			return AFTER_PROPOSE_PHASE_CALLBACK ;
	case smlEVENT_BEFORE_DECISION_PHASE:		return BEFORE_DECISION_CYCLE_CALLBACK ;
	case smlEVENT_AFTER_DECISION_PHASE:			return AFTER_DECISION_CYCLE_CALLBACK ;
	case smlEVENT_BEFORE_APPLY_PHASE:			return BEFORE_APPLY_PHASE_CALLBACK ;
	case smlEVENT_AFTER_APPLY_PHASE:			return AFTER_APPLY_PHASE_CALLBACK ;
	case smlEVENT_BEFORE_OUTPUT_PHASE:			return BEFORE_OUTPUT_PHASE_CALLBACK ;
	case smlEVENT_AFTER_OUTPUT_PHASE:			return AFTER_OUTPUT_PHASE_CALLBACK ;
	case smlEVENT_BEFORE_PREFERENCE_PHASE:		return BEFORE_PREFERENCE_PHASE_CALLBACK ;
	case smlEVENT_AFTER_PREFERENCE_PHASE:		return AFTER_PREFERENCE_PHASE_CALLBACK ;
	case smlEVENT_BEFORE_WM_PHASE:				return BEFORE_WM_PHASE_CALLBACK ;
	case smlEVENT_AFTER_WM_PHASE:				return AFTER_WM_PHASE_CALLBACK ;

	case smlEVENT_BEFORE_DECISION_CYCLE:		return BEFORE_DECISION_CYCLE_CALLBACK ;
	case smlEVENT_AFTER_DECISION_CYCLE:			return AFTER_DECISION_CYCLE_CALLBACK ;

	case smlEVENT_MAX_MEMORY_USAGE_EXCEEDED:	return MAX_MEMORY_USAGE_CALLBACK ;
	case smlEVENT_AFTER_INTERRUPT:				return AFTER_INTERRUPT_CALLBACK ;	// Implemented in SML
	case smlEVENT_AFTER_HALTED:					return AFTER_HALTED_CALLBACK ;		// Implemented in SML
	case smlEVENT_BEFORE_RUN_STARTS:			return BEFORE_RUN_STARTS_CALLBACK ;	// Implemented in SML
	case smlEVENT_AFTER_RUN_ENDS:				return AFTER_RUN_ENDS_CALLBACK ;	// Implemented in SML
	case smlEVENT_BEFORE_RUNNING:				return BEFORE_RUNNING_CALLBACK ;	// Implemented in SML
	case smlEVENT_AFTER_RUNNING:				return AFTER_RUNNING_CALLBACK ;		// Implemented in SML

	case smlEVENT_OUTPUT_PHASE_CALLBACK:		return OUTPUT_PHASE_CALLBACK ;
	case smlEVENT_INPUT_PHASE_CALLBACK:			return INPUT_PHASE_CALLBACK ;

	case smlEVENT_PRINT:						return PRINT_CALLBACK ;
	case smlEVENT_XML_TRACE_OUTPUT:				return XML_GENERATION_CALLBACK ;
	}

	return smlEVENT_INVALID_EVENT ;
}

// Returns true if this is a callback that is implemented in the kernel.
// Some events are implemented directly in SML.
bool KernelCallback::IsCallbackImplementedInKernel(int eventID)
{
	// We map these events onto multiple kernel callbacks
	if (eventID == smlEVENT_BEFORE_PHASE_EXECUTED || eventID == smlEVENT_AFTER_PHASE_EXECUTED)
		return true ;

	int callback = InternalGetCallbackFromEventID(eventID) ;

	return (callback != smlEVENT_INVALID_EVENT) ;
}

int KernelCallback::GetCallbackFromEventID(int eventID)
{
	int callback = InternalGetCallbackFromEventID(eventID) ;
	assert (callback != smlEVENT_INVALID_EVENT);

	return callback ;
}

KernelCallback::~KernelCallback()
{
	ClearKernelCallback() ;
}

void KernelCallback::ClearKernelCallback()
{
	for (std::map<int, bool>::iterator mapIter = m_Registered.begin() ; mapIter != m_Registered.end() ; mapIter++)
	{
		int eventID = mapIter->first ;
		bool registered = mapIter->second ;

		if (registered)
			UnregisterWithKernel(eventID) ;
	}
	m_Registered.clear() ;
}

bool KernelCallback::IsRegisteredWithKernel(int eventID)
{
	return (m_Registered[eventID] == true) ;
}

void KernelCallback::RegisterWithKernel(int eventID)
{
	// Should only register once
	assert(m_Registered[eventID] != true) ;
	m_Registered[eventID] = true ;

	// Base the id on the address of this object which ensures it's unique
	std::ostringstream buffer;
	buffer << "id_0x" << this << "_evt_" << eventID;
	std::string callbackID = buffer.str() ;

	// Did you remember to call SetAgentSML() before registering this callback?
	assert(m_pCallbackAgentSML) ;

	agent* pAgent = m_pCallbackAgentSML->GetSoarAgent() ;

	if (eventID == smlEVENT_OUTPUT_PHASE_CALLBACK)
	{
		add_output_function(pAgent, KernelCallbackStatic, this, NULL, smlEVENT_OUTPUT_PHASE_CALLBACK, "output-link") ;
	}
	else if (eventID != smlEVENT_BEFORE_PHASE_EXECUTED && eventID != smlEVENT_AFTER_PHASE_EXECUTED)
	{
		// This is the standard case
		SOAR_CALLBACK_TYPE callbackType = SOAR_CALLBACK_TYPE(GetCallbackFromEventID(eventID)) ;
		soar_add_callback (pAgent, callbackType, KernelCallbackStatic, eventID, this, NULL, callbackID.c_str()) ;
	}
	else
	{
		// BEFORE_PHASE and AFTER_PHASE are implemented by listening for all lower level phase events
		int beforePhaseEvents[] = { smlEVENT_BEFORE_INPUT_PHASE, smlEVENT_BEFORE_PROPOSE_PHASE, smlEVENT_BEFORE_DECISION_PHASE, smlEVENT_BEFORE_APPLY_PHASE, smlEVENT_BEFORE_OUTPUT_PHASE, smlEVENT_BEFORE_PREFERENCE_PHASE, smlEVENT_BEFORE_WM_PHASE } ;
		int afterPhaseEvents[]  = { smlEVENT_AFTER_INPUT_PHASE, smlEVENT_AFTER_PROPOSE_PHASE, smlEVENT_AFTER_DECISION_PHASE, smlEVENT_AFTER_APPLY_PHASE, smlEVENT_AFTER_OUTPUT_PHASE, smlEVENT_AFTER_PREFERENCE_PHASE, smlEVENT_AFTER_WM_PHASE } ;

		int* events = (eventID == smlEVENT_BEFORE_PHASE_EXECUTED) ? beforePhaseEvents : afterPhaseEvents ;

		for (int i = 0 ; i < 7 ; i++)
		{
			// Note that we register with the kernel for a specific phase event, while passing in the more general BEFORE_PHASE_EXECUTED or AFTER_PHASE_EXECUTED
			// as the eventID so the event comes back to us with that label allowing us to pass it directly up to the caller who is expecting an
			// event of that type, not a subevent.
			int phaseEvent = events[i] ;
			SOAR_CALLBACK_TYPE callbackType = SOAR_CALLBACK_TYPE(GetCallbackFromEventID(phaseEvent)) ;
			soar_add_callback (pAgent, callbackType, KernelCallbackStatic, eventID, this, NULL, callbackID.c_str()) ;
		}
	}
}

void KernelCallback::UnregisterWithKernel(int eventID)
{
	if (m_Registered[eventID] != true)
		return ;

	m_Registered[eventID] = false ;

	std::ostringstream buffer;
	buffer << "id_0x" << this << "_evt_" << eventID;
	std::string callbackID = buffer.str() ;

	agent* pAgent = m_pCallbackAgentSML->GetSoarAgent() ;

	if (eventID == smlEVENT_OUTPUT_PHASE_CALLBACK)
	{
		// NLD: proposed fix to bug 1049 
		// remove_output_function(pAgent, "output-link") ;
	}
	else if (eventID != smlEVENT_BEFORE_PHASE_EXECUTED && eventID != smlEVENT_AFTER_PHASE_EXECUTED)
	{
		SOAR_CALLBACK_TYPE callbackType = SOAR_CALLBACK_TYPE(GetCallbackFromEventID(eventID));
		soar_remove_callback(pAgent, callbackType, callbackID.c_str()) ;
	}
	else
	{
		// BEFORE_PHASE and AFTER_PHASE are implemented by listening for all lower level phase events
		int beforePhaseEvents[] = { smlEVENT_BEFORE_INPUT_PHASE, smlEVENT_BEFORE_PROPOSE_PHASE, smlEVENT_BEFORE_DECISION_PHASE, smlEVENT_BEFORE_APPLY_PHASE, smlEVENT_BEFORE_OUTPUT_PHASE, smlEVENT_BEFORE_PREFERENCE_PHASE, smlEVENT_BEFORE_WM_PHASE } ;
		int afterPhaseEvents[]  = { smlEVENT_AFTER_INPUT_PHASE, smlEVENT_AFTER_PROPOSE_PHASE, smlEVENT_AFTER_DECISION_PHASE, smlEVENT_AFTER_APPLY_PHASE, smlEVENT_AFTER_OUTPUT_PHASE, smlEVENT_AFTER_PREFERENCE_PHASE, smlEVENT_AFTER_WM_PHASE } ;

		int* events = (eventID == smlEVENT_BEFORE_PHASE_EXECUTED) ? beforePhaseEvents : afterPhaseEvents ;

		for (int i = 0 ; i < 7 ; i++)
		{
			int phaseEvent = events[i] ;
			SOAR_CALLBACK_TYPE callbackType = SOAR_CALLBACK_TYPE(GetCallbackFromEventID(phaseEvent)) ;
			soar_remove_callback (pAgent, callbackType, callbackID.c_str()) ;
		}
	}
}

