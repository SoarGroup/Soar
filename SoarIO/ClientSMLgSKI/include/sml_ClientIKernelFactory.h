#ifndef SML_CLIENT_IKERNEL_FACTORY
#define SML_CLIENT_IKERNEL_FACTORY

#include "sml_ClientIRelease.h"

enum sml_KernelFactoryErrorCode
{
	KERNELFACTORY_ERROR_NONE = 0,
	KERNELFACTORY_ERROR_NULL_RESPONSE,
	KERNELFACTORY_ERROR_KERNEL_CREATION_FAILED
};

namespace sml
{
class IKernel ;

class IKernelFactory : public IRelease
{
public:
	//NOTE !!!
	//This function deviates from the gSKI interface because it is no longer
	// a const member function. The client sml's error flagging is part of the
	//object state, and changes, unlike gSKI's version, which passes Error*s in
	virtual sml::IKernel* Create(const char*           szInstanceName     = 0,
                    egSKIThreadingModel   eTModel            = gSKI_MULTI_THREAD, 
                    egSKIProcessType      ePType             = gSKI_ANY_PROCESS, 
                    const char*           szLocation         = 0, 
                    const char*           szLogLocation = 0,
                    egSKILogActivityLevel eLogActivity       = gSKI_LOG_ERRORS,
					gSKI::Error*                err                = 0) = 0 ;// const;

	virtual void DestroyKernel(sml::IKernel* krnl, gSKI::Error* err=0) = 0 ;

protected:
	// Delete this by releasing the factory
	virtual ~IKernelFactory() { } ;
};



}// closes namespace
#endif //SML_CLIENT_KERNEL_FACTORY