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
	friend WorkingMemory ;

protected:
	// The value for this wme is a string
	std::string		m_Value ;

public:
	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType()	;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() { return m_Value.c_str() ; }

	// Returns the current value
	char const* GetValue() { return m_Value.c_str() ; }

protected:
	StringElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pValue) ;
	virtual ~StringElement(void);

	void SetValue(char const* pValue)
	{
		m_Value = pValue ;
	}
};

}	// namespace

#endif // SML_STRING_ELEMENT_H
