/////////////////////////////////////////////////////////////////
// Release class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Class used to "release" (i.e. delete) an object.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientRelease.h"
#include "sml_Connection.h"

using namespace sml ;

void Release::ReleaseObject(IRelease* pObject, gSKI::Error* err)
{
	AnalyzeXML response ;
	pObject->GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IRelease_Release, pObject->GetId()) ;
}
