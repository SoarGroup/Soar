/////////////////////////////////////////////////////////////////
// StringElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has a string value.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientStringElement.h"
#include "sml_Connection.h"

using namespace sml ;

StringElement::StringElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pValue) : WMElement(pAgent, pID, pAttributeName)
{
	m_Value = pValue ;
}

StringElement::~StringElement(void)
{
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* StringElement::GetValueType()
{
	return sml_Names::kTypeString ;
}
