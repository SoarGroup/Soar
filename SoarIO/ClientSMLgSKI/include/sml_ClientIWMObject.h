#ifndef SML_IWMOBJECT_H
#define SML_IWMOBJECT_H

#include "sml_ClientObject.h"
#include "sml_ClientIIterator.h"
#include "sml_ClientIRelease.h"

namespace sml
{

class IWMObject : public IRelease
{
public:
	virtual ~IWMObject() { } ;

	virtual sml::tIWmeIterator* GetWMEs(const char* attributeName = 0,
							egSKISymbolType valueType = gSKI_ANY_SYMBOL,
							gSKI::Error* err = 0) const = 0 ;

	virtual void Release(gSKI::Error* err = 0) = 0;

};

} //close sml namespace

#endif //SML_WMOBJECT_H