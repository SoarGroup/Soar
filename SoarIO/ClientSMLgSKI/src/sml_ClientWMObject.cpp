#include "sml_ClientWMObject.h"
#include "sml_ClientWMIterator.h"
#include "sml_ClientRelease.h"
#include "sml_ClientSML.h"
#include "sml_ClientIterator.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"

using namespace sml ;

WMObject::WMObject(char const* pID, ClientSML* pClientSML)
{
	SetId(pID) ;
	SetClientSML(pClientSML) ;
}

	
sml::tIWmeIterator* WMObject::GetWMEs(const char* attributeName,
							egSKISymbolType valueType,
							gSKI::Error* err) const
{
	AnalyzeXML response ;
	tIWmeIterator* pValue = NULL ;

	char buffer[kMinBufferSize] ;
	Int2String(valueType, buffer, sizeof(buffer)) ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWMObject_GetWMEs, GetId(),
		sml_Names::kParamAttribute, attributeName,
		sml_Names::kParamValue, buffer))
	{
		// NOTE: The caller is responsible for deleting this.  This matches gSKI's definition.
		pValue = new tWmeIterator(response.GetResultString(), GetClientSML()) ;
		pValue->IsValid() ;
	}

	return pValue ;
}

void WMObject::Release(gSKI::Error* err)
{
	Release::ReleaseObject(this) ;

	delete this ;
}
