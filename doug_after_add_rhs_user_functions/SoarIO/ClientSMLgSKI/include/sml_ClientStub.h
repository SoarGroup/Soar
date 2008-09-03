//////////////////////////////////////////////////////////////////////
// ClientCreateKernelFactory function 
//
// Author: Jon Voigt, University of Michigan
// Date  : August 2004
//
// Declares a globally accessible function for creating sml Kernel
// factories
//
// NOTE:  Caller is responsible for cleaning up the created factory
//////////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_STUB_H
#define SML_CLIENT_STUB_H

namespace sml
{
	class IKernelFactory ;
	class Connection ;
}

extern "C" 
{
	sml::IKernelFactory* sml_CreateKernelFactory(sml::Connection* pConnection) ;
}


#endif
