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

class StringElement : public WMElement
{
protected:
	// The value for this wme is a string
	std::string		m_Value ;

public:
	StringElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pValue) ;
	virtual ~StringElement(void);

	// Returns the type of the value stored here (e.g. "string" or "int" etc.)
	virtual char const* GetValueType()	;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() { return m_Value.c_str() ; }

};

}	// namespace

#endif // SML_STRING_ELEMENT_H
