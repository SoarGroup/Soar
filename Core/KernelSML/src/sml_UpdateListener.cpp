#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// UpdateListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/////////////////////////////////////////////////////////////////

#include "sml_UpdateListener.h"
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
bool UpdateListener::AddListener(egSKIUpdateEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool UpdateListener::RemoveListener(egSKIUpdateEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	return last ;
}

// Called when a "RunEvent" occurs in the kernel
void UpdateListener::HandleEvent(egSKIUpdateEventId eventID, int runFlags)
{
	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIUpdateEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Convert phase to a string
	char runStr[kMinBufferSize] ;
	Int2String(runFlags, runStr, sizeof(runStr)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamValue, runStr) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
