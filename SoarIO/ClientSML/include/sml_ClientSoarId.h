/////////////////////////////////////////////////////////////////
// SoarId class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID as its value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_SOAR_ID_H
#define SML_SOAR_ID_H

#include "sml_ClientWMElement.h"

#include <string>

namespace sml {

class SoarId : WMElement
{
protected:
	// The value for this id, which is a string identifier (e.g. I3)
	// We'll use upper case for Soar IDs and lower case for client IDs
	// (sometimes the client has to generate these before they are assigned by the kernel)
	std::string	m_Identifier ;

public:
	// This version is only needed at the top of the tree (e.g. the input link)
	SoarId(Agent* pAgent, char const* pIdentifier);

	// The normal case (where there is a parent id)
	SoarId(Agent* pAgent, SoarId* pID, char const* pAttributeName, char const* pIdentifier) ;

	virtual ~SoarId(void);
};

}	// namespace

#endif // SML_SOAR_ID_H
