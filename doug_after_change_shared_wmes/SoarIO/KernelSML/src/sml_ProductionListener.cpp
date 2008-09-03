#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

#include "sml_ProductionListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Production.h"
#include "IgSKI_ProductionManager.h"
#include "sml_KernelSML.h"

#include "assert.h"

using namespace sml ;

// Uncomment this symbol to disable print output buffering.
// #define DISABLE_PRINT_OUTPUT_BUFFERING

// Returns true if this is the first connection listening for this event
bool ProductionListener::AddListener(egSKIProductionEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		m_pAgent->GetProductionManager()->AddProductionListener(eventID, this) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool ProductionListener::RemoveListener(egSKIProductionEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		m_pAgent->GetProductionManager()->RemoveProductionListener(eventID, this) ;
	}

	return last ;
}

// Called when a "ProductionEvent" occurs in the kernel
void ProductionListener::HandleEvent(egSKIProductionEventId eventID, gSKI::IAgent* agentPtr, gSKI::IProduction* prod, gSKI::IProductionInstance* match)
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
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

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
