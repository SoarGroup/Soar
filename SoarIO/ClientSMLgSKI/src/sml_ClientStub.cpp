#include "sml_ClientStub.h"
#include "sml_ClientKernelFactory.h"
#include "sml_Connection.h"
#include "sml_ClientSML.h"

using namespace sml;

static ElementXML* CallFromKernel(Connection* pConnection, ElementXML* pIncoming, void* pUserData)
{
	ClientSML* pClientSML = (ClientSML*)pUserData ;

	return pClientSML->ReceivedCallFromKernel(pConnection, pIncoming) ;
}

extern "C"
{
	//NOTE:  Caller is responsible for cleaning up the Kernel factory
	IKernelFactory* sml_CreateKernelFactory(sml::Connection* pConnection)
	{
		ClientSML* pClientSML = new ClientSML() ;
		pClientSML->SetConnection(pConnection) ;

		pConnection->RegisterCallback(CallFromKernel, pClientSML, sml_Names::kDocType_Call, true) ;

		AnalyzeXML response ;
		std::string kfId = "none" ;
		
		if (pConnection->SendClassCommand(&response, sml_Names::kgSKI_CreateKernelFactory))
		{
			kfId = response.GetResultString();
		}
		else
		{
			// If something goes wrong here, we need to let the caller know.
			// This is the most likely point of failure, as we're loading up everything.
			return NULL ;
		}

		return new sml::KernelFactory(kfId.c_str(), pClientSML);
	}
}
