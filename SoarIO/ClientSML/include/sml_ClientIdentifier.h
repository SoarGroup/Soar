/////////////////////////////////////////////////////////////////
// SoarId class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID as its value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_IDENTIFIER_H
#define SML_IDENTIFIER_H

#include "sml_ClientWMElement.h"

#include <string>
#include <vector>

namespace sml {

class StringElement ;

class Identifier : WMElement
{
protected:
	// The value for this id, which is a string identifier (e.g. I3)
	// We'll use upper case for Soar IDs and lower case for client IDs
	// (sometimes the client has to generate these before they are assigned by the kernel)
	std::string	m_Identifier ;

	// The list of WMEs owned by this identifier.
	// (When we delete this identifier we'll delete all these automatically)
	std::vector<WMElement*>		m_Children ;

public:
	// This version is only needed at the top of the tree (e.g. the input link)
	Identifier(Agent* pAgent, char const* pIdentifier);

	// The normal case (where there is a parent id)
	Identifier(Agent* pAgent, Identifier* pID, char const* pAttributeName, char const* pIdentifier) ;

	virtual ~Identifier(void);

	// Create a new WME with this identifier as its parent
	StringElement* CreateStringWME(char const* pAttribute, char const* pValue) ;

	virtual char const* GetValueType() ;

	// Returns a string form of the value stored here.
	virtual char const* GetValueAsString() { return m_Identifier.c_str() ; }

};

}	// namespace

#endif // SML_IDENTIFIER_H
