
#include "sml_Connection.h"
#include "sml_Client.h"

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

int main(int argc, char* argv[])
{
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 62 ;

	cout << "TestClientSML app starting..." << endl << endl;

	cout << "Creating Connection..." << endl << endl;

	// We'll do the test in a block, so everything should have been
	// deleted when we test for memory leaks.
	{
		sml::Kernel* pKernel = sml::Kernel::CreateEmbeddedConnection("KernelSML") ;

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		sml::Agent* pAgent = pKernel->CreateAgent("test") ;

		if (!pAgent)
			cout << "Error creating agent" << endl ;

		pAgent->LoadProductions("testsml.soar") ;

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
		Identifier* pID2 = pAgent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;

		ok = pAgent->Commit() ;

		pAgent->Run(2) ;

		// Delete one of the shared WMEs to make sure that's ok
		pAgent->DestroyWME(pID) ;
		pAgent->Commit() ;

		// Nothing should match here
		pAgent->Run(2) ;

		// Then add some tic tac toe stuff which should trigger output
		Identifier* pSquare = pAgent->CreateIdWME(pInputLink, "square") ;
		StringElement* pEmpty = pAgent->CreateStringWME(pSquare, "content", "EMPTY") ;
		IntElement* pRow = pAgent->CreateIntWME(pSquare, "row", 1) ;
		IntElement* pCol = pAgent->CreateIntWME(pSquare, "col", 2) ;

		ok = pAgent->Commit() ;

		// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
//		pAgent->Run(2) ; // Have to run 2 decisions as we may not be stopped at the right phase for input->decision->output it seems
		pAgent->RunTilOutput(20) ;	// Should just run a decision or two.

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
		pAgent->Run(2) ;

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

		// Cycle forever until debugger quits (if we're using the tcl debugger)
		while (pKernel->CheckForIncomingCommands())
		{
			SLEEP(10) ;
		}

		// Explicitly destroy our agent as a test, before we delete the kernel itself.
		ok = pKernel->DestroyAgent(pAgent) ;

		if (!ok)
			cout << "*** ERROR: Failed to destroy agent properly ***" ;

		delete pKernel ;

		//destroy connection
		//cout << "Closing connection..." << endl << endl;
		//pConnection->CloseConnection();
		//delete pConnection ;
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