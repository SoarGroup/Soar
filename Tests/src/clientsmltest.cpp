#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>
#include <sstream>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"

class ClientSMLTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( ClientSMLTest );	

	CPPUNIT_TEST( testEmbeddedDirectInit );
	CPPUNIT_TEST( testEmbeddedDirect );
	CPPUNIT_TEST( testEmbedded );
	CPPUNIT_TEST( testNewThread );
	CPPUNIT_TEST( testNewThreadNoAutoCommit );
	CPPUNIT_TEST( testRemote );
	CPPUNIT_TEST( testRemoteNoAutoCommit );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

protected:
	void testEmbeddedDirectInit();
	void testEmbeddedDirect();
	void testEmbedded();
	void testNewThread();
	void testNewThreadNoAutoCommit();
	void testRemote();
	void testRemoteNoAutoCommit();

public:
	// callbacks
	static void MyShutdownHandler(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel);
	static void MyDeletionHandler(sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent);
	static void MySystemEventHandler( sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel );
	static void MyCreationHandler( sml::smlAgentEventId id, void* pUserData, sml::Agent* pAgent );
	static void MyEchoEventHandler( sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMsg );
	static void MyProductionHandler( sml::smlProductionEventId id, void* pUserData, sml::Agent* pAgent, char const* pProdName, char const* pInstantiation );
	static std::string MyClientMessageHandler( sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessageType, char const* pMessage);
	static std::string MyFilterHandler( sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessageType, char const* pCommandLine);
	static void MyRunEventHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase);
	static void MyUpdateEventHandler( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags);
	static void MyOutputNotificationHandler(void* pUserData, sml::Agent* pAgent);
	static void MyRunSelfRemovingHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase);
	static std::string MyStringEventHandler( sml::smlStringEventId id, void* pUserData, sml::Kernel* pKernel, char const* pData );
	static void MyDuplicateRunEventHandler( sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase );
	static void MyPrintEventHandler( sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage );
	static void MyXMLEventHandler( sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML );
	static void MyInterruptHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase);
	static std::string MyRhsFunctionHandler(sml::smlRhsEventId id, void* pUserData, sml::Agent* pAgent, char const* pFunctionName, char const* pArgument);

private:
	void createKernel( bool embedded, bool useClientThread, bool fullyOptimized, bool simpleInitSoar, bool autoCommit, int port = 12121 );
	void doTest();
	void doAgentTest( sml::Agent* pAgent );
	void spawnListener();
	void cleanUpListener();

	static const std::string kBaseName;
	static sml::ClientXML* clientXMLStorage;
	static bool verbose;

	sml::Kernel* pKernel;
	int numberAgents; // This number determines how many agents are created.  We create <n>, test <n> and then delete <n>
	bool simpleInitSoar;
	bool remote;

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
#endif // _WIN32

};

CPPUNIT_TEST_SUITE_REGISTRATION( ClientSMLTest ); 

const std::string ClientSMLTest::kBaseName( "test-client-sml" );
sml::ClientXML* ClientSMLTest::clientXMLStorage = 0;
bool ClientSMLTest::verbose = false;

void ClientSMLTest::setUp()
{
	pKernel = NULL;
	numberAgents = 1;
	simpleInitSoar = false;
	clientXMLStorage = NULL;
	remote = false;

	// kernel initialized in test
}

void ClientSMLTest::MyShutdownHandler(sml::smlSystemEventId, void* pUserData, sml::Kernel*)
{
	if ( verbose ) std::cout << "Received before system shutdown event" << std::endl ;
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void ClientSMLTest::MyDeletionHandler(sml::smlAgentEventId, void* pUserData, sml::Agent*)
{
	if ( verbose ) std::cout << "Received notification before agent was deleted" << std::endl ;
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void ClientSMLTest::tearDown()
{
	if ( !pKernel ) return;

	// Agent deletion
	if ( verbose ) std::cout << "Destroy the agent now" << std::endl ;
	for (int agentDeletions = 0 ; agentDeletions < numberAgents ; agentDeletions++)
	{
		std::stringstream name;
		name << kBaseName << 1 + agentDeletions;

		sml::Agent* pAgent = pKernel->GetAgent( name.str().c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		// The Before_Agent_Destroyed callback is a tricky one so we'll register for it to test it.
		// We need to get this callback just before the agentSML data is deleted (otherwise there'll be no way to send/receive the callback)
		// and then continue on to delete the agent after we've responded to the callback.
		// Interestingly, we don't explicitly unregister this callback because the agent has already been destroyed so
		// that's another test, that this callback is cleaned up correctly (and automatically).
		bool deletionHandlerReceived( false );
		pKernel->RegisterForAgentEvent( sml::smlEVENT_BEFORE_AGENT_DESTROYED, ClientSMLTest::MyDeletionHandler, &deletionHandlerReceived ) ;

		// Explicitly destroy our agent as a test, before we delete the kernel itself.
		// (Actually, if this is a remote connection we need to do this or the agent
		//  will remain alive).
		CPPUNIT_ASSERT( pKernel->DestroyAgent( pAgent ) );
		CPPUNIT_ASSERT( deletionHandlerReceived );
		deletionHandlerReceived = false;
	}

	if ( verbose ) std::cout << "Calling shutdown on the kernel now" << std::endl ;

	bool shutdownHandlerReceived( false );
	pKernel->RegisterForSystemEvent( sml::smlEVENT_BEFORE_SHUTDOWN, ClientSMLTest::MyShutdownHandler, &shutdownHandlerReceived ) ;

	if ( remote )
	{
		std::string shutdownResponse = pKernel->SendClientMessage(0, "test-listener", "shutdown") ;
		CPPUNIT_ASSERT( shutdownResponse == "ok" );	
		
		// The kernel, at this point, has to be assumed to be not running.

		pKernel->Shutdown() ;
		// BUGBUG ?
		// With a remote kernel, we can't both shut it down like this 
		// (with a client message) and receive a smlEVENT_BEFORE_SHUTDOWN 
		// event because when the listener shuts itself down it does not 
		// fire the smlEVENT_BEFORE_SHUTDOWN event handler, because it is
		// already shutdown. 
		// I think the client side SML would have to recognize that 
		// something is waiting to receive that message when Shutdown() 
		// (above) is called. I'm not sure this is feasable, so this 
		// might need to just be a known problem/workaround.
		// We could have the listener sleep for an arbitrary amount of 
		// time after receiving the shutdown call (above) but this seems
		// kludgey. How much time to wait? Why not just handle this 
		// special case on this side since you will have had to have 
		// written the listener for it to shutdown via user command 
		// anyway?
		//CPPUNIT_ASSERT( shutdownHandlerReceived );

	} else {
		pKernel->Shutdown() ;
		CPPUNIT_ASSERT( shutdownHandlerReceived );
	}

	if ( verbose ) std::cout << "Shutdown completed now" << std::endl ;

	// Delete the kernel.  If this is an embedded connection this destroys the kernel.
	// If it's a remote connection we just disconnect.
	delete pKernel ;
	pKernel = NULL;

	if ( remote )
	{
#ifdef _WIN32
		cleanUpListener();
		if ( verbose ) std::cout << "Cleaned up listener." << std::endl;
#endif // _WIN32
	}
}

void ClientSMLTest::testEmbeddedDirectInit()
{
	numberAgents = 1;
	createKernel(true, true, true, true, true);
	simpleInitSoar = true;

	doTest();
}

void ClientSMLTest::testEmbeddedDirect()
{
	numberAgents = 1;
	createKernel(true, true, true, false, true);

	doTest();
}

void ClientSMLTest::testEmbedded()
{
	numberAgents = 1;
	createKernel(true, true, false, false, true);

	doTest();
}

void ClientSMLTest::testNewThread()
{
	numberAgents = 1;
	createKernel(true, false, false, false, true);

	doTest();
}

void ClientSMLTest::testNewThreadNoAutoCommit()
{
	numberAgents = 1;
	createKernel(true, false, false, false, false);

	doTest();
}

void ClientSMLTest::testRemote()
{
#ifdef _WIN32
	remote = true;
	spawnListener();

	if ( verbose ) std::cout << "Spawned listener." << std::endl;

	numberAgents = 1;
	createKernel( false, false, false, false, true );

	doTest();

	if ( verbose ) std::cout << "Test complete." << std::endl;

#endif // _WIN32
}

void ClientSMLTest::testRemoteNoAutoCommit()
{
#ifdef _WIN32
	remote = true;
	spawnListener();

	if ( verbose ) std::cout << "Spawned listener." << std::endl;

	numberAgents = 1;
	createKernel( false, false, false, false, false );

	doTest();

	if ( verbose ) std::cout << "Test complete." << std::endl;
#endif // _WIN32
}

void ClientSMLTest::spawnListener()
{
	// Spawning a new process is radically different on windows vs linux.
	// Instead of writing an abstraction layer, I'm just going to put platform-
	// specific code here.

#ifdef _WIN32
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
	BOOL success = CreateProcess( L"Tests.exe",
        L"Tests.exe --listener",        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi );          // Pointer to PROCESS_INFORMATION structure

	std::stringstream errorMessage;
	errorMessage << "CreateProcess error code: " << GetLastError();
	CPPUNIT_ASSERT_MESSAGE( errorMessage.str().c_str(), success );
#endif // _WIN32

	sml::Sleep( 1, 0 );
}

void ClientSMLTest::cleanUpListener()
{
#ifdef _WIN32
	// Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#endif // _WIN32
}

void ClientSMLTest::createKernel( bool embedded, bool useClientThread, bool fullyOptimized, bool simpleInitSoar, bool autoCommit, int port )
{
	pKernel = embedded ?
		(useClientThread ? sml::Kernel::CreateKernelInCurrentThread( sml::Kernel::GetDefaultLibraryName(), fullyOptimized, sml::Kernel::GetDefaultPort())
		: sml::Kernel::CreateKernelInNewThread("SoarKernelSML", sml::Kernel::GetDefaultPort()))
		: sml::Kernel::CreateRemoteConnection(true, NULL, port) ;

	CPPUNIT_ASSERT( pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	pKernel->SetAutoCommit( false ) ;
}

void ClientSMLTest::MySystemEventHandler( sml::smlSystemEventId, void*, sml::Kernel* )
{
	// see comments above registration line
	if ( verbose ) std::cout << "Received kernel event" << std::endl ;
}

void ClientSMLTest::MyCreationHandler( sml::smlAgentEventId, void* pUserData, sml::Agent* )
{
	if ( verbose ) std::cout << "Received notification when agent was created" << std::endl ;
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void ClientSMLTest::MyEchoEventHandler( sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMsg )
{
	if ( verbose && pMsg )
		std::cout << " ----> Received an echo event with contents: " << pMsg << std::endl ;    
	CPPUNIT_ASSERT( pMsg != NULL );
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void ClientSMLTest::MyProductionHandler( sml::smlProductionEventId id, void* pUserData, sml::Agent*, char const*, char const* )
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt += 1 ;

	CPPUNIT_ASSERT( id == sml::smlEVENT_BEFORE_PRODUCTION_REMOVED );
}

void ClientSMLTest::doTest()
{
	// Set this to true to give us lots of extra debug information on remote clients
	// (useful in a test app like this).
    // pKernel->SetTraceCommunications(true) ;

	if ( verbose ) std::cout << "Soar kernel version " << pKernel->GetSoarKernelVersion() << std::endl ;
	if ( verbose ) std::cout << "Soar client version " << pKernel->GetSoarClientVersion() << std::endl ;
	if ( verbose ) std::cout << "SML version " << pKernel->GetSMLVersion() << std::endl ;

	CPPUNIT_ASSERT( std::string( pKernel->GetSoarKernelVersion() ) == std::string( pKernel->GetSoarClientVersion() ) );

	// Register a kernel event handler...unfortunately I can't seem to find an event
	// that gSKI actually fires, so this handler won't get called.  Still, the code is there
	// on the SML side should anyone ever hook up this type of event inside the kernel/gSKI...
	pKernel->RegisterForSystemEvent( sml::smlEVENT_AFTER_RESTART, ClientSMLTest::MySystemEventHandler, NULL ) ;

	bool creationHandlerReceived( false );
	pKernel->RegisterForAgentEvent( sml::smlEVENT_AFTER_AGENT_CREATED, ClientSMLTest::MyCreationHandler, &creationHandlerReceived ) ;
	
	// Report the number of agents (always 0 unless this is a remote connection to a CLI or some such)
	CPPUNIT_ASSERT( pKernel->GetNumberAgents() == 0 );

	// PHASE ONE: Agent creation
	for ( int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++ )
	{
		std::stringstream name;
		name << kBaseName << 1 + agentCounter;

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		sml::Agent* pAgent = pKernel->CreateAgent( name.str().c_str() ) ;
		CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );
		CPPUNIT_ASSERT( pAgent != NULL );
		CPPUNIT_ASSERT( creationHandlerReceived );
		creationHandlerReceived = false;

		if (simpleInitSoar) 
		{
			sml::Sleep(0, 200) ;

			if ( verbose ) std::cout << "Performing simple init-soar..." << std::endl;
			pAgent->InitSoar() ;

			sml::Sleep(0, 200) ;

			// that's it for this test
			return;
		}

		// TODO boost filesystem
		std::stringstream path;
		path << pKernel->GetLibraryLocation() << "/Tests/testsml.soar" ;

		// Listen to the echo of the load
		bool echoHandlerReceived( false );
		int echoCallback = pAgent->RegisterForPrintEvent( sml::smlEVENT_ECHO, ClientSMLTest::MyEchoEventHandler, &echoHandlerReceived ) ;
		
		pAgent->LoadProductions( path.str().c_str(), true ) ;
		CPPUNIT_ASSERT_MESSAGE( "LoadProductions", pAgent->GetLastCommandLineResult() );
		// BUGBUG: this does not work
		//CPPUNIT_ASSERT( echoHandlerReceived );
		echoHandlerReceived = false;
		CPPUNIT_ASSERT_MESSAGE( pAgent->GetLastErrorDescription(), pAgent->UnregisterForPrintEvent(echoCallback) );
		CPPUNIT_ASSERT_MESSAGE( pAgent->GetLastErrorDescription(), !pAgent->HadError() );

		if ( verbose ) std::cout << "Loaded productions" << std::endl ;

		CPPUNIT_ASSERT( pAgent->IsProductionLoaded( "apply*move" ) );
		CPPUNIT_ASSERT( !pAgent->IsProductionLoaded( "made*up*name" ) );

		int excisedCount( 0 );
		int prodCall = pAgent->RegisterForProductionEvent( sml::smlEVENT_BEFORE_PRODUCTION_REMOVED, ClientSMLTest::MyProductionHandler, &excisedCount ) ;

		pAgent->ExecuteCommandLine( "excise --all" ) ;
		CPPUNIT_ASSERT_MESSAGE( "excise --all", pAgent->GetLastCommandLineResult() );
		CPPUNIT_ASSERT( excisedCount > 0 );

		pAgent->LoadProductions( path.str().c_str(), true ) ;
		CPPUNIT_ASSERT_MESSAGE( "LoadProductions", pAgent->GetLastCommandLineResult() );
		CPPUNIT_ASSERT( !echoHandlerReceived );	// We unregistered this

		CPPUNIT_ASSERT( pAgent->UnregisterForProductionEvent( prodCall ) );
	}

	// PHASE TWO: Agent tests
	for (int agentTests = 0 ; agentTests < numberAgents ; agentTests++)
	{
		std::stringstream name;
		name << kBaseName << 1 + agentTests;

		sml::Agent* pAgent = pKernel->GetAgent( name.str().c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		// Run a suite of tests on this agent
		doAgentTest( pAgent ) ;
	}
}

std::string ClientSMLTest::MyClientMessageHandler( sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pMessage)
{
	std::stringstream res;
	res << "handler-message" << pMessage;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;

	return res.str() ;
}

// This is a very dumb filter--it adds "--depth 2" to all commands passed to it.
std::string ClientSMLTest::MyFilterHandler( sml::smlRhsEventId, void* pUserData, sml::Agent*, char const*, char const* pCommandLine)
{
	if ( verbose ) std::cout << "Received xml " << pCommandLine << std::endl ;

	sml::ElementXML* pXML = sml::ElementXML::ParseXMLFromString( pCommandLine ) ;
	CPPUNIT_ASSERT( pXML );
	CPPUNIT_ASSERT( pXML->GetAttribute( sml::sml_Names::kFilterCommand ) );

	std::stringstream commandLine;
	commandLine << pXML->GetAttribute( sml::sml_Names::kFilterCommand ) << " --depth 2";

	// Replace the command attribute in the XML
	CPPUNIT_ASSERT( pXML->AddAttribute( sml::sml_Names::kFilterCommand, commandLine.str().c_str() ) );

	// Convert the XML back to a string and put it into a std::string ready to return
	char *pXMLString = pXML->GenerateXMLString(true) ;
	CPPUNIT_ASSERT( pXMLString );
	std::string res( pXMLString );

	pXML->DeleteString( pXMLString );
	delete pXML ;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;

	return res ;
}

void ClientSMLTest::MyRunEventHandler( sml::smlRunEventId, void* pUserData, sml::Agent*, sml::smlPhase)
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt = *pInt + 1 ;

	if ( verbose ) std::cout << "Received an event callback" << std::endl ;
}

void ClientSMLTest::MyUpdateEventHandler( sml::smlUpdateEventId, void* pUserData, sml::Kernel*, sml::smlRunFlags )
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt = *pInt + 1 ;

	if ( verbose ) std::cout << "Received an update callback" << std::endl ;
}

void ClientSMLTest::MyOutputNotificationHandler(void* pUserData, sml::Agent*)
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );

	// Increase the count
	*pInt = *pInt + 1 ;

	if ( verbose ) std::cout << "Received an output notification callback" << std::endl ;
}

void ClientSMLTest::MyRunSelfRemovingHandler( sml::smlRunEventId, void* pUserData, sml::Agent* pAgent, sml::smlPhase)
{
	CPPUNIT_ASSERT( pUserData );
	int* myCallback = static_cast< int* >( pUserData );

	// This callback removes itself from the list of callbacks 
	// as a test to see if we can do that inside a callback handler.
	CPPUNIT_ASSERT( *myCallback != -1 );
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent( *myCallback ) );

	*myCallback = -1 ;
}

std::string ClientSMLTest::MyStringEventHandler( sml::smlStringEventId id, void* pUserData, sml::Kernel*, char const* )
{
	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;

	// new: string events need to return empty string on success
	return "";
}

// Register a second handler for the same event, to make sure that's ok.
void ClientSMLTest::MyDuplicateRunEventHandler( sml::smlRunEventId, void* pUserData, sml::Agent*, sml::smlPhase )
{
	CPPUNIT_ASSERT( pUserData );
	int* pInt = static_cast< int* >( pUserData );
	CPPUNIT_ASSERT( *pInt == 25 );
}

void ClientSMLTest::MyPrintEventHandler( sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMessage )
{
	// In this case the user data is a string we're building up
	CPPUNIT_ASSERT( pUserData );
	std::stringstream* pTrace = static_cast< std::stringstream* >( pUserData );

	(*pTrace) << pMessage ;
}

void ClientSMLTest::MyXMLEventHandler( sml::smlXMLEventId, void* pUserData, sml::Agent*, sml::ClientXML* pXML )
{
	// pXML should be some structured trace output.
	// Let's examine it a bit.
	// We'll start by turning it back into XML so we can look at it in the debugger.
	char* pStr = pXML->GenerateXMLString( true ) ;
	CPPUNIT_ASSERT( pStr );

	// This will always succeed.  If this isn't really trace XML
	// the methods checking on tag names etc. will just fail
	sml::ClientTraceXML* pRootXML = pXML->ConvertToTraceXML() ;
	CPPUNIT_ASSERT( pRootXML );

	// The root object is just a <trace> tag.  The substance is in the children
	// so we'll get the first child which should exist.
	sml::ClientTraceXML childXML ;
	CPPUNIT_ASSERT( pRootXML->GetChild( &childXML, 0 ) );
	sml::ClientTraceXML* pTraceXML = &childXML ;

	if (pTraceXML->IsTagState())
	{
		CPPUNIT_ASSERT( pTraceXML->GetDecisionCycleCount() );
		std::string count = pTraceXML->GetDecisionCycleCount() ;

		CPPUNIT_ASSERT( pTraceXML->GetStateID() );
		std::string stateID = pTraceXML->GetStateID() ;

		CPPUNIT_ASSERT( pTraceXML->GetImpasseObject() );
		std::string impasseObject = pTraceXML->GetImpasseObject() ;

		CPPUNIT_ASSERT( pTraceXML->GetImpasseType() );
		std::string impasseType = pTraceXML->GetImpasseType() ;

		if ( verbose ) std::cout << "Trace ==> " << count << ":" << stateID << " (" << impasseObject << " " << impasseType << ")" << std::endl ;
	}

	// Make a copy of the object we've been passed which should remain valid
	// after the event handler has completed.  We only keep the last message
	// in this test.  This is a stress test for our memory allocation logic.
	// We're not allowed to keep pXML that we're passed, but we can copy it and keep the copy.
	// (The copy is very efficient, the underlying object is ref-counted).
	if (clientXMLStorage != NULL)
		delete clientXMLStorage ;
	clientXMLStorage = new sml::ClientXML(pXML) ;

	pXML->DeleteString( pStr ) ;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

void ClientSMLTest::MyInterruptHandler(sml::smlRunEventId, void* pUserData, sml::Agent* pAgent, sml::smlPhase)
{
	pAgent->GetKernel()->StopAllAgents() ;

	CPPUNIT_ASSERT( pUserData );
	bool* pHandlerReceived = static_cast< bool* >( pUserData );
	*pHandlerReceived = true;
}

std::string ClientSMLTest::MyRhsFunctionHandler(sml::smlRhsEventId, void*, sml::Agent*, char const*, char const* pArgument)
{
	if ( verbose ) std::cout << "Received rhs function call with argument: " << pArgument << std::endl ;

	std::string res = "my rhs result " ;
	res += pArgument ;

	return res ;
}

void ClientSMLTest::doAgentTest( sml::Agent* pAgent )
{
	// a number of tests below depend on running full decision cycles.
	pAgent->ExecuteCommandLine( "set-stop-phase --before --input" ) ;
	CPPUNIT_ASSERT_MESSAGE( "set-stop-phase --before --input", pAgent->GetLastCommandLineResult() );

	// Record a RHS function
	int callback_rhs1 = pKernel->AddRhsFunction("test-rhs", ClientSMLTest::MyRhsFunctionHandler, 0) ; 
	int callback_rhs_dup = pKernel->AddRhsFunction("test-rhs", ClientSMLTest::MyRhsFunctionHandler, 0) ;

	CPPUNIT_ASSERT_MESSAGE( "Duplicate RHS function registration should be detected and be ignored", callback_rhs_dup == callback_rhs1 );

	// Record a client message handler
	bool clientHandlerReceived( false );
	int clientCallback = pKernel->RegisterForClientMessageEvent( "test-client", ClientSMLTest::MyClientMessageHandler, &clientHandlerReceived ) ;

	// This is a bit dopey--but we'll send a message to ourselves for this test
	std::string response = pKernel->SendClientMessage(pAgent, "test-client", "test-message") ;
	CPPUNIT_ASSERT( clientHandlerReceived );
	clientHandlerReceived = false;
	CPPUNIT_ASSERT( response == "handler-messagetest-message" );

	CPPUNIT_ASSERT( pKernel->UnregisterForClientMessageEvent( clientCallback ) );

	// Record a filter
	bool filterHandlerReceived( false );
	int clientFilter = pKernel->RegisterForClientMessageEvent( sml::sml_Names::kFilterName, ClientSMLTest::MyFilterHandler, &filterHandlerReceived ) ;

	// Our filter adds "--depth 2" to all commands
	// so this should give us the result of "print s1 --depth 2"
	std::string command = pAgent->ExecuteCommandLine("print s1") ;
	CPPUNIT_ASSERT_MESSAGE( "print s1", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( filterHandlerReceived );
	filterHandlerReceived = false;

	// TODO: check output
	if ( verbose ) std::cout << command << std::endl ;

	// This is important -- if we don't unregister all subsequent commands will
	// come to our filter and promptly fail!
	CPPUNIT_ASSERT( pKernel->UnregisterForClientMessageEvent( clientFilter ) );

	sml::Identifier* pInputLink = pAgent->GetInputLink() ;
	CPPUNIT_ASSERT( pInputLink );

	pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );

	//int inputReceived = pAgent->RegisterForXMLEvent(smlEVENT_XML_INPUT_RECEIVED, MyXMLInputReceivedHandler, 0) ;

	// Some simple tests
	sml::StringElement* pWME = pAgent->CreateStringWME( pInputLink, "my-att", "my-value" ) ;
	CPPUNIT_ASSERT( pWME );

	// This is to test a bug where an identifier isn't fully removed from working memory (you can still print it) after it is destroyed.
	sml::Identifier* pIDRemoveTest = pAgent->CreateIdWME( pInputLink, "foo" ) ;
	CPPUNIT_ASSERT( pIDRemoveTest );
	CPPUNIT_ASSERT( pAgent->CreateFloatWME( pIDRemoveTest, "bar", 1.23 ) );

	CPPUNIT_ASSERT( pIDRemoveTest->GetValueAsString() );

	sml::Identifier* pID = pAgent->CreateIdWME( pInputLink, "plane" ) ;
	CPPUNIT_ASSERT( pID );

	// Trigger for inputWme update change problem
	sml::StringElement* pWMEtest = pAgent->CreateStringWME( pID, "typeTest", "Boeing747" ) ;
	CPPUNIT_ASSERT( pWMEtest );

	CPPUNIT_ASSERT( pAgent->Commit() );

	pAgent->RunSelf(1) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pAgent->DestroyWME( pIDRemoveTest ) );
	CPPUNIT_ASSERT( pAgent->Commit() );

	//pAgent->RunSelf(1) ;
	CPPUNIT_ASSERT( pAgent->ExecuteCommandLine("print i2 --depth 3") );
	CPPUNIT_ASSERT_MESSAGE( "print i2 --depth 3", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pAgent->ExecuteCommandLine("print F1") );	// BUGBUG: This wme remains in memory even after we add the "RunSelf" at which point it should be gone.
	CPPUNIT_ASSERT_MESSAGE( "print F1", pAgent->GetLastCommandLineResult() );

	pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );

	//pAgent->UnregisterForXMLEvent(inputReceived) ;

	CPPUNIT_ASSERT( pAgent->CreateStringWME(pID, "type", "Boeing747") );

	sml::IntElement* pWME2    = pAgent->CreateIntWME(pID, "speed", 200) ;
	CPPUNIT_ASSERT( pWME2 );

	sml::FloatElement* pWME3  = pAgent->CreateFloatWME(pID, "direction", 50.5) ;
	CPPUNIT_ASSERT( pWME3 );

	CPPUNIT_ASSERT( pAgent->Commit() );

	pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );
	
	// Test the blink option
	pAgent->SetBlinkIfNoChange( false ) ;

	long timeTag1 = pWME3->GetTimeTag() ;
	pAgent->Update( pWME3, 50.5 ) ;	// Should not change the wme, so timetag should be the same
	
	long timeTag2 = pWME3->GetTimeTag() ;
	pAgent->SetBlinkIfNoChange( true ) ;	// Back to the default
	pAgent->Update( pWME3, 50.5 ) ;	// Should change the wme, so timetag should be different
	
	long timeTag3 = pWME3->GetTimeTag() ;

	CPPUNIT_ASSERT_MESSAGE( "Error in handling of SetBlinkIfNoChange flag", timeTag1 == timeTag2 );
	CPPUNIT_ASSERT_MESSAGE( "Error in handling of SetBlinkIfNoChange flag", timeTag2 != timeTag3 );

	// Remove a wme
	CPPUNIT_ASSERT( pAgent->DestroyWME( pWME3 ) );

	// Change the speed to 300
	pAgent->Update( pWME2, 300 ) ;

	// Create a new WME that shares the same id as plane
	// BUGBUG: This is triggering an assert and memory leak now after the changes
	// to InputWME not calling Update() immediately.  For now I've removed the test until
	// we have time to figure out what's going wrong.
	//Identifier* pID2 = pAgent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;
	//unused(pID2);

	CPPUNIT_ASSERT( pAgent->Commit() );

	/*
	printWMEs(pAgent->GetInputLink()) ;
	std::string printInput1 = pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput1 << endl ;
	cout << endl << "Now work with the input link" << endl ;
	*/

	// Delete one of the shared WMEs to make sure that's ok
	//pAgent->DestroyWME(pID) ;
	//pAgent->Commit() ;

	pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );
	
	// Throw in a pattern as a test
	std::string pattern = pAgent->ExecuteCommandLine( "print -i (s1 ^* *)" ) ;
	CPPUNIT_ASSERT_MESSAGE( "print -i (s1 ^* *)", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pKernel->ExpandCommandLine( "p s1" ) );
	CPPUNIT_ASSERT( pKernel->IsRunCommand( "d 3" ) );
	CPPUNIT_ASSERT( pKernel->IsRunCommand( "e 5" ) );
	CPPUNIT_ASSERT( pKernel->IsRunCommand( "run -d 10" ) );

	// Test calling CommandLineXML.
	sml::ClientAnalyzedXML xml ;
	CPPUNIT_ASSERT( pKernel->ExecuteCommandLineXML( "set-library-location", NULL, &xml ) );
	
	std::string path( xml.GetArgString( sml::sml_Names::kParamDirectory ) );

	// Check that we got some string back
	CPPUNIT_ASSERT( path.length() >= 3 );

	// 2nd Test calling CommandLineXML.
	sml::ClientAnalyzedXML xml2 ;
	CPPUNIT_ASSERT( pKernel->ExecuteCommandLineXML( "print -i --depth 3 s1", pAgent->GetAgentName(), &xml2 ) );
	
	char* xmlString = xml2.GenerateXMLString( true );
	CPPUNIT_ASSERT( xmlString );

	sml::ElementXML const* pResult = xml2.GetResultTag() ;
	CPPUNIT_ASSERT( pResult );

	// The XML format of "print" is a <trace> tag containing a series of
	// a) <wme> tags (if this is an --internal print) or
	// b) <id> tags that contain <wme> tags if this is not an --internal print.
	sml::ElementXML traceChild ;
	CPPUNIT_ASSERT( pResult->GetChild( &traceChild, 0 ) );

	int nChildren = traceChild.GetNumberChildren() ;
	sml::ElementXML wmeChild ;
	for (int i = 0 ; i < nChildren ; i++)
	{
		traceChild.GetChild( &wmeChild, i ) ;
		char* wmeString = wmeChild.GenerateXMLString( true ) ;
		CPPUNIT_ASSERT( wmeString );
		if ( verbose ) std::cout << wmeString << std::endl ;
		wmeChild.DeleteString( wmeString ) ;
	}
	xml2.DeleteString(xmlString) ;

	// Test that we get a callback after the decision cycle runs
	// We'll pass in an "int" and use it to count decisions (just as an example of passing user data around)
	int count( 0 );
	int callback1 = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, ClientSMLTest::MyRunEventHandler, &count );
	int callback_dup = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, ClientSMLTest::MyRunEventHandler, &count );

	CPPUNIT_ASSERT_MESSAGE( "Duplicate handler registration should be detected and be ignored", callback1 == callback_dup );

	// This callback unregisters itself in the callback -- as a test to see if we can do that safely.
	int selfRemovingCallback( -1 );
	selfRemovingCallback = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, ClientSMLTest::MyRunSelfRemovingHandler, &selfRemovingCallback ) ;

	// Register for a String event
	bool stringEventHandlerReceived( false );
	int stringCall = pKernel->RegisterForStringEvent( sml::smlEVENT_EDIT_PRODUCTION, ClientSMLTest::MyStringEventHandler, &stringEventHandlerReceived ) ;
	CPPUNIT_ASSERT( pKernel->ExecuteCommandLine( "edit-production my*production", NULL ) );
	CPPUNIT_ASSERT_MESSAGE( "edit-production my*production", pAgent->GetLastCommandLineResult() );
	CPPUNIT_ASSERT( stringEventHandlerReceived );
	stringEventHandlerReceived = false;
	CPPUNIT_ASSERT( pKernel->UnregisterForStringEvent( stringCall ) );

	// Register another handler for the same event, to make sure we can do that.
	// Register this one ahead of the previous handler (so it will fire before MyRunEventHandler)
	bool addToBack = true ;
	int testData( 25 ) ;
	int callback2 = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, ClientSMLTest::MyDuplicateRunEventHandler, &testData, !addToBack) ;

	// Run returns the result (succeeded, failed etc.)
	// To catch the trace output we have to register a print event listener
	std::stringstream trace ;	// We'll pass this into the handler and build up the output in it
	std::string structured ;	// Structured trace goes here
	int callbackp = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, ClientSMLTest::MyPrintEventHandler, &trace) ;

	bool xmlEventHandlerReceived( false );
	int callbackx = pAgent->RegisterForXMLEvent( sml::smlEVENT_XML_TRACE_OUTPUT, ClientSMLTest::MyXMLEventHandler, &xmlEventHandlerReceived ) ;

	int beforeCount( 0 );
	int afterCount( 0 );
	int callback_before = pAgent->RegisterForRunEvent( sml::smlEVENT_BEFORE_RUN_STARTS, ClientSMLTest::MyRunEventHandler, &beforeCount ) ;
	int callback_after = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_RUN_ENDS, ClientSMLTest::MyRunEventHandler, &afterCount ) ;

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
	int outputPhases( 0 );
	int callback_u = pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, ClientSMLTest::MyUpdateEventHandler, &outputPhases ) ;

	int phaseCount( 0 );
	int callbackPhase = pAgent->RegisterForRunEvent( sml::smlEVENT_BEFORE_PHASE_EXECUTED, ClientSMLTest::MyRunEventHandler, &phaseCount ) ;

	// Nothing should match here
	pAgent->RunSelf(4) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", pAgent->GetLastCommandLineResult() );

	// Should be one output phase per decision
	CPPUNIT_ASSERT( outputPhases == 4 );

	CPPUNIT_ASSERT( pAgent->WasAgentOnRunList() );

	CPPUNIT_ASSERT( pAgent->GetResultOfLastRun() == sml::sml_RUN_COMPLETED );

	// Should be 5 phases per decision
	/* Not true now we support stopping before/after phases when running by decision.
	if (phaseCount != 20)
	{
		cout << "Error receiving phase events" << endl ;
		return false ;
	}
	*/

	CPPUNIT_ASSERT( beforeCount == 1 );
	CPPUNIT_ASSERT( afterCount == 1 );

	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callbackPhase) );

	CPPUNIT_ASSERT( xmlEventHandlerReceived );

	// By this point the static variable ClientXMLStorage should have been filled in 
	// and it should be valid, even though the event handler for MyXMLEventHandler has completed.
	CPPUNIT_ASSERT_MESSAGE( "Error receiving XML trace events", clientXMLStorage != NULL );

	// If we crash on this access there's a problem with the ref-counting of
	// the XML message we're passed in MyXMLEventHandler.
	CPPUNIT_ASSERT( clientXMLStorage->ConvertToTraceXML()->IsTagTrace() );

	delete clientXMLStorage ;
	clientXMLStorage = NULL ;

	CPPUNIT_ASSERT( pAgent->UnregisterForXMLEvent(callbackx) );
	CPPUNIT_ASSERT( pAgent->UnregisterForPrintEvent(callbackp) );
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callback_before) );
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callback_after) );
	CPPUNIT_ASSERT( pKernel->UnregisterForUpdateEvent(callback_u) );

	// Print out the standard trace and the same thing as a structured XML trace
	if ( verbose ) std::cout << trace.str() << std::endl ;
	trace.clear();
	if ( verbose ) std::cout << structured << std::endl ;

	/*
	printWMEs(pAgent->GetInputLink()) ;
	std::string printInput = pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput << endl ;
	*/

	// Synchronizing the input link means we make our client copy match
	// the current state of the agent.  We would generally only do this from
	// a different client, but we can test here to see if it does nothing
	// (except deleting and recreating the structures).
	//cout << "Input link before synchronization" << endl ;
	//printWMEs( pAgent->GetInputLink() ) ;

	bool synch = pAgent->SynchronizeInputLink() ;

	if (synch)
	{
		//cout << "Results of synchronizing the input link:" << endl ;
		//printWMEs(pAgent->GetInputLink()) ;
		//cout << endl ;

		CPPUNIT_ASSERT( pAgent->GetInputLink()->GetNumberChildren() != 0 );
	}

	// Then add some tic tac toe stuff which should trigger output
	sml::Identifier* pSquare = pAgent->CreateIdWME(pAgent->GetInputLink(), "square") ;
	CPPUNIT_ASSERT( pSquare );
	sml::StringElement* pEmpty = pAgent->CreateStringWME(pSquare, "content", "RANDOM") ;
	CPPUNIT_ASSERT( pEmpty );
	sml::IntElement* pRow = pAgent->CreateIntWME(pSquare, "row", 1) ;
	CPPUNIT_ASSERT( pRow );
	sml::IntElement* pCol = pAgent->CreateIntWME(pSquare, "col", 2) ;
	CPPUNIT_ASSERT( pCol );

	CPPUNIT_ASSERT( pAgent->Commit() );

	// Update the square's value to be empty.  This ensures that the update
	// call is doing something.  Otherwise, when we run we won't get a match.
	pAgent->Update(pEmpty, "EMPTY") ;
	CPPUNIT_ASSERT( pAgent->Commit() );

	int myCount( 0 );
	int callback_run_count = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, ClientSMLTest::MyRunEventHandler, &myCount) ;

	int outputsGenerated( 0 ) ;
	int callback_g = pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_GENERATED_OUTPUT, ClientSMLTest::MyUpdateEventHandler, &outputsGenerated) ;

	int outputNotifications( 0 ) ;
	int callback_notify = pAgent->RegisterForOutputNotification( ClientSMLTest::MyOutputNotificationHandler, &outputNotifications) ;

	// Can't test this at the same time as testing the getCommand() methods as registering for this clears the output link information
	//int outputHandler = pAgent->AddOutputHandler("move", MyOutputEventHandler, NULL) ;

	if ( verbose ) std::cout << "About to do first run-til-output" << std::endl ;

	int callbackp1 = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, ClientSMLTest::MyPrintEventHandler, &trace) ;

	// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
	// We'll use RunAll just to test it out.  Could use RunSelf and get same result (presumably)
	pKernel->RunAllTilOutput() ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)
	CPPUNIT_ASSERT_MESSAGE( "RunAllTilOutput", pAgent->GetLastCommandLineResult() );

	// We should stop quickly (after a decision or two)
	CPPUNIT_ASSERT_MESSAGE( "Error in RunTilOutput -- it didn't stop on the output", myCount <= 10 );
	CPPUNIT_ASSERT_MESSAGE( "Error in callback handler for MyRunEventHandler -- failed to update count", myCount > 0 );

	if ( verbose ) std::cout << "Agent ran for " << myCount << " decisions before we got output" << std::endl ;
	if ( verbose ) std::cout << trace.str() << std::endl ;
	trace.clear();

	CPPUNIT_ASSERT_MESSAGE( "Error in AFTER_ALL_GENERATED event.", outputsGenerated == 1 );

	CPPUNIT_ASSERT_MESSAGE( "Error in OUTPUT_NOTIFICATION event.", outputNotifications == 1 );

	// Reset the agent and repeat the process to check whether init-soar works.
	pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );
	
	pAgent->RunSelfTilOutput() ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelfTilOutput", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pAgent->UnregisterForOutputNotification(callback_notify) );
	CPPUNIT_ASSERT( pKernel->UnregisterForUpdateEvent(callback_g) );
	CPPUNIT_ASSERT( pAgent->UnregisterForPrintEvent(callbackp1) );

	//cout << "Time to dump output link" << endl ;

	CPPUNIT_ASSERT( pAgent->GetOutputLink() );
	//printWMEs(pAgent->GetOutputLink()) ;

	// Now update the output link with "status complete"
	sml::Identifier* pMove = static_cast< sml::Identifier* >( pAgent->GetOutputLink()->FindByAttribute("move", 0) );
	CPPUNIT_ASSERT( pMove );

	// Try to find an attribute that's missing to make sure we get null back
	sml::Identifier* pMissing = static_cast< sml::Identifier* >( pAgent->GetOutputLink()->FindByAttribute("not-there",0) );
	CPPUNIT_ASSERT( !pMissing );

	sml::Identifier* pMissingInput = static_cast< sml::Identifier* >( pAgent->GetInputLink()->FindByAttribute("not-there",0) );
	CPPUNIT_ASSERT( !pMissingInput );

	// We add an "alternative" to check that we handle shared WMEs correctly.
	// Look it up here.
	sml::Identifier* pAlt = static_cast< sml::Identifier* >( pAgent->GetOutputLink()->FindByAttribute("alternative", 0) );
	CPPUNIT_ASSERT( pAlt );

	// Should also be able to get the command through the "GetCommands" route which tests
	// whether we've flagged the right wmes as "just added" or not.
	int numberCommands = pAgent->GetNumberCommands() ;
	CPPUNIT_ASSERT( numberCommands == 2);

	// Get the first two commands (move and alternate)
	sml::Identifier* pCommand1 = pAgent->GetCommand(0) ;
	sml::Identifier* pCommand2 = pAgent->GetCommand(1) ;
	CPPUNIT_ASSERT( std::string( pCommand1->GetCommandName() ) == "move" || std::string( pCommand2->GetCommandName() ) == "move" );

	pAgent->ClearOutputLinkChanges() ;

	int clearedNumberCommands = pAgent->GetNumberCommands() ;
	CPPUNIT_ASSERT( clearedNumberCommands == 0);

	if ( verbose ) std::cout << "Marking command as completed." << std::endl ;
	sml::StringElement* pCompleted = pAgent->CreateStringWME(pMove, "status", "complete") ;
	CPPUNIT_ASSERT( pCompleted );

	CPPUNIT_ASSERT( pAgent->Commit() );

	// Test the ability to resynch the output link -- this should throw away our current output link representation
	// and explicitly rebuild it to match what the agent currently has.
	bool synchOutput = pAgent->SynchronizeOutputLink() ;

	// This isn't supported for direct connections, so we need to check it did something first.
	if (synchOutput)
	{
		//cout << "Synched output link  -- should be the same as before" << endl ;
		//printWMEs(pAgent->GetOutputLink()) ;

		// Find the move command again--just to make sure it came back after the synch
		sml::Identifier* pMove = static_cast< sml::Identifier* >( pAgent->GetOutputLink()->FindByAttribute("move", 0) );
		CPPUNIT_ASSERT( pMove );

		// We add an "alternative" to check that we handle shared WMEs correctly.
		// Look it up here.
		sml::Identifier* pAlt = static_cast< sml::Identifier* >( pAgent->GetOutputLink()->FindByAttribute("alternative", 0) );
		CPPUNIT_ASSERT( pAlt );

		// Should also be able to get the command through the "GetCommands" route which tests
		// whether we've flagged the right wmes as "just added" or not.
		int numberCommands = pAgent->GetNumberCommands() ;
		CPPUNIT_ASSERT( numberCommands == 2);

		// Get the first two commands (move and alternate)
		sml::Identifier* pCommand1 = pAgent->GetCommand(0) ;
		sml::Identifier* pCommand2 = pAgent->GetCommand(1) ;
		CPPUNIT_ASSERT( std::string( pCommand1->GetCommandName() ) == "move" || std::string( pCommand2->GetCommandName() ) == "move" );
	}

	// The move command should be deleted in response to the
	// the status complete getting added
	pAgent->RunSelf(2) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", pAgent->GetLastCommandLineResult() );

	// Dump out the output link again.
	//if (pAgent->GetOutputLink())
	//{
	//	printWMEs(pAgent->GetOutputLink()) ;
	//}

	// Test that we can interrupt a run by registering a handler that
	// interrupts Soar immediately after a decision cycle.
	// Removed the test part for now. Stats doesn't report anything.
	bool interruptHandlerReceived( false );
	int callback3 = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, ClientSMLTest::MyInterruptHandler, &interruptHandlerReceived ) ;

	pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );

	pAgent->RunSelf(20) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( interruptHandlerReceived );
	interruptHandlerReceived = false;

	//CPPUNIT_ASSERT( pAgent->ExecuteCommandLine("stats") );
	//std::string stats( pAgent->ExecuteCommandLine("stats") );
	//CPPUNIT_ASSERT_MESSAGE( "stats", pAgent->GetLastCommandLineResult() );
	//size_t pos = stats.find( "1 decision cycles" ) ;

/*
	if (pos == std::string.npos)
	{
		cout << "*** ERROR: Failed to interrupt Soar during a run." << endl ;
		return false ;
	}
*/
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callback3) );

	CPPUNIT_ASSERT( pKernel->RemoveRhsFunction(callback_rhs1) );

	/* These comments haven't kept up with the test -- does a lot more now
	cout << endl << "If this test worked should see something like this (above here):" << endl ;
	cout << "Top Identifier I3" << endl << "(I3 ^move M1)" << endl << "(M1 ^row 1)" << endl ;
	cout << "(M1 ^col 1)" << endl << "(I3 ^alternative M1)" << endl ;
	cout << "And then after the command is marked as completed (during the test):" << endl ;
	cout << "Top Identifier I3" << endl ;
	cout << "Together with about 6 received events" << endl ;
	*/

	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callback1) );
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callback2) );
	CPPUNIT_ASSERT( pAgent->UnregisterForRunEvent(callback_run_count) );
}

