/////////////////////////////////////////////////////////////////
// General header for sml_Client* headers
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This header will bring in all of the abstract interface headers
// for the sml_Client libraries and the client stub used to
// create the first concrete class.
//
// The idea is that this should be the only file a client needs
// to include in order to use the files (a bit like windows.h for windows).
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_HEADERS
#define SML_CLIENT_HEADERS

#include "sml_ClientIAgent.h"
#include "sml_ClientIAgentManager.h"
#include "sml_ClientIInputLink.h"
#include "sml_ClientIInputProducer.h"
#include "sml_ClientIKernel.h"
#include "sml_ClientIOutputProcessor.h"
#include "sml_ClientISymbol.h"
#include "sml_ClientIWME.h"
#include "sml_ClientIIterator.h"
#include "sml_ClientIWorkingMemory.h"
#include "sml_ClientIKernelFactory.h"
#include "sml_ClientIOutputLink.h"
#include "sml_ClientIWMObject.h"
#include "sml_ClientStub.h"

#endif	// SML_CLIENT_HEADERS