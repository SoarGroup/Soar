/////////////////////////////////////////////////////////////////
// IntElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has a string value.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientIntElement.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"

using namespace sml ;

IntElement::IntElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, int value) : WMElement(pAgent, pID, pAttributeName)
{
	m_Value = value ;
}

IntElement::~IntElement(void)
{
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* IntElement::GetValueType()
{
	return sml_Names::kTypeInt;
}

// Returns a string form of the value stored here.
char const* IntElement::GetValueAsString()
{
	char buffer[kMinBufferSize] ;
	Int2String(m_Value, buffer, sizeof(buffer)) ;

	m_StringForm = buffer ;
	return m_StringForm.c_str() ;
}
