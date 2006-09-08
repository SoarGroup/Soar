#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

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

#include "sml_EmbeddedConnection.h"	// For direct methods
#include "sml_ClientAgent.h"

#include <iostream>     
#include <sstream>     
#include <iomanip>

using namespace sml ;

FloatElement::FloatElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, double value, long timeTag) : WMElement(pAgent, pParent, pID, pAttributeName, timeTag)
{
	m_Value = value ;
}

FloatElement::~FloatElement(void)
{
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* FloatElement::GetValueType() const
{
	return sml_Names::kTypeDouble ;
}

// Returns a string form of the value stored here.
char const* FloatElement::GetValueAsString() const
{
	// Convert double to a string
	std::ostringstream ostr ;
	ostr << m_Value ;

	// We keep ownership of the result here.
	FloatElement* pThis = (FloatElement*)this ;
	pThis->m_StringForm = ostr.str () ;

	return m_StringForm.c_str() ;
}

#ifdef SML_DIRECT
Direct_WME_Handle FloatElement::DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag)
{
	Direct_WME_Handle wme = ((EmbeddedConnection*)GetAgent()->GetConnection())->DirectAddWME_Double(wm, wmobject, timeTag, GetAttribute(), GetValue()) ;
	return wme ;
}
#endif
