#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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

using namespace sml ;

WMElement::WMElement(Agent* pAgent, Identifier* pID, char const* pAttributeName, long timeTag)
{
	m_TimeTag = timeTag ;

	// Record the agent that owns this wme.
	m_Agent			= pAgent ;

	// id and attribute name can both be NULL if this is at the top of the tree.
	if (pAttributeName)
		m_AttributeName = pAttributeName ;

	m_ID = NULL ;

	if (pID)
		m_ID = pID->GetSymbol() ;

#ifdef SML_DIRECT
	m_WME = 0 ;
#endif
}

WMElement::~WMElement(void)
{
#ifdef SML_DIRECT
	// If we're using the direct connection methods, we need to release the gSKI object
	// that we own.
	if (m_WME && GetAgent()->GetConnection()->IsDirectConnection())
	{
		((EmbeddedConnection*)GetAgent()->GetConnection())->DirectReleaseWME(m_WME) ;
	}
#endif
}

void WMElement::GenerateNewTimeTag()
{
	// Generate a new time tag for this wme
	m_TimeTag = GetAgent()->GetWM()->GenerateTimeTag() ;
}

// Send over to the kernel again
void WMElement::Refresh()
{
#ifdef SML_DIRECT
	IdentifierSymbol* parent = GetIdentifier() ;
	Direct_WorkingMemory_Handle wm = parent->GetWorkingMemoryHandle() ;

	if (GetAgent()->GetConnection()->IsDirectConnection())
	{
		// Add the new value immediately
		Direct_WME_Handle wme = DirectAdd(wm, parent->GetWMObjectHandle()) ;
		SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return ;
	}
#endif

	GetAgent()->GetWM()->GetInputDeltaList()->AddWME(this) ;
}
