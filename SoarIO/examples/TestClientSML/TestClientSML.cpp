#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

//#include "sml_Connection.h"
#include "sml_Client.h"

#include "../../Profiler/include/simple_timer.h"

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

#ifdef _MSC_VER
#include <crtdbg.h>
#endif // _MSC_VER

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

void SimpleRemoteConnection()
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

void MyRunEventHandler(smlEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	int* pInt = (int*)pUserData ;
	if (*pInt != 25)
		cout << "***ERROR*** getting user data from callback" << endl ;

	cout << "Received an event callback" << endl ;
}

// Register a second handler for the same event, to make sure that's ok.
void MyDuplicateRunEventHandler(smlEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	int* pInt = (int*)pUserData ;
	if (*pInt != 25)
		cout << "***ERROR*** getting user data from callback" << endl ;

	cout << "Received the event in my 2nd handler too" << endl ;
}

void MySystemEventHandler(smlEventId id, void* pUserData, Kernel* pKernel)
{
	cout << "Received kernel event" << endl ;
}

bool TestSML(bool embedded, bool useClientThread, bool fullyOptimized)
{
	cout << "TestClientSML app starting..." << endl << endl;

	cout << "Creating Connection..." << endl << endl;

	// We'll do the test in a block, so everything should have been
	// deleted when we test for memory leaks.
	{
		SimpleTimer timer ;

		// Create the appropriate type of connection
		sml::Kernel* pKernel = embedded ? sml::Kernel::CreateEmbeddedConnection("KernelSML", useClientThread, fullyOptimized)
										: sml::Kernel::CreateRemoteConnection(true, NULL) ;

		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl ;
			return false ;
		}

		// Give us lots of extra debug information on remote clients
		// (useful in a test app like this).
		pKernel->SetTraceCommunications(true) ;

		// Register a kernel event handler...unfortunately I can't seem to find an event
		// that gSKI actually fires, so this handler won't get called.  Still, the code is there
		// on the SML side should anyone ever hook up this type of event inside the kernel/gSKI...
		pKernel->RegisterForSystemEvent(smlEVENT_AFTER_RESTART, MySystemEventHandler, NULL) ;

		char const* name = "test-client-sml" ;

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		sml::Agent* pAgent = pKernel->CreateAgent(name) ;

		double time = timer.Elapsed() ;
		cout << "Time to initialize kernel and create agent: " << time << endl ;

		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl ;
			return false ;
		}

		if (!pAgent)
			cout << "Error creating agent" << endl ;

		bool load = pAgent->LoadProductions("testsml.soar") ;

		if (pAgent->HadError())
		{
			cout << "ERROR loading productions: " << pAgent->GetLastErrorDescription() << endl ;
			return false ;
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

		// Throw in a quick init-soar which will cause us to send over the input link again
		// This is just to test init-soar works.
		pAgent->InitSoar() ;

		// Remove a wme
		pAgent->DestroyWME(pWME3) ;

		// Change the speed to 300
		pAgent->Update(pWME2, 300) ;

		// Added some code to check function call speed
		// We're not usually going to need this
		/*
		{
			SimpleTimer speed ;
			speed.Start() ;
			for (int i = 0 ; i < 10000 ; i++)
			{
				pAgent->Update(pWME2, i) ;
				pAgent->Commit() ;
			}
			double speedTime = speed.Elapsed() ;
			cout << " Speed test time " << speedTime << endl ;
		}
		*/
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
		int userData = 25 ;
		pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunEventHandler, &userData) ;

		// Register another handler for the same event, to make sure we can do that.
		pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyDuplicateRunEventHandler, &userData) ;

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
//		trace = pAgent->Run(2) ; // Have to run 2 decisions as we may not be stopped at the right phase for input->decision->output it seems
		trace = pAgent->RunTilOutput(20) ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)

		// Reset the agent and repeat the process to check whether init-soar works.
		pAgent->InitSoar() ;
		trace = pAgent->RunTilOutput(20) ;

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
			else
			{
				return false ;
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
				return false ;
			}

			pAgent->ClearOutputLinkChanges() ;

			int clearedNumberCommands = pAgent->GetNumberCommands() ;

			if (clearedNumberCommands != 0)
			{
				cout << "*** ERROR: Clearing the list of output link changes failed" << endl ;
				return false ;
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
			return false ;
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
			return false ;
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

		pAgent->UnregisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunEventHandler, &userData) ;

		// Explicitly destroy our agent as a test, before we delete the kernel itself.
		// (Actually, if this is a remote connection we need to do this or the agent
		//  will remain alive).
		ok = pKernel->DestroyAgent(pAgent) ;

		if (!ok)
		{
			cout << "*** ERROR: Failed to destroy agent properly ***" ;
			return false ;
		}

		delete pKernel ;

		//destroy connection
		//cout << "Closing connection..." << endl << endl;
		//pConnection->CloseConnection();
		//delete pConnection ;
	}// closes testing block scope

	return true ;
}

bool FullEmbeddedTest()
{
	// Embeddded using direct calls
	bool ok = TestSML(true, true, true) ;

	// Embedded not using direct calls
	ok = ok && TestSML(true, true, false) ;

	// Embedded running on thread inside kernel
	ok = ok && TestSML(true, false, false) ;

	return ok ;
}

bool RemoteTest()
{
	// Remote connection.
	// (For this to work need to run a listener--usually TestCommandLineInterface to receive the commands).
	bool ok = TestSML(false, false, false) ;
	return ok ;
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
	bool success = true ;

	if (argc > 1)
		success = RemoteTest() ;
		//success = TestSML(true, true, true) ;
		//SimpleEmbeddedConnection() ;
		//SimpleRemoteConnection() ;
	else
		success = FullEmbeddedTest() ;

	if (success)
		cout << "\nTests SUCCEEDED" << endl ;
	else
		cout << "\n*** ERROR *** Tests FAILED" << endl ;

	double time = timer.Elapsed() ;
	cout << "Total run time: " << time << endl ;

	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

#ifdef _MSC_VER
	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <return> to exit\n") ;
	char line[100] ;
	char* str = gets(line) ;
}
