#include <portability.h>

/////////////////////////////////////////////////////////////////
// WMElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This is the base class for all working memory elements.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientWMElement.h"
#include "sml_ClientAgent.h"
#include "sml_ClientIdentifier.h"

#include "sml_EmbeddedConnection.h"	// For direct methods
#include "sml_ClientDirect.h"

#include "assert.h"

using namespace sml ;

WMElement::WMElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, long long timeTag)
{
	m_TimeTag = timeTag;
	m_Agent = pAgent;
	m_ID = NULL;
	// BADBAD: duplicated code in both ctors

	// parent and attribute name can both be NULL if this is at the top of the tree.
	if (pAttributeName)
		m_AttributeName = pAttributeName ;

	if (pID)
		m_IDName = pID ;

	if (pParentSymbol)
		m_ID = pParentSymbol ;
}

WMElement::~WMElement(void)
{
	//std::cout << "~WMElement: " << m_TimeTag << std::endl;
}

// See comments in header.
void WMElement::SetSymbol(IdentifierSymbol* p_ID)
{
	m_ID = p_ID;
	m_IDName.assign(p_ID->GetIdentifierSymbol());
}

void WMElement::GenerateNewTimeTag()
{
	// Generate a new time tag for this wme
	m_TimeTag = GetAgent()->GetWM()->GenerateTimeTag64() ;
}

// Send over to the kernel again
void WMElement::Refresh()
{
#ifdef SML_DIRECT

	if (GetAgent()->GetConnection()->IsDirectConnection())
	{
		EmbeddedConnection* pConnection = static_cast<EmbeddedConnection*>( GetAgent()->GetConnection() );
		Direct_AgentSML_Handle agentSMLHandle = pConnection->DirectGetAgentSMLHandle( GetAgent()->GetAgentName() );

		// Add the new value immediately
		DirectAdd( agentSMLHandle, GetTimeTag64() ) ;

		// Return immediately, without adding it to the commit list.
		return ;
	}
#endif

	GetAgent()->GetWM()->GetInputDeltaList()->AddWME(this) ;
}

bool WMElement::DestroyWME()
{
	return this->m_Agent->GetWM()->DestroyWME(this);
}

void WMElement::DebugString(std::string& result)
{
	std::stringstream ss;
	ss << "(" << GetTimeTag64() << ": " << GetIdentifierName() << " ^" << GetAttribute() << " " << GetValueAsString() << ")";
	result.assign(ss.str());
}