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
*       smlEVENT_XML_TRACE_OUTPUT
*/
/////////////////////////////////////////////////////////////////

#include "sml_XMLListener.h"

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_KernelHelpers.h"

#include "assert.h"

using namespace sml ;
using namespace soarxml ;

void XMLListener::Init(KernelSML* pKernelSML, AgentSML* pAgentSML)
{
	m_pKernelSML = pKernelSML ;
	m_EnablePrintCallback = true ;

	SetAgentSML(pAgentSML) ;
}

// Returns true if this is the first connection listening for this event
bool XMLListener::AddListener(smlXMLEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	if (first && eventID == smlEVENT_XML_TRACE_OUTPUT)
	{
		RegisterWithKernel(eventID) ;
	}

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool XMLListener::RemoveListener(smlXMLEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	if (last && eventID == smlEVENT_XML_TRACE_OUTPUT)
	{
		UnregisterWithKernel(eventID) ;
	}

	return last ;
}

void XMLListener::OnKernelEvent(int eventIDIn, AgentSML* pAgentSML, void* pCallDataIn)
{
	// If the print callbacks have been disabled, then don't forward this message
	// on to the clients.  This allows us to use the print callback within the kernel to
	// retrieve information without it appearing in the trace.  (One day we won't need to do this enable/disable game).
	if (!m_EnablePrintCallback)
		return ;

	ElementXML* pXMLTrace = static_cast< ElementXML* >( pCallDataIn );
	smlXMLEventId eventID = static_cast< smlXMLEventId >( eventIDIn );
	
	// Nothing waiting to be sent, so we're done.
	if ( pXMLTrace->GetNumberChildren() == 0 )
		return ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlXMLEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're going to send.
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);

	// NOTE: For this trace message we require that the agent name be the first param here (so we can look it up quickly)
	// and any other changes to the structure of this message should be carefully checked to make sure they don't
	// break the client side code as this path is more brittle and faster than the rest of the message passing.
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_pCallbackAgentSML->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);

	// Add it as a child of this message.  This is just moving a few pointers around, nothing is getting copied.
	// The structure of the message is <sml><command></command><trace></trace></sml>
	pMsg->AddChild(pXMLTrace) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pAgentSML, pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}

// Echo the list of wmes received back to any listeners
void XMLListener::FireInputReceivedEvent(soarxml::ElementXML const* pCommands)
{
	smlXMLEventId eventID = smlEVENT_XML_INPUT_RECEIVED ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlXMLEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// Make a copy of pCommands and send it out.

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're going to send.
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event);
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event);

	// Add the agent parameter and as a side-effect, get a pointer to the <command> tag.  This is an optimization.
	ElementXML_Handle hCommand = pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, m_pCallbackAgentSML->GetName()) ;
	soarxml::ElementXML command(hCommand) ;

	// Copy the list of wmes from the input message over
	int nWmes = pCommands->GetNumberChildren() ;
	for (int i = 0 ; i < nWmes ; i++)
	{
		soarxml::ElementXML wme ;
		pCommands->GetChild(&wme, i) ;

		if (wme.IsTag(sml_Names::kTagWME))
		{
			soarxml::ElementXML* pCopy = wme.MakeCopy() ;
			command.AddChild(pCopy) ;
		}
	}

	// This is important.  We are working with a subpart of pMsg.
	// If we retain ownership of the handle and delete the object
	// it will release the handle...deleting part of our message.
	command.Detach() ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(m_pCallbackAgentSML, pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}

