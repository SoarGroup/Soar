#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

//#include "sml_Connection.h"
#include "sml_Client.h"

//#include "../../Profiler/include/simple_timer.h"

// Define a sleep
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define SLEEP Sleep
#else
#include <unistd.h>
#define SLEEP usleep
#endif

// helps quell warnings
#ifndef unused
#define unused(x) (void)(x)
#endif

// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif // _MSC_VER

#include <iostream>
#include <fstream>
#include <string>

#if HAVE_STRINGS_H
#include <strings.h>
#if HAVE_STRCASECMP
#define stricmp strcasecmp
#endif // HAVE_STRCASECMP
#endif // HAVE_STRINGS_H

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

std::string ListenerRhsFunctionHandler(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument)
{
	// Return the argument we are passed plus some of our text
	cout << "Received rhs function call with argument: " << pArgument << endl ;

	std::string res = "my listener result " ;
	res += pArgument ;

	return res ;
}

// Create a process that listens for remote commands and lives
// for "life" 10'ths of a second (e.g. 10 = live for 1 second)
bool SimpleListener(int life)
{
	// Choose how to connect (usually use NewThread) but for
	// testing currentThread can be helpful.
	bool useCurrentThread = false ;

	// Create the kernel instance
	sml::Kernel* pKernel = useCurrentThread ? sml::Kernel::CreateKernelInCurrentThread("SoarKernelSML") :
											  sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	// Register here so we can test the order that RHS functions are called
	int callback_rhs1 = pKernel->AddRhsFunction("test-rhs", &ListenerRhsFunctionHandler, 0) ;
	unused(callback_rhs1);

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	int pause = 100 ;

	// If we're running in this thread we need to wake up more rapidly.
	if (useCurrentThread)
	{ life *= 10 ; pause /= 10 ; }

	for (int i = 0 ; i < life ; i++)
	{
		// Don't need to check for incoming as we're using the NewThread model.
		// (If we switch to Client we'd need to cut this sleep down a lot to
		//  get any kind of responsiveness).
		if (useCurrentThread) {
			bool check = pKernel->CheckForIncomingCommands() ;
			unused(check);
		}

		SLEEP(pause) ;
	}

	delete pKernel ;

	return true ;
}

void MyRunEventHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	int* pInt = (int*)pUserData ;

	// Increase the count
	*pInt = *pInt + 1 ;

	cout << "Received an event callback" << endl ;
}

// Register a second handler for the same event, to make sure that's ok.
void MyDuplicateRunEventHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	int* pInt = (int*)pUserData ;
	if (*pInt != 25)
		cout << "***ERROR*** getting user data from callback" << endl ;

	cout << "Received the event in my 2nd handler too" << endl ;
}

void MyInterruptHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	pAgent->GetKernel()->StopAllAgents() ;
}

void MySystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	cout << "Received kernel event" << endl ;
}

void MyDeletionHandler(smlAgentEventId id, void* pUserData, Agent* pAgent)
{
	cout << "Received notification before agent was deleted" << endl ;
}

void MyCreationHandler(smlAgentEventId id, void* pUserData, Agent* pAgent)
{
	cout << "Received notification when agent was created" << endl ;
}

void MyAgentEventHandler(smlAgentEventId id, void* pUserData, Agent* pAgent)
{
	cout << "Received agent event" << endl ;
}

void MySystemHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	int* pInt = (int*)pUserData ;

	// Increase the count
	*pInt = *pInt + 1 ;

	if (id == smlEVENT_SYSTEM_START)
		cout << "Received system start" << endl ;
	else if (id == smlEVENT_SYSTEM_STOP)
		cout << "Received system stop" << endl ;
}

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
	// In this case the user data is a string we're building up
	std::string* pTrace = (std::string*)pUserData ;

	(*pTrace) += pMessage ;
}

static ClientXML* s_ClientXMLStorage = 0 ;

void MyXMLEventHandler(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML)
{
	// pXML should be some structured trace output.
	// Let's examine it a bit.
	// We'll start by turning it back into XML so we can look at it in the debugger.
	char* pStr = pXML->GenerateXMLString(true) ;

	// This will always succeed.  If this isn't really trace XML
	// the methods checking on tag names etc. will just fail
	ClientTraceXML* pRootXML = pXML->ConvertToTraceXML() ;

	// The root object is just a <trace> tag.  The substance is in the children
	// so we'll get the first child which should exist.
	ClientTraceXML childXML ;
	bool ok = pRootXML->GetChild(&childXML, 0) ;
	ClientTraceXML* pTraceXML = &childXML ;

	if (!ok)
	{
		cout << "*** Error getting child from trace XML object" << endl ;
		return ;
	}

	if (pTraceXML->IsTagState())
	{
		std::string count = pTraceXML->GetDecisionCycleCount() ;
		std::string stateID = pTraceXML->GetStateID() ;
		std::string impasseObject = pTraceXML->GetImpasseObject() ;
		std::string impasseType = pTraceXML->GetImpasseType() ;

		cout << "Trace ==> " << count << ":" << stateID << " (" << impasseObject << " " << impasseType << ")" << endl ;
	}

	// Make a copy of the object we've been passed which should remain valid
	// after the event handler has completed.  We only keep the last message
	// in this test.  This is a stress test for our memory allocation logic.
	// We're not allowed to keep pXML that we're passed, but we can copy it and keep the copy.
	// (The copy is very efficient, the underlying object is ref-counted).
	if (s_ClientXMLStorage != NULL)
		delete s_ClientXMLStorage ;
	s_ClientXMLStorage = new ClientXML(pXML) ;

	pXML->DeleteString(pStr) ;
}

std::string MyRhsFunctionHandler(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument)
{
	cout << "Received rhs function call with argument: " << pArgument << endl ;

	std::string res = "my rhs result " ;
	res += pArgument ;

	return res ;
}

bool TestAgent(Kernel* pKernel, Agent* pAgent, bool doInitSoars)
{
	// Record a RHS function
	int callback_rhs1 = pKernel->AddRhsFunction("test-rhs", &MyRhsFunctionHandler, 0) ; 
	int callback_rhs_dup = pKernel->AddRhsFunction("test-rhs", &MyRhsFunctionHandler, 0) ;

	if (callback_rhs_dup != callback_rhs1)
	{
		cout << "Error registering a duplicate RHS function.  This should be detected and be ignored" << endl ;
		return false ;
	}

	Identifier* pInputLink = pAgent->GetInputLink() ;
	if (!pInputLink)
		cout << "Error getting input link" << endl ;

	if (doInitSoars)
		pAgent->InitSoar() ;

	cout << "Done our first init-soar" << endl ;

	// Some simple tests
	StringElement* pWME = pAgent->CreateStringWME(pInputLink, "my-att", "my-value") ;
	unused(pWME);
	Identifier* pID = pAgent->CreateIdWME(pInputLink, "plane") ;

	bool ok = pAgent->Commit() ;
	if (doInitSoars)
		pAgent->InitSoar() ;

	StringElement* pWME1 = pAgent->CreateStringWME(pID, "type", "Boeing747") ;
	unused(pWME1);
	IntElement* pWME2    = pAgent->CreateIntWME(pID, "speed", 200) ;
	FloatElement* pWME3  = pAgent->CreateFloatWME(pID, "direction", 50.5) ;

	ok = pAgent->Commit() ;

	if (doInitSoars)
		pAgent->InitSoar() ;
	
	// Remove a wme
	pAgent->DestroyWME(pWME3) ;

	// Change the speed to 300
	pAgent->Update(pWME2, 300) ;

	// Create a new WME that shares the same id as plane
	Identifier* pID2 = pAgent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;

	ok = pAgent->Commit() ;

	/*
	printWMEs(pAgent->GetInputLink()) ;
	std::string printInput1 = pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput1 << endl ;
	cout << endl << "Now work with the input link" << endl ;
	*/

	// Delete one of the shared WMEs to make sure that's ok
	pAgent->DestroyWME(pID) ;
	pAgent->Commit() ;

	if (doInitSoars)
		pAgent->InitSoar() ;

	// Throw in a pattern as a test
	std::string pattern = pAgent->ExecuteCommandLine("print -i (s1 ^* *)") ;

	std::string expand = pKernel->ExpandCommandLine("p s1") ;
	bool isRunCommand = pKernel->IsRunCommand("d 3") ;
	isRunCommand = isRunCommand && pKernel->IsRunCommand("e 5") ;
	isRunCommand = isRunCommand && pKernel->IsRunCommand("run -d 10") ;

	if (!isRunCommand)
	{
		cout << "Error expanding run command aliases" << endl ;
		return false ;
	}

	// Test that we get a callback after the decision cycle runs
	// We'll pass in an "int" and use it to count decisions (just as an example of passing user data around)
	int count ;
	count = 0 ;
	int callback1 = pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunEventHandler, &count) ;
	int callback_dup = pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunEventHandler, &count) ;

	if (callback1 != callback_dup)
	{
		cout << "Error when registering the same function twice.  Should detect this and ignore the second registration" << endl ;
		return false ;
	}

	// Register another handler for the same event, to make sure we can do that.
	// Register this one ahead of the previous handler (so it will fire before MyRunEventHandler)
	bool addToBack = true ;
	int testData ;
	testData = 25 ;
	int callback2 = pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyDuplicateRunEventHandler, &testData, !addToBack) ;

	// Run returns the result (succeeded, failed etc.)
	// To catch the trace output we have to register a print event listener
	std::string trace ;	// We'll pass this into the handler and build up the output in it
	std::string structured ;	// Structured trace goes here
	int callbackp = pAgent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, &trace) ;
	int callbackx = pAgent->RegisterForXMLEvent(smlEVENT_XML_TRACE_OUTPUT, MyXMLEventHandler, NULL) ;

	int beforeCount = 0 ;
	int afterCount = 0 ;
	int callback_before = pAgent->RegisterForRunEvent(smlEVENT_BEFORE_RUN_STARTS, MyRunEventHandler, &beforeCount) ;
	int callback_after = pAgent->RegisterForRunEvent(smlEVENT_AFTER_RUN_ENDS, MyRunEventHandler, &afterCount) ;

	//Some temp code to generate more complex watch traces.  Not usually part of the test
	/*
	Identifier* pSquare1 = pAgent->CreateIdWME(pInputLink, "square") ;
	StringElement* pEmpty1 = pAgent->CreateStringWME(pSquare1, "content", "RANDOM") ;
	IntElement* pRow1 = pAgent->CreateIntWME(pSquare1, "row", 1) ;
	IntElement* pCol1 = pAgent->CreateIntWME(pSquare1, "col", 2) ;
	pAgent->Update(pEmpty1, "EMPTY") ;
	ok = pAgent->Commit() ;
	pAgent->ExecuteCommandLine("watch 3") ;
	*/

	// Nothing should match here
	std::string result = pAgent->RunSelf(4) ;

	if (beforeCount != 1 || afterCount != 1)
	{
		cout << "Error receiving BFORE_RUN_STARTS/AFTER_RUN_ENDS events" << endl ;
		return false ;
	}

	// By this point the static variable ClientXMLStorage should have been filled in 
	// and it should be valid, even though the event handler for MyXMLEventHandler has completed.
	if (s_ClientXMLStorage == NULL)
	{
		cout << "Error receiving XML trace events" << endl ;
		return false ;
	}

	// If we crash on this access there's a problem with the ref-counting of
	// the XML message we're passed in MyXMLEventHandler.
	if (!s_ClientXMLStorage->ConvertToTraceXML()->IsTagTrace())
	{
		cout << "Error in storing an XML trace event" << endl ;
		return false ;
	}
	delete s_ClientXMLStorage ;
	s_ClientXMLStorage = NULL ;

	pAgent->UnregisterForXMLEvent(callbackx) ;
	pAgent->UnregisterForPrintEvent(callbackp) ;
	pAgent->UnregisterForRunEvent(callback_before) ;
	pAgent->UnregisterForRunEvent(callback_after) ;

	// Print out the standard trace and the same thing as a structured XML trace
	cout << trace << endl ;
	cout << structured << endl ;

	/*
	printWMEs(pAgent->GetInputLink()) ;
	std::string printInput = pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput << endl ;
	*/

	// Then add some tic tac toe stuff which should trigger output
	Identifier* pSquare = pAgent->CreateIdWME(pInputLink, "square") ;
	StringElement* pEmpty = pAgent->CreateStringWME(pSquare, "content", "RANDOM") ;
	IntElement* pRow = pAgent->CreateIntWME(pSquare, "row", 1) ;
	unused(pRow);
	IntElement* pCol = pAgent->CreateIntWME(pSquare, "col", 2) ;
	unused(pCol);

	ok = pAgent->Commit() ;

	// Update the square's value to be empty.  This ensures that the update
	// call is doing something.  Otherwise, when we run we won't get a match.
	pAgent->Update(pEmpty, "EMPTY") ;
	ok = pAgent->Commit() ;

	int myCount ;
	myCount = 0 ;
	int callback_run_count = pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunEventHandler, &myCount) ;

	//cout << "About to do first run-til-output" << endl ;

	// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
	// We'll use RunAll just to test it out.  Could use RunSelf and get same result (presumably)
	trace = pKernel->RunAllTilOutput(20) ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)

	// We should stop quickly (after a decision or two)
	if (myCount > 10)
	{
		cout << "Error in RunTilOutput -- it didn't stop on the output" << endl ;
		return false ;
	}
	if (myCount == 0)
	{
		cout << "Error in callback handler for MyRunEventHandler -- failed to update count" << endl ;
		return false ;
	}
	cout << "Agent ran for " << myCount << " decisions before we got output" << endl ;

	// Reset the agent and repeat the process to check whether init-soar works.
	if (doInitSoars)
	{
		pAgent->InitSoar() ;
		trace = pAgent->RunSelfTilOutput(20) ;
	}

	bool ioOK = false ;

	//cout << "Time to dump output link" << endl ;

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
			unused(pCompleted);
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
	trace = pAgent->RunSelf(2) ;

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

	// Test that we can interrupt a run by registering a handler that
	// interrupts Soar immediately after a decision cycle.
	// Removed the test part for now. Stats doesn't report anything.
	int callback3 = pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyInterruptHandler, 0) ;

	if (doInitSoars)
		pAgent->InitSoar() ;

	pAgent->RunSelf(20) ;

	std::string stats = pAgent->ExecuteCommandLine("stats") ;
	size_t pos = stats.find("1 decision cycles") ;
	unused(pos);
/*
	if (pos == std::string.npos)
	{
		cout << "*** ERROR: Failed to interrupt Soar during a run." << endl ;
		return false ;
	}
*/
	bool unreg = pAgent->UnregisterForRunEvent(callback3) ;

	if (!unreg)
	{
		cout << "Error unregistering callback3" << endl ;
		return false ;
	}

	bool unregRhs = pKernel->RemoveRhsFunction(callback_rhs1) ;
	unused(unregRhs);

	if (!unreg)
	{
		cout << "Error unregistering rhs function" << endl ;
		return false ;
	}

	/* These comments haven't kept up with the test -- does a lot more now
	cout << endl << "If this test worked should see something like this (above here):" << endl ;
	cout << "Top Identifier I3" << endl << "(I3 ^move M1)" << endl << "(M1 ^row 1)" << endl ;
	cout << "(M1 ^col 1)" << endl << "(I3 ^alternative M1)" << endl ;
	cout << "And then after the command is marked as completed (during the test):" << endl ;
	cout << "Top Identifier I3" << endl ;
	cout << "Together with about 6 received events" << endl ;
	*/

	cout << "Destroy the agent now" << endl ;

	unreg = pAgent->UnregisterForRunEvent(callback1) ;
	unreg = unreg && pAgent->UnregisterForRunEvent(callback2) ;
	unreg = unreg && pAgent->UnregisterForRunEvent(callback_run_count) ;

	if (!unreg)
	{
		cout << "Error unregistering callback1" << endl ;
		return false ;
	}

	return true ;
}

void MyXMLEventHandlerTimer(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML)
{
	// Don't do any work in the timer case -- but register the event handler so we generate
	// the data and send it to us.

    //This code is for debugging what comes out of the trace
    /*
    std::fstream fs;
    fs.open("test.xml", std::ios_base::out | std::ios_base::app);
    char* temp = pXML->GenerateXMLString(true);
    fs << temp;
    ClientXML::DeleteString(temp);
    fs.close();
    */
}

bool TimeTest(bool embedded, bool useClientThread, bool fullyOptimized)
{
	// Create the appropriate type of connection
	sml::Kernel* pKernel = embedded ?
		(useClientThread ? sml::Kernel::CreateKernelInCurrentThread("SoarKernelSML", fullyOptimized, Kernel::GetDefaultPort())
		: sml::Kernel::CreateKernelInNewThread("SoarKernelSML", Kernel::GetDefaultPort()))
		: sml::Kernel::CreateRemoteConnection(true, NULL) ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	// This number determines how many agents are created.  We create <n>, test <n> and then delete <n>
	int numberAgents = 1 ;

	sml::Agent* pFirst = NULL ;

	// PHASE ONE: Agent creation
	for (int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++)
	{
		std::string name = "test-client-sml" ;
		name.push_back('1' + agentCounter) ;

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		sml::Agent* pAgent = pKernel->CreateAgent(name.c_str()) ;

		if (!pFirst)
			pFirst = pAgent ;

		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl ;
			return false ;
		}

		if (!pAgent)
		{
			cout << "Error creating agent" << endl ;
			return false ;
		}

		bool ok = true ;

		std::string path = pKernel->GetLibraryLocation() ;
		path += "/demos/towers-of-hanoi/towers-of-hanoi.soar" ;

		bool load = pAgent->LoadProductions(path.c_str()) ;
		unused(load);

		if (pAgent->HadError())
		{
			cout << "ERROR loading productions: " << pAgent->GetLastErrorDescription() << endl ;
			return false ;
		}

		cout << "Loaded productions" << endl ;

		if (!ok)
			return false ;

		pAgent->RegisterForXMLEvent(smlEVENT_XML_TRACE_OUTPUT, &MyXMLEventHandlerTimer, NULL) ;
		pAgent->ExecuteCommandLine("watch 5") ;
	}

	std::string result = pFirst->ExecuteCommandLine("time run 100") ;
	cout << result << endl ;

	// Need to get rid of the kernel explictly.
	delete pKernel ;

	return true ;
}

bool TestSML(bool embedded, bool useClientThread, bool fullyOptimized, bool simpleInitSoar)
{
	cout << "TestClientSML app starting..." << endl << endl;

	cout << "Creating Connection..." << endl << endl;

	// We'll do the test in a block, so everything should have been
	// deleted when we test for memory leaks.
	{
		//SimpleTimer timer ;

		// Create the appropriate type of connection
		sml::Kernel* pKernel = embedded ?
			(useClientThread ? sml::Kernel::CreateKernelInCurrentThread("SoarKernelSML", fullyOptimized, Kernel::GetDefaultPort())
			: sml::Kernel::CreateKernelInNewThread("SoarKernelSML", Kernel::GetDefaultPort()))
			: sml::Kernel::CreateRemoteConnection(true, NULL) ;

		if (pKernel->HadError())
		{
			cout << pKernel->GetLastErrorDescription() << endl ;
			return false ;
		}

		// Set this to true to give us lots of extra debug information on remote clients
		// (useful in a test app like this).
	    // pKernel->SetTraceCommunications(true) ;

		// Register a kernel event handler...unfortunately I can't seem to find an event
		// that gSKI actually fires, so this handler won't get called.  Still, the code is there
		// on the SML side should anyone ever hook up this type of event inside the kernel/gSKI...
		pKernel->RegisterForSystemEvent(smlEVENT_AFTER_RESTART, MySystemEventHandler, NULL) ;

		int callback5 = pKernel->RegisterForAgentEvent(smlEVENT_AFTER_AGENT_CREATED, &MyCreationHandler, 0) ;
		unused(callback5);

		int nAgents = pKernel->GetNumberAgents() ;

		// Report the number of agents (always 0 unless this is a remote connection to a CLI or some such)
		cout << "Existing agents: " << nAgents << endl ;

		// This number determines how many agents are created.  We create <n>, test <n> and then delete <n>
		int numberAgents = 1 ;

		// PHASE ONE: Agent creation
		for (int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++)
		{
			std::string name = "test-client-sml" ;
			name.push_back('1' + agentCounter) ;

			// NOTE: We don't delete the agent pointer.  It's owned by the kernel
			sml::Agent* pAgent = pKernel->CreateAgent(name.c_str()) ;

			//double time = timer.Elapsed() ;
			//cout << "Time to initialize kernel and create agent: " << time << endl ;

			if (pKernel->HadError())
			{
				cout << pKernel->GetLastErrorDescription() << endl ;
				return false ;
			}

			if (!pAgent)
				cout << "Error creating agent" << endl ;

			bool ok = true ;

			if (simpleInitSoar) 
			{
				SLEEP(200) ;

				cout << "Performing simple init-soar..." << endl << endl;
				pAgent->InitSoar() ;

				SLEEP(200) ;

				ok = pKernel->DestroyAgent(pAgent) ;
				if (!ok)
				{
					cout << "*** ERROR: Failed to destroy agent properly ***" ;
					return false ;
				}
				delete pKernel ;
				return ok;
			}

			std::string path = pKernel->GetLibraryLocation() ;
			path += "/tests/testsml.soar" ;

			bool load = pAgent->LoadProductions(path.c_str()) ;
			unused(load);

			if (pAgent->HadError())
			{
				cout << "ERROR loading productions: " << pAgent->GetLastErrorDescription() << endl ;
				return false ;
			}

			cout << "Loaded productions" << endl ;

			if (!ok)
				return false ;
		}

		// PHASE TWO: Agent tests
		for (int agentTests = 0 ; agentTests < numberAgents ; agentTests++)
		{
			std::string name = "test-client-sml" ;
			name.push_back('1' + agentTests) ;

			Agent* pAgent = pKernel->GetAgent(name.c_str()) ;

			if (!pAgent)
				return false ;

			// Run a suite of tests on this agent
			bool ok = TestAgent(pKernel, pAgent, true) ;

			if (!ok)
				return false ;
		}

		// PHASE THREE: Agent creation
		for (int agentDeletions = 0 ; agentDeletions < numberAgents ; agentDeletions++)
		{
			std::string name = "test-client-sml" ;
			name.push_back('1' + agentDeletions) ;

			Agent* pAgent = pKernel->GetAgent(name.c_str()) ;

			if (!pAgent)
				return false ;

			// The Before_Agent_Destroyed callback is a tricky one so we'll register for it to test it.
			// We need to get this callback just before the agentSML data is deleted (otherwise there'll be no way to send/receive the callback)
			// and then continue on to delete the agent after we've responded to the callback.
			// Interestingly, we don't explicitly unregister this callback because the agent has already been destroyed so
			// that's another test, that this callback is cleaned up correctly (and automatically).
			int callback4 = pKernel->RegisterForAgentEvent(smlEVENT_BEFORE_AGENT_DESTROYED, &MyDeletionHandler, 0) ;
			unused(callback4);

			// Explicitly destroy our agent as a test, before we delete the kernel itself.
			// (Actually, if this is a remote connection we need to do this or the agent
			//  will remain alive).
			bool ok = pKernel->DestroyAgent(pAgent) ;

			if (!ok)
			{
				cout << "*** ERROR: Failed to destroy agent properly ***" ;
				return false ;
			}
		}

		// Delete the kernel.  If this is an embedded connection this destroys the kernel.
		// If it's a remote connection we just disconnect.
		delete pKernel ;

	}// closes testing block scope

	return true ;
}

bool FullTimeTest()
{
	// Embeddded using direct calls
	bool ok = TimeTest(true, true, true) ;

	// Embedded not using direct calls
//	ok = ok && TestSML(true, true, false) ;

	// Embedded running on thread inside kernel
//	ok = ok && TestSML(true, false, false) ;

	return ok ;
}

bool FullEmbeddedTest()
{
	// Simple embedded, direct init-soar
	bool ok = TestSML(true, true, true, true) ;

	// Embeddded using direct calls
	ok = ok && TestSML(true, true, true, false) ;

	// Embedded not using direct calls
	ok = ok && TestSML(true, true, false, false) ;

	// Embedded running on thread inside kernel
	ok = ok && TestSML(true, false, false, false) ;

	return ok ;
}

bool RemoteTest()
{
	// Remote connection.
	// (For this to work need to run a listener--usually TestCommandLineInterface to receive the commands).
	bool ok = TestSML(false, false, false, false) ;
	return ok ;
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
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 389 ;

	//SimpleTimer timer ;

	bool stopAtEnd = true ;
	bool remote    = false ;
	bool listener  = false ;
	bool timeTest  = false ;
	int  life      = 3000 ;	// Default is to live for 3000 seconds (5 mins) as a listener

	// For now, any argument on the command line makes us create a remote connection.
	// Later we'll try passing in an ip address/port number.
	bool success = true ;

	// Read the command line options:
	// -nostop : don't ask user to hit return at the end
	// -remote : run the test over a remote connection -- needs a listening client to already be running.
	// -listener : create a listening client (so we can run remote tests) which lives for 300 secs (5 mins)
	// -shortlistener : create a listening client that lives for 15 secs
	// -time : run a time trial on some functionality
	if (argc > 1)
	{
		for (int i = 1 ; i < argc ; i++)
		{
			if (!stricmp(argv[i], "-nostop"))
				stopAtEnd = false ;
			if (!stricmp(argv[i], "-remote"))
				remote = true ;
			if (!stricmp(argv[i], "-listener"))
				listener = true ;
			if (!stricmp(argv[i], "-time"))
				timeTest = true ;
			if (!stricmp(argv[i], "-shortlistener"))
			{
				listener = true ;
				life = 150 ; // Live for 15 secs only
			}
		}
	}

	// Running a listener isn't really a test.  It just provides the other side for a remote test
	// (so run one instance as a listener and then a second as a -remote test).
	if (listener)
		SimpleListener(life) ;
	else if (timeTest)
		FullTimeTest() ;
	else
	{
		if (remote)
			success = RemoteTest() ;
		else
			success = FullEmbeddedTest() ;

		ReportResult(remote ? "testclientsml-remote" : "testclientsml", success) ;

		//double time = timer.Elapsed() ;
		//cout << "Total run time: " << time << endl ;
	}

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

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	if (stopAtEnd)
	{
		printf("\n\nPress <return> to exit\n") ;
		char line[100] ;
		char* str = gets(line) ;
		unused(str);
	}
#endif // _MSC_VER
}
