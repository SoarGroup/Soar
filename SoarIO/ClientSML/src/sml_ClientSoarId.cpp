/////////////////////////////////////////////////////////////////
// SoarId class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID value.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientSoarId.h"

using namespace sml ;

// This version is only needed at the top of the tree (e.g. the input link)
SoarId::SoarId(Agent* pAgent, char const* pIdentifier) : WMElement(pAgent, NULL, NULL)
{
	m_Identifier = pIdentifier ;
}

// The normal case (where there is a parent id)
SoarId::SoarId(Agent* pAgent, SoarId* pID, char const* pAttributeName, char const* pIdentifier) : WMElement(pAgent, pID, pAttributeName)
{
	m_Identifier = pIdentifier ;
}

SoarId::~SoarId(void)
{
}
