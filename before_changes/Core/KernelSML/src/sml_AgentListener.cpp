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
*     smlEVENT_AFTER_AGENT_CREATED,
*     smlEVENT_BEFORE_AGENT_DESTROYED,
*	  smlEVENT_BEFORE_AGENTS_RUN_STEP,
*     smlEVENT_BEFORE_AGENT_REINITIALIZED,
*     smlEVENT_AFTER_AGENT_REINITIALIZED,
*/
/////////////////////////////////////////////////////////////////

#include "sml_AgentListener.h"

#include "sml_Utils.h"
#include "sml_OutputListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "assert.h"

using namespace sml ;

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

void AgentListener::Init(KernelSML* pKernelSML)
{
	m_pKernelSML = pKernelSML ;
}

// Returns true if this is the first connection listening for this event
bool AgentListener::AddListener(smlAgentEventId eventID, Connection* pConnection)
{
	bool first = EventManager<smlAgentEventId>::BaseAddListener(eventID, pConnection) ;

	//if (first && eventID == smlEVENT_BEFORE_AGENT_DESTROYED)
	//{
	//	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(static_cast<egSKIAgentEventId>(eventID), this) ;
	//}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool AgentListener::RemoveListener(smlAgentEventId eventID, Connection* pConnection)
{
    bool last = EventManager<smlAgentEventId>::BaseRemoveListener(eventID, pConnection) ;

	//// Unregister from the kernel -- except for the two events that this class is internally listening for.
	//if (last && eventID == smlEVENT_BEFORE_AGENT_DESTROYED)
	//{
	//	m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(static_cast<egSKIAgentEventId>(eventID), this) ;
	//}

	return last ;
}

// Called when an event occurs in the kernel
void AgentListener::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
	unused(pCallData) ;
	OnEvent(static_cast<smlAgentEventId>(eventID), pAgentSML) ;
}

//// Called when an "AgentEvent" occurs in gSKI
//void AgentListener::HandleEvent(egSKIAgentEventId eventIdIn, gSKI::Agent* pAgent)
//{
//	smlAgentEventId eventId = static_cast<smlAgentEventId>(eventIdIn);
//
//	AgentSML* pAgentSML = m_pKernelSML->GetAgentSML( pAgent->GetName() ) ;
//	assert(pAgentSML) ;
//
//	OnEvent(eventId, pAgentSML) ;
//}
//
void AgentListener::OnEvent(smlAgentEventId eventID, AgentSML* pAgentSML)
{
	// Pass init-soar events over to the output listener so it can do some cleanup before and after the init-soar
	// Then send them on to everyone else like normal.
	if (eventID == smlEVENT_BEFORE_AGENT_REINITIALIZED || eventID == smlEVENT_AFTER_AGENT_REINITIALIZED)
	{
		// This is a bit clumsy.  I think the reinitialized events should really be sent to the agent not to the agent manager
		// (which is a kernel event) so we need to do this extra lookup stage.  If it was an agent event, we could directly
		// attach it to the agent handler.
		pAgentSML->GetOutputListener()->ReinitializeEvent( eventID ) ;
	}

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlAgentEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	// Pass the agent in the "name" parameter not the "agent" parameter as this is a kernel
	// level event, not an agent level one (because you need to register with the kernel to get "agent created").
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, pAgentSML->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pAgentSML, pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
