#ifndef SML_IKERNEL_H
#define SML_IKERNEL_H

#include "sml_ClientObject.h"

enum sml_KernelErrorCode
{
	KERNEL_ERROR_NONE = 0,
	KERNEL_ERROR_MANAGER_CREATION_FAILED,
	KERNEL_ERROR_MANAGER_FAILED_TO_CREATE_KERNEL,
	KERNEL_ERROR_MALFORMED_XML
};

namespace sml
{

class IAgentManager;
class IKernelFactory;
class Connection ;

class IKernel : public ClientObject
{
public:
	virtual ~IKernel() { } ;

	virtual sml::IAgentManager* GetAgentManager(gSKI::Error* err = 0) = 0 ;

	virtual void Run(unsigned long n, enum unit u) = 0 ;

	virtual void Stop(char* reason) = 0 ;

//	virtual int GetNumberOfAgents() const = 0 ;

	virtual void Initialize() = 0 ;
};

}//closes namespace

#endif //SML_KERNEL_H
