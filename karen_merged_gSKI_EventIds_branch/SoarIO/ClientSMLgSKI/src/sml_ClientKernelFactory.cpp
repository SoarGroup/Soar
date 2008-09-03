#include "sml_ClientKernelFactory.h"
#include "sml_ClientKernel.h"
#include "sml_ClientRelease.h"
#include "sml_ClientSML.h"

#include "sml_Connection.h"

#include <algorithm>
#include <string.h>

//FIXME Devvan's debug includes
#include <iostream>
using std::cin; using std::cout;

using namespace sml;

KernelFactory::KernelFactory(char const* pID, ClientSML* pClientSML)
{
	SetClientSML(pClientSML) ;
	SetId(pID) ;
}

KernelFactory::~KernelFactory()
{
cout << "KernelFactory destructor.  My pointer is : " << GetId() << endl;
	CleanUp() ;

	// Under this model we own the client SML object, so delete it
	ClientSML* pClientSML = GetClientSML() ;
	delete pClientSML ;
}

void KernelFactory::CleanUp()
{
	// We'll delete each kernel that's still in our map
	// Following gSKI accurately, we shouldn't do this and the user must release each kernel explicitly,
	// but this seems to make sense for now.
	// Have to be careful with how we do this deletion, because the DestroyKernel calls
	// will modify the kernel map, so we'll do it like this:
	Kernel *pKernel = (Kernel*)m_Kernels.getFirst() ;

	while (pKernel)
	{
		DestroyKernel(pKernel) ;
		pKernel = (Kernel*)m_Kernels.getFirst() ;
	}

	// Make sure all are gone
	m_Kernels.clear() ;
}

sml::IKernel* KernelFactory::Create(const char*           szInstanceName,
                      egSKIThreadingModel   eTModel, 
                      egSKIProcessType      ePType, 
                      const char*           szLocation, 
                      const char*           szLogLocation,
                      egSKILogActivityLevel eLogActivity,
					  gSKI::Error* ) //const
{
	AnalyzeXML response ;

	std::string kernelID;
	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IKernelFactory_Create, GetId()))
	{
		if(response.GetErrorTag())
		{
	cout << "KernelFactory::constructor:  Error in reponse to create Kernel..." << std::endl;
			const ElementXML* errorDocument = response.GetErrorTag();
	cout << "KernelFactory::constructor:  msg in error tag is: " << errorDocument->GetCharacterData() << endl;
			smlErrorDescription = "Failed to create the kernel";
			smlErrorCode = KERNELFACTORY_ERROR_KERNEL_CREATION_FAILED;
			return 0;
		}
		else
			kernelID = response.GetResultString();
	}
	Kernel* kernel = new Kernel(kernelID.c_str(), GetClientSML());

	//we are responsible for deleting this (!) so we need to record it
	m_Kernels.add(kernel) ;

	return kernel;
}

void KernelFactory::DestroyKernel(sml::IKernel* kernel, gSKI::Error*)
{
	// We need to delete our local IKernel object first, so it can clean up objects
	// it owns, which might be in the true kernel.  Then we can send the message and delete the real IKernel.
	std::string id = kernel->GetId() ;

	m_Kernels.remove(id.c_str()) ;

	AnalyzeXML response ;
	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IKernelFactory_DestroyKernel, GetId(),
		sml_Names::kParamKernel, id.c_str()) ;
}

void KernelFactory::Release(gSKI::Error* err)
{
	// Have to delete the kernels *before* we release
	// the factory.
	CleanUp() ;

	Release::ReleaseObject(this, err) ;

	// Now delete ourselves (you use Release not delete to remove this object).
	delete(this) ;
}
