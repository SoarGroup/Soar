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

#include "assert.h"

using namespace sml ;

WMElement::WMElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, long timeTag)
{
	m_TimeTag = timeTag ;

	// Record the agent that owns this wme.
	m_Agent			= pAgent ;

	m_ID = NULL ;

	// parent and attribute name can both be NULL if this is at the top of the tree.
	if (pAttributeName)
		m_AttributeName = pAttributeName ;

	if (pID)
		m_IDName = pID ;

	if (pParent)
		m_ID = pParent->GetSymbol() ;

#ifdef SML_DIRECT
	m_WME = 0 ;
#endif
}

WMElement::~WMElement(void)
{
}

void WMElement::SetParent(Identifier* pParent)
{
	// We should only set the parent once and only to the same
	// string value as the ID we used when we constructed the WME.
	// (This method is only called when we have dangling parts of a graph while
	//  it's being built).
	assert(m_ID == NULL) ;
	assert(m_IDName.compare(pParent->GetValueAsString()) == 0) ;

	m_ID = pParent->GetSymbol() ;
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
		Direct_WME_Handle wme = DirectAdd(wm, parent->GetWMObjectHandle(), GetTimeTag()) ;
		SetWMEHandle(wme) ;

		// Return immediately, without adding it to the commit list.
		return ;
	}
#endif

	GetAgent()->GetWM()->GetInputDeltaList()->AddWME(this) ;
}
