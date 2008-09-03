#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// PrintListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*       gSKIEVENT_PRINT
*/
/////////////////////////////////////////////////////////////////

#include "sml_PrintListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"

#include "assert.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool PrintListener::AddListener(egSKIPrintEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first)
	{
		// Echo events are SML only.  Everything else is implemented in gSKI
		if (eventID != gSKIEVENT_ECHO)
			m_pAgent->AddPrintListener(eventID, this); 

		// Register for specific events at which point we'll flush the buffer for this event
		m_pAgentOutputFlusher[eventID-gSKIEVENT_FIRST_PRINT_EVENT] = new AgentOutputFlusher(this, m_pAgent, eventID);
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool PrintListener::RemoveListener(egSKIPrintEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last)
	{
		// Echo events are SML only.  Everything else is implemented in gSKI
		if (eventID != gSKIEVENT_ECHO)
			m_pAgent->RemovePrintListener(eventID, this); 

		// Unregister for the events when we'll flush the buffer
		delete m_pAgentOutputFlusher[eventID-gSKIEVENT_FIRST_PRINT_EVENT] ;
		m_pAgentOutputFlusher[eventID-gSKIEVENT_FIRST_PRINT_EVENT] = NULL ;
	}

	return last ;
}

// Called when a "PrintEvent" occurs in the kernel
void PrintListener::HandleEvent(egSKIPrintEventId eventID, gSKI::IAgent* agentPtr, const char* msg) 
{
	unused(eventID);
	unused(agentPtr);

	// We're assuming this is correct in the flush output function, so we should check it here
	assert(agentPtr == m_pAgent);

	// If the print callbacks have been disabled, then don't forward this message
	// on to the clients.  This allows us to use the print callback within the kernel to
	// retrieve information without it appearing in the trace.  (One day we won't need to do this enable/disable game).
	if (!m_EnablePrintCallback && eventID == gSKIEVENT_PRINT)
		return ;

	int nBuffer = eventID - gSKIEVENT_FIRST_PRINT_EVENT ;
	assert(nBuffer >= 0 && nBuffer < kNumberPrintEvents) ;

	// Buffer print output to be flushed later
	m_BufferedPrintOutput[nBuffer] += msg;
}

void PrintListener::FlushOutput(Connection* pSourceConnection, egSKIPrintEventId eventID) 
{
	int buffer = eventID - gSKIEVENT_FIRST_PRINT_EVENT ;

	// Nothing waiting to be sent, so we're done.
	if (!m_BufferedPrintOutput[buffer].size())
		return ;

	ConnectionListIter connectionIter = GetBegin(eventID);

	// Nobody is listenening for this event.  That's an error as we should unregister from the kernel in that case.
	if (connectionIter == GetEnd(eventID))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Send the message out
	AnalyzeXML response ;

	// For non-echo events, just send it normally
	if (eventID != gSKIEVENT_ECHO)
	{
		// Build the SML message we're going to send.
		ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);
		pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_pAgent->GetName());
		pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);
		pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamMessage, m_BufferedPrintOutput[buffer].c_str());

		// Send the message out
		AnalyzeXML response ;
		SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

		// Clean up
		delete pMsg ;
	}
	else
	{
		// For echo events build up the message on a connection by connection basis.
		// This allows us to tell a sender if they were the cause of a given message so a client
		// can filter out echoes from commands they originated.
		while (connectionIter != GetEnd(eventID))
		{
			pConnection = *connectionIter ;
			connectionIter++ ;

			// Build the SML message we're going to send.
			ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);
			pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_pAgent->GetName());
			pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);
			pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamMessage, m_BufferedPrintOutput[buffer].c_str());
			pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamSelf, (pSourceConnection == pConnection) ? sml_Names::kTrue : sml_Names::kFalse) ;

			#ifdef _DEBUG
				// Generate a text form of the XML so we can look at it in the debugger.
				char* pStr = pMsg->GenerateXMLString(true) ;
			#endif

			// It would be faster to just send a message here without waiting for a response
			// but that could produce incorrect behavior if the client expects to act *during*
			// the event that we're notifying them about (e.g. notification that we're in the input phase).
			pConnection->SendMessageGetResponse(&response, pMsg) ;

			#ifdef _DEBUG
				// Clean up the string form of the message.
				pMsg->DeleteString(pStr) ;
			#endif

			// Clean up
			delete pMsg ;
		}
	}

	// Clear the buffer now that it's been sent
	m_BufferedPrintOutput[buffer].clear();
}
