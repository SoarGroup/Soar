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
#include "sml_Connection.h"
#include "sml_ClientStringElement.h"
#include "sml_TagWme.h"

using namespace sml ;

WorkingMemory::WorkingMemory()
{
	m_InputLink = NULL ;
	m_Agent		= NULL ;
	m_TimeTagCounter = 0 ;
}

WorkingMemory::~WorkingMemory()
{
	delete m_InputLink ;
}

Connection* WorkingMemory::GetConnection() const
{
	return GetAgent()->GetConnection() ;
}

char const* WorkingMemory::GetAgentName() const
{
	return GetAgent()->GetName() ;
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
			m_InputLink = new Identifier(GetAgent(), response.GetResultString()) ;
		}
	}

	return m_InputLink ;
}

/*************************************************************
* @brief Builds a new WME that has a string value.
*		 The agent retains ownership of this object.
*		 The returned object is valid until the caller
*		 deletes the parent identifier.
*************************************************************/
StringElement* WorkingMemory::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	StringElement* pWME = parent->CreateStringWME(pAttribute, pValue) ;

	// Add it to our list of changes that need to be sent to Soar.
	m_DeltaList.AddWME(pWME) ;

	return pWME ;
}


/*************************************************************
* @brief Generate a unique integer id (a time tag)
*************************************************************/
long WorkingMemory::GenerateTimeTag()
{
	// We use negative tags on the client, so we don't mistake them
	// for ones from the kernel.
	m_TimeTagCounter-- ;

	return m_TimeTagCounter ;
}

/*************************************************************
* @brief Send the most recent list of changes to working memory
*		 over to the kernel.
*************************************************************/
bool WorkingMemory::Commit()
{
	int deltas = m_DeltaList.GetSize() ;

	// If nothing has changed, we have no work to do.
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
		Delta* pDelta = m_DeltaList.GetDelta(i) ;
		WMElement* pElement = pDelta->GetWME() ;

		// Create the wme tag
		TagWme* pWme = new TagWme() ;

		if (pDelta->isAdd())
		{
			// For adds we send everything
			pWme->SetIdentifier(pElement->GetIdentifier()->GetValueAsString()) ;
			pWme->SetAttribute(pElement->GetAttribute()) ;
			pWme->SetValue(pElement->GetValueAsString(), pElement->GetValueType()) ;
			pWme->SetTimeTag(pElement->GetTimeTag()) ;
			pWme->SetActionAdd() ;
		}
		else
		{
			// For removes, we just use the time tag
			pWme->SetTimeTag(pElement->GetTimeTag()) ;
			pWme->SetActionRemove() ;
		}

		command.AddChild(pWme) ;
	}

#ifdef _DEBUG
	std::string msg = pMsg->GenerateXMLString(true) ;
#endif

	// Send the message
	AnalyzeXML response ;
	bool ok = GetConnection()->SendMessageGetResponse(&response, pMsg) ;

	return ok ;
}
