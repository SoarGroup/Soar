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

using namespace sml ;

WMElement::WMElement(Agent* pAgent, SoarId* pID, char const* pAttributeName)
{
	// id and attribute name can both be NULL if this is at the top of the tree.
	m_Agent			= pAgent ;
	m_ID			= pID ;

	if (pAttributeName)
		m_AttributeName = pAttributeName ;
}

WMElement::~WMElement(void)
{
}
