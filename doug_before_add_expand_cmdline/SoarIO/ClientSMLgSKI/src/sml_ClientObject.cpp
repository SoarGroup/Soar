/////////////////////////////////////////////////////////////////
// ClientObject class
//
// Author: Douglas Pearson	www.threepenny.net
// Date  : August 2004
//
// This is the base class for all SML Client objects.
// The class is a thin wrapper around an "id" which refers to
// a specific gSKI object.  KernelSML maps between these id's
// and the actual gSKI object as messages are sent to it.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientObject.h"
#include "sml_Connection.h"
#include "sml_ClientSML.h"

using namespace sml ;

Connection* ClientObject::GetConnection() const
{
	if (!m_ClientSML)
		return NULL ;

	return m_ClientSML->GetConnection() ;
}
