
//#include "sml_Connection.h"
#include "sml_Client.h"

#include "..\..\Profiler\include\simple_timer.h"

// Define a sleep
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define SLEEP Sleep
#else
#include <unistd.h>
#define SLEEP usleep
#endif

// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include <string>

using namespace sml ;

using std::cout;
using std::cin;
using std::endl;
using std::string;

void printWMEs(WMElement const* pRoot)
{
	if (pRoot->GetParent() == NULL)
		cout << "Top Identifier " << pRoot->GetValueAsString() << endl ;
	else
	{
		cout << "(" << pRoot->GetParent()->GetIdentifierSymbol() << " ^" << pRoot->GetAttribute() << " " << pRoot->GetValueAsString() << ")" << endl ;
	}

	if (pRoot->IsIdentifier())
	{
		Identifier* pID = (Identifier*)pRoot ;
		int size = pID->GetNumberChildren() ;

		for (int i = 0 ; i < size ; i++)
		{
			WMElement const* pWME = pID->GetChild(i) ;

			printWMEs(pWME) ;
		}
	}
}

void RemoteConnection()
{
	sml::Kernel* pKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;

	if (pKernel)
	{
		sml::Agent* pAgent = pKernel->CreateAgent("test2") ;

		// Should be two agents here if we started a command line instance as the embedded connection
		// which creates an agent automatically (in the current test app)
		int nAgents = pKernel->GetNumberAgents() ;

		for (int i = 0 ; i < nAgents ; i++)
		{
			pAgent = pKernel->GetAgentByIndex(i) ;
			cout << "Found agent: " << pAgent->GetAgentName() << endl ;
		}

		SLEEP(1000) ;
	}

	delete pKernel ;
}

void SimpleEmbeddedConnection()
{
	// Create the kernel instance
	sml::Kernel* pKernel = sml::Kernel::CreateEmbeddedConnection("KernelSML", false) ;

	// Sleep for 40 seconds and then quit
	SLEEP(40*1000) ;

	delete pKernel ;
}

void MyRunEventHandler(smlEventId id, Agent* pAgent, smlPhase phase)
{
	cout << "Received an event callback" << endl ;
}

void EmbeddedConnection()
{
	cout << "TestClientSML app starting..." << endl << endl;

	cout << "Creating Connection..." << endl << endl;

	// We'll do the test in a block, so everything should have been
	// deleted when we test for memory leaks.
	{
		SimpleTimer timer ;

//		sml::Kernel* pKernel = sml::Kernel::CreateEmbeddedConnection("KernelSML", false) ;
		sml::Kernel* pKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;

		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl ;
			return ;
		}

		pKernel->SetTraceCommunications(true) ;

		char const* name = "test-client-sml" ;

		sml::Agent* pAgent = pKernel->GetAgent(name) ;

		if (!pAgent)
		{
			// NOTE: We don't delete the agent pointer.  It's owned by the kernel
			pAgent = pKernel->CreateAgent(name) ;
		}
		else
		{
			pAgent->InitSoar() ;
		}

		double time = timer.Elapsed() ;
		cout << "Time to initialize kernel and create agent: " << time << endl ;

		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl ;
			return ;
		}

		if (!pAgent)
			cout << "Error creating agent" << endl ;

		bool load = pAgent->LoadProductions("testsml.soar") ;

		if (pAgent->HadError())
		{
			cout << "ERROR loading productions: " << pAgent->GetLastErrorDescription() << endl ;
			return ;
		}

		Identifier* pInputLink = pAgent->GetInputLink() ;

		if (!pInputLink)
			cout << "Error getting input link" << endl ;

		// Some simple tests
		StringElement* pWME = pAgent->CreateStringWME(pInputLink, "my-att", "my-value") ;

		Identifier* pID = pAgent->CreateIdWME(pInputLink, "plane") ;
		StringElement* pWME1 = pAgent->CreateStringWME(pID, "type", "Boeing747") ;
		IntElement* pWME2    = pAgent->CreateIntWME(pID, "speed", 200) ;
		FloatElement* pWME3  = pAgent->CreateFloatWME(pID, "direction", 50.5) ;

		bool ok = pAgent->Commit() ;

		// Remove a wme
		pAgent->DestroyWME(pWME3) ;

		// Change the speed to 300
		pAgent->Update(pWME2, 300) ;

		// Create a new WME that shares the same id as plane
		//Identifier* pID2 = pAgent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;

		ok = pAgent->Commit() ;

		std::string trace = pAgent->Run(2) ;

		// Delete one of the shared WMEs to make sure that's ok
		// BUGBUG: Turns out in the current gSKI implementation this can cause a crash, if we do proper cleanup
		// If we release the children of pID here (as we should) then if there is a shared WME (like pID2)
		// it is left with a reference to children that have been deleted.  I'm pretty sure it's a ref counting bug in gSKI's AddWmeObjectLink method.
		// The fix I'm using here is to not create the shared WME (pID2) above here.
		pAgent->DestroyWME(pID) ;
		pAgent->Commit() ;

		// Test that we get a callback after the decision cycle runs
		pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunEventHandler) ;

		// Nothing should match here
		trace = pAgent->Run(2) ;

		// Then add some tic tac toe stuff which should trigger output
		Identifier* pSquare = pAgent->CreateIdWME(pInputLink, "square") ;
		StringElement* pEmpty = pAgent->CreateStringWME(pSquare, "content", "RANDOM") ;
		IntElement* pRow = pAgent->CreateIntWME(pSquare, "row", 1) ;
		IntElement* pCol = pAgent->CreateIntWME(pSquare, "col", 2) ;

		ok = pAgent->Commit() ;

		// Update the square's value to be empty.  This ensures that the update
		// call is doing something.  Otherwise, when we run we won't get a match.
		pAgent->Update(pEmpty, "EMPTY") ;
		ok = pAgent->Commit() ;

		// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
//		pAgent->Run(2) ; // Have to run 2 decisions as we may not be stopped at the right phase for input->decision->output it seems
		trace = pAgent->RunTilOutput(20) ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)

		bool ioOK = false ;

		// If we have output, dump it out.
		if (pAgent->GetOutputLink())
		{
			printWMEs(pAgent->GetOutputLink()) ;

			// Now update the output link with "status complete"
			Identifier* pMove = (Identifier*)pAgent->GetOutputLink()->FindByAttribute("move", 0) ;

			// We add an "alternative" to check that we handle shared WMEs correctly.
			// Look it up here.
			Identifier* pAlt = (Identifier*)pAgent->GetOutputLink()->FindByAttribute("alternative", 0) ;

			if (pAlt)
			{
				cout << "Found alternative " << pAlt->GetValueAsString() << endl ;
			}
	
			// Should also be able to get the command through the "GetCommands" route which tests
			// whether we've flagged the right wmes as "just added" or not.
			int numberCommands = pAgent->GetNumberCommands() ;

			// Get the first two commands (move and alternate)
			Identifier* pCommand1 = pAgent->GetCommand(0) ;
			Identifier* pCommand2 = pAgent->GetCommand(1) ;

			if (numberCommands == 2 && (strcmp(pCommand1->GetCommandName(), "move") == 0 || (strcmp(pCommand2->GetCommandName(), "move") == 0)))
			{
				cout << "Found move command" << endl ;
			}
			else
			{
				cout << "*** ERROR: Failed to find the move command" << endl ;
			}

			pAgent->ClearOutputLinkChanges() ;

			int clearedNumberCommands = pAgent->GetNumberCommands() ;

			if (clearedNumberCommands != 0)
			{
				cout << "*** ERROR: Clearing the list of output link changes failed" << endl ;
			}

			if (pMove)
			{
				cout << "Marking command as completed." << endl ;
				StringElement* pCompleted = pAgent->CreateStringWME(pMove, "status", "complete") ;
				ioOK = true ;
			}

			pAgent->Commit() ;
		}
		else
		{
			cout << " ERROR: No output generated." << endl ;
		}

		// The move command should be deleted in response to the
		// the status complete getting added
		trace = pAgent->Run(2) ;

		// Dump out the output link again.
		if (pAgent->GetOutputLink())
		{
			printWMEs(pAgent->GetOutputLink()) ;
		}

		if (!ioOK)
		{
			cout << "*** ERROR: Test failed to send and receive output correctly." << endl ;
		}

		cout << endl << "If this test worked should see something like this (above here):" << endl ;
		cout << "Top Identifier I3" << endl << "(I3 ^move M1)" << endl << "(M1 ^row 1)" << endl ;
		cout << "(M1 ^col 1)" << endl << "(I3 ^alternative M1)" << endl ;
		cout << "And then after the command is marked as completed (during the test):" << endl ;
		cout << "Top Identifier I3" << endl ;
		cout << "Together with about 6 received events" << endl ;

		// Cycle forever until debugger quits (if we're using the tcl debugger)
		/*
		while (pKernel->CheckForIncomingCommands())
		{
			SLEEP(10) ;
		}
		*/

		cout << "Destroy the agent now" << endl ;

		// Explicitly destroy our agent as a test, before we delete the kernel itself.
		// (Actually, if this is a remote connection we need to do this or the agent
		//  will remain alive).
		ok = pKernel->DestroyAgent(pAgent) ;

		if (!ok)
			cout << "*** ERROR: Failed to destroy agent properly ***" ;

		delete pKernel ;

		//destroy connection
		//cout << "Closing connection..." << endl << endl;
		//pConnection->CloseConnection();
		//delete pConnection ;
	}// closes testing block scope
}

int main(int argc, char* argv[])
{
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 140 ;

	SimpleTimer timer ;

	// For now, any argument on the command line makes us create a remote connection.
	// Later we'll try passing in an ip address/port number.
	if (argc > 1)
		SimpleEmbeddedConnection() ;
		//RemoteConnection() ;
	else
		EmbeddedConnection() ;

	double time = timer.Elapsed() ;
	cout << "Total run time: " << time << endl ;

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

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <return> to exit\n") ;
	char line[100] ;
	char* str = gets(line) ;
}
