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

using namespace sml ;

WMElement::WMElement(Agent* pAgent, Identifier* pID, char const* pAttributeName)
{
	// Generate a time tag for this wme
	m_TimeTag = pAgent->GetWM()->GenerateTimeTag() ;

	// Record the agent that owns this wme.
	m_Agent			= pAgent ;

	// id and attribute name can both be NULL if this is at the top of the tree.
	if (pAttributeName)
		m_AttributeName = pAttributeName ;

	m_ID			= pID ;
}

WMElement::~WMElement(void)
{
}

void WMElement::GenerateNewTimeTag()
{
	// Generate a new time tag for this wme
	m_TimeTag = GetAgent()->GetWM()->GenerateTimeTag() ;
}
