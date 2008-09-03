#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
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

using namespace sml ;

StringElement::StringElement(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, char const* pValue, long timeTag) : WMElement(pAgent, pParent, pID, pAttributeName, timeTag)
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
Direct_WME_Handle StringElement::DirectAdd(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle wmobject, long timeTag)
{
	Direct_WME_Handle wme = ((EmbeddedConnection*)GetAgent()->GetConnection())->DirectAddWME_String(wm, wmobject, timeTag, GetAttribute(), GetValue()) ;
	return wme ;
}
#endif
