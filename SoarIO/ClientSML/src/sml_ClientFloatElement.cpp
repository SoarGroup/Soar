/////////////////////////////////////////////////////////////////
// FloatElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has a string value.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientFloatElement.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"

#include <iostream>     
#include <sstream>     
#include <iomanip>

using namespace sml ;

FloatElement::FloatElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, double value) : WMElement(pAgent, pID, pAttributeName)
{
	m_Value = value ;
}

FloatElement::~FloatElement(void)
{
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* FloatElement::GetValueType()
{
	return sml_Names::kTypeDouble ;
}

// Returns a string form of the value stored here.
char const* FloatElement::GetValueAsString()
{
	// Convert double to a string
	std::ostringstream ostr ;
	ostr << m_Value ;

	// We keep ownership of the result here.
	m_StringForm = ostr.str () ;

	return m_StringForm.c_str() ;
}
