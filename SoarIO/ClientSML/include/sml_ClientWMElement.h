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

namespace sml {

class Agent ;
class SoarId ;

class WMElement
{
protected:
	// The agent which owns this WME.
	Agent*	m_Agent ;
	
	// The id for this wme (can be NULL if we're at the top of the tree)
	SoarId*	m_ID ;

	// The attribute name for this wme (the value is owned by the derived class)
	std::string m_AttributeName ;

public:
	WMElement(Agent* pAgent, SoarId* pID, char const* pAttributeName);
	virtual ~WMElement(void);

	Agent*		GetAgent()	{ return m_Agent ; }

	// Two accessors for the ID as people think about it in different ways
	SoarId*		GetParent() { return m_ID ; }
	SoarId*		GetID()		{ return m_ID ; }

	char const*	GetAttribute()	{ return m_AttributeName.c_str() ; }

	// Remove from working memory
	void Remove() { }

private:
	// NOT IMPLEMENTED
	WMElement( const WMElement & rhs );
	WMElement& operator=(const WMElement& rhs);

};

}	// namespace

#endif // SML_WORKING_MEMORY_ELEMENT_H
