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
*     // Agent events
*     @li gSKIEVENT_BEFORE_SMALLEST_STEP
*     @li gSKIEVENT_AFTER_SMALLEST_STEP
*     @li gSKIEVENT_BEFORE_ELABORATION_CYCLE
*     @li gSKIEVENT_AFTER_ELABORATION_CYCLE
*     @li gSKIEVENT_BEFORE_PHASE_EXECUTED
*     @li gSKIEVENT_AFTER_PHASE_EXECUTED
*     @li gSKIEVENT_BEFORE_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_INTERRUPT
*     @li gSKIEVENT_BEFORE_RUNNING
*     @li gSKIEVENT_AFTER_RUNNING
*
*      // Agent manager
*      gSKIEVENT_AFTER_AGENT_CREATED,
*      gSKIEVENT_BEFORE_AGENT_DESTROYED,
*      gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
*      gSKIEVENT_AFTER_AGENT_REINITIALIZED,
*
*      // Production Manager
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

#include "sml_AgentListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Production.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"
#include "sml_KernelSML.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool AgentListener::AddListener(egSKIEventId eventID, Connection* pConnection)
{
	bool first = EventManager::AddListener(eventID, pConnection) ;

	if (first)
	{
		if (IsRunEvent(eventID))
			m_Agent->AddRunListener(eventID, this) ;
		else if (IsProductionEvent(eventID))
			m_Agent->GetProductionManager()->AddProductionListener(eventID, this) ;
		else if (IsAgentEvent(eventID))
			KernelSML::GetKernelSML()->GetKernel()->GetAgentManager()->AddAgentListener(eventID, this) ;
		else if (IsPrintEvent(eventID))					// added by voigtjr
			m_Agent->AddPrintListener(eventID, this); 
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool AgentListener::RemoveListener(egSKIEventId eventID, Connection* pConnection)
{
	bool last = EventManager::RemoveListener(eventID, pConnection) ;

	if (last)
	{
		if (IsRunEvent(eventID))
			m_Agent->RemoveRunListener(eventID, this) ;
		else if (IsProductionEvent(eventID))
			m_Agent->GetProductionManager()->RemoveProductionListener(eventID, this) ;
		else if (IsAgentEvent(eventID))
			KernelSML::GetKernelSML()->GetKernel()->GetAgentManager()->RemoveAgentListener(eventID, this) ;
		else if (IsPrintEvent(eventID))					// added by voigtjr
			m_Agent->RemovePrintListener(eventID, this); 
	}

	return last ;
}

// Called when a "RunEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr, egSKIPhaseType phase)
{
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

	// Convert phase to a string
	char phaseStr[kMinBufferSize] ;
	Int2String(phase, phaseStr, sizeof(phaseStr)) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamPhase, phaseStr) ;

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

		// BUGBUG: The decision to wait for a response (which is necessary) causes another problem.
		// If we create an agent through the embedded connection, register for an event and do
		// the same from a remote connection and then run those agents from the remote connection.
		// The problem is that the embedded connection may not be listening for incoming messages at all
		// (the one that issues the run command is guaranteed to be waiting because it's inside its own "sendmessagegetresponse" call).
		// So this send never gets a response because that connection is not listening for any incoming commands.
		// For example, the cli-test program has this problem.  It's waiting for the user to type commands while in the background
		// the remote connection runs Soar, causing events (possibly) to fire back to the embedded connection which is waiting
		// in the keyboard handler and never sees the message so it never replies.
		pConnection->SendMessageGetResponse(&response, pMsg) ;

		connectionIter++ ;
	}

	// Clean up
	delete pMsg ;
}

// Called when an "AgentEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr)
{
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
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
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

// Called when a "ProductionEvent" occurs in the kernel
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr, gSKI::IProduction* prod, gSKI::IProductionInstance* match)
{
	// This class isn't implemented in gSKI yet.
	unused(match) ;

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
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, prod->GetName()) ;

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

// Called when a "PrintEvent" occurs in the kernel (I think) (voigtjr)
void AgentListener::HandleEvent(egSKIEventId eventID, gSKI::IAgent* agentPtr, const char* msg) {

	ConnectionListIter connectionIter = GetBegin(eventID);

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(eventID))
		return;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char event[kMinBufferSize];
	Int2String(eventID, event, sizeof(event));

	// Build the SML message we're going to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamMessage, msg);

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true);
	pMsg->DeleteString(pStr);
#endif

	// Send this message to all listeners
	ConnectionListIter end = GetEnd(eventID);

	AnalyzeXML response;

	while (connectionIter != end)
	{
		pConnection = *connectionIter;

		// It would be faster to just send a message here without waiting for a response
		// but that could produce incorrect behavior if the client expects to act *during*
		// the event that we're notifying them about (e.g. notification that we're in the input phase).
		pConnection->SendMessageGetResponse(&response, pMsg);

		connectionIter++;
	}

	// Clean up
	delete pMsg;
}

