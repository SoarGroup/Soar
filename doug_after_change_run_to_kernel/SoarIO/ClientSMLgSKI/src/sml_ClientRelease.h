/////////////////////////////////////////////////////////////////
// Release class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Class used to "release" (i.e. delete) an object.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_RELEASE_H
#define SML_CLIENT_RELEASE_H

#include "sml_ClientIRelease.h"

namespace sml {

// Rather than implementing IRelease, I'm defining these
// as static methods, so that other classes can call here
// to implement their IRelease methods.  This seems to
// resolve some potential problems in the class hierarchy
// and better matches gSKI.
class Release
{
public:
	static void ReleaseObject(IRelease* pObject, gSKI::Error* err = 0) ;

//    virtual bool IsClientOwned(Error* err = 0) const = 0;
};

}

#endif
