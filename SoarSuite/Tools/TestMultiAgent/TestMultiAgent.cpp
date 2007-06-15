/////////////////////////////////////////////////////////////////
// TestMultiAgent
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : March 2006
//
// Test app to specifically check on multi agent behavior of SML systems.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include <assert.h>

#include "sml_Client.h"

// Define a sleep
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
void SLEEP(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
	Sleep((secs * 1000) + msecs) ;
}
#else
#include <time.h>
void SLEEP(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
	struct timespec sleeptime;
	sleeptime.tv_sec = secs;
	sleeptime.tv_nsec = msecs * 1000000;
	nanosleep(&sleeptime, 0);
}
#endif

// helps quell warnings
#ifndef unused
#define unused(x) (void)(x)
#endif

// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif // _MSC_VER

#include <iostream>
#include <fstream>
#include <string.h>

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

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
	// In this case the user data is a string we're building up
	std::string* pTrace = (std::string*)pUserData ;

	(*pTrace) += pMessage ;
}

bool CreateInput(sml::Agent* pAgent, int value)
{
	// This agent adds value1 to value2 inside the agent and puts the total on the output link.
	// We take it from the output link and puts it on the input link to generate a running total.
	sml::Identifier* pInputLink = pAgent->GetInputLink() ;
	sml::Identifier* pAdd = pAgent->CreateIdWME(pInputLink, "add") ;
	sml::WMElement* pValue1 = pAgent->CreateIntWME(pAdd, "value1", 2) ;
	sml::WMElement* pValue2 = pAgent->CreateIntWME(pAdd, "value2", value) ;

	pAgent->Commit() ;

	return pInputLink && pAdd && pValue1 && pValue2 ;
}

bool UpdateInput(sml::Agent* pAgent, int value)
{
	// Set value2 to a new value, triggering a new calculation
	sml::Identifier* pInputLink = pAgent->GetInputLink() ;
	sml::Identifier* pAdd = pInputLink->FindByAttribute("add", 0)->ConvertToIdentifier() ;
	assert(pAdd) ;

	sml::IntElement* pValue2 = pAdd->FindByAttribute("value2", 0)->ConvertToIntElement() ;
	assert(pValue2) ;

	pAgent->Update(pValue2, value) ;

	pAgent->Commit() ;

	return true ;
}

void MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, smlRunFlags runFlags)
{
	int agents = pKernel->GetNumberAgents() ;
	for (int agent = 0 ; agent < agents ; agent++)
	{
		sml::Agent* pAgent = pKernel->GetAgentByIndex(agent) ;

		char const* pIOString = pAgent->ExecuteCommandLine("print --depth 4 i1") ;
		cout << pIOString << endl ;

		// Make sure we can get the output link (had a bug where this wouldn't always work)
		sml::Identifier* pOutputLink = pAgent->GetOutputLink() ;
		assert(pOutputLink) ;

		// Read in the commands
		int numberCommands = pAgent->GetNumberCommands() ;
		for (int i = 0 ; i < numberCommands ; i++)
		{
			sml::Identifier* pCommand = pAgent->GetCommand(i) ;			
			char const* pName = pCommand->GetCommandName() ;

			assert(pName) ;
			assert(strcmp(pName, "result") == 0) ;

			// Receive the new total
			char const* pTotal = pCommand->GetParameterValue("total") ;
			assert(pTotal) ;
			std::string total = pTotal ;
			int intTotal = atoi(total.c_str()) ;

			// Mark command as completed in working memory
			pCommand->AddStatusComplete() ;

			// Place a new addition request on the input link
			UpdateInput(pAgent, intTotal) ;
		}
		pAgent->ClearOutputLinkChanges() ;
	}
}

void ReportAgentStatus(sml::Kernel* pKernel, int numberAgents, std::string trace[])
{
	for (int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++)
	{
		sml::Agent* pAgent = pKernel->GetAgentByIndex(agentCounter) ;
		cout << "Trace from agent " << pAgent->GetAgentName() << endl ;

		std::string input = pAgent->ExecuteCommandLine("print --depth 3 i2") ;
		cout << "Input link " << endl << input << endl ;

		cout << trace[agentCounter] << endl << endl ;

		// We need to clear this after it's been printed or the next time we print it
		// we'll get the entire trace from 0
		trace[agentCounter] = "" ;
	}
}

void InitAll(sml::Kernel* pKernel)
{
	int agents = pKernel->GetNumberAgents() ;
	for (int i = 0 ; i < agents ; i++)
	{
		sml::Agent* pAgent = pKernel->GetAgentByIndex(i) ;
		std::string initRes = pAgent->InitSoar() ;
		cout << initRes << endl ;
	}
}

bool TestMulti(int numberAgents)
{
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	// We'll require commits, just so we're testing that path
	pKernel->SetAutoCommit(false) ;

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	const int kMaxAgents = 100 ;

	if (numberAgents >= kMaxAgents)
	{
		cout << "Max agents is " << kMaxAgents << endl ;
		return false ;
	}

	std::string names[kMaxAgents] ;
	sml::Agent* agents[kMaxAgents] ;
	std::string trace[kMaxAgents] ;
	int			callbackPrint[kMaxAgents] ;

	// Create the agents
	for (int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++)
	{
		std::string name = "agent" ;
		name.push_back('1' + agentCounter) ;
		names[agentCounter] = name ;

		sml::Agent* pAgent   = pKernel->CreateAgent(name.c_str()) ;
		agents[agentCounter] = pAgent ;

		std::string path = std::string(pKernel->GetLibraryLocation()) + "/Tests/testmulti.soar" ;
		bool ok = pAgent->LoadProductions(path.c_str()) ;

		ok = ok && CreateInput(pAgent, 0) ;

		if (!ok)
			return false ;

		// Collect the trace output from the run
		callbackPrint[agentCounter] = pAgent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, &trace[agentCounter]) ;
	}

	int	callbackUpdate = pKernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_GENERATED_OUTPUT, MyUpdateEventHandler, NULL) ;

	// Run for a first set of output, so we can see whether that worked
	pKernel->RunAllTilOutput() ;

	// Print out some information
	ReportAgentStatus(pKernel, numberAgents, trace) ;

	// Now get serious about a decent run
	const int kFirstRun = 5 ;
	for (int i = 0 ; i < kFirstRun ; i++)
	{
		// Run for a bit
		char const* pResult = pKernel->RunAllTilOutput() ;
	}

	ReportAgentStatus(pKernel, numberAgents, trace) ;

	// Toss in an init-soar and then go on a bit further
	InitAll(pKernel) ;

	// Second run
	const int kSecondRun = 5 ;
	for (int i = 0 ; i < kSecondRun ; i++)
	{
		// Run for a bit
		char const* pResult = pKernel->RunAllTilOutput() ;
	}

	ReportAgentStatus(pKernel, numberAgents, trace) ;

	cout << "Calling shutdown on the kernel now" << endl ;
	pKernel->Shutdown() ;
	cout << "Shutdown completed now" << endl ;

	// Delete the kernel.  If this is an embedded connection this destroys the kernel.
	// If it's a remote connection we just disconnect.
	delete pKernel ;

	return true ;
}

// We create a file to say we succeeded or not, deleting any existing results beforehand
// The filename shows if things succeeded or not and the contents can explain further.
void ReportResult(std::string testName, bool success)
{
	// Decide on the filename to use for success/failure
	std::string kSuccess = testName + "-success.txt" ;
	std::string kFailure = testName + "-failure.txt" ;

	// Remove any existing result files
	remove(kSuccess.c_str()) ;
	remove(kFailure.c_str()) ;

	// Create the output file
	std::ofstream outfile (success ? kSuccess.c_str() : kFailure.c_str());

	if (success)
	{
		outfile << "\nTests SUCCEEDED" << endl ;
		cout << "\nTests SUCCEEDED" << endl ;
	}
	else
	{
		outfile << "\n*** ERROR *** Tests FAILED" << endl ;
		cout << "\n*** ERROR *** Tests FAILED" << endl ;
	}

	outfile.close();
}

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 550 ;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif // _MSC_VER

	bool stopAtEnd = true ;

	// For now, any argument on the command line makes us create a remote connection.
	// Later we'll try passing in an ip address/port number.
	bool success = true ;

	// Read the command line options:
	// -nostop : don't ask user to hit return at the end
	if (argc > 1)
	{
		for (int i = 1 ; i < argc ; i++)
		{
			if (!strcasecmp(argv[i], "-nostop"))
				stopAtEnd = false ;
		}
	}

	success = TestMulti(2) ;

	ReportResult("testmultiagent", success) ;

	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

	/*
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

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	if (stopAtEnd)
	{
		printf("\n\nPress <return> to exit\n") ;
		char line[100] ;
		char* str = gets(line) ;
		unused(str);
	}
#endif // _MSC_VER
	*/

	if (!success) return 1;
	return 0;
}
