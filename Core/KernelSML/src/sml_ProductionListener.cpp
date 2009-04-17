#include <portability.h>

/////////////////////////////////////////////////////////////////
// ProductionListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*      smlEVENT_AFTER_PRODUCTION_ADDED,
*      smlEVENT_BEFORE_PRODUCTION_REMOVED,
*     //smlEVENT_BEFORE_PRODUCTION_FIRED,
*      smlEVENT_AFTER_PRODUCTION_FIRED,
*      smlEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

#include "sml_ProductionListener.h"

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "KernelHeaders.h"

#include "assert.h"

using namespace sml ;

void ProductionListener::Init(KernelSML* pKernelSML, AgentSML* pAgentSML)
{
	m_pKernelSML = pKernelSML ;
	SetAgentSML(pAgentSML) ;
}

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

// Returns true if this is the first connection listening for this event
bool ProductionListener::AddListener(smlProductionEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		this->RegisterWithKernel(eventID) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool ProductionListener::RemoveListener(smlProductionEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		this->UnregisterWithKernel(eventID) ;
	}

	return last ;
}

void ProductionListener::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
	// TODO: all event handlers should be doing this:
	assert(IsProductionEventID(eventID)) ;

	smlProductionEventId smlEventID = smlProductionEventId(eventID) ;	

	std::string productionName ;

	// We're either passed a production* or an instantiation* depending on the type of event
	production* p = 0;
	if (smlEventID == smlEVENT_AFTER_PRODUCTION_ADDED || smlEventID == smlEVENT_BEFORE_PRODUCTION_REMOVED)
	{
		p = (production*) pCallData ;
	}
	else
	{
		instantiation* inst = (instantiation*) pCallData ;
		assert(inst) ;
		p = inst->prod ;
	}

	assert(p) ;
	assert(p->name->sc.name) ;
	productionName = p->name->sc.name ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlProductionEventId>::GetBegin(smlProductionEventId(eventID), &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentSML->GetName()) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamName, productionName.c_str()) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pAgentSML, pConnection, pMsg, &response, connectionIter, GetEnd(smlProductionEventId(eventID))) ;

	// Clean up
	delete pMsg ;
}

