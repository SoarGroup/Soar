/////////////////////////////////////////////////////////////////
// Identifier class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID as its value.
//
// Also contains "IdentifierSymbol".  This represents the value.
//
// So for (I6 ^name N1) the triplet is the WME (Identifier class)
// and "N1" is the IdentifierSymbol.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_IDENTIFIER_H
#define SML_IDENTIFIER_H

#include "sml_ClientWMElement.h"
#include "sml_ClientDirect.h"

#include <string>
#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <list>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

namespace sml {

class StringElement ;
class WorkingMemory ;
class Identifier ;

// Two identifiers (two wmes) can have the same value because this is a graph not a tree
// so we need to represent that separately.
class IdentifierSymbol
{
	friend class Identifier ;	// Provide direct access to children.

protected:
	// The value for this id, which is a string identifier (e.g. I3)
	// We'll use upper case for Soar IDs and lower case for client IDs
	// (sometimes the client has to generate these before they are assigned by the kernel)
	std::string	m_Symbol ;

	// The list of WMEs owned by this identifier.
	// (When we delete this identifier we'll delete all these automatically)
	std::list<WMElement*>		m_Children ;

	// The list of WMEs that are using this symbol as their identifier
	// (Usually just one value in this list)
	std::list<Identifier*>		m_UsedBy ;

	// This is true if the list of children of this identifier was changed.  The client chooses when to clear these flags.
	bool m_AreChildrenModified ;

// The gSKI objects for this wme.  This allows us to optimize the embedded connection.
#ifdef SML_DIRECT
protected:
	Direct_WorkingMemory_Handle	m_WM ;
	Direct_WMObject_Handle		m_WMObject ;

public:
	Direct_WorkingMemory_Handle GetWorkingMemoryHandle()		{ return m_WM ; }
	Direct_WMObject_Handle		GetWMObjectHandle()				{ return m_WMObject ; }
#endif

public:
	IdentifierSymbol(Identifier* pIdentifier) ;
	~IdentifierSymbol() ;

	char const* GetIdentifierSymbol()			{ return m_Symbol.c_str() ; }
	void SetIdentifierSymbol(char const* pID)   { m_Symbol = pID ; }

	bool AreChildrenModified()				{ return m_AreChildrenModified ; }
	void SetAreChildrenModified(bool state) { m_AreChildrenModified = state ; }

	// Indicates that an identifier is no longer using this as its value
	void NoLongerUsedBy(Identifier* pIdentifier)  { m_UsedBy.remove(pIdentifier) ; }
	void UsedBy(Identifier* pIdentifier)		  { m_UsedBy.push_back(pIdentifier) ; }

	bool IsFirstUser(Identifier* pIdentifier)
	{
		if (m_UsedBy.size() == 0)
			return false ;

		Identifier* front = m_UsedBy.front() ;
		return (front == pIdentifier) ;
	}

	Identifier* GetFirstUser() { return m_UsedBy.front() ; }

	int  GetNumberUsing()						{ return (int)m_UsedBy.size() ; }

	// Have this identifier take ownership of this WME.  So when the identifier is deleted
	// it will delete the WME.
	void AddChild(WMElement* pWME) ;

	void RemoveChild(WMElement* pWME) ;
} ;

class Identifier : public WMElement
{
	// Make the members all protected, so users dont' access them by accident.
	// Instead, only open them up to the working memory class to use.
	friend class WorkingMemory ;
	friend class WMElement ;
	friend class Agent ;

public:
	typedef std::list<WMElement*>::iterator ChildrenIter ;
	typedef std::list<WMElement*>::const_iterator ChildrenConstIter ;

protected:
	// Two identifiers (i.e. two wmes) can share the same identifier value
	// So each identifier has a pointer to a symbol object, but two could share the same object.
	IdentifierSymbol* m_pSymbol ;

	IdentifierSymbol* GetSymbol() { return m_pSymbol ; }

	ChildrenIter GetChildrenBegin() { return m_pSymbol->m_Children.begin() ; }
	ChildrenIter GetChildrenEnd()   { return m_pSymbol->m_Children.end() ; }

	void SetParent(Identifier* pParent) ;

public:
	virtual char const* GetValueType() const ;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() const { return m_pSymbol->GetIdentifierSymbol() ; }

	// The Identifier class overrides this to return true.  (The poor man's RTTI).
	virtual bool IsIdentifier() const { return true ; }
	
	virtual Identifier* ConvertToIdentifier() { return this; }

	/*************************************************************
	* @brief Searches for a child of this identifier that has pID as
	*		 its value (and is itself an identifier).
	*		 (The search is recursive over all children).
	*
	*		 There can be multiple WMEs that share the same identifier value.
	*
	* @param pId	The id to look for (e.g. "O4" -- kernel side or "p3" -- client side)
	* @param index	If non-zero, finds the n-th match
	*************************************************************/
	Identifier* FindIdentifier(char const* pID, int index = 0) const ;

	/*************************************************************
	* @brief Searches for a child of this identifier that has the given
	*		 time tag.
	*		 (The search is recursive over all children).
	*
	* @param timeTag	The tag to look for (e.g. +12 for kernel side or -15 for client side)
	*************************************************************/
	WMElement* FindFromTimeTag(long timeTag) const ;

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
	* @brief Adds "^status complete" as a child of this identifier.
	*************************************************************/
	void AddStatusComplete() ;

	/*************************************************************
	* @brief Adds "^status error" as a child of this identifier.
	*************************************************************/
	void AddStatusError() ;

	/*************************************************************
	* @brief Adds "^error-code <code>" as a child of this identifier.
	*************************************************************/
	void AddErrorCode(int errorCode) ;

	/*************************************************************
	* @brief Returns the number of children
	*************************************************************/
	int		GetNumberChildren() { return (int)m_pSymbol->m_Children.size() ; }

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
	bool AreChildrenModified() { return m_pSymbol->AreChildrenModified() ; }

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
	Identifier(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, char const* pIdentifier, long timeTag) ;

	// The shared id case (where there is a parent id and value is an identifier that already exists)
	Identifier(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, Identifier* pLinkedIdentifier, long timeTag) ;

	virtual ~Identifier(void);

	void AddChild(WMElement* pWME) { m_pSymbol->AddChild(pWME) ; }

	void RemoveChild(WMElement* pWME) { m_pSymbol->RemoveChild(pWME) ; }

#ifdef SML_DIRECT
	void SetWorkingMemoryHandle(Direct_WorkingMemory_Handle wm) { m_pSymbol->m_WM = wm ; }
	void SetWMObjectHandle(Direct_WMObject_Handle wmobject)		{ m_pSymbol->m_WMObject = wmobject ; }

	Direct_WorkingMemory_Handle GetWorkingMemoryHandle()		{ return m_pSymbol->m_WM ; }
	Direct_WMObject_Handle		GetWMObjectHandle()				{ return m_pSymbol->m_WMObject ; }

	// Reset all handles to 0
	void ClearAllWMObjectHandles() ;

	virtual Direct_WME_Handle DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag) ;
#endif

	// Send over to the kernel again
	virtual void Refresh() ;
};

}	// namespace

#endif // SML_IDENTIFIER_H
