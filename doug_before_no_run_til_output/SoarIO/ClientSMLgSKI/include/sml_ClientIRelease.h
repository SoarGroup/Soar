/////////////////////////////////////////////////////////////////
// IRelease class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Abstract class used to "release" (i.e. delete) an object.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_I_RELEASE_H
#define SML_CLIENT_I_RELEASE_H

#include "sml_ClientObject.h"

namespace sml {

class IRelease : public ClientObject
{
public:
    virtual ~IRelease() {}

	virtual void Release(gSKI::Error* err = 0) = 0;

//    virtual bool IsClientOwned(Error* err = 0) const = 0;
};

}

#endif
