/////////////////////////////////////////////////////////////////
// SoarId class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID value.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientIdentifier.h"
#include "sml_ClientStringElement.h"
#include "sml_Connection.h"

using namespace sml ;

// This version is only needed at the top of the tree (e.g. the input link)
Identifier::Identifier(Agent* pAgent, char const* pIdentifier) : WMElement(pAgent, NULL, NULL)
{
	m_Identifier = pIdentifier ;
}

// The normal case (where there is a parent id)
Identifier::Identifier(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pIdentifier) : WMElement(pAgent, pID, pAttributeName)
{
	m_Identifier = pIdentifier ;
}

Identifier::~Identifier(void)
{
	// We own all of these children, so delete them when we are deleted.
	for (ChildrenIter iter = m_Children.begin() ; iter != m_Children.end() ; iter++)
	{
		WMElement* pWME = *iter ;
		delete pWME ;
	}
}

void Identifier::AddChild(WMElement* pWME)
{
	m_Children.push_back(pWME) ;
}

void Identifier::RemoveChild(WMElement* pWME)
{
	m_Children.remove(pWME) ;
}

// Returns the type of the value stored here (e.g. "string" or "int" etc.)
char const* Identifier::GetValueType()
{
	return sml_Names::kTypeID ;
}
