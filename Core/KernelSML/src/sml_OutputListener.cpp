#include <portability.h>

/////////////////////////////////////////////////////////////////
// OutputListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// the agent adds wmes to the output link.
//
/////////////////////////////////////////////////////////////////

#include "sml_OutputListener.h"

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_TagWme.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "KernelHeaders.h"

#ifdef _DEBUG
// Comment this in to debug init-soar and inputwme::update calls
//#define DEBUG_UPDATE
#endif

#include <vector>

using namespace sml ;

TagWme* OutputListener::CreateTagIOWme( AgentSML* pAgent, io_wme* wme ) 
{
	// Create the wme tag
	TagWme* pTag = new TagWme() ;

	// Look up the type of value this is
	char const* pValueType = AgentSML::GetValueType( wme->value->sc.common_symbol_info.symbol_type ) ;

	// For additions we send everything
	pTag->SetIdentifier( symbol_to_string( pAgent->GetSoarAgent(), wme->id, true, 0, 0 ) ) ;
	pTag->SetAttribute( symbol_to_string( pAgent->GetSoarAgent(), wme->attr, true, 0, 0 ) ) ;
	pTag->SetValue( symbol_to_string( pAgent->GetSoarAgent(), wme->value, true, 0, 0 ), pValueType ) ;

	long clientTimetag = pAgent->GetClientTimetag( wme->timetag );
	if ( clientTimetag < 0 )
	{
		// valid client timetag
		pTag->SetTimeTag( clientTimetag ) ;
	}
	else
	{
		pTag->SetTimeTag( wme->timetag ) ;
	}

	pTag->SetActionAdd() ;

	return pTag ;
}

TagWme* OutputListener::CreateTagWme( AgentSML* pAgent, wme* wme )
{
	// Create the wme tag
	TagWme* pTag = new TagWme() ;

	// Look up the type of value this is
	char const* pValueType = AgentSML::GetValueType( wme->value->sc.common_symbol_info.symbol_type ) ;

	// For additions we send everything
	pTag->SetIdentifier( symbol_to_string( pAgent->GetSoarAgent(), wme->id, true, 0, 0 ) ) ;
	pTag->SetAttribute( symbol_to_string( pAgent->GetSoarAgent(), wme->attr, true, 0, 0 ) ) ;
	pTag->SetValue( symbol_to_string( pAgent->GetSoarAgent(), wme->value, true, 0, 0 ), pValueType ) ;

	long clientTimetag = pAgent->GetClientTimetag( wme->timetag );
	if ( clientTimetag < 0 )
	{
		// valid client timetag
		pTag->SetTimeTag( clientTimetag ) ;
	}
	else
	{
		pTag->SetTimeTag( wme->timetag ) ;
	}

	pTag->SetActionAdd() ;

	return pTag ;
}

void OutputListener::Init(KernelSML* pKernelSML, AgentSML* pAgentSML)
{
	m_KernelSML = pKernelSML ;
	SetAgentSML(pAgentSML) ;
}

// Called when an event occurs in the kernel
void OutputListener::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
	output_call_info* oinfo = static_cast<output_call_info*>(pCallData);
	int outputMode = oinfo->mode;

	io_wme* pWmes = oinfo->outputs ;
	SendOutput((smlWorkingMemoryEventId)eventID, pAgentSML, outputMode, pWmes) ;
}

// OutputMode is one of:
// #define ADDED_OUTPUT_COMMAND 1
// #define MODIFIED_OUTPUT_COMMAND 2
// #define REMOVED_OUTPUT_COMMAND 3

void OutputListener::SendOutput(smlWorkingMemoryEventId eventId, AgentSML* pAgentSML, int outputMode, io_wme* io_wmelist)
{
	unused(outputMode) ;

	if (eventId != smlEVENT_OUTPUT_PHASE_CALLBACK)
		return ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlWorkingMemoryEventId>::GetBegin(eventId, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Build the SML message we're doing to send.
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Output) ;

	// Add the agent parameter and as a side-effect, get a pointer to the <command> tag.  This is an optimization.
	ElementXML_Handle hCommand = pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, pAgentSML->GetName()) ;
	soarxml::ElementXML command(hCommand) ;

	// We are passed a list of all wmes in the transitive closure (TC) of the output link.
	// We need to decide which of these we've already seen before, so we can just send the
	// changes over to the client (rather than sending the entire TC each time).

	// Reset everything in the current list of tags to "not in use".  After we've processed all wmes,
	// any still in this state have been removed.
	for (OutputTimeTagIter iter = m_TimeTags.begin() ; iter != m_TimeTags.end() ; iter++)
	{
		iter->second = false ;
	}

	// Start with the output link itself
	// The kernel seems to only output this itself during link initialization
	// and we might be connecting up after that.  Including it twice will not hurt on the client side.
	output_link *ol = pAgentSML->GetSoarAgent()->existing_output_links ;	// This is technically a list but we only support one output link
	TagWme* pOutputLinkWme = OutputListener::CreateTagWme( pAgentSML, ol->link_wme) ;
	command.AddChild(pOutputLinkWme) ;

	for (io_wme* wme = io_wmelist ; wme != NIL ; wme = wme->next)
	{
		// Build the list of WME changes
		long timeTag = wme->timetag ;

		// See if we've already sent this wme to the client
		OutputTimeTagIter iter = m_TimeTags.find(timeTag) ;

		if (iter != m_TimeTags.end())
		{
			// This is a time tag we've already sent over, so mark it as still being in use
			iter->second = true ;
			continue ;
		}

		// If we reach here we need to send the wme to the client and add it to the list
		// of tags currently in use.
		m_TimeTags[timeTag] = true ;

		// Create the wme tag
		TagWme* pTag = CreateTagIOWme( pAgentSML, wme ) ;

		// Add it as a child of the command tag
		command.AddChild(pTag) ;
	}

	// At this point we check the list of time tags and any which are not marked as "in use" must
	// have been deleted, so we need to send them over to the client as deletions.
	for (OutputTimeTagIter iter = m_TimeTags.begin() ; iter != m_TimeTags.end() ;)
	{
		// Ignore time tags that are still in use.
		if (iter->second == true)
		{
			// We have to do manual iteration because we're deleting elements
			// as we go and that invalidates iterators if we're not careful.
			iter++ ;
			continue ;
		}

		long timeTag = iter->first ;

		// Create the wme tag
		TagWme* pTag = new TagWme() ;

		// For deletions we just send the time tag
		pTag->SetTimeTag(timeTag) ;
		pTag->SetActionRemove() ;

		// Add it as a child of the command tag
		command.AddChild(pTag) ;

		// Delete the entry from the time tag map
		m_TimeTags.erase(iter++);
	}

	// This is important.  We are working with a subpart of pMsg.
	// If we retain ownership of the handle and delete the object
	// it will release the handle...deleting part of our message.
	command.Detach() ;

	smlWorkingMemoryEventId eventID = smlEVENT_OUTPUT_PHASE_CALLBACK ;

#ifdef _DEBUG
	// Convert the XML to a string so we can look at it in the debugger
	char *pStr = pMsg->GenerateXMLString(true) ;
#endif

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

#ifdef _DEBUG
	pMsg->DeleteString(pStr) ;
#endif

	// Clean up
	delete pMsg ;
}

// Register for the events that KernelSML itself needs to know about in order to work correctly.
void OutputListener::RegisterForKernelSMLEvents()
{
	// Listen for output callback events so we can send this output over to the clients
	this->RegisterWithKernel(smlEVENT_OUTPUT_PHASE_CALLBACK) ;
	//m_Agent->GetOutputLink()->GetOutputMemory()->AddWorkingMemoryListener(smlEVENT_OUTPUT_PHASE_CALLBACK, this) ;
}

void OutputListener::UnRegisterForKernelSMLEvents()
{
	this->UnregisterWithKernel(smlEVENT_OUTPUT_PHASE_CALLBACK) ;
	//m_Agent->GetOutputLink()->GetOutputMemory()->RemoveWorkingMemoryListener(smlEVENT_OUTPUT_PHASE_CALLBACK, this) ;
}

// Returns true if this is the first connection listening for this event
bool OutputListener::AddListener(smlWorkingMemoryEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	// For other listeners (AgentListener, KernelListener) we register with the kernel at this point if this is the first
	// listener being added.
	// However, for output we also use this listener to listen on the kernel side for events (to make KernelSML function correctly)
	// so the kernel listener is always in place.  We don't need to add the kernel listener or remove it if all connections stop listening
	// for it (because even if no clients are interested in this event, KernelSML remains interested).

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool OutputListener::RemoveListener(smlWorkingMemoryEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	// For other listeners (AgentListener, KernelListener) we register with the kernel at this point if this is the first
	// listener being added.
	// However, for output we also use this listener to listen on the kernel side for events (to make KernelSML function correctly)
	// so the kernel listener is always in place.  We don't need to add the kernel listener or remove it if all connections stop listening
	// for it (because even if no clients are interested in this event, KernelSML remains interested).

	return last ;
}

// Agent event listener (called when soar has been or is about to be re-initialized)
// BADBAD: This shouldn't really be handled in a class called OutputListener.
void OutputListener::ReinitializeEvent(smlAgentEventId eventId) 
{
	// Before the kernel is re-initialized we have to release all input WMEs.
	// If we don't do this, gSKI will fail to re-initialize the kernel correctly as it
	// assumes that all WMEs are deleted prior to reinitializing the kernel and if we have
	// outstanding references to the objects it has no way to make the deletion happen.
	if (eventId == smlEVENT_BEFORE_AGENT_REINITIALIZED)
	{
#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("OutputListener before agent reinitialized received - start.") ;
#endif
		if (m_pCallbackAgentSML)
		{
			m_pCallbackAgentSML->ReleaseAllWmes() ;
		}

#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("OutputListener before agent reinitialized received - end.") ;
#endif
	}

	// After the kernel has been re-initialized we need to send everything on the output link over again
	// when Soar is next run.
	if (eventId == smlEVENT_AFTER_AGENT_REINITIALIZED)
	{
		// When the user types init-soar, we need to reset everything, so when the agent is next run
		// we will send over output link information again.  The client also needs to register for this event
		// (which it does automatically) so it can clear out its output tree to match.
		m_TimeTags.clear() ;
	}
}
