
#include "sml_Connection.h"
#include "sml_Client.h"

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

		pAgent->LoadProductions("test.soar") ;

		Identifier* pInputLink = pAgent->GetInputLink() ;

		if (!pInputLink)
			cout << "Error getting input link" << endl ;

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

		ok = pAgent->Commit() ;

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