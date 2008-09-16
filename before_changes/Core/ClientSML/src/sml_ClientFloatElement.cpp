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
#include "sml_ClientIdentifier.h"

#include <iostream>     
#include <sstream>     
#include <iomanip>

using namespace sml ;

FloatElement::FloatElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, double value, long timeTag) 
: WMElement(pAgent, pParent->GetSymbol(), pID, pAttributeName, timeTag)
{
	m_Value = value ;
}

FloatElement::FloatElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, double value, long timeTag) 
: WMElement(pAgent, pParentSymbol, pID, pAttributeName, timeTag)
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
void FloatElement::DirectAdd(Direct_AgentSML_Handle pAgentSML, long timeTag)
{
	EmbeddedConnection* pConnection = static_cast<EmbeddedConnection*>(GetAgent()->GetConnection());
	pConnection->DirectAddWME_Double( pAgentSML, m_ID->GetIdentifierSymbol(), GetAttribute(), GetValue(), timeTag);
}
#endif
