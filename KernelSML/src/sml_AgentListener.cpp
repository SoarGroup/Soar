#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

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
#include "sml_OutputListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "gSKI_AgentManager.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "assert.h"

using namespace sml ;

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

void AgentListener::Init(KernelSML* pKernelSML)
{
	m_pKernelSML = pKernelSML ;

	// Listen for "before" init-soar events (we need to know when these happen so we can release all WMEs on the input link, otherwise gSKI will fail to re-init the kernel correctly.)
	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(gSKIEVENT_BEFORE_AGENT_REINITIALIZED, this, false) ;

	// Listen for "after" init-soar events (we need to know when these happen so we can resend the output link over to the client)
	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(gSKIEVENT_AFTER_AGENT_REINITIALIZED, this, false) ;
}

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

	// Unregister from the kernel -- except for the two events that this class is internally listening for.
	if (last && eventID != gSKIEVENT_BEFORE_AGENT_REINITIALIZED && eventID != gSKIEVENT_AFTER_AGENT_REINITIALIZED)
	{
		m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(eventID, this) ;
	}

	return last ;
}

// Called when an "AgentEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIAgentEventId eventID, gSKI::Agent* agentPtr)
{
	// Pass init-soar events over to the output listener so it can do some cleanup before and after the init-soar
	// Then send them on to everyone else like normal.
	if (eventID == gSKIEVENT_BEFORE_AGENT_REINITIALIZED || eventID == gSKIEVENT_AFTER_AGENT_REINITIALIZED)
	{
		// This is a bit clumsy.  I think the reinitialized events should really be sent to the agent not to the agent manager
		// (which is a kernel event) so we need to do this extra lookup stage.  If it was an agent event, we could directly
		// attach it to the agent handler.
		AgentSML* pAgent = m_pKernelSML->GetAgentSML(agentPtr) ;
		assert(pAgent) ;
		pAgent->GetOutputListener()->HandleEvent(eventID, agentPtr) ;
	}

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIAgentEventId>::GetBegin(eventID, &connectionIter))
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
