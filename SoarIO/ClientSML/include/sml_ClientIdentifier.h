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

public:
	typedef std::list<WMElement*>::const_iterator ChildrenConstIter ;

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
	*
	* @param pAttribute		The name of the attribute to match
	* @param index			0 based index of values for this attribute
	*					   (> 0 only needed for multi-valued attributes)
	*************************************************************/
	WMElement* GetAttribute(char const* pAttribute, int index) const ;

	/*************************************************************
	* @brief Returns the start (or end) of the list of children of this
	*		 identifier.
	*************************************************************/
	ChildrenConstIter GetChildrenIteratorBegin() const { return m_Children.begin() ; }
	ChildrenConstIter GetChildrenIteratorEnd()	 const { return m_Children.end() ; }

protected:
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
