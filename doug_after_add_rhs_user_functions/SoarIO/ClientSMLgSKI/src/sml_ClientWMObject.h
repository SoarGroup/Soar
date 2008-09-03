#ifndef SML_WMOBJECT_H
#define SML_WMOBJECT_H

#include "sml_ClientIWMObject.h"

namespace sml
{
class ClientSML ;

class WMObject : public IWMObject
{

public:
	WMObject(char const* pID, ClientSML* pClientSML) ;

	virtual sml::tIWmeIterator* GetWMEs(const char* attributeName = 0,
							egSKISymbolType valueType = gSKI_ANY_SYMBOL,
							gSKI::Error* err = 0) const;

	void WMObject::Release(gSKI::Error* err) ;
};

} //close sml namespace

#endif //SML_WMOBJECT_H