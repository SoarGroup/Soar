
#include "sml_Client.h"

#include "sml_ElementXML.h"
#include "sml_MessageSML.h"
#include "sml_Connection.h"
#include "sml_Names.h"

#include "sml_RemoteConnection.h"
#include "EmbeddedSMLInterface.h"

// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include <string>

using namespace sml;
using std::cout;
using std::cin;
using std::endl;
using std::string;

class TestInputProducer : public IInputProducer
{
public:
	void Update(IWorkingMemory* pWM, IWMObject* pWMObject)
	{
		cout << "TestInputProducer's update function was called" << endl ;
		
		// This may be a deviation from gSKI.
		// What's more, we might need to just delete this object (so we don't modify the gSKI original)
		// It's all very complicated...
		if (pWMObject)
			delete pWMObject ;
	}
} ;

class TestOutputProcessor : public IOutputProcessor
{
public:
	void ProcessOutput(IWorkingMemory* pWM, IWMObject* pWMObject)
	{
		cout << "TestOutputProcessor's update function was called" << endl ;

		if (pWMObject)
			delete pWMObject ;
	}
} ;

void TestInput(IAgent* pAgent, TestInputProducer* pInputProducer)
{
	if (!pAgent)
		return ;

	IInputLink* pInputLink = pAgent->GetInputLink() ;
	
	// Do it again, just to check that this doesn't cause leaks
	IInputLink* pInputLink2 = pAgent->GetInputLink() ;

	if (!pInputLink)
	{
		cout << "Failed to get the input link" << endl ;
		return ;
	}

	IWMObject* pRoot = 0 ;
	pInputLink->GetRootObject(&pRoot) ;

	if (!pRoot)
	{
		cout << "Failed to get the root object for the input link" << endl ;
		return ;
	}

	pInputLink->AddInputProducer(pRoot, pInputProducer) ;

	pRoot->Release() ;
	
	// The input link is now owned by the Agent,
	// so we don't have to delete it ourselves.
//	delete pInputLink ;

	cout << "Added the input producer" << endl ;
}

void TestOutput(IAgent* pAgent, TestOutputProcessor* pOutput)
{
	if (!pAgent)
		return ;

	IOutputLink* pOutputLink = pAgent->GetOutputLink() ;
	
	if (!pOutputLink)
	{
		cout << "Failed to get the output link" << endl ;
		return ;
	}

	pOutputLink->AddOutputProcessor("", pOutput) ;

	cout << "Added the output processor" << endl ;
}

/*
static ElementXML* CallFromKernel(Connection* pConnection, ElementXML* pIncoming, void* pUserData)
{
	TestInputProducer* pIP = (TestInputProducer*)pUserData ;

	// BUGBUG: For the real system we need to use a map to route
	// this message to the right input producer.
	pIP->Update(NULL, NULL) ;

	return NULL ;
}
*/
int main(int argc, char* argv[])
{
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 103 ;

	cout << "TestClientSML app starting..." << endl << endl;

	cout << "Creating Connection..." << endl << endl;

	// We'll do the test in a block, so everything should have been
	// deleted when we test for memory leaks.
	{
		TestInputProducer inputProducer ;

		//create a connection
		ErrorCode error;
		Connection* pConnection = Connection::CreateEmbeddedConnection("KernelSML", &error) ;

//		pConnection->RegisterCallback(CallFromKernel, &inputProducer, sml_Names::kDocType_Call, true) ;

		cout << "Creating Kernel Factory..." << endl << endl;
		
//--------------------------------
//TEST creating kernel factory 1/19
//--------------------------------
		IKernelFactory* kf = sml_CreateKernelFactory(pConnection);

		if(kf)
			cout << "Successfully created kernel factory" << endl;
		else
			cout << "Failed to create kernel factory" << endl;

		cout << "Creating kernel..." << endl;
		//TODO error checking

//--------------------------------------
//TEST creating kernel factory create
//------------------------------------
		IKernel* kernel = kf->Create();

		//TODO error checking
		if(!kernel)
		{
			cout << "TestApp:: kernel creation failed. Aborting. " << endl;
		}

		else
		{
			cout << "Successfully created kernel" << endl;
			IAgentManager* agentManager = kernel->GetAgentManager();
//--------------------------------
//TEST get agent manager 4/19
//--------------------------------

			//problem with agent manager
			if(!agentManager || agentManager->GetLastError() != AGENTMANAGER_ERROR_NONE)
			{
				cout << "Test app:: Since there was a problem in the agent manager construction, aborting  \
					the remainder of the test app" << endl;
			}
			//everything looks good
			else
			{
				//create an agent
				cout << "Creating Agent..." << endl << endl;
//--------------------------------
//TEST add agent 7/19
//--------------------------------
				IAgent* testAgent = agentManager->AddAgent("gskiTicAgent", "tictactoe.soar");//many def params omitted (prods, etc)

				if(!testAgent)
				{
					cout << "Agent not created: critical error!" << endl ;
				}
				else
				{
					//cout << "Destroying Agent..." << endl << endl;
					//destroy agent
					//agentManager->DestroyAgent(testAgent);  not implemented yet
				}

				if(agentManager->GetLastError())
				{
					cout << "AgentManager error: " << agentManager->GetLastErrorDescription() << endl;
				}

				TestInput(testAgent, &inputProducer) ;

				// Run Soar for one decision.
				testAgent->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1) ;

				// testAgent is owned by AgentManager object, so we don't delete it.
//				if(testAgent)
//					delete testAgent;
			}

		}//kernel successfully created

		//destroy connection
		cout << "Closing connection..." << endl << endl;

		// Deleting the kernel factory sends messages to KernelSML
		// so we have to do this *before* we close the connection.
		kf->Release() ; //thise cleans up the kernels, which cleans up the agent managers, which cleans up the agents

		pConnection->CloseConnection();
		delete pConnection ;


	}// closes testing block scope

	//A deliberate memory leak which I can use to test the memory checking code is working.
	//char* pTest = new char[10] ;

	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
	//_CrtMemDumpAllObjectsSince(&memstate) ;

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <return> to exit\n") ;
	char line[100] ;
	char* str = gets(line) ;


	return 0;
}