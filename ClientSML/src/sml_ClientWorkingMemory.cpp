#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// WorkingMemory class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to represent Soar's working memory.
// We maintain a copy of this on the client so we can just
// send changes over to the kernel.
//
// Basic method is that working memory is stored as a tree
// of Element objects.
// When the client makes a change to the tree, we modify the tree
// and add the change to the list of changes to make to WM.
// At some point, we actually send that list of changes over.
// We should be able to be clever about collapsing changes together
// in the list of deltas (e.g. change value A->B->C can remove the
// A->B change (since it's overwritten by B->C).
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientWorkingMemory.h"
#include "sml_ClientIdentifier.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Connection.h"
#include "sml_ClientStringElement.h"
#include "sml_ClientIntElement.h"
#include "sml_ClientFloatElement.h"
#include "sml_TagWme.h"
#include "sml_StringOps.h"
#include "sml_Errors.h"

#include "sml_EmbeddedConnection.h"	// For access to direct methods
#include "sml_ClientDirect.h"
#include <assert.h>

#include "sock_Debug.h"

using namespace sml ;

WorkingMemory::WorkingMemory()
{
	m_InputLink  = NULL ;
	m_OutputLink = NULL ;
	m_Agent		 = NULL ;
}

WorkingMemory::~WorkingMemory()
{
	delete m_OutputLink ;
	delete m_InputLink ;
}

Connection* WorkingMemory::GetConnection() const
{
	return GetAgent()->GetConnection() ;
}

char const* WorkingMemory::GetAgentName() const
{
	return GetAgent()->GetAgentName() ;
}

// Searches for an identifier object that matches this id.
Identifier*	WorkingMemory::FindIdentifier(char const* pID, bool searchInput, bool searchOutput, int index)
{
	// BADBAD: For better speed we could keep a map of identifiers in use and just look this up.
	Identifier* pMatch = NULL ;

	if (searchInput)
	{
		if (m_InputLink)
			pMatch = m_InputLink->FindIdentifier(pID, index) ;
	}

	if (searchOutput && !pMatch)
	{
		if (m_OutputLink)
			pMatch = m_OutputLink->FindIdentifier(pID, index) ;

		if (!pMatch && !m_OutputOrphans.empty())
			pMatch = FindIdentifierInWmeList(&m_OutputOrphans, pID) ;
	}

	return pMatch ;
}

// Finds the first WME in the list that has the given string as its identifier
WMElement* WorkingMemory::SearchWmeListForID(WmeList* pWmeList, char const* pID, bool deleteFromList)
{
	for (WmeListIter iter = pWmeList->begin() ; iter != pWmeList->end() ; iter++)
	{
		WMElement* pWME = *iter ;
		if (strcmp(pWME->GetIdentifierName(), pID) == 0)
		{
			if (deleteFromList)
				pWmeList->erase(iter) ;
			return pWME ;
		}
	}

	return NULL ;
}

// Finds the first WME that has the identifier as its value
Identifier* WorkingMemory::FindIdentifierInWmeList(WmeList* pWmeList, char const* pID)
{
	for (WmeListIter iter = pWmeList->begin() ; iter != pWmeList->end() ; iter++)
	{
		WMElement* pWME = *iter ;

		if (pWME->IsIdentifier())
		{
			Identifier* pIdentifier = (Identifier*)pWME ;

			// This test includes checking for "this" being a match
			Identifier* pMatch = pIdentifier->FindIdentifier(pID) ;
			return pMatch ;
		}
	}

	return NULL ;
}

// Create a new WME of the appropriate type based on this information.
WMElement* WorkingMemory::CreateWME(Identifier* pParent, char const* pID, char const* pAttribute, char const* pValue, char const* pType, long timeTag)
{
	// Value is an identifier
	if (strcmp(pType, sml_Names::kTypeID) == 0)
		return new Identifier(GetAgent(), pParent, pID, pAttribute, pValue, timeTag) ;

	// Value is a string
	if (strcmp(pType, sml_Names::kTypeString) == 0)
		return new StringElement(GetAgent(), pParent, pID, pAttribute, pValue, timeTag) ;

	// Value is an int
	if (strcmp(pType, sml_Names::kTypeInt) == 0)
	{
		int value = atoi(pValue) ;
		return new IntElement(GetAgent(), pParent, pID, pAttribute, value, timeTag) ;
	}

	// Value is a float
	if (strcmp(pType, sml_Names::kTypeDouble) == 0)
	{
		double value = atof(pValue) ;
		return new FloatElement(GetAgent(), pParent, pID, pAttribute, value, timeTag) ;
	}

	return NULL ;
}

void WorkingMemory::RecordAddition(WMElement* pWME)
{
	// For additions, the delta list does not take ownership of the wme.
	m_OutputDeltaList.AddWME(pWME) ;

	// Mark this wme as having just been added (in case the client would prefer
	// to walk the tree).
	pWME->SetJustAdded(true) ;
}

void WorkingMemory::RecordDeletion(WMElement* pWME)
{
	// This list takes ownership of the deleted wme.
	// When the item is removed from the delta list it will be deleted.
	m_OutputDeltaList.RemoveWME(pWME) ;
}

// Clear the delta list and also reset all state flags.
void WorkingMemory::ClearOutputLinkChanges()
{
	// Clear the list, deleting any WMEs that it owns
	m_OutputDeltaList.Clear(true) ;

	// We only maintain this information for values on the output link
	// as the client knows what's happening on the input link (presumably)
	// as it is controlling the creation of those objects.
	if (m_OutputLink)
	{
		// Reset the information about how the output link just changed.
		// This is definitely being maintained.
		m_OutputLink->ClearJustAdded() ;
		m_OutputLink->ClearChildrenModified() ;
	}
}

/*************************************************************
* @brief This function is called when an output wme has been added.
*
* @param pWmeXML	The output WME being added
* @param tracing	True if generating debug output
*************************************************************/
bool WorkingMemory::ReceivedOutputAddition(ElementXML* pWmeXML, bool tracing)
{
	// We're adding structure to the output link
	char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// These IDs will be kernel side ids (e.g. "I3" not "i3")
	char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
	char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
	char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
	char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// These will be kernel side time tags (e.g. +5 not -7)

	// Set the default value
	if (!pType)
		pType = sml_Names::kTypeString ;

	// Check we got everything we need
	if (!pID || !pAttribute || !pValue || !pTimeTag)
		return false ;

	if (tracing)
	{
		PrintDebugFormat("Received output wme: %s ^%s %s (time tag %s)", pID, pAttribute, pValue, pTimeTag) ;
	}

	long timeTag = atoi(pTimeTag) ;

	// Find the parent wme that we're adding this new wme to
	// (Actually, there can be multiple WMEs that have this identifier
	//  as its value, but any one will do because the true parent is the
	//  identifier symbol which is the same for any identifiers).
	Identifier* pParent = FindIdentifier(pID, false, true) ;
	WMElement* pAddWme = NULL ;

	if (pParent)
	{
		// Create a client side wme object to match the output wme and add it to
		// our tree of objects.
		pAddWme = CreateWME(pParent, pID, pAttribute, pValue, pType, timeTag) ;
		if (pAddWme)
		{
			pParent->AddChild(pAddWme) ;

			// Make a record that this wme was added so we can alert the client to this change.
			RecordAddition(pAddWme) ;
		}
		else
		{
			PrintDebugFormat("Unable to create an output wme -- type was not recognized") ;
			GetAgent()->SetDetailedError(Error::kOutputError, "Unable to create an output wme -- type was not recognized") ;
		}
	}
	else
	{
		// See if this is the output-link itself (we want to keep a handle to that specially)
		if (!m_OutputLink && IsStringEqualIgnoreCase(pAttribute, sml_Names::kOutputLinkName))
		{
			m_OutputLink = new Identifier(GetAgent(), pValue, timeTag) ;
		} else
		{
			// If we reach here we've received output which is out of order (e.g. (Y ^att value) before (X ^att Y))
			// so there's no parent to connect it to.  We'll create the wme, keep it on a special list of orphans
			// and try to reconnect it later.
			pAddWme = CreateWME(NULL, pID, pAttribute, pValue, pType, timeTag) ;

			if (tracing)
				PrintDebugFormat("Received output wme (orphaned): %s ^%s %s (time tag %s)", pID, pAttribute, pValue, pTimeTag) ;

			if (pAddWme)
				m_OutputOrphans.push_back(pAddWme) ;
		}
	}

	// If we have an output wme still waiting to be connected to its parent
	// and we get in a new wme that is creating an identifier see if they match up.
	if (pAddWme && pAddWme->IsIdentifier() && !m_OutputOrphans.empty())
	{
		TryToAttachOrphanedChildren(pAddWme->ConvertToIdentifier()) ;
	}

	return true ;
}

/*************************************************************
* @brief Some output WMEs will come to us "out of order".
*		 That's to say, a child of an identifier appears before
*		 the identifier (e.g. (X ^name me) before (Y ^person X)).
*		 This function searches the list of wmes that haven't been
*		 attached to an identifier yet and attaches them together
*		 (if the identifier strings match).
*		 By the end of a single output message all children should have
*		 been attached (and no longer be orphans).
*
* @param pPossibleParent	The identifier that may have orphaned children to attach.
*************************************************************/
bool WorkingMemory::TryToAttachOrphanedChildren(Identifier* pPossibleParent)
{
	if (m_OutputOrphans.empty())
		return false ;

	bool deleteFromList = true ;
	WMElement* pWme = SearchWmeListForID(&m_OutputOrphans, pPossibleParent->GetValueAsString(), deleteFromList) ;

	while (pWme)
	{
		pWme->SetParent(pPossibleParent) ;
		pPossibleParent->AddChild(pWme) ;

		if (this->GetAgent()->GetKernel()->IsTracingCommunications())
			PrintDebugFormat("Adding orphaned child to this ID: %s ^%s %s (time tag %d)", pWme->GetIdentifierName(), pWme->GetAttribute(), pWme->GetValueAsString(), pWme->GetTimeTag()) ;

		// If the wme being attached is itself an identifier, we have to check in turn to see if it has any orphaned children
		if (pWme->IsIdentifier())
			TryToAttachOrphanedChildren(pWme->ConvertToIdentifier()) ;

		// Make a record that this wme was added so we can alert the client to this change.
		RecordAddition(pWme) ;

		pWme = SearchWmeListForID(&m_OutputOrphans, pPossibleParent->GetValueAsString(), deleteFromList) ;
	}

	return true ;
}

/*************************************************************
* @brief This function is called when an output wme has been removed.
*
* @param pWmeXML	The output WME being removed
* @param tracing	True if generating debug output
*************************************************************/
bool WorkingMemory::ReceivedOutputRemoval(ElementXML* pWmeXML, bool tracing)
{
	// We're removing structure from the output link
	char const* pTimeTag = pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// These will usually be kernel side time tags (e.g. +5 not -7)

	long timeTag = atoi(pTimeTag) ;

	// If we have no output link we can't delete things from it.
	if (!m_OutputLink)
		return false ;

	// Find the WME which matches this tag.
	// This may fail as we may have removed the parent of this WME already in the series of remove commands.
	WMElement* pWME = m_OutputLink->FindFromTimeTag(timeTag) ;

	// Delete the WME
	if (pWME && pWME->GetParent())
	{
		if (tracing)
			PrintDebugFormat("Removing output wme: time tag %s", pTimeTag) ;

		pWME->GetParent()->RemoveChild(pWME) ;

		// Make a record that this wme was removed, so we can tell the client about it.
		// This recording will also involve deleting the wme.
		RecordDeletion(pWME) ;
	}
	else
	{
		if (tracing)
			PrintDebugFormat("Remove output wme request (seems to already be gone): time tag %s", pTimeTag) ;
		return false ;
	}

	return true ;
}

/*************************************************************
* @brief This function is called when output is received
*		 from the Soar kernel.
*
* @param pIncoming	The output command (list of wmes added/removed from output link)
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
bool WorkingMemory::ReceivedOutput(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;	// No need to reply

#ifdef _DEBUG
	char * pMsgText = pIncoming->GetCommandTag()->GenerateXMLString(true) ;
#endif

	// Get the command tag which contains the list of wmes
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;

	int nChildren = pCommand->GetNumberChildren() ;

	ElementXML wmeXML(NULL) ;
	ElementXML* pWmeXML = &wmeXML ;

	bool ok = true ;

	// Make sure the output orphans list is empty
	// We'll use this to store output wmes that have no parent identifier yet
	// (this is rare but can happen if the kernel generates wmes in an unusual order)
	m_OutputOrphans.clear() ;

	bool tracing = this->GetAgent()->GetKernel()->IsTracingCommunications() ;

	for (int i = 0 ; i < nChildren ; i++)
	{
		pCommand->GetChild(&wmeXML, i) ;

		// Ignore tags that aren't wmes.
		if (!pWmeXML->IsTag(sml_Names::kTagWME))
			continue ;

		// Find out if this is an add or a remove
		char const* pAction = pWmeXML->GetAttribute(sml_Names::kWME_Action) ;

		if (!pAction)
			continue ;

		bool add = IsStringEqual(pAction, sml_Names::kValueAdd) ;
		bool remove = IsStringEqual(pAction, sml_Names::kValueRemove) ;

		if (add)
		{
			ok = ReceivedOutputAddition(pWmeXML, tracing) && ok ;
		}
		else if (remove)
		{
			ok = ReceivedOutputRemoval(pWmeXML, tracing) && ok ;
		}
	}

	// Check that we managed to reconnect all of the orphaned wmes
	if (!m_OutputOrphans.empty())
	{
		ok = false ;

		if (tracing)
			PrintDebugFormat("Some output WMEs have no matching parent IDs -- they are ophans.  This is bad.") ;

		GetAgent()->SetDetailedError(Error::kOutputError, "Some output WMEs have no matching parent IDs -- they are ophans.  This is bad.") ;
		m_OutputOrphans.clear() ;	// Have to discard them.
	}

	// Let anyone listening for the output notification know that output was just received.
	GetAgent()->FireOutputNotification() ;

	// Call any handlers registered to listen for output
	// (This is one way to retrieve output).
	// We do this at the end of the output handler so that all of the children of the wme
	// have been received and recorded before we call the handler.
	if (GetAgent()->IsRegisteredForOutputEvent() && m_OutputLink)
	{
		int nWmes = m_OutputDeltaList.GetSize() ;
		for (int i = 0 ; i < nWmes ; i++)
		{
			WMElement* pWme = m_OutputDeltaList.GetDeltaWME(i)->getWME() ;

			if (pWme->GetParent() == NULL || pWme->GetParent()->GetIdentifierSymbol() == NULL)
				continue ;

			// We're only looking for top-level wmes on the output link so check if our identifier's symbol
			// matches the output link's value
			if (strcmp(pWme->GetParent()->GetIdentifierSymbol(), m_OutputLink->GetValueAsString()) == 0)
			{
				// Notify anyone who's listening to this event
				GetAgent()->ReceivedOutputEvent(pWme) ;
			}
		}

		// This is potentially wrong, but I think we should now clear the list of changes.
		// If someone is working with the callback handler model it would be very hard for them to call this
		// themselves and without this call somewhere we'll get mutiple calls to the same handlers.
		ClearOutputLinkChanges() ;
	}

#ifdef _DEBUG
		ElementXML::DeleteString(pMsgText) ;
#endif

	// Returns false if any of the adds/removes fails
	return ok ;
}

/*************************************************************
* @brief Returns the id object for the input link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* WorkingMemory::GetInputLink()
{
	if (!m_InputLink)
	{
		AnalyzeXML response ;

		if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetInputLink, GetAgentName()))
		{
			m_InputLink = new Identifier(GetAgent(), response.GetResultString(), GenerateTimeTag()) ;

#ifdef SML_DIRECT
			if (GetConnection()->IsDirectConnection())
			{
				Direct_WorkingMemory_Handle wm = ((EmbeddedConnection*)GetConnection())->DirectGetWorkingMemory(GetAgent()->GetAgentName(), true) ;
				Direct_WMObject_Handle wmobject = ((EmbeddedConnection*)GetConnection())->DirectGetRoot(GetAgent()->GetAgentName(), true) ;
				m_InputLink->SetWorkingMemoryHandle(wm) ;
				m_InputLink->SetWMObjectHandle(wmobject) ;
			}
#endif
		}
	}

	return m_InputLink ;
}

/*************************************************************
* @brief This method is used to update this client's representation
*		 of the input link to match what is currently on the agent's
*		 input link.
*
*		 NOTE: This is the reverse of how a client normally uses the input link
*		 but can be useful for tools that wish to debug or monitor changes in the input link.
*
*		 NOTE: If two clients try to modify the input link at once we don't
*		 make any guarantees about what will or won't work.
*************************************************************/
bool WorkingMemory::SynchronizeInputLink()
{
	// Not supported for direct connections
	if (GetConnection()->IsDirectConnection())
		return false ;

	AnalyzeXML response ;

	// Call to the kernel to get the current state of the input link
	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetAllInput, GetAgentName()) ;
	
	if (!ok)
		return false ;

#ifdef _DEBUG
	char* pStr = response.GenerateXMLString(true) ;
#endif

	// Erase the existing input link and create a new representation from scratch
	delete m_InputLink ;
	m_InputLink = NULL ;

	GetInputLink() ;

	// Get the result tag which contains the list of wmes
	ElementXML const* pMain = response.GetResultTag() ;

	int nChildren = pMain->GetNumberChildren() ;

	ElementXML wmeXML(NULL) ;
	ElementXML* pWmeXML = &wmeXML ;

	bool tracing = this->GetAgent()->GetKernel()->IsTracingCommunications() ;

	for (int i = 0 ; i < nChildren ; i++)
	{
		pMain->GetChild(&wmeXML, i) ;

		// Ignore tags that aren't wmes.
		if (!pWmeXML->IsTag(sml_Names::kTagWME))
			continue ;

		// Get the wme information
		char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// These IDs will be kernel side ids (e.g. "I3" not "i3")
		char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
		char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
		char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
		char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// These will be kernel side time tags (e.g. +5 not -7)

		// Set the default value
		if (!pType)
			pType = sml_Names::kTypeString ;

		// Check we got everything we need
		if (!pID || !pAttribute || !pValue || !pTimeTag)
			continue ;

		if (tracing)
		{
			PrintDebugFormat("Received input wme: %s ^%s %s (time tag %s)", pID, pAttribute, pValue, pTimeTag) ;
		}

		long timeTag = atoi(pTimeTag) ;

		// Find the parent wme that we're adding this new wme to
		// (Actually, there can be multiple WMEs that have this identifier
		//  as its value, but any one will do because the true parent is the
		//  identifier symbol which is the same for any identifiers).
		Identifier* pParent = FindIdentifier(pID, true, false) ;
		WMElement* pAddWme = NULL ;

		if (pParent)
		{
			// Create a client side wme object to match the input wme and add it to
			// our tree of objects.
			pAddWme = CreateWME(pParent, pID, pAttribute, pValue, pType, timeTag) ;
			if (pAddWme)
			{
				pParent->AddChild(pAddWme) ;
			}
			else
			{
				PrintDebugFormat("Unable to create an input wme -- type was not recognized") ;
				GetAgent()->SetDetailedError(Error::kOutputError, "Unable to create an input wme -- type was not recognized") ;
			}
		}
		else
		{
			if (tracing)
				PrintDebugFormat("Received input wme (orphaned): %s ^%s %s (time tag %s)", pID, pAttribute, pValue, pTimeTag) ;
		}
	}

#ifdef _DEBUG
	response.DeleteString(pStr) ;
#endif

	// Returns false if had any errors
	return ok ;
}

/*************************************************************
* @brief This method is used to update this client's representation
*		 of the output link to match what is currently on the agent's
*		 output link.
*		 Calling this method recreates the entire output link tree on the
*		 client side, invalidating any existing pointers.
*
*		 NOTE: Calling this method shouldn't generally be necessary as the output link
*		 structures in the client are usually automatically kept in synch with the agent.
*		 It's here in case a client connects to an existing kernel and agent
*		 and wants to get up to date on the current state of the output link.
*************************************************************/
bool WorkingMemory::SynchronizeOutputLink()
{
	// Not supported for direct connections
	if (GetConnection()->IsDirectConnection())
		return false ;

	AnalyzeXML incoming ;
	ElementXML response ;

	// Call to the kernel to get the current state of the output link
	bool ok = GetConnection()->SendAgentCommand(&incoming, sml_Names::kCommand_GetAllOutput, GetAgentName()) ;
	
	if (!ok)
		return false ;

#ifdef _DEBUG
	char* pStr = incoming.GenerateXMLString(true) ;
#endif

	// Erase the existing output link and create a new representation from scratch
	delete m_OutputLink ;
	m_OutputLink = NULL ;

	// Process the new list of output -- as if it had just occurred in the agent (when in fact we're just synching with it)
	ok = ReceivedOutput(&incoming, &response) ;

#ifdef _DEBUG
	incoming.DeleteString(pStr) ;
#endif

	// Returns false if had any errors
	return ok ;
}

/*************************************************************
* @brief Returns the id object for the output link.
*		 The agent retains ownership of this object.
*
* The value will be NULL until the first output phase has executed
* in the Soar kernel and we've received the information from the kernel.
*************************************************************/
Identifier* WorkingMemory::GetOutputLink()
{
	return m_OutputLink ;
}

/*************************************************************
* @brief Builds a new WME that has a string value and schedules
*		 it for addition to Soar's input link.
*
*		 The agent retains ownership of this object.
*		 The returned object is valid until the caller
*		 deletes the parent identifier.
*		 The WME is not added to Soar's input link until the
*		 client calls "Commit".
*************************************************************/
StringElement* WorkingMemory::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	StringElement* pWME = new StringElement(GetAgent(), parent, parent->GetValueAsString(), pAttribute, pValue, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

#ifdef SML_DIRECT
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;
	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Add the wme immediately and return the new object.
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddWME_String(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pAttribute, pValue) ;
		pWME->SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return pWME ;
	}
#endif

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an int as its value.
*************************************************************/
IntElement* WorkingMemory::CreateIntWME(Identifier* parent, char const* pAttribute, int value)
{
	IntElement* pWME = new IntElement(GetAgent(), parent, parent->GetValueAsString(), pAttribute, value, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

#ifdef SML_DIRECT
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;
	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Add the wme immediately and return the new object.
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddWME_Int(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pAttribute, value) ;
		pWME->SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return pWME ;
	}
#endif

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 a floating point value.
*************************************************************/
FloatElement* WorkingMemory::CreateFloatWME(Identifier* parent, char const* pAttribute, double value)
{
	FloatElement* pWME = new FloatElement(GetAgent(), parent, parent->GetValueAsString(), pAttribute, value, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

#ifdef SML_DIRECT
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;
	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Add the wme immediately and return the new object.
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddWME_Double(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pAttribute, value) ;
		pWME->SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return pWME ;
	}
#endif

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Update the value of an existing WME.
*		 The value is not actually sent to the kernel
*		 until "Commit" is called.
*************************************************************/
void WorkingMemory::UpdateString(StringElement* pWME, char const* pValue)
{
	if (!pWME || !pValue)
		return ;

	// Changing the value logically is a remove and then an add

	// Get the tag of the value to remove
	long removeTimeTag = pWME->GetTimeTag() ;

	// Change the value and the time tag (this is equivalent to us deleting the old object
	// and then creating a new one).
	pWME->SetValue(pValue) ;
	pWME->GenerateNewTimeTag() ;

#ifdef SML_DIRECT
	IdentifierSymbol* parent = pWME->GetIdentifier() ;
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;

	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Remove the existing wme.  This releases the gSKI WME as well, invalidating our WMEHandle which we will replace in a moment.
		((EmbeddedConnection*)GetConnection())->DirectRemoveWME(wm, pWME->GetWMEHandle(), removeTimeTag) ;

		// Add the new value immediately
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddWME_String(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pWME->GetAttribute(), pValue) ;
		pWME->SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return ;
	}
#endif

	// Add it to the list of changes that need to be sent to Soar.
	m_DeltaList.UpdateWME(removeTimeTag, pWME) ;
}

void WorkingMemory::UpdateInt(IntElement* pWME, int value)
{
	if (!pWME)
		return ;

	// Changing the value logically is a remove and then an add

	// Get the tag of the value to remove
	long removeTimeTag = pWME->GetTimeTag() ;

	// Change the value and the time tag (this is equivalent to us deleting the old object
	// and then creating a new one).
	pWME->SetValue(value) ;
	pWME->GenerateNewTimeTag() ;

#ifdef SML_DIRECT
	IdentifierSymbol* parent = pWME->GetIdentifier() ;
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;

	if (GetConnection()->IsDirectConnection())
	{
		// Remove the existing wme.  This releases the gSKI WME as well, invalidating our WMEHandle which we will replace in a moment.
		((EmbeddedConnection*)GetConnection())->DirectRemoveWME(wm, pWME->GetWMEHandle(), removeTimeTag) ;

		// Add the new value immediately
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddWME_Int(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pWME->GetAttribute(), value) ;
		pWME->SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return ;
	}
#endif

	// Add it to the list of changes that need to be sent to Soar.
	m_DeltaList.UpdateWME(removeTimeTag, pWME) ;
}

void WorkingMemory::UpdateFloat(FloatElement* pWME, double value)
{
	if (!pWME)
		return ;

	// Changing the value logically is a remove and then an add

	// Get the tag of the value to remove
	long removeTimeTag = pWME->GetTimeTag() ;

	// Change the value and the time tag (this is equivalent to us deleting the old object
	// and then creating a new one).
	pWME->SetValue(value) ;
	pWME->GenerateNewTimeTag() ;

#ifdef SML_DIRECT
	IdentifierSymbol* parent = pWME->GetIdentifier() ;
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;

	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Remove the existing wme.  This releases the gSKI WME as well, invalidating our WMEHandle which we will replace in a moment.
		((EmbeddedConnection*)GetConnection())->DirectRemoveWME(wm, pWME->GetWMEHandle(), removeTimeTag) ;

		// Add the new value immediately
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddWME_Double(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pWME->GetAttribute(), value) ;
		pWME->SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return ;
	}
#endif

	// Add it to the list of changes that need to be sent to Soar.
	m_DeltaList.UpdateWME(removeTimeTag, pWME) ;
}

/*************************************************************
* @brief Create a new ID for use by the client.
*		 The kernel will assign its own ids when the WME
*		 is really added, so it needs to know to map back and forth.
*		 To make that mapping clear, we generate IDs that use
*		 a lower case first letter, ensuring they aren't mistaken
*		 for IDs the kernel creates (although this should just help
*		 humans keep track of what's going on--shouldn't matter to
*		 the system).
*************************************************************/
void WorkingMemory::GenerateNewID(char const* pAttribute, std::string* pID)
{
	int id = GetAgent()->GetKernel()->GenerateNextID() ;

	char buffer[kMinBufferSize] ;
	Int2String(id, buffer, sizeof(buffer)) ;

	// we'll start our ids with lower case so we can distinguish them
	// from soar id's.  We'll take the first letter of the attribute,
	// much as soar does, but always add a unique number to the back,
	// so the choice of initial letter really isn't important.
	char letter = pAttribute[0] ;

	// Convert to lowercase
	if (letter >= 'A' || letter <= 'Z')
		letter = letter - 'A' + 'a' ;

	// Make sure we got a letter here (just in case)
	if (letter < 'a' || letter > 'z')
		letter = 'a' ;

	// Return the result
	*pID = letter ;
	pID->append(buffer) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an identifier as its value.
*************************************************************/
Identifier* WorkingMemory::CreateIdWME(Identifier* parent, char const* pAttribute)
{
	// Create a new, unique id (e.g. "i3").  This id will be mapped to a different id
	// in the kernel.
	std::string id ;
	GenerateNewID(pAttribute, &id) ;

	Identifier* pWME = new Identifier(GetAgent(), parent, parent->GetValueAsString(), pAttribute, id.c_str(), GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

#ifdef SML_DIRECT
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;

	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Add the wme immediately and return the new object.
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectAddID(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pAttribute) ;
		Direct_WMObject_Handle wmobject = ((EmbeddedConnection*)GetConnection())->DirectGetThisWMObject(wm, wme) ;
		pWME->SetWMEHandle(wme) ;
		pWME->SetWMObjectHandle(wmobject) ;

		// Return immediately, without adding it to the commit list.
		return pWME ;
	}
#endif

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Creates a new WME that has an identifier as its value.
*		 The value in this case is the same as an existing identifier.
*		 This allows us to create a graph rather than a tree.
*************************************************************/
Identifier*	WorkingMemory::CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue)
{
	// Look up the id from the existing identifier
	std::string id = pSharedValue->GetValueAsString() ;

	// Create the new WME with the same value
	Identifier* pWME = new Identifier(GetAgent(), parent, parent->GetValueAsString(), pAttribute, pSharedValue, GenerateTimeTag()) ;

	// Record that the identifer owns this new WME
	parent->AddChild(pWME) ;

#ifdef SML_DIRECT
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;
	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Add the wme immediately and return the new object.
		Direct_WME_Handle wme = ((EmbeddedConnection*)GetConnection())->DirectLinkID(wm, parent->GetWMObjectHandle(), pWME->GetTimeTag(), pAttribute, pSharedValue->GetWMObjectHandle()) ;
		Direct_WMObject_Handle wmobject = ((EmbeddedConnection*)GetConnection())->DirectGetThisWMObject(wm, wme) ;
		pWME->SetWMEHandle(wme) ;
		pWME->SetWMObjectHandle(wmobject) ;

		// Return immediately, without adding it to the commit list.
		return pWME ;
	}
#endif

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}

/*************************************************************
* @brief Schedules a WME from deletion from the input link and removes
*		 it from the client's model of working memory.
*
*		 The caller should not access this WME after calling
*		 DestroyWME().
*		 The WME is not removed from the input link until
*		 the client calls "Commit"
*************************************************************/
bool WorkingMemory::DestroyWME(WMElement* pWME)
{
	IdentifierSymbol* parent = pWME->GetIdentifier() ;

	// We can't delete top level WMEs (e.g. the WME that represents the input-link's ID)
	// Those are architecturally created.
	if (parent == NULL)
		return false ;

	// The parent identifier no longer owns this WME
	parent->RemoveChild(pWME) ;

#ifdef SML_DIRECT
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;

	if (GetConnection()->IsDirectConnection() && wm)
	{
		// Remove the wme immediately, which also invalidates the handle, so clear it immediately
		// (or we'll crash in the destructor for pWME just below)
		((EmbeddedConnection*)GetConnection())->DirectRemoveWME(wm, pWME->GetWMEHandle(), pWME->GetTimeTag()) ;
		pWME->SetWMEHandle(0) ;

		// Return immediately, without adding it to the commit list
		delete pWME ;

		return true ;
	}
#endif

	// Add it to the list of changes to send to Soar.
	m_DeltaList.RemoveWME(pWME->GetTimeTag()) ;

	// Now we can delete it
	delete pWME ;

	return true ;
}

/*************************************************************
* @brief Generate a unique integer id (a time tag)
*************************************************************/
long WorkingMemory::GenerateTimeTag()
{
	// We use negative tags on the client, so we don't mistake them
	// for ones from the real kernel.
	int tag = GetAgent()->GetKernel()->GenerateNextTimeTag() ;

	return tag ;
}

/*************************************************************
* @brief Returns true if wmes have been added and not yet committed.
*************************************************************/
bool WorkingMemory::IsCommitRequired()
{
	return (m_DeltaList.GetSize() != 0) ;
}

/*************************************************************
* @brief Send the most recent list of changes to working memory
*		 over to the kernel.
*************************************************************/
bool WorkingMemory::Commit()
{
	int deltas = m_DeltaList.GetSize() ;

	// If nothing has changed, we have no work to do.
	// This allows us to call Commit() multiple times without causing problems
	// as later calls will be ignored if the current set of changes has been sent already.
	if (deltas == 0)
		return true ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = GetConnection()->CreateSMLCommand(sml_Names::kCommand_Input) ;

	// Add the agent parameter and as a side-effect, get a pointer to the <command> tag.  This is an optimization.
	ElementXML_Handle hCommand = GetConnection()->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, GetAgentName()) ;
	ElementXML command(hCommand) ;

	// Build the list of WME changes
	for (int i = 0 ; i < deltas ; i++)
	{
		// Get the next change
		TagWme* pDelta = m_DeltaList.GetDelta(i) ;

		// Add it as a child of the command tag
		// (the command takes ownership of the delta)
		command.AddChild(pDelta) ;
	}

	// This is important.  We are working with a subpart of pMsg.
	// If we retain ownership of the handle and delete the object
	// it will release the handle...deleting part of our message.
	command.Detach() ;

	// We have transfered the list of deltas over to pMsg
	// so we need to clear the list, but not delete the tags that it contains.
	m_DeltaList.Clear(false) ;

#ifdef _DEBUG
	// Generate a text form of the XML so we can look at it in the debugger.
	char* pStr = pMsg->GenerateXMLString(true) ;
	pMsg->DeleteString(pStr) ;
#endif

	// Send the message
	AnalyzeXML response ;
	bool ok = GetConnection()->SendMessageGetResponse(&response, pMsg) ;

	// Clean up
	delete pMsg ;

	return ok ;
}

/*************************************************************
* @brief Resend the complete input link to the kernel
*		 and remove our output link structures.
*		 We do this when the user issues an "init-soar" event.
*		 There should be no reason for the client to call this method directly.
*************************************************************/
void WorkingMemory::Refresh()
{
	if (m_InputLink)
	{
		AnalyzeXML response ;

		// Start by getting the input link identifier
		if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetInputLink, GetAgentName()))
		{
			// Technically we should reset the value of the input link identifier itself, but it should never
			// change value (since it's architecturally created) and also adding a method to set the identifier value
			// is a bad idea if we only need it here.
			std::string result = response.GetResultString() ;

	#ifdef SML_DIRECT
			// Get the current input link object ids
			if (GetConnection()->IsDirectConnection())
			{
				m_InputLink->ClearAllWMObjectHandles() ;

				Direct_WorkingMemory_Handle wm = ((EmbeddedConnection*)GetConnection())->DirectGetWorkingMemory(GetAgent()->GetAgentName(), true) ;
				Direct_WMObject_Handle wmobject = ((EmbeddedConnection*)GetConnection())->DirectGetRoot(GetAgent()->GetAgentName(), true) ;
				m_InputLink->SetWorkingMemoryHandle(wm) ;
				m_InputLink->SetWMObjectHandle(wmobject) ;
			}
	#endif
		}

		m_InputLink->Refresh() ;

		// Send the new input link over to the kernel
		Commit() ;
	}

	// Remove the output link tree (it will be rebuilt when the agent next runs).
	if (m_OutputLink)
	{
		delete m_OutputLink ;
		m_OutputLink = NULL ;
	}
}

