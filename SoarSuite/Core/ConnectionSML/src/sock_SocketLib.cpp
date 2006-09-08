#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// SocketLib class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Handles initilization and termination of the socket library for
// the appropriate platform.
//
/////////////////////////////////////////////////////////////////

#include "sock_SocketLib.h"
#include "sock_OSspecific.h"

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SocketLib::SocketLib()
{
	InitializeOperatingSystemSocketLibrary() ;
}

SocketLib::~SocketLib()
{
	TerminateOperatingSystemSocketLibrary() ;
}
