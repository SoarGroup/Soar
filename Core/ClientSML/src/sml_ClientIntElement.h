/////////////////////////////////////////////////////////////////
// IntElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an int value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_INT_ELEMENT_H
#define SML_INT_ELEMENT_H

#include "sml_ClientWMElement.h"

#include <string>

namespace sml {

class WorkingMemory ;
class Identifier ;

class IntElement : public WMElement
{
	// Allow working memory to create these objects directly (user must use agent class to do this)
	friend class WorkingMemory ;

protected:
	// The value for this wme is an int
	int		m_Value ;

public:
	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType() const	;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() const ;

	int GetValue() const { return m_Value ; }
	
	virtual IntElement* ConvertToIntElement() { return this; }

	/*************************************************************
	* @brief Update the value of an existing WME.
	*		 If "auto commit" is turned off in ClientKernel,
	*		 the value is not actually sent to the kernel
	*		 until "Commit" is called.
	*
	*		 If "BlinkIfNoChange" is false then updating a wme to the
	*		 same value it already had will be ignored.
	*		 This value is true by default, so updating a wme to the same
	*		 value causes the wme to be deleted and a new identical one to be added
	*		 which will trigger rules to rematch.
	*		 You can turn this flag on and off around a set of calls to update if you wish.
	*************************************************************/
	void	Update(int value);

protected:
	IntElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, int value, long timeTag) ;
	IntElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, int value, long timeTag) ;
	virtual ~IntElement(void);

	void SetValue(int value)
	{
		m_Value = value ;
	}

#ifdef SML_DIRECT
	virtual void DirectAdd(Direct_AgentSML_Handle pAgentSML, long timeTag) ;
#endif
};

}	// namespace

#endif // SML_INT_ELEMENT_H
