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
#include "IgSKI_Wme.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_WMObject.h"

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

void OutputListener::HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist)
{
	unused(change) ;

	if (eventId != gSKIEVENT_OUTPUT_PHASE_CALLBACK)
		return ;

	if (m_StopOnOutput)
	{
		// If we've been asked to interrupt Soar when we receive output, then do so now.
		// I'm not really clear on the correct parameters for stopping Soar here and what impact the choices will have.
		agentPtr->Interrupt(gSKI_STOP_AFTER_SMALLEST_STEP, gSKI_STOP_BY_RETURNING) ;

		// Clear the flag, so we don't keep trying to stop.  The caller can reset it before the next run if they wish.
		m_StopOnOutput = false ;
	}

	ConnectionListIter connectionIter = GetBegin(gSKIEVENT_OUTPUT_PHASE_CALLBACK) ;

	// Whoops--nobody is listening to this event.
	// We can't unregister it from the kernel, or "stop on output" would stop working.
	if (connectionIter == GetEnd(gSKIEVENT_OUTPUT_PHASE_CALLBACK))
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
		// The returned value points to the next item in the list

		// voigtjr: this is not legal in gcc (nor defined in the sgi stl standard)
		//iter = m_TimeTags.erase(iter) ;
		// TODO: inefficient, fix:
		m_TimeTags.erase(iter);
		iter = m_TimeTags.begin();

	}

	// This is important.  We are working with a subpart of pMsg.
	// If we retain ownership of the handle and delete the object
	// it will release the handle...deleting part of our message.
	command.Detach() ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
	pMsg->DeleteString(pStr) ;
#endif

	// Send this message to all listeners
	ConnectionListIter end = GetEnd(gSKIEVENT_OUTPUT_PHASE_CALLBACK) ;

	while (connectionIter != end)
	{
		pConnection = *connectionIter ;

		// When Soar is sending output, we don't want to wait for a response (which contains no information anyway)
		// in case the caller is slow to respond to the incoming message.
		pConnection->SendMessage(pMsg) ;

		connectionIter++ ;
	}

	// Clean up
	delete pMsg ;
}
