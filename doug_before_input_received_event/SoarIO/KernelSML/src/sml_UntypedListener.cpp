#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// UntypedListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : June 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent.
//
/////////////////////////////////////////////////////////////////

#include "sml_UntypedListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Production.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "assert.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool UntypedListener::AddListener(egSKIUntypedEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool UntypedListener::RemoveListener(egSKIUntypedEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	return last ;
}

// Called when a event occurs in the kernel
void UntypedListener::HandleEvent(egSKIUntypedEventId eventID, void* data)
{
	ConnectionListIter connectionIter = GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

	// How the data is sent is event specific (for this untyped event)
	switch (eventID)
	{
	case gSKIEVENT_EDIT_PRODUCTION:
	{
		char const* pProduction = (char const*)data ;
		if (pProduction)
			pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamValue, pProduction) ;
		break ;
	}
	default:
		break ;
	}

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
