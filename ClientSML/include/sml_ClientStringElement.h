/////////////////////////////////////////////////////////////////
// StringElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has a string value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_STRING_ELEMENT_H
#define SML_STRING_ELEMENT_H

#include "sml_ClientWMElement.h"

#include <string>

namespace sml {

class WorkingMemory ;
class Identifier ;

class StringElement : public WMElement
{
	// Allow working memory to create these objects directly (user must use agent class to do this)
	friend class WorkingMemory ;

protected:
	// The value for this wme is a string
	std::string		m_Value ;

public:
	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType() const	;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() const { return m_Value.c_str() ; }

	// Returns the current value
	char const* GetValue() const { return m_Value.c_str() ; }
	
	virtual StringElement* ConvertToStringElement() { return this; }

protected:
	StringElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, char const* pValue, long timeTag) ;
	virtual ~StringElement(void);

	void SetValue(char const* pValue)
	{
		m_Value = pValue ;
	}

#ifdef SML_DIRECT
	virtual Direct_WME_Handle DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag) ;
#endif
};

}	// namespace

#endif // SML_STRING_ELEMENT_H
