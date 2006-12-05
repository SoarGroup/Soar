#include "sml_Client.h"

#include <iostream>
#include <string>

using namespace std;

bool bSuccess;

void PrintCallbackHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage) {
	string s = pMessage;
	if(s == "Success!") {
		bSuccess = true;
	}
}

void main() {

	bSuccess = false;

	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;
	
	assert(pKernel);
	if(pKernel->HadError()) {
		cout << "Error: " << pKernel->GetLastErrorDescription() << endl;
		exit(1);
	}

	pKernel->LoadExternalLibrary("TestExternalLibraryLib");

	sml::Agent* pAgent;
	pAgent = pKernel->CreateAgent("soar1") ;

	int callbackID1 = pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);

	bool prodsloaded = pAgent->LoadProductions("../Tests/TestExternalLibrary.soar");

	if(prodsloaded) {
		pKernel->RunAllAgents(1);
		if(bSuccess) { cout << "Success!" << endl; }
		else { cout << "Test failed; RHS function did not fire." << endl; }
	} else {
		cout << "Failed to load productions from ../Tests/TestExternalLibrary.soar.  Is working directory set to SoarLibrary/bin?" << endl;
	}

	pKernel->Shutdown();
	delete pKernel ;

}