/////////////////////////////////////////////////////////////////
// SoarId class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID value.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientIdentifier.h"
#include "sml_ClientStringElement.h"
#include "sml_Connection.h"

using namespace sml ;

// This version is only needed at the top of the tree (e.g. the input link)
Identifier::Identifier(Agent* pAgent, char const* pIdentifier, long timeTag) : WMElement(pAgent, NULL, NULL, timeTag)
{
	m_Identifier = pIdentifier ;
}

// The normal case (where there is a parent id)
Identifier::Identifier(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pIdentifier, long timeTag) : WMElement(pAgent, pID, pAttributeName, timeTag)
{
	m_Identifier = pIdentifier ;
}

Identifier::~Identifier(void)
{
	// We own all of these children, so delete them when we are deleted.
	for (ChildrenIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;
		delete pWME ;
	}
}

/*************************************************************
* @brief Returns the n-th WME that has the given attribute
*		 and this identifier as its parent (or NULL).
*
* @param pAttribute		The name of the attribute to match
* @param index			0 based index of values for this attribute
*					   (> 0 only needed for multi-valued attributes)
*************************************************************/
WMElement* Identifier::FindByAttribute(char const* pAttribute, int index) const
{
	for (ChildrenConstIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		if (strcmpi(pWME->GetAttribute(), pAttribute) == 0)
		{
			if (index == 0)
				return pWME ;
			index-- ;
		}
	}

	return NULL ;
}

/*************************************************************
* @brief Searches for a child of this identifier that has pID as
*		 its value (and is itself an identifier).
*		 (The search is recursive over all children).
*
* @param pIncoming	The id to look for (e.g. "O4" -- kernel side or "p3" -- client side)
*************************************************************/
Identifier* Identifier::FindIdentifier(char const* pID) const
{
	if (strcmp(this->GetValueAsString(), pID) == 0)
		return (Identifier*)this ;

	// Go through each child in turn and if it's an identifier search its children for a matching id.
	for (ChildrenConstIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		// If this is an identifer, we search deeper for the match
		if (pWME->IsIdentifier())
		{
			Identifier* pMatch = ((Identifier*)pWME)->FindIdentifier(pID) ;

			if (pMatch)
				return pMatch ;
		}
	}

	return NULL ;
}

/*************************************************************
* @brief Clear the "just added" flag for this and all children
*		 (The search is recursive over all children).
*************************************************************/
void Identifier::ClearJustAdded()
{
	this->SetJustAdded(false) ;

	// Go through each child in turn
	for (ChildrenConstIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		pWME->SetJustAdded(false) ;

		// If this is an identifer, we clear all of its children.
		if (pWME->IsIdentifier())
			((Identifier*)pWME)->ClearJustAdded() ;
	}
}

/*************************************************************
* @brief Clear the "just added" flag for this and all children
*		 (The search is recursive over all children).
*************************************************************/
void Identifier::ClearChildrenModified()
{
	this->m_AreChildrenModified = false ;

	// Go through each child in turn
	for (ChildrenConstIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		// If this is an identifer, we clear it and all of its children.
		if (pWME->IsIdentifier())
			((Identifier*)pWME)->ClearChildrenModified() ;
	}
}

/*************************************************************
* @brief Searches for a child of this identifier that has the given
*		 time tag.
*		 (The search is recursive over all children).
*
* @param timeTag	The tag to look for (e.g. +12 for kernel side or -15 for client side)
*************************************************************/
WMElement* Identifier::FindTimeTag(long timeTag) const
{
	if (this->GetTimeTag() == timeTag)
		return (WMElement*)this ;

	// Go through each child in turn and if it's an identifier search its children for a matching id.
	for (ChildrenConstIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		if (pWME->GetTimeTag() == timeTag)
			return pWME ;

		// If this is an identifer, we search deeper for the match
		if (pWME->IsIdentifier())
		{
			WMElement* pResult = ((Identifier*)pWME)->FindTimeTag(timeTag) ;

			if (pResult)
				return pResult ;
		}
	}

	return NULL ;
}

/*************************************************************
* @brief Gets the n-th child.
*        Ownership of this WME is retained by the agent.
*
*		 This is an O(n) operation.  We could expose the iterator directly
*		 but we want to export this interface to Java/Tcl etc. and this is easier.
*************************************************************/
WMElement* Identifier::GetChild(int index)
{
	for (ChildrenIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		if (index == 0)
			return *iter ;
		index-- ;
	}

	return NULL ;
}

void Identifier::AddChild(WMElement* pWME)
{
	// Record that we're changing the list of children in case the
	// client would like to know that this identifier was changed in some fashion.
	this->m_AreChildrenModified = true ;

	m_Children.push_back(pWME) ;
}

void Identifier::RemoveChild(WMElement* pWME)
{
	// Record that we're changing the list of children in case the
	// client would like to know that this identifier was changed in some fashion.
	this->m_AreChildrenModified = true ;

	m_Children.remove(pWME) ;
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* Identifier::GetValueType() const
{
	return sml_Names::kTypeID ;
}
