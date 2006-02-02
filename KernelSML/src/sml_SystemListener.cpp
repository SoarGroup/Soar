#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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
*      gSKIEVENT_BEFORE_SHUTDOWN            = 1,
*      gSKIEVENT_AFTER_CONNECTION_LOST,
*      gSKIEVENT_BEFORE_RESTART,
*      gSKIEVENT_AFTER_RESTART,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_ADDED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*/////////////////////////////////////////////////////////////////

#include "sml_SystemListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"
#include "sml_Events.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool SystemListener::AddListener(egSKISystemEventId eventID, Connection* pConnection)
{
    bool first = EventManager<egSKISystemEventId>::BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		m_pKernelSML->GetKernel()->AddSystemListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool SystemListener::RemoveListener(egSKISystemEventId eventID, Connection* pConnection)
{
	bool last = EventManager<egSKISystemEventId>::BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		m_pKernelSML->GetKernel()->RemoveSystemListener(eventID, this) ;
	}

	return last ;
}

// Initialize this listener
void SystemListener::Init(KernelSML* pKernel)
{
	m_pKernelSML = pKernel ;
}

// Called when a "SystemEvent" occurs in the kernel
void SystemListener::HandleEvent(egSKISystemEventId eventID, gSKI::IKernel* kernel)
{
	// We don't send the kernel over because we only support a single kernel object in SML
	unused(kernel) ;

	// The system start event can be suppressed by a client.
	// This allows us to run a Soar agent without running the associated simulation
	// (which should be listening for system-start/system-stop events).
	if (eventID == gSKIEVENT_SYSTEM_START)
	{
		bool suppress = m_pKernelSML->IsSystemStartSuppressed() ;

		// The flag is reset forcing the client to repeatedly suppress the system
		// start event each time they wish to run Soar and not generate this event.
		m_pKernelSML->SetSuppressSystemStart(false) ;

		if (suppress)
			return ;
	}

	// Similarly, system stop can be suppressed.
	if (eventID == gSKIEVENT_SYSTEM_STOP)
	{
		bool suppress = m_pKernelSML->IsSystemStopSuppressed() ;

		// Clear our flags that control this event
		m_pKernelSML->RequireSystemStop(false) ;
		m_pKernelSML->SetSuppressSystemStop(false) ;

		if (suppress)
			return ;
	}

	// BADBAD: voigtjr VS2005 workaround
	if (!EventManager<egSKISystemEventId>::HasEvents(eventID)) {
		return;
	}

	ConnectionListIter connectionIter = EventManager<egSKISystemEventId>::GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == EventManager<egSKISystemEventId>::GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
