#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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
#include "sml_Connection.h"
#include "sml_TagWme.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "IgSKI_Wme.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_OutputLink.h"
#include "gSKI_AgentManager.h"
#include "IgSKI_WorkingMemory.h"

#ifdef _DEBUG
// Comment this in to debug init-soar and inputwme::update calls
//#define DEBUG_UPDATE
#endif

#ifdef DEBUG_UPDATE
#include "sock_Debug.h"	// For PrintDebugFormat
#endif

#include <vector>

using namespace sml ;

static char const* GetValueType(egSKISymbolType type)
{
	switch (type)
	{
	case gSKI_DOUBLE: return sml_Names::kTypeDouble ;
	case gSKI_INT:	  return sml_Names::kTypeInt ;
	case gSKI_STRING: return sml_Names::kTypeString ;
	case gSKI_OBJECT: return sml_Names::kTypeID ;
	default: return NULL ;
	}
}

// Register for the events that KernelSML itself needs to know about in order to work correctly.
void OutputListener::RegisterForKernelSMLEvents()
{
	// Listen for output callback events so we can send this output over to the clients
	m_Agent->GetOutputLink()->GetOutputMemory()->AddWorkingMemoryListener(gSKIEVENT_OUTPUT_PHASE_CALLBACK, this) ;
}

void OutputListener::UnRegisterForKernelSMLEvents()
{
	m_Agent->GetOutputLink()->GetOutputMemory()->RemoveWorkingMemoryListener(gSKIEVENT_OUTPUT_PHASE_CALLBACK, this) ;
}

// Returns true if this is the first connection listening for this event
bool OutputListener::AddListener(egSKIWorkingMemoryEventId eventID, Connection* pConnection)
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
bool OutputListener::RemoveListener(egSKIWorkingMemoryEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	// For other listeners (AgentListener, KernelListener) we register with the kernel at this point if this is the first
	// listener being added.
	// However, for output we also use this listener to listen on the kernel side for events (to make KernelSML function correctly)
	// so the kernel listener is always in place.  We don't need to add the kernel listener or remove it if all connections stop listening
	// for it (because even if no clients are interested in this event, KernelSML remains interested).

	return last ;
}

void OutputListener::HandleEvent(egSKIWorkingMemoryEventId eventId, gSKI::Agent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist)
{
	unused(change) ;

	if (eventId != gSKIEVENT_OUTPUT_PHASE_CALLBACK)
		return ;

	if (wmelist->GetNumElements() == 0)
		return ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIWorkingMemoryEventId>::GetBegin(eventId, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Output) ;

	// Add the agent parameter and as a side-effect, get a pointer to the <command> tag.  This is an optimization.
	ElementXML_Handle hCommand = pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, agentPtr->GetName()) ;
	ElementXML command(hCommand) ;

	// We are passed a list of all wmes in the transitive closure (TC) of the output link.
	// We need to decide which of these we've already seen before, so we can just send the
	// changes over to the client (rather than sending the entire TC each time).

	// Reset everything in the current list of tags to "not in use".  After we've processed all wmes,
	// any still in this state have been removed.
	for (OutputTimeTagIter iter = m_TimeTags.begin() ; iter != m_TimeTags.end() ; iter++)
	{
		iter->second = false ;
	}

	// Build the list of WME changes
	for(; wmelist->IsValid(); wmelist->Next())
	{
		// Get the next wme
		gSKI::IWme* pWME = wmelist->GetVal();

		long timeTag = pWME->GetTimeTag() ;

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
		TagWme* pTag = new TagWme() ;

		// Look up the type of value this is
		egSKISymbolType type = pWME->GetValue()->GetType() ;
		char const* pValueType = GetValueType(type) ;

		// For additions we send everything
		pTag->SetIdentifier(pWME->GetOwningObject()->GetId()->GetString()) ;
		pTag->SetAttribute(pWME->GetAttribute()->GetString()) ;
		pTag->SetValue(pWME->GetValue()->GetString(), pValueType) ;
		pTag->SetTimeTag(pWME->GetTimeTag()) ;
		pTag->SetActionAdd() ;

		// Add it as a child of the command tag
		command.AddChild(pTag) ;

		// Values retrieved via "GetVal" have to be released.
		// Ah, but not if they come from an Iterator rather than an IteratorWithRelease.
		// At least, it seems like if I call Release here it causes a crash on exit, while if I don't all seems well.
		//pWME->Release();
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

	egSKIWorkingMemoryEventId eventID = gSKIEVENT_OUTPUT_PHASE_CALLBACK ;

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

// Agent event listener (called when soar has been or is about to be re-initialized)
// BADBAD: This shouldn't really be handled in a class called OutputListener.
void OutputListener::HandleEvent(egSKIAgentEventId eventId, gSKI::Agent* agentPtr)
{
	// Before the kernel is re-initialized we have to release all input WMEs.
	// If we don't do this, gSKI will fail to re-initialize the kernel correctly as it
	// assumes that all WMEs are deleted prior to reinitializing the kernel and if we have
	// outstanding references to the objects it has no way to make the deletion happen.
	if (eventId == gSKIEVENT_BEFORE_AGENT_REINITIALIZED)
	{
		AgentSML* pAgent = this->m_KernelSML->GetAgentSML(agentPtr) ;

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML before agent reinitialized received - start.") ;
#endif
		if (pAgent)
		{
			pAgent->ReleaseAllWmes() ;
		}

#ifdef DEBUG_UPDATE
	PrintDebugFormat("AgentSML before agent reinitialized received - end.") ;
#endif
	}

	// After the kernel has been re-initialized we need to send everything on the output link over again
	// when Soar is next run.
	if (eventId == gSKIEVENT_AFTER_AGENT_REINITIALIZED)
	{
		// When the user types init-soar, we need to reset everything, so when the agent is next run
		// we will send over output link information again.  The client also needs to register for this event
		// (which it does automatically) so it can clear out its output tree to match.
		m_TimeTags.clear() ;
	}
}
