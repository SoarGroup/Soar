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

using namespace sml ;

StringElement::StringElement(Agent* pAgent, SoarId* pID, char const* pAttributeName, char const* pValue) : WMElement(pAgent, pID, pAttributeName)
{
	m_Value = pValue ;
}

StringElement::~StringElement(void)
{
}
