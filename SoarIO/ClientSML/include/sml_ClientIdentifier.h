/////////////////////////////////////////////////////////////////
// SoarId class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID as its value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_IDENTIFIER_H
#define SML_IDENTIFIER_H

#include "sml_ClientWMElement.h"

#include <string>
#include <list>

namespace sml {

class StringElement ;
class WorkingMemory ;

class Identifier : public WMElement
{
	// Make the members all protected, so users dont' access them by accident.
	// Instead, only open them up to the working memory class to use.
	friend WorkingMemory ;

protected:
	// The value for this id, which is a string identifier (e.g. I3)
	// We'll use upper case for Soar IDs and lower case for client IDs
	// (sometimes the client has to generate these before they are assigned by the kernel)
	std::string	m_Identifier ;

	// The list of WMEs owned by this identifier.
	// (When we delete this identifier we'll delete all these automatically)
	std::list<WMElement*>		m_Children ;
	typedef std::list<WMElement*>::iterator ChildrenIter ;
	typedef std::list<WMElement*>::const_iterator ChildrenConstIter ;

	// This is true if the list of children of this identifier was changed.  The client chooses when to clear these flags.
	bool m_AreChildrenModified ;

public:
	virtual char const* GetValueType() const ;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() const { return m_Identifier.c_str() ; }

	// The Identifier class overrides this to return true.  (The poor man's RTTI).
	virtual bool IsIdentifier() const { return true ; }

	/*************************************************************
	* @brief Searches for a child of this identifier that has pID as
	*		 its value (and is itself an identifier).
	*		 (The search is recursive over all children).
	*
	* @param pIncoming	The id to look for (e.g. "O4" -- kernel side or "p3" -- client side)
	*************************************************************/
	Identifier* FindIdentifier(char const* pID) const ;

	/*************************************************************
	* @brief Searches for a child of this identifier that has the given
	*		 time tag.
	*		 (The search is recursive over all children).
	*
	* @param timeTag	The tag to look for (e.g. +12 for kernel side or -15 for client side)
	*************************************************************/
	WMElement* FindTimeTag(long timeTag) const ;

	/*************************************************************
	* @brief Returns the n-th WME that has the given attribute
	*		 and this identifier as its parent (or NULL).
	*		 (<ID> ^attribute <WME>) - returns "WME"
	*
	* @param pAttribute		The name of the attribute to match
	* @param index			0 based index of values for this attribute
	*					   (> 0 only needed for multi-valued attributes)
	*************************************************************/
	WMElement* FindByAttribute(char const* pAttribute, int index) const ;

	/*************************************************************
	* @brief Returns the value (as a string) of the first WME with this attribute.
	*		(<ID> ^attribute value) - returns "value"
	*
	* @param pAttribute		The name of the attribute to match
	*************************************************************/
	char const* GetParameterValue(char const* pAttribute) const { WMElement* pWME = FindByAttribute(pAttribute, 0) ; return pWME ? pWME->GetValueAsString() : NULL ; }

	/*************************************************************
	* @brief Returns the "command name" for a top-level identifier on the output-link.
	*		 That is for output-link O1 (O1 ^move M3) returns "move".
	*************************************************************/
	char const* GetCommandName() const { return this->GetAttribute() ; }

	/*************************************************************
	* @brief Returns the number of children
	*************************************************************/
	int		GetNumberChildren() { return (int)m_Children.size() ; }

	/*************************************************************
	* @brief Gets the n-th child.
	*        Ownership of this WME is retained by the agent.
	*
	*		 This is an O(n) operation.  We could expose the iterator directly
	*		 but we want to export this interface to Java/Tcl etc. and this is easier.
	*************************************************************/
	WMElement* GetChild(int index) ;

	/*************************************************************
	* @brief This is true if the list of children of this identifier has changed.
	*		 The client chooses when to clear these flags.
	*************************************************************/
	bool AreChildrenModified() { return m_AreChildrenModified ; }

protected:
	/*************************************************************
	* @brief Clear the "just added" flag for this identifier and all children (recursively)
	*************************************************************/
	void ClearJustAdded() ;

	/*************************************************************
	* @brief Clear the "children modified" flag for this identifier and all children (recursively)
	*************************************************************/
	void ClearChildrenModified() ;

	// This version is only needed at the top of the tree (e.g. the input link)
	Identifier(Agent* pAgent, char const* pIdentifier, long timeTag);

	// The normal case (where there is a parent id)
	Identifier(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pIdentifier, long timeTag) ;

	virtual ~Identifier(void);

	// Have this identifier take ownership of this WME.  So when the identifier is deleted
	// it will delete the WME.
	void AddChild(WMElement* pWME) ;

	void RemoveChild(WMElement* pWME) ;
};

}	// namespace

#endif // SML_IDENTIFIER_H
