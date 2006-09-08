#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// Identifier class
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
#include "sml_ClientAgent.h"
#include "sml_StringOps.h"
#include <assert.h>

#include "sml_ClientDirect.h"
#include "sml_EmbeddedConnection.h"	// For direct methods

using namespace sml ;

IdentifierSymbol::IdentifierSymbol(Identifier* pIdentifier)
{
	m_UsedBy.push_back(pIdentifier) ;

#ifdef SML_DIRECT
	m_WM = 0 ;
	m_WMObject = 0 ;
#endif
}

IdentifierSymbol::~IdentifierSymbol()
{
	// Nobody should be using this symbol when we're deleted.
	assert (GetNumberUsing() == 0) ;

	// We own all of these children, so delete them when we are deleted.
	for (Identifier::ChildrenIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;
		delete pWME ;
	}
}

void IdentifierSymbol::AddChild(WMElement* pWME)
{
	// Record that we're changing the list of children in case the
	// client would like to know that this identifier was changed in some fashion.
	SetAreChildrenModified(true) ;

	m_Children.push_back(pWME) ;
}

void IdentifierSymbol::RemoveChild(WMElement* pWME)
{
	// Record that we're changing the list of children in case the
	// client would like to know that this identifier was changed in some fashion.
	SetAreChildrenModified(true) ;

	m_Children.remove(pWME) ;
}

// This version is only needed at the top of the tree (e.g. the input link)
Identifier::Identifier(Agent* pAgent, char const* pIdentifier, long timeTag) : WMElement(pAgent, NULL, NULL, NULL, timeTag)
{
	m_pSymbol = new IdentifierSymbol(this) ;
	m_pSymbol->SetIdentifierSymbol(pIdentifier) ;
}

// The normal case (where there is a parent id)
Identifier::Identifier(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, char const* pIdentifier, long timeTag) : WMElement(pAgent, pParent, pID, pAttributeName, timeTag)
{
	m_pSymbol = new IdentifierSymbol(this) ;
	m_pSymbol->SetIdentifierSymbol(pIdentifier) ;

#ifdef SML_DIRECT
	// Pass along with working memory object.  (Note: If you pass id's from input-link to output-link this just breaks gSKI all over the place, so please don't).
	if (pParent)
		m_pSymbol->m_WM = pParent->GetSymbol()->m_WM ;
	else
		m_pSymbol->m_WM = NULL ;
#endif
}

// Creating one identifier to have the same value as another
Identifier::Identifier(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, Identifier* pLinkedIdentifier, long timeTag) : WMElement(pAgent, pParent, pID, pAttributeName, timeTag)
{
	m_pSymbol = pLinkedIdentifier->m_pSymbol ;
	m_pSymbol->UsedBy(this) ;

#ifdef SML_DIRECT
	// Pass along with working memory object.  (Note: If you pass id's from input-link to output-link this just breaks gSKI all over the place, so please don't).
	if (pParent)
		m_pSymbol->m_WM = pParent->GetSymbol()->m_WM ;
	else
		m_pSymbol->m_WM = NULL ;
#endif
}

Identifier::~Identifier(void)
{
	// Indicate this identifier is no longer using the identifier symbol
	m_pSymbol->NoLongerUsedBy(this) ;

	// Decide if we need to delete the identifier symbol (or is someone else still using it)
	if (m_pSymbol->GetNumberUsing() == 0)
	{
		delete m_pSymbol ;
	}

	m_pSymbol = NULL ;
}

void Identifier::SetParent(Identifier* pParent)
{
	WMElement::SetParent(pParent) ;

#ifdef SML_DIRECT
	// Pass along with working memory object.  (Note: If you pass id's from input-link to output-link this just breaks gSKI all over the place, so please don't).
	m_pSymbol->m_WM = pParent->GetSymbol()->m_WM ;
#endif
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
	for (ChildrenConstIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		if (IsStringEqualIgnoreCase(pWME->GetAttribute(), pAttribute))
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
*		 There can be multiple WMEs that share the same identifier value.
*
* @param pIncoming	The id to look for (e.g. "O4" -- kernel side or "p3" -- client side)
* @param index	If non-zero, finds the n-th match
*************************************************************/
Identifier* Identifier::FindIdentifier(char const* pID, int index) const
{
	if (strcmp(this->GetValueAsString(), pID) == 0)
	{
		if (index == 0)
			return (Identifier*)this ;
		index-- ;
	}

	// Go through each child in turn and if it's an identifier search its children for a matching id.
	for (ChildrenConstIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
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
* @brief Adds "^status complete" as a child of this identifier.
*************************************************************/
void Identifier::AddStatusComplete()
{
	GetAgent()->CreateStringWME(this, "status", "complete") ;
}

/*************************************************************
* @brief Adds "^status error" as a child of this identifier.
*************************************************************/
void Identifier::AddStatusError()
{
	GetAgent()->CreateStringWME(this, "status", "error") ;
}

/*************************************************************
* @brief Adds "^error-code <code>" as a child of this identifier.
*************************************************************/
void Identifier::AddErrorCode(int errorCode)
{
	GetAgent()->CreateIntWME(this, "error-code", errorCode) ;
}

/*************************************************************
* @brief Clear the "just added" flag for this and all children
*		 (The search is recursive over all children).
*************************************************************/
void Identifier::ClearJustAdded()
{
	this->SetJustAdded(false) ;

	// Go through each child in turn
	for (ChildrenConstIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
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
	this->m_pSymbol->SetAreChildrenModified(false) ;

	// Go through each child in turn
	for (ChildrenConstIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
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
WMElement* Identifier::FindFromTimeTag(long timeTag) const
{
	// SLOWSLOW: We could use a hash table to speed this up and replace O(n) with O(1).
	// Right now that will only impact performance when elements are removed from the output link,
	// but if clients start to use this (or the output link is really large for an application)
	// the saving could be significant.

	if (this->GetTimeTag() == timeTag)
		return (WMElement*)this ;

	// Go through each child in turn and if it's an identifier search its children for a matching id.
	for (ChildrenConstIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;

		if (pWME->GetTimeTag() == timeTag)
			return pWME ;

		// If this is an identifer, we search deeper for the match
		if (pWME->IsIdentifier())
		{
			WMElement* pResult = ((Identifier*)pWME)->FindFromTimeTag(timeTag) ;

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
	for (ChildrenIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
	{
		if (index == 0)
			return *iter ;
		index-- ;
	}

	return NULL ;
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* Identifier::GetValueType() const
{
	return sml_Names::kTypeID ;
}

// Send over to the kernel again
void Identifier::Refresh()
{
	// Add this identifier into the tree (except for input link which is auto created)
	if (GetAgent()->GetInputLink() != this)
		WMElement::Refresh() ;

	// If this symbol appears as the value for several identifier objects we only
	// want to add the child WMEs once.
	if (m_pSymbol->IsFirstUser(this))
	{
		for (ChildrenIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
		{
			WMElement* pWME = *iter ;
			pWME->Refresh() ;
		}
	}
}

#ifdef SML_DIRECT
void Identifier::ClearAllWMObjectHandles()
{
	SetWMObjectHandle(0) ;

	for (ChildrenIter iter = m_pSymbol->m_Children.begin() ; iter != m_pSymbol->m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;
		pWME->ClearAllWMObjectHandles() ;
	}
}

Direct_WME_Handle Identifier::DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag)
{
	// If this identifier is sharing ID values with other identifiers, we add the first object
	// and then link all subsequent ones together.
	Direct_WME_Handle wme = 0 ;
	if (m_pSymbol->IsFirstUser(this))
	{
		wme = ((EmbeddedConnection*)GetAgent()->GetConnection())->DirectAddID(wm, wmobject, timeTag, GetAttribute()) ;
	}
	else
	{
		Identifier* pSharedID = m_pSymbol->GetFirstUser() ;
		Direct_WMObject_Handle sharedWMObject = pSharedID->GetWMObjectHandle() ;

		// Fatal error during init-soar for direct connection
		// This wmobject value should have already been created through an add call
		// when the first user of the symbol was called.
		assert(sharedWMObject != 0) ;

		wme = ((EmbeddedConnection*)GetAgent()->GetConnection())->DirectLinkID(wm, wmobject, timeTag, GetAttribute(), sharedWMObject) ;
	}

	Direct_WMObject_Handle newwmobject = ((EmbeddedConnection*)GetAgent()->GetConnection())->DirectGetThisWMObject(wm, wme) ;
	SetWMObjectHandle(newwmobject) ;
	return wme ;
}
#endif
