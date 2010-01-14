#include <portability.h>

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

#include "sml_EmbeddedConnection.h"	// For direct methods
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"

#include "misc.h"

using namespace sml ;

IntElement::IntElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, int value, long timeTag) 
: WMElement(pAgent, pParent->GetSymbol(), pID, pAttributeName, timeTag)
{
	m_Value = value ;
}

IntElement::IntElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, int value, long timeTag) 
: WMElement(pAgent, pParentSymbol, pID, pAttributeName, timeTag)
{
	m_Value = value ;
}

IntElement::~IntElement(void)
{
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* IntElement::GetValueType() const
{
	return sml_Names::kTypeInt;
}

// Returns a string form of the value stored here.
char const* IntElement::GetValueAsString() const
{
	// note: this is a bit of a hack but the previous 
	// hack casted around the member function's const keyword
	static std::string temp; // c_str() needs to remain valid
	to_string( m_Value, temp );
	return temp.c_str() ;
}

#ifdef SML_DIRECT
void IntElement::DirectAdd(Direct_AgentSML_Handle pAgentSML, long timeTag)
{
	EmbeddedConnection* pConnection = static_cast<EmbeddedConnection*>(GetAgent()->GetConnection());
	pConnection->DirectAddWME_Double( pAgentSML, m_ID->GetIdentifierSymbol(), GetAttribute(), GetValue(), timeTag);
}
#endif

void IntElement::Update(int value) 
{ 
	this->m_Agent->GetWM()->UpdateInt(this, value); 
}
