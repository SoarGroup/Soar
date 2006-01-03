#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// AgentListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*     gSKIEVENT_AFTER_AGENT_CREATED,
*     gSKIEVENT_BEFORE_AGENT_DESTROYED,
*	  gSKIEVENT_BEFORE_AGENTS_RUN_STEP,
*     gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
*     gSKIEVENT_AFTER_AGENT_REINITIALIZED,
*/
/////////////////////////////////////////////////////////////////

#include "sml_AgentListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_AgentManager.h"
#include "sml_KernelSML.h"

#include "assert.h"

using namespace sml ;

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

// Returns true if this is the first connection listening for this event
bool AgentListener::AddListener(egSKIAgentEventId eventID, Connection* pConnection)
{
	bool first = EventManager<egSKIAgentEventId>::BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool AgentListener::RemoveListener(egSKIAgentEventId eventID, Connection* pConnection)
{
    bool last = EventManager<egSKIAgentEventId>::BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(eventID, this) ;
	}

	return last ;
}

// Called when an "AgentEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIAgentEventId eventID, gSKI::IAgent* agentPtr)
{
	ConnectionListIter connectionIter = EventManager<egSKIAgentEventId>::GetBegin(eventID) ;

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == EventManager<egSKIAgentEventId>::GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	// Pass the agent in the "name" parameter not the "agent" parameter as this is a kernel
	// level event, not an agent level one (because you need to register with the kernel to get "agent created").
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	if (agentPtr) pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
