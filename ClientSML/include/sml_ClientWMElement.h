/////////////////////////////////////////////////////////////////
// WMElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This is the base class for all working memory elements.
// Every WME consists of an ID, attribute and value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_WORKING_MEMORY_ELEMENT_H
#define SML_WORKING_MEMORY_ELEMENT_H

#include <string>

#include "sml_ClientDirect.h"

namespace sml {

class Agent ;
class Identifier ;
class IdentifierSymbol ;
class IntElement ;
class FloatElement ;
class StringElement ;
class WorkingMemory ;
class RemoveDelta ;
class WMDelta ;

class WMElement
{
	// Making most methods protected, so users don't use them directly by accident.
	// But allow working memory to work with them directly.
	friend class WorkingMemory ;
	friend class Identifier ;			// Access to just added information
	friend class WMDelta ;				// Allow it to destroy WMEs
	friend class IdentifierSymbol ;		// Allow it to destroy WMEs

protected:
	// The agent which owns this WME.
	Agent*	m_Agent ;
	
	// The time tag (a unique id for this WME)
	// We used negative values so it's clear that this time tag is a client side tag.
	long	m_TimeTag ;

	// The identifier symbol as a string.  This can be necessary when connecting up
	// disconnected segments of a graph.
	std::string			m_IDName ;

	// The id for this wme (can be NULL if we're at the top of the tree)
	IdentifierSymbol*	m_ID ;

	// The attribute name for this wme (the value is owned by the derived class)
	std::string m_AttributeName ;

	// This is true if the wme was just added.  The client chooses when to clear these flags.
	bool	m_JustAdded ;

#ifdef SML_DIRECT
	Direct_WME_Handle	m_WME ;
#endif

public:
	// This is true if the wme was just added.  The client chooses when to clear these flags.
	// This is only maintained for output wmes (the client controls input wmes).
	bool	IsJustAdded() { return m_JustAdded ; }

	// Two accessors for the ID as people think about it in different ways
	IdentifierSymbol*		GetParent()		const	{ return m_ID ; }
	IdentifierSymbol*		GetIdentifier()	const	{ return m_ID ; }

	// This will always be valid even if we no identifier symbol
	char const* GetIdentifierName() const { return m_IDName.c_str() ; }

	char const*	GetAttribute() const	{ return m_AttributeName.c_str() ; }

	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType() const = 0 ;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() const = 0 ;

	long		GetTimeTag() const	{ return m_TimeTag ; }

	// The Identifier class overrides this to return true.  (The poor man's RTTI).
	virtual bool IsIdentifier() const { return false ; }
	
	// Class conversions for SWIG language bridges. (The even poorer man's RTTI).
	// Each subclass overrides the appropriate method.
	virtual Identifier* ConvertToIdentifier() { return NULL; }
	virtual IntElement* ConvertToIntElement() { return NULL; }
	virtual FloatElement* ConvertToFloatElement() { return NULL; }
	virtual StringElement* ConvertToStringElement() { return NULL; }

protected:
	// Keep these protected, so user can only create and destroy WMEs through
	// the methods exposed in the agent class.  This makes it clear that the
	// agent owns all objects.
	WMElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, long timeTag);
	virtual ~WMElement(void);

	void	SetJustAdded(bool state) { m_JustAdded = state ; }

	void SetParent(Identifier* pParent) ;

	Agent*		GetAgent()	{ return m_Agent ; }

	// If we update the value we need to assign a new time tag to this WME.
	// That's because we're really doing a delete followed by an add
	// and the add would create a new time tag.
	void GenerateNewTimeTag() ;

	// Send over to the kernel again
	virtual void Refresh() ;

#ifdef SML_DIRECT
	void SetWMEHandle(Direct_WME_Handle wme)	{ m_WME = wme ; }
	Direct_WME_Handle GetWMEHandle()			{ return m_WME ; }

	void ClearAllWMObjectHandles()				{ m_WME = 0 ; }

	virtual Direct_WME_Handle DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag) = 0 ;
#endif

private:
	// NOT IMPLEMENTED
	WMElement( const WMElement & rhs );
	WMElement& operator=(const WMElement& rhs);

};

}	// namespace

#endif // SML_WORKING_MEMORY_ELEMENT_H
