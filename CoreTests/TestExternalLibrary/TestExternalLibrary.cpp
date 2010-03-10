#include <portability.h>

#include "sml_Utils.h"
#include "sml_Client.h"
#include "sml_Names.h"
#include "Export.h"

#include <string>
#include <iostream>
#include <sstream>

bool success = false;

void PrintCallbackHandler(sml::smlPrintEventId, void*, sml::Agent*, char const* pMessage) 
{
	if(std::string( pMessage ) == "myRHSTest") success = true;
}

void assertMessage(const char* message, bool assertion)
{
	if (assertion) return;
	std::cerr << "Failure: " << message << std::endl;
	exit(1);
}

int main(int argc, char** argv)
{
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread() ;
	assertMessage("Kernel is null.", pKernel != 0);
	assertMessage(pKernel->GetLastErrorDescription(), pKernel->HadError() == false);

	const std::string loadResult = pKernel->LoadExternalLibrary("TestExternalLibraryLib");
	assertMessage(loadResult.c_str(), loadResult.empty());

	sml::Agent* pAgent = pKernel->CreateAgent("soar");
	assertMessage(pKernel->GetLastErrorDescription(), pAgent != 0);

	pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, PrintCallbackHandler, 0 );
	std::string spMessage = pAgent->ExecuteCommandLine("sp {test (state <s> ^superstate nil) --> (write (exec test))}");
	assertMessage(spMessage.c_str(), pAgent->GetLastCommandLineResult());

	pKernel->RunAllAgents(1);
	assertMessage("RHS function did not fire", success);

	pKernel->Shutdown();
	delete pKernel ;

	exit(0);
	return 0;
}
