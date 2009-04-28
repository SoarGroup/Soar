#include <portability.h>

/////////////////////////////////////////////////////////////////
// SystemListener class file.
//
// Author: Doug Pearson, www.threepenny.net
// Date  : October 2004
//
// This class's HandleEvent method is called when
// specific events occur within the kernel:
/*
*      // System events
*      smlEVENT_BEFORE_SHUTDOWN            = 1,
*      smlEVENT_AFTER_CONNECTION_LOST,
*      smlEVENT_BEFORE_RESTART,
*      smlEVENT_AFTER_RESTART,
*      smlEVENT_BEFORE_RHS_FUNCTION_ADDED,
*      smlEVENT_AFTER_RHS_FUNCTION_ADDED,
*      smlEVENT_BEFORE_RHS_FUNCTION_REMOVED,
*      smlEVENT_AFTER_RHS_FUNCTION_REMOVED,
*      smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
*      smlEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*/////////////////////////////////////////////////////////////////

#include "sml_SystemListener.h"

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_KernelSML.h"
#include "sml_Events.h"
#include "sml_AgentSML.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool SystemListener::AddListener(smlSystemEventId eventID, Connection* pConnection)
{
    bool first = EventManager<smlSystemEventId>::BaseAddListener(eventID, pConnection) ;

	/* DJP: System events can't be implemented below SML because the kernel itself is agent based (it has no concept of something larger than an agent)
	if (first)
	{
		m_pKernelSML->GetKernel()->AddSystemListener(eventID, this) ;
	}
	*/

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool SystemListener::RemoveListener(smlSystemEventId eventID, Connection* pConnection)
{
	bool last = EventManager<smlSystemEventId>::BaseRemoveListener(eventID, pConnection) ;

	/* DJP: System events can't be implemented below SML because the kernel itself is agent based (it has no concept of something larger than an agent)
	if (last)
	{
		m_pKernelSML->GetKernel()->RemoveSystemListener(eventID, this) ;
	}
	*/

	return last ;
}

// Initialize this listener
void SystemListener::Init(KernelSML* pKernel)
{
	m_pKernelSML = pKernel ;
}

// Called when an event occurs in the kernel
void SystemListener::OnKernelEvent(int eventIDIn, AgentSML* pAgentSML, void* /*pCallData*/)
{
	// All system events are currently implemented directly in kernel SML so there's
	// no underlying kernel callbacks to connect to.
	// If we ever change that, this is where the callbacks would come in.

	smlSystemEventId eventID = static_cast<smlSystemEventId>(eventIDIn);

	// The system start event can be suppressed by a client.
	// This allows us to run a Soar agent without running the associated simulation
	// (which should be listening for system-start/system-stop events).

	// DJP May 2007: This was an earlier idea about how we'd control systems through SML.  I'm
	// not sure this model (and these events) are relevant anymore.
	if (eventID == smlEVENT_SYSTEM_START)
	{
		bool suppress = m_pKernelSML->IsSystemStartSuppressed() ;

		// The flag is reset forcing the client to repeatedly suppress the system
		// start event each time they wish to run Soar and not generate this event.
		m_pKernelSML->SetSuppressSystemStart(false) ;

		if (suppress)
			return ;
	}

	// Similarly, system stop can be suppressed.
	if (eventID == smlEVENT_SYSTEM_STOP)
	{
		bool suppress = m_pKernelSML->IsSystemStopSuppressed() ;

		// Clear our flags that control this event
		m_pKernelSML->RequireSystemStop(false) ;
		m_pKernelSML->SetSuppressSystemStop(false) ;

		if (suppress)
			return ;
	}

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlSystemEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* eventString = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, eventString) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pAgentSML, pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
