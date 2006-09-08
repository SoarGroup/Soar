#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// XMLListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*       gSKIEVENT_XML_TRACE_OUTPUT
*/
/////////////////////////////////////////////////////////////////

#include "sml_XMLListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"

#include "assert.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool XMLListener::AddListener(egSKIXMLEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first && eventID == gSKIEVENT_XML_TRACE_OUTPUT)
	{
		// Listen for XML trace events within gSKI
		m_pAgent->AddXMLListener(eventID, this); 

		// Register for specific events at which point we'll flush the buffer for this event
		m_pAgentOutputFlusher[eventID-gSKIEVENT_XML_TRACE_OUTPUT] = new AgentOutputFlusher(this, m_pAgent, eventID);
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool XMLListener::RemoveListener(egSKIXMLEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last && eventID == gSKIEVENT_XML_TRACE_OUTPUT)
	{
		// Unregister for the XML trace events in gSKI
		m_pAgent->RemoveXMLListener(eventID, this); 

		// Unregister for the events when we'll flush the buffer
		delete m_pAgentOutputFlusher[eventID-gSKIEVENT_XML_TRACE_OUTPUT] ;
		m_pAgentOutputFlusher[eventID-gSKIEVENT_XML_TRACE_OUTPUT] = NULL ;
	}

	return last ;
}

// Called when a "PrintEvent" occurs in the kernel
void XMLListener::HandleEvent(egSKIXMLEventId eventId, gSKI::Agent* agentPtr, const char* funcType, const char* attOrTag, const char* value)
{
	// We're assuming this is correct in the flush output function, so we should check it here
	assert(agentPtr == m_pAgent);
    unused(agentPtr); // quell warning on VS.NET for release builds

	// The value can be NULL if this is a begin/end tag event.
	assert(funcType) ;
	assert(attOrTag) ;

	// If the print callbacks have been disabled, then don't forward this message
	// on to the clients.  This allows us to use the print callback within the kernel to
	// retrieve information without it appearing in the trace.  (One day we won't need to do this enable/disable game).
	if (!m_EnablePrintCallback)
		return ;

	int nBuffer = eventId - gSKIEVENT_XML_TRACE_OUTPUT ;
	assert(nBuffer >= 0 && nBuffer < kNumberEvents) ;

	// Buffer xml output to be flushed later
	XMLTrace* xmlTrace = &m_BufferedXMLOutput[nBuffer] ;

	// We need to decide what type of operation this is and we'd like to do that
	// fairly efficiently so we'll switch based on the first character.
	char ch = funcType[0] ;

	switch (ch)
	{
	case 'b' : 
		if (strcmp(sml_Names::kFunctionBeginTag, funcType) == 0)
		{
			xmlTrace->BeginTag(attOrTag) ;
		}
		break ;
	case 'e':
		if (strcmp(sml_Names::kFunctionEndTag, funcType) == 0)
		{
			xmlTrace->EndTag(attOrTag) ;
		}
		break ;
	case 'a':
		if (strcmp(sml_Names::kFunctionAddAttribute, funcType) == 0)
		{
			xmlTrace->AddAttribute(attOrTag, value) ;
		}
		break ;
	default:
		// This is an unknown function type
		assert(ch == 'b' || ch == 'e' || ch == 'a') ;
		break ;
	}
}

void XMLListener::FlushOutput(egSKIXMLEventId eventID) 
{
	int buffer = eventID - gSKIEVENT_XML_TRACE_OUTPUT ;

	// Nothing waiting to be sent, so we're done.
	XMLTrace* xmlTrace = &m_BufferedXMLOutput[buffer] ;
	
	if (xmlTrace->IsEmpty())
		return ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIXMLEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're going to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);

	// NOTE: For this trace message we require that the agent name be the first param here (so we can look it up quickly)
	// and any other changes to the structure of this message should be carefully checked to make sure they don't
	// break the client side code as this path is more brittle and faster than the rest of the message passing.
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_pAgent->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);

	// Extract the XML object from the xmlTrace object and
	// add it as a child of this message.  This is just moving a few pointers around, nothing is getting copied.
	// The structure of the message is <sml><command></command><trace></trace></sml>
	ElementXML_Handle xmlHandle = xmlTrace->Detach() ;
	ElementXML* pXMLTrace = new ElementXML(xmlHandle) ;
	pMsg->AddChild(pXMLTrace) ;

	// Clear the XML trace object, completing the flush.
	xmlTrace->Reset() ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}

// Echo the list of wmes received back to any listeners
void XMLListener::FireInputReceivedEvent(ElementXML const* pCommands)
{
	egSKIXMLEventId eventID = gSKIEVENT_XML_INPUT_RECEIVED ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIXMLEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// Make a copy of pCommands and send it out.

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're going to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);

	// Add the agent parameter and as a side-effect, get a pointer to the <command> tag.  This is an optimization.
	ElementXML_Handle hCommand = pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_pAgent->GetName()) ;
	ElementXML command(hCommand) ;

	// Copy the list of wmes from the input message over
	int nWmes = pCommands->GetNumberChildren() ;
	for (int i = 0 ; i < nWmes ; i++)
	{
		ElementXML wme ;
		pCommands->GetChild(&wme, i) ;

		if (wme.IsTag(sml_Names::kTagWME))
		{
			ElementXML* pCopy = wme.MakeCopy() ;
			command.AddChild(pCopy) ;
		}
	}

	// This is important.  We are working with a subpart of pMsg.
	// If we retain ownership of the handle and delete the object
	// it will release the handle...deleting part of our message.
	command.Detach() ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}

