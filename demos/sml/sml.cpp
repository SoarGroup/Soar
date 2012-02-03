#include <iostream>
#include <sml_Client.h>

using namespace std;
using namespace sml;

int main() {
	Kernel* pKernel = Kernel::CreateKernelInNewThread() ;
	
	if (pKernel->HadError()) {
		cout << pKernel->GetLastErrorDescription() << endl;
		exit(1);
	}
	Agent* pAgent = pKernel->CreateAgent("test") ;
	
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		exit(1);
	}
	
	if (pAgent->HadError())
	{
		cout << pAgent->GetLastErrorDescription() << endl ;
		exit(1);
	}
	Identifier* pInputLink = pAgent->GetInputLink() ;
	
	pAgent->SendSVSInput("a obj1 world v 0 0 0 0 0 1 0 1 0 0 1 1 1 0 0 1 0 1 1 1 0 1 1 1");
	
	// Run Soar for 2 decisions
	pAgent->RunSelf(2) ;
	
	cout << pAgent->ExecuteCommandLine("p -d 10 s4") << endl;
	
	// Shutdown and clean up
	pKernel->Shutdown() ;   // Deletes all agents (unless using a remote connection)
	delete pKernel ;                // Deletes the kernel itself

} // end main