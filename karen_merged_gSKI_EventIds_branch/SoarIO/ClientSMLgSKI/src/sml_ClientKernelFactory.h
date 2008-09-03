#ifndef SML_CLIENT_KERNEL_FACTORY
#define SML_CLIENT_KERNEL_FACTORY

#include "sml_ClientIKernelFactory.h"
#include "sml_IdMap.h"

namespace sml
{
class Kernel ;

class KernelFactory : public IKernelFactory
{
public:	
	KernelFactory(char const* pID, ClientSML* pClientSML);

	//NOTE !!!
	//This function deviates from the gSKI interface because it is no longer
	// a const member function. The client sml's error flagging is part of the
	//object state, and changes, unlike gSKI's version, which passes Error*s in
	sml::IKernel* Create(const char*           szInstanceName     = 0,
                    egSKIThreadingModel   eTModel            = gSKI_MULTI_THREAD, 
                    egSKIProcessType      ePType             = gSKI_ANY_PROCESS, 
                    const char*           szLocation         = 0, 
                    const char*           szLogLocation = 0,
                    egSKILogActivityLevel eLogActivity       = gSKI_LOG_ERRORS,
					gSKI::Error*                err                = 0);// const;

	void DestroyKernel(sml::IKernel* krnl, gSKI::Error* err=0);

	virtual void Release(gSKI::Error* err = 0) ;

protected:
	void CleanUp() ;

	// Delete this through the release method
	virtual ~KernelFactory();

	IdMap	m_Kernels ;
};



}// closes namespace
#endif //SML_CLIENT_KERNEL_FACTORY