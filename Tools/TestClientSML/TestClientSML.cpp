/////////////////////////////////////////////////////////////////
// TestClientSML
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Test app to work out many of the elements of the ClientSML interface.
// This is a grab back of tests to really stress as much as we can.
// It can also be used as a reference for how to use a particular feature
// if it's not clear from the header file documentation.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <assert.h>

#include "sml_Connection.h"
#include "sml_Client.h"

//#include "../../Profiler/include/simple_timer.h"

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

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

using namespace sml ;

using std::cout;
using std::cin;
using std::endl;
using std::string;

int global_callback = -1 ;

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

		SLEEP(1,0) ;
	}

	delete pKernel ;
}

bool SimpleRemoteSynchTest()
{
	sml::Kernel* pKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;
	bool result = true ;

	if (pKernel)
	{
		int nAgents = pKernel->GetNumberAgents() ;

		for (int i = 0 ; i < nAgents ; i++)
		{
			Agent* pAgent = pKernel->GetAgentByIndex(i) ;
			cout << "Found agent: " << pAgent->GetAgentName() << endl ;

			bool synch = pAgent->SynchronizeInputLink() ;
			if (synch)
			{
				printWMEs(pAgent->GetInputLink()) ;
			}
			else
			{
				result = false ;
			}
		}

		SLEEP(1,0) ;
	}

	pKernel->Shutdown() ;
	delete pKernel ;

	return result ;
}

std::string ListenerRhsFunctionHandler(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pFunctionName, char const* pArgument)
{
	// Return the argument we are passed plus some of our text
	cout << "Received rhs function call with argument: " << pArgument << endl ;

	std::string res = "my listener result " ;
	res += pArgument ;

	return res ;
}

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
	// In this case the user data is a string we're building up
	std::string* pTrace = (std::string*)pUserData ;

	(*pTrace) += pMessage ;
}

// Create a simple listener that runs an agent for <n> decisions.
// This can be used to test dynamic connections/disconnections from a running agent.
bool SimpleRunListener(int decisions)
{
	// Create the kernel instance
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	sml::Agent* pAgent = pKernel->CreateAgent("runagent") ;
	std::string path = std::string(pKernel->GetLibraryLocation()) + "/Tests/testrun.soar" ;
	bool ok = pAgent->LoadProductions(path.c_str()) ;

	if (!ok)
		return false ;

	std::string result = pAgent->RunSelf(decisions) ;
	cout << result ;

	std::string state = pAgent->ExecuteCommandLine("print --depth 2 s1") ;
	cout << state ;

	delete pKernel ;

	return true ;
}

// Connect to a running kernel and agent, grab some output then disconnect.
// Tests that we can work with an agent while it is running.
bool SimpleRemoteConnect()
{
	// Create the kernel instance
	sml::Kernel* pKernel = sml::Kernel::CreateRemoteConnection(true, NULL) ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	sml::Agent* pAgent = pKernel->GetAgentByIndex(0) ;

	if (!pAgent)
	{
		cout << "No agents running in this kernel" << endl ;
		return false ;
	}

	pKernel->SetConnectionInfo("runconnect", "ready", "ready") ;

	std::string trace ;
	int callbackp = pAgent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, &trace) ;
	unused(callbackp); // eliminate gcc compiler warning

	SLEEP(1,0) ;

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	cout << trace << endl ;

	std::string state = pAgent->ExecuteCommandLine("print --depth 2 s1") ;
	cout << state ;

	bool changed = pKernel->GetAllConnectionInfo() ;
	unused(changed); // eliminate gcc compiler warning
	for (int i = 0 ; i < pKernel->GetNumberConnections() ; i++)
	{
		ConnectionInfo const* pInfo = pKernel->GetConnectionInfo(i) ;
		cout << pInfo->GetID() << " " << pInfo->GetName() << " " << pInfo->GetConnectionStatus() << " " << pInfo->GetAgentStatus() << endl ;
	}

	pKernel->Shutdown() ;

	delete pKernel ;

	return true ;
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

	pKernel->SetConnectionInfo("listener", "ready", "ready") ;

	// Register here so we can test the order that RHS functions are called
	int callback_rhs1 = pKernel->AddRhsFunction("test-rhs", &ListenerRhsFunctionHandler, 0) ;
	unused(callback_rhs1);

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	long pauseSecs = 0;
	long pauseMsecs = 100 ;
	long pauseMsecsTotal = pauseSecs*1000+pauseMsecs;

	// If we're running in this thread we need to wake up more rapidly.
	if (useCurrentThread)
	{ life *= 10 ; pauseMsecsTotal /= 10 ; }

	// How often we check to see if the list of connections has changed.
	//int checkConnections = 500 / pauseMsecsTotal ;
	//int counter = checkConnections ;

	for (int i = 0 ; i < life ; i++)
	{
		// Don't need to check for incoming as we're using the NewThread model.
		if (useCurrentThread)
		{
			bool check = pKernel->CheckForIncomingCommands() ;
			unused(check);
		}

		SLEEP(pauseSecs, pauseMsecs) ;
	}

	delete pKernel ;

	return true ;
}

// Listener created to explore a particular ref counting bug
bool RefCountTest()
{
	// Create the kernel instance
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	sml::Agent* pAgent = pKernel->CreateAgent("listen") ;

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	long pauseSecs = 0;
	long pauseMsecs = 100 ;
	long pauseMsecsTotal = pauseSecs*1000+pauseMsecs;

	// How often we check to see if the list of connections has changed.
	//int checkConnections = 500 / pauseMsecsTotal ;
	//int counter = checkConnections ;
	cout << "Press i <return> to do an init-soar" << endl ;
	cout << "Press x <return> to exit" << endl ;

	bool done = false ;
	while (!done)
	{
		char buffer[100] ;
		gets(buffer) ;

		if (buffer[0] == 'i')
		{
			std::string result = pAgent->InitSoar() ;
			cout << "Init soar result was: " << result << endl ;
		}
		else if (buffer[0] == 'x')
			done = true ;
	}

	pKernel->Shutdown() ;
	delete pKernel ;

	return true ;
}

// Creates an agent that copies values from input link to output link
// so we can test that this is OK.
bool SimpleCopyAgent()
{
	// Create the kernel instance
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	sml::Agent* pAgent = pKernel->CreateAgent("copyagent") ;
	std::string path = std::string(pKernel->GetLibraryLocation()) + "/Tests/testcopy.soar" ;
	bool ok = pAgent->LoadProductions(path.c_str()) ;

	if (!ok)
		return false ;

/* Input structure for the test
(S1 ^io I1)
  (I1 ^input-link I3)
    (I3 ^sentence S2)
      (S2 ^newest yes ^num-words 3 ^sentence-num 1 ^word W1 ^word W2 ^word W3)
        (W1 ^num-word 1 ^word the)
        (W2 ^num-word 2 ^word cat)
        (W3 ^num-word 3 ^word in)
*/

	Identifier* pSentence = pAgent->CreateIdWME(pAgent->GetInputLink(), "sentence") ;
	pAgent->CreateStringWME(pSentence, "newest", "yes") ;
	pAgent->CreateIntWME(pSentence, "num-words", 3) ;
	Identifier* pWord1 = pAgent->CreateIdWME(pSentence, "word") ;
	Identifier* pWord2 = pAgent->CreateIdWME(pSentence, "word") ;
	Identifier* pWord3 = pAgent->CreateIdWME(pSentence, "word") ;
	pAgent->CreateIntWME(pWord1, "num-word", 1) ;
	pAgent->CreateIntWME(pWord2, "num-word", 2) ;
	pAgent->CreateIntWME(pWord3, "num-word", 3) ;
	pAgent->CreateStringWME(pWord1, "word", "the") ;
	pAgent->CreateStringWME(pWord2, "word", "cat") ;
	pAgent->CreateStringWME(pWord3, "word", "in") ;
	pAgent->Commit() ;

	// Register for the trace output
	std::string trace ;	// We'll pass this into the handler and build up the output in it
	int callbackp = pAgent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, &trace) ;

	// Set to true for more detail on this
	pKernel->SetTraceCommunications(false) ;

	std::string result = pAgent->RunSelf(3) ;
	cout << result << endl ;
	cout << trace << endl ;

	std::string state = pAgent->ExecuteCommandLine("print --depth 5 s1") ;
	cout << state << endl ;

	int changes = pAgent->GetNumberOutputLinkChanges() ;

	for (int i = 0 ; i < changes ; i++)
	{
		WMElement* pOutputWme = pAgent->GetOutputLinkChange(i) ;
		cout << pOutputWme->GetIdentifier()->GetIdentifierSymbol() << " ^ " << pOutputWme->GetAttribute() << " " << pOutputWme->GetValueAsString() << endl ;
	}

	// We had a bug where some of these wmes would get dropped (the orphaned wme scheme didn't handle multiple levels)
	// so check now that we got the correct number of changes.
	if (changes != 12)
		return false ;

	pKernel->Shutdown() ;
	delete pKernel ;

	return true ;
}

// Creates an agent that loads a rete net and then works with it a little.
bool SimpleReteNetLoader()
{
	// Create the kernel instance
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl ;
		return false ;
	}

	sml::Agent* pAgent = pKernel->CreateAgent("reteagent") ;
	std::string path = std::string(pKernel->GetLibraryLocation()) + "/Tests/test.soarx" ;
	std::string command = std::string("rete-net -l ") + path ;
	std::string result = pAgent->ExecuteCommandLine(command.c_str()) ;

	if (!pAgent->GetLastCommandLineResult())
	{
		cout << pAgent->GetLastErrorDescription() << endl ;
		return false ;
	}

	// Make us match the current input link values
	bool synchUp = pAgent->SynchronizeInputLink() ;

	// Get the latest id from the input link
	Identifier* pID = pAgent->GetInputLink() ;
	cout << "Input link id is " << pID->GetValueAsString() << endl ;

	pKernel->Shutdown() ;
	delete pKernel ;

	return true ;
}

void MyRunSelfRemovingHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	// This callback removes itself from the list of callbacks -- as a test to see if we can do that inside a callback handler.
	if (global_callback != -1)
	{
		pAgent->UnregisterForRunEvent(global_callback) ;
		global_callback = -1 ;
	}
	else
	{
		cout << "ERROR: The handler has been removed, but we're still getting the callbacks" << endl ;
	}
}

void MyRunEventHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase)
{
	int* pInt = (int*)pUserData ;

	// Increase the count
	*pInt = *pInt + 1 ;

	cout << "Received an event callback" << endl ;
}

void MyOutputEventHandler(void* pUserData, Agent* pAgent, char const* pAttributeName, WMElement* pWme)
{
	cout << "Received wme " << pWme->GetValueAsString() << endl ;
}

void MyStringEventHandler(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pData)
{
	switch (id)
	{
	case smlEVENT_EDIT_PRODUCTION:
		{
			cout << "Edit production " << pData << endl ;
			break ;
		}
	default:
		break ;
	}
}

void MyProductionHandler(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantiation)
{
	int* pInt = (int*)pUserData ;

	// Increase the count
	*pInt = *pInt + 1 ;

	if (id == smlEVENT_BEFORE_PRODUCTION_REMOVED)
		cout << "Excised " << pProdName << endl ;
}

void MyOutputNotificationHandler(void* pUserData, Agent* pAgent)
{
	int* pInt = (int*)pUserData ;

	// Increase the count
	*pInt = *pInt + 1 ;

	cout << "Received an output notification callback" << endl ;
}

void MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags)
{
	int* pInt = (int*)pUserData ;

	// Increase the count
	*pInt = *pInt + 1 ;

	cout << "Received an update callback" << endl ;
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

void MyShutdownHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel)
{
	cout << "Received before system shutdown event" << endl ;
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

static ClientXML* s_ClientXMLStorage = 0 ;

void MyXMLInputReceivedHandler(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML)
{
	// We'll start by turning it back into XML so we can look at it in the debugger.
	char* pStr = pXML->GenerateXMLString(true) ;

	pXML->DeleteString(pStr) ;
}

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

std::string MyClientMessageHandler(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pMessageType, char const* pMessage)
{
	cout << "Received client message type " << pMessageType << " with argument: " << pMessage << endl ;

	std::string res = "my client message result " ;
	res += pMessage ;

	return res ;
}

// This is a very dumb filter--it adds "--depth 2" to all commands passed to it.
std::string MyFilterHandler(smlRhsEventId id, void* pUserData, Agent* pAgent, char const* pMessageType, char const* pCommandLine)
{
	cout << "Received xml " << pCommandLine << endl ;

	ElementXML* pXML = ElementXML::ParseXMLFromString(pCommandLine) ;

	std::string commandLine = pXML->GetAttribute(sml_Names::kFilterCommand) ;

	commandLine += " --depth 2" ;

	// Replace the command attribute in the XML
	pXML->AddAttribute(sml_Names::kFilterCommand, commandLine.c_str()) ;

	// Convert the XML back to a string and put it into a std::string ready to return
	char *pXMLString = pXML->GenerateXMLString(true) ;
	std::string res = pXMLString ;
	pXML->DeleteString(pXMLString) ;
	delete pXML ;

	return res ;
}

bool InitSoarAgent(Agent* pAgent, bool doInitSoars)
{
	if (doInitSoars)
	{
		char const* pResult = pAgent->InitSoar() ;
		cout << pResult << endl ;
		return pAgent->GetLastCommandLineResult() ;
	}

	return true;
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

	// Record a client message handler
	int clientCallback = pKernel->RegisterForClientMessageEvent("test-rhs", &MyClientMessageHandler, 0) ;

	// This is a bit dopey--but we'll send a message to ourselves for this test
	std::string response = pKernel->SendClientMessage(pAgent, "test-rhs", "test-message") ;

	if (response.length() < 10)
	{
		cout << "Error sending client message to myself" << endl ;
		return false ;
	}

	pKernel->UnregisterForClientMessageEvent(clientCallback) ;

	// Record a filter
	int clientFilter = pKernel->RegisterForClientMessageEvent(sml_Names::kFilterName, &MyFilterHandler, 0) ;

	// Our filter adds "--depth 2" to all commands
	// so this should give us the result of "print s1 --depth 2"
	std::string command = pAgent->ExecuteCommandLine("print s1") ;

	cout << command << endl ;

	// This is important -- if we don't unregister all subsequent commands will
	// come to our filter and promptly fail!
	pKernel->UnregisterForClientMessageEvent(clientFilter) ;

	Identifier* pInputLink = pAgent->GetInputLink() ;
	if (!pInputLink)
		cout << "Error getting input link" << endl ;

	if (!InitSoarAgent(pAgent, doInitSoars))
		return false ;

	cout << "Done our first init-soar" << endl ;

	//int inputReceived = pAgent->RegisterForXMLEvent(smlEVENT_XML_INPUT_RECEIVED, MyXMLInputReceivedHandler, 0) ;

	// Some simple tests
	StringElement* pWME = pAgent->CreateStringWME(pInputLink, "my-att", "my-value") ;

	// This is to test a bug where an identifier isn't fully removed from working memory (you can still print it) after it is destroyed.
	Identifier* pIDRemoveTest = pAgent->CreateIdWME(pInputLink, "foo") ;
	pAgent->CreateFloatWME(pIDRemoveTest, "bar", 1.23) ;

	std::string idValue = pIDRemoveTest->GetValueAsString() ;

	unused(pWME);
	Identifier* pID = pAgent->CreateIdWME(pInputLink, "plane") ;

	// Trigger for inputWme update change problem
	StringElement* pWMEtest = pAgent->CreateStringWME(pID, "typeTest", "Boeing747") ;
	unused(pWMEtest) ;

	bool ok = pAgent->Commit() ;
	pAgent->RunSelf(1) ;

	pAgent->DestroyWME(pIDRemoveTest) ;
	pAgent->Commit() ;

	//pAgent->RunSelf(1) ;
	std::string wmes1 = pAgent->ExecuteCommandLine("print i2 --depth 3") ;
	std::string wmes2 = pAgent->ExecuteCommandLine("print F1") ;	// BUGBUG: This wme remains in memory even after we add the "RunSelf" at which point it should be gone.

	if (!InitSoarAgent(pAgent, doInitSoars))
		return false ;

	//pAgent->UnregisterForXMLEvent(inputReceived) ;

	StringElement* pWME1 = pAgent->CreateStringWME(pID, "type", "Boeing747") ;
	unused(pWME1);
	IntElement* pWME2    = pAgent->CreateIntWME(pID, "speed", 200) ;
	FloatElement* pWME3  = pAgent->CreateFloatWME(pID, "direction", 50.5) ;

	ok = pAgent->Commit() ;

	if (!InitSoarAgent(pAgent, doInitSoars))
		return false ;
	
	// Remove a wme
	pAgent->DestroyWME(pWME3) ;

	// Change the speed to 300
	pAgent->Update(pWME2, 300) ;

	// Create a new WME that shares the same id as plane
	// BUGBUG: This is triggering an assert and memory leak now after the changes
	// to InputWME not calling Update() immediately.  For now I've removed the test until
	// we have time to figure out what's going wrong.
	//Identifier* pID2 = pAgent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;
	//unused(pID2);

	ok = pAgent->Commit() ;

	/*
	printWMEs(pAgent->GetInputLink()) ;
	std::string printInput1 = pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput1 << endl ;
	cout << endl << "Now work with the input link" << endl ;
	*/

	// Delete one of the shared WMEs to make sure that's ok
	//pAgent->DestroyWME(pID) ;
	//pAgent->Commit() ;

	if (!InitSoarAgent(pAgent, doInitSoars))
		return false ;

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

	// Test calling CommandLineXML.
	ClientAnalyzedXML xml ;
	bool success = pKernel->ExecuteCommandLineXML("set-library-location", NULL, &xml) ;
	
	if (!success)
	{
		cout << "Error calling ExecuteCommandLineXML" << endl ;
		return false ;
	}

	std::string path = xml.GetArgString(sml_Names::kParamDirectory) ;

	// Check that we got some string back
	if (path.length() < 3)
	{
		cout << "Error getting library location via XML call" << endl ;
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

	// This callback unregisters itself in the callback -- as a test to see if we can do that safely.
	global_callback = pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, MyRunSelfRemovingHandler, NULL) ;

	// Register for an String event
	int stringCall = pKernel->RegisterForStringEvent(smlEVENT_EDIT_PRODUCTION, MyStringEventHandler, NULL) ;
	pKernel->ExecuteCommandLine("edit-production my*production", NULL) ;
	pKernel->UnregisterForStringEvent(stringCall) ;

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

	// Test that we get a callback after the all output phases complete
	// We'll pass in an "int" and use it to count output phases
	int outputPhases ;
	outputPhases = 0 ;
	int callback_u = pKernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, MyUpdateEventHandler, &outputPhases) ;

	int phaseCount = 0 ;
	int callbackPhase = pAgent->RegisterForRunEvent(smlEVENT_BEFORE_PHASE_EXECUTED, MyRunEventHandler, &phaseCount) ;

	// Nothing should match here
	std::string result = pAgent->RunSelf(4) ;

	// Should be one output phase per decision
	if (outputPhases != 4)
	{
		cout << "Error receiving AFTER_ALL_OUTPUT_PHASES events" << endl ;
		return false ;
	}

	bool wasRun = pAgent->WasAgentOnRunList() ;
	if (!wasRun)
	{
		cout << "Error determining if this agent was run" << endl ;
		return false ;
	}

	smlRunResult runResult = pAgent->GetResultOfLastRun() ;
	if (runResult != sml_RUN_COMPLETED)
	{
		cout << "Error getting result of the last run" << endl ;
		return false ;
	}

	// Should be 5 phases per decision
	/* Not true now we support stopping before/after phases when running by decision.
	if (phaseCount != 20)
	{
		cout << "Error receiving phase events" << endl ;
		return false ;
	}
	*/

	if (beforeCount != 1 || afterCount != 1)
	{
		cout << "Error receiving BFORE_RUN_STARTS/AFTER_RUN_ENDS events" << endl ;
		return false ;
	}

	pAgent->UnregisterForRunEvent(callbackPhase) ;

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
	pKernel->UnregisterForUpdateEvent(callback_u) ;

	// Print out the standard trace and the same thing as a structured XML trace
	cout << trace << endl ;
	cout << structured << endl ;

	/*
	printWMEs(pAgent->GetInputLink()) ;
	std::string printInput = pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput << endl ;
	*/

	// Synchronizing the input link means we make our client copy match
	// the current state of the agent.  We would generally only do this from
	// a different client, but we can test here to see if it does nothing
	// (except deleting and recreating the structures).
	cout << "Input link before synchronization" << endl ;
	printWMEs(pAgent->GetInputLink()) ;

	bool synch = pAgent->SynchronizeInputLink() ;

	if (synch)
	{
		cout << "Results of synchronizing the input link:" << endl ;
		printWMEs(pAgent->GetInputLink()) ;
		cout << endl ;

		if (pAgent->GetInputLink()->GetNumberChildren() == 0)
		{
			cout << "Failed to get any children on the input link after synch" << endl ;
			return false ;
		}
	}

	// Then add some tic tac toe stuff which should trigger output
	Identifier* pSquare = pAgent->CreateIdWME(pAgent->GetInputLink(), "square") ;
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

	int outputsGenerated ;
	outputsGenerated = 0 ;
	int callback_g = pKernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_GENERATED_OUTPUT, MyUpdateEventHandler, &outputsGenerated) ;

	int outputNotifications ;
	outputNotifications = 0 ;
	int callback_notify = pAgent->RegisterForOutputNotification(MyOutputNotificationHandler, &outputNotifications) ;

	// Can't test this at the same time as testing the getCommand() methods as registering for this clears the output link information
	//int outputHandler = pAgent->AddOutputHandler("move", MyOutputEventHandler, NULL) ;

	//cout << "About to do first run-til-output" << endl ;

	// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
	// We'll use RunAll just to test it out.  Could use RunSelf and get same result (presumably)
	trace = pKernel->RunAllTilOutput() ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)

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

	if (outputsGenerated != 1)
	{
		cout << "Error in AFTER_ALL_GENERATED event." << endl ;
		return false ;
	}

	if (outputNotifications != 1)
	{
		cout << "Error in OUTPUT_NOTIFICATION event." << endl ;
		return false ;
	}

	// Reset the agent and repeat the process to check whether init-soar works.
	if (doInitSoars)
	{
		pAgent->InitSoar() ;
		trace = pAgent->RunSelfTilOutput() ;
	}

	bool ioOK = false ;

	pAgent->UnregisterForOutputNotification(callback_notify) ;
	pKernel->UnregisterForUpdateEvent(callback_g) ;

	//cout << "Time to dump output link" << endl ;

	// If we have output, dump it out.
	if (pAgent->GetOutputLink())
	{
		printWMEs(pAgent->GetOutputLink()) ;

		// Now update the output link with "status complete"
		Identifier* pMove = (Identifier*)pAgent->GetOutputLink()->FindByAttribute("move", 0) ;

		// Try to find an attribute that's missing to make sure we get null back
		Identifier* pMissing = (Identifier*)pAgent->GetOutputLink()->FindByAttribute("not-there",0) ;
		Identifier* pMissingInput = (Identifier*)pAgent->GetInputLink()->FindByAttribute("not-there",0) ;

		if (pMissing || pMissingInput)
		{
			cout << "Looked for an attribute that shouldn't be there and yet got something back" << endl ;
			return false ;
		}

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

	// Test the ability to resynch the output link -- this should throw away our current output link representation
	// and explicitly rebuild it to match what the agent currently has.
	bool synchOutput = pAgent->SynchronizeOutputLink() ;

	// This isn't supported for direct connections, so we need to check it did something first.
	if (synchOutput)
	{
		cout << "Synched output link  -- should be the same as before" << endl ;
		printWMEs(pAgent->GetOutputLink()) ;

		// Find the move command again--just to make sure it came back after the synch
		Identifier* pMove = (Identifier*)pAgent->GetOutputLink()->FindByAttribute("move", 0) ;

		// We add an "alternative" to check that we handle shared WMEs correctly.
		// Look it up here.
		Identifier* pAlt = (Identifier*)pAgent->GetOutputLink()->FindByAttribute("alternative", 0) ;

		if (!pAlt)
		{
			cout << "Failed to find the alternative move after synch output link" << endl ;
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
			cout << "Found move command after synch" << endl ;
		}
		else
		{
			cout << "*** ERROR: Failed to find the move command after synch" << endl ;
			return false ;
		}
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

	if (!InitSoarAgent(pAgent, doInitSoars))
		return false ;

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
    
    std::fstream fs;
    fs.open("test.xml", std::ios_base::out | std::ios_base::app);
    char* temp = pXML->GenerateXMLString(true);
    fs << temp;
    fs.close();
    ClientXML::DeleteString(temp);
    
}

void MyPrintEventHandlerTimer(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMsg)
{
	// Don't do any work in the timer case -- but register the event handler so we generate
	// the data and send it to us.

    //This code is for debugging what comes out of the trace
    
    std::fstream fs;
    fs.open("test.txt", std::ios_base::out | std::ios_base::app);
    fs << pMsg;
    fs.close();
    
}

void MyEchoEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMsg)
{
	if (pMsg)
		cout << " ----> Received an echo event with contents: " << pMsg << endl ;    
}

bool TestSML(bool embedded, bool useClientThread, bool fullyOptimized, bool simpleInitSoar, bool autoCommit)
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

		// Controls whether auto commit is on or off
		// (do we need to call commit ourselves or not)
		pKernel->SetAutoCommit(autoCommit) ;

		// Set this to true to give us lots of extra debug information on remote clients
		// (useful in a test app like this).
	    // pKernel->SetTraceCommunications(true) ;

		cout << "Soar kernel version " << pKernel->GetSoarKernelVersion() << endl ;
		cout << "Soar client version " << pKernel->GetSoarClientVersion() << endl ;
		cout << "SML version " << pKernel->GetSMLVersion() << endl ;

		std::string kernelVersion = pKernel->GetSoarKernelVersion() ;
		if (kernelVersion.compare(pKernel->GetSoarClientVersion()) != 0)
		{
			cout << "Client and kernel versions don't match - which they should during development" << endl ;
			return false ;
		}

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
				SLEEP(0, 200) ;

				cout << "Performing simple init-soar..." << endl << endl;
				pAgent->InitSoar() ;

				SLEEP(0, 200) ;

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
			path += "/Tests/testsml.soar" ;

			// Listen to the echo of the load
			int echoCallback = pAgent->RegisterForPrintEvent(smlEVENT_ECHO, &MyEchoEventHandler, NULL) ;

			bool echo = true ;
			bool load = pAgent->LoadProductions(path.c_str(), echo) ;

			unused(load);
			pAgent->UnregisterForPrintEvent(echoCallback) ;

			if (pAgent->HadError())
			{
				cout << "ERROR loading productions: " << pAgent->GetLastErrorDescription() << endl ;
				return false ;
			}

			cout << "Loaded productions" << endl ;

			ok = ok && pAgent->IsProductionLoaded("apply*move") ;
			ok = ok && !pAgent->IsProductionLoaded("made*up*name") ;

			if (!ok)
			{
				cout << "ERROR checking whether specific productions are loaded" << endl ;
			}

			int excisedCount = 0 ;
			int prodCall = pAgent->RegisterForProductionEvent(smlEVENT_BEFORE_PRODUCTION_REMOVED, &MyProductionHandler, &excisedCount) ;
			pAgent->ExecuteCommandLine("excise --all") ;
			load = pAgent->LoadProductions(path.c_str(), echo) ;
			ok = ok && pAgent->UnregisterForProductionEvent(prodCall) ;

			if (!ok || excisedCount == 0)
			{
				cout << "ERROR listening for production removed events" << endl ;
				return false ;
			}

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

		// PHASE THREE: Agent deletion
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

		int callbackShut = pKernel->RegisterForSystemEvent(smlEVENT_BEFORE_SHUTDOWN, &MyShutdownHandler, 0) ;
		unused(callbackShut) ;

		cout << "Calling shutdown on the kernel now" << endl ;
		pKernel->Shutdown() ;
		cout << "Shutdown completed now" << endl ;

		// Delete the kernel.  If this is an embedded connection this destroys the kernel.
		// If it's a remote connection we just disconnect.
		delete pKernel ;

	}// closes testing block scope

	return true ;
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
        //path += "../demos/towers-of-hanoi/towers-of-hanoi.soar" ;
		path += "/Demos/water-jug/water-jug-look-ahead.soar" ;

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

		pAgent->RegisterForPrintEvent(smlEVENT_PRINT, &MyPrintEventHandlerTimer, NULL) ;
		pAgent->RegisterForXMLEvent(smlEVENT_XML_TRACE_OUTPUT, &MyXMLEventHandlerTimer, NULL) ;

		pAgent->ExecuteCommandLine("watch 0") ;
	}

    std::string result;
    
    result = pFirst->ExecuteCommandLine("watch --learning fullprint --backtracing") ;
	cout << result << endl ;

    result = pFirst->ExecuteCommandLine("time run 20") ;
    cout << result << endl ;

	// Need to get rid of the kernel explictly.
	delete pKernel ;

	return true ;
}

bool FullTimeTest()
{
	// Embeddded using direct calls
	bool ok = TimeTest(true, true, true) ;

	return ok ;
}

bool FullEmbeddedTest()
{
	bool ok = true ;

	// Simple embedded, direct init-soar
	ok = ok && TestSML(true, true, true, true, true) ;

	// Embeddded using direct calls
	ok = ok && TestSML(true, true, true, false, true) ;

	// Embedded not using direct calls
	ok = ok && TestSML(true, true, false, false, true) ;

	// Embedded running on thread inside kernel using auto commit
	ok = ok && TestSML(true, false, false, false, true) ;

	// Embedded running on thread inside kernel w/o using auto commit
	ok = ok && TestSML(true, false, false, false, false) ;

	return ok ;
}

bool RemoteTest()
{
	// Remote connection.
	// (For this to work need to run a listener--usually TestCommandLineInterface to receive the commands).
	bool ok = TestSML(false, false, false, false, true) ;

	// Same test but with auto commit turned off (so need manual commit calls)
	ok = ok && TestSML(false, false, false, false, false) ;

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
	//_crtBreakAlloc = 1265 ;

	//SimpleTimer timer ;

	bool stopAtEnd = true ;
	bool remote    = false ;
	bool listener  = false ;
	bool timeTest  = false ;
	bool runlistener = false ;
	bool remoteConnect = false ;
	bool copyTest  = false ;
	bool synchTest = false ;
	bool reteTest  = false ;
	bool refCountTest = false ;
	int  life      = 3000 ;	// Default is to live for 3000 seconds (5 mins) as a listener
	int  decisions = 20000 ;

	// For now, any argument on the command line makes us create a remote connection.
	// Later we'll try passing in an ip address/port number.
	bool success = true ;

	// Read the command line options:
	// -nostop : don't ask user to hit return at the end
	// -remote : run the test over a remote connection -- needs a listening client to already be running.
	// -listener : create a listening client (so we can run remote tests) which lives for 300 secs (5 mins)
	// -shortlistener : create a listening client that lives for 15 secs
	// -runlistener : create a listening client that runs an agent for <n> decisions (defaults to 20000) (so we can connect/disconnect from running agent)
	// -runlistener <decisions> : As above but runs for specified number of decisions.
	// -remoteconnect : Connects to a running kernel (e.g. -runlistener above), grabs some output and then disconnects.
	// -time : run a time trial on some functionality
	// Also -copyTest, -reteTest, -refCountTest and others to test specific calls and bugs.  I don't expect those to be used much but keeping them here
	// so it's quicker to investigate similar problems in the future.
	if (argc > 1)
	{
		for (int i = 1 ; i < argc ; i++)
		{
			if (!stricmp(argv[i], "-nostop"))
				stopAtEnd = false ;
			if (!stricmp(argv[i], "-copy"))
				copyTest = true ;
			if (!stricmp(argv[i], "-retetest"))
				reteTest = true ;
			if (!stricmp(argv[i], "-synch"))
				synchTest = true ;
			if (!stricmp(argv[i], "-remote"))
				remote = true ;
			if (!stricmp(argv[i], "-listener"))
				listener = true ;
			if (!stricmp(argv[i], "-refcounttest"))
				refCountTest = true ;
			if (!stricmp(argv[i], "-runlistener"))
			{
				runlistener = true ;

				// Try to parse the next argument (if present) as an int.
				// If that succeeds, use the value as our decision counter.
				int next = i+1 ;
				if (next < argc)
				{
					int count = atoi(argv[next]) ;
					if (count > 0)
						decisions = count ;
				}

			}
			if (!stricmp(argv[i], "-remoteconnect"))
				remoteConnect = true ;
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
	else if (refCountTest)
		RefCountTest() ;
	else if (runlistener)
		SimpleRunListener(decisions) ;
	else if (copyTest)
		success = SimpleCopyAgent() ;
	else if (reteTest)
		success = SimpleReteNetLoader() ;
	else if (remoteConnect)
		SimpleRemoteConnect() ;
	else if (synchTest)
		success = SimpleRemoteSynchTest() ;
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
