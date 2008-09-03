#include "sml_ClientKernel.h"
#include "sml_ClientKernelFactory.h"
#include "sml_ClientAgentManager.h"
#include "sml_Connection.h"

#include <iostream>
#include <string>

using namespace sml;
using std::cout; using std::endl;

Kernel::Kernel(char const* pID, ClientSML* pClientSML)
{
	SetClientSML(pClientSML) ;
	SetId(pID) ;
	pAgentManager = NULL ;
}


Kernel::~Kernel()
{
	delete pAgentManager;

	ClearError();
}




void Kernel::Run(unsigned long n, enum unit u)
{

}

void Kernel::Stop(char* reason)
{

}



void Kernel::Initialize()
{
	ClearError();
	// fixme - do WHAT with the connection?
	//connection->Close();
}


IAgentManager* Kernel::GetAgentManager(gSKI::Error*)
{
	if(!pAgentManager)
	{
		AnalyzeXML response ;

		if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IKernel_GetAgentManager, GetId()))
		{
			pAgentManager = new AgentManager(response.GetResultString(), GetClientSML());
		}
	}

	return pAgentManager;
}
