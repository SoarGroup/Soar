#ifndef SML_KERNEL_H
#define SML_KERNEL_H

//#include "sml_Connection.h"
#include "sml_ClientIKernel.h"

namespace sml
{

class Kernel : public IKernel
{

public:
	Kernel(char const* pID, ClientSML* pClientSML);

	virtual ~Kernel();

	sml::IAgentManager* GetAgentManager(gSKI::Error* err = 0);

	void Run(unsigned long n, enum unit u);

	void Stop(char* reason);

//	int GetNumberOfAgents() const;

	void Initialize();

	sml::IAgentManager* pAgentManager;
};

}//closes namespace

#endif //SML_KERNEL_H
