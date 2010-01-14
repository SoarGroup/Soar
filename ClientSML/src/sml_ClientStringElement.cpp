#include <portability.h>

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

#include "sml_EmbeddedConnection.h"	// For direct methods
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"

using namespace sml ;

StringElement::StringElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, char const* pValue, long timeTag) 
: WMElement(pAgent, pParent->GetSymbol(), pID, pAttributeName, timeTag)
{
	m_Value = pValue ;
}

StringElement::StringElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, char const* pValue, long timeTag) 
: WMElement(pAgent, pParentSymbol, pID, pAttributeName, timeTag)
{
	m_Value = pValue ;
}

StringElement::~StringElement(void)
{
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* StringElement::GetValueType() const
{
	return sml_Names::kTypeString ;
}

#ifdef SML_DIRECT
void StringElement::DirectAdd(Direct_AgentSML_Handle pAgentSML, long timeTag)
{
	EmbeddedConnection* pConnection = static_cast<EmbeddedConnection*>(GetAgent()->GetConnection());
	pConnection->DirectAddWME_String( pAgentSML, m_ID->GetIdentifierSymbol(), GetAttribute(), GetValue(), timeTag);
}
#endif

void StringElement::Update(char const* pValue) 
{ 
	this->m_Agent->GetWM()->UpdateString(this, pValue); 
}
