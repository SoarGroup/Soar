#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// KernelListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
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

#include "sml_KernelListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool KernelListener::AddListener(egSKIEventId eventID, Connection* pConnection)
{
	bool first = EventManager::AddListener(eventID, pConnection) ;

	if (first)
	{
		if (IsSystemEvent(eventID))
			KernelSML::GetKernelSML()->GetKernel()->AddSystemListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool KernelListener::RemoveListener(egSKIEventId eventID, Connection* pConnection)
{
	bool last = EventManager::RemoveListener(eventID, pConnection) ;

	if (last)
	{
		if (IsSystemEvent(eventID))
			KernelSML::GetKernelSML()->GetKernel()->RemoveSystemListener(eventID, this) ;
	}

	return last ;
}

// Called when a "SystemEvent" occurs in the kernel
void KernelListener::HandleEvent(egSKIEventId eventID, gSKI::IKernel* kernel)
{
	// We don't send the kernel over because we only support a single kernel object in SML
	unused(kernel) ;

	ConnectionListIter connectionIter = GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char event[kMinBufferSize] ;
	Int2String(eventID, event, sizeof(event)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
	pMsg->DeleteString(pStr) ;
#endif

	// Send this message to all listeners
	ConnectionListIter end = GetEnd(eventID) ;

	AnalyzeXML response ;

	while (connectionIter != end)
	{
		pConnection = *connectionIter ;

		// It would be faster to just send a message here without waiting for a response
		// but that could produce incorrect behavior if the client expects to act *during*
		// the event that we're notifying them about (e.g. notification that we're in the input phase).
		pConnection->SendMessageGetResponse(&response, pMsg) ;

		connectionIter++ ;
	}

	// Clean up
	delete pMsg ;
}
