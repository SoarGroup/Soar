#include <iostream>
#include <string>

#include "Towers.h"
#include "SoarAgent.h"


#ifdef USE_GSKI_DIRECT_NOT_SML
	//gSKI directives
	#include "IgSKI_KernelFactory.h"
	#include "IgSKI_Kernel.h"
	#include "IgSKI_AgentManager.h"
	#include "IgSKI_InputProducer.h"
	#include "IgSKI_OutputProcessor.h"
	#include "IgSKI_SymbolFactory.h"
	#include "IgSKI_InputLink.h"
	#include "IgSKI_OutputLink.h"
	#include "IgSKI_WorkingMemory.h"
	#include "gSKI.h"
	#include "gSKI_Stub.h"

	using namespace gSKI;
#else
	//SML Directives
	#include "sml_Client.h"
	using namespace sml;
#endif



using std::cout; using std::cin; using std::string;

const int defaultNumTowers = 3;
const int defaultNumdisks = 11;

int main(int argc, char* argv[])
{

	cout << "***Welcome to Towers of Hanoi***" << endl << endl;

	bool doPrinting = true;
	int numTowers = defaultNumTowers;
	int numdisks = defaultNumdisks;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "false"))
			doPrinting = false;
		// @TODO more checking, for robustness 
	}

	if(argc > 2)
	{
		numTowers = atoi(argv[3]);
		if(numTowers < 3)
			numTowers = 3;
	}

	//It would be flexible to read in the number of disks, but the productions are hardcoded to 11
	//if(argc > 3)
	//{
	//	numdisks = atoi(argv[3]);
	//	if(numdisks < 5)
	//		numdisks = 5; 

	//}

	// create kernel factory
#ifdef USE_GSKI_DIRECT_NOT_SML
	IKernelFactory* kFactory = gSKI_CreateKernelFactory();
#else
	//create connection for SML version
	ErrorCode error;
	Connection* pConnection = Connection::CreateEmbeddedConnection("KernelSML", &error);
	IKernelFactor* kFactory = sml_CreateKernelFactory(pConnection);
#endif

	// create kernel
	IKernel* kernel = kFactory->Create();
	IAgentManager* manager = kernel->GetAgentManager();
	gSKI::IAgent* agent = manager->AddAgent("gskiTicAgent", "tictactoe.soar");

	IInputLink* iLink = agent->GetInputLink();


	HanoiWorld hanoi(iLink, doPrinting, numTowers);

	SoarAgent soarAgent(agent, &hanoi);

	if(doPrinting)
		hanoi.Print();


	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <non-whitespace char> to exit\n") ;
	string garbage;
	cin>>garbage;
	return 0;
}

