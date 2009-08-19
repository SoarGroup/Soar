#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>
#include <sstream>
#include <bitset>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "sml_ClientWMElement.h"
#include "sml_Names.h"
#include "thread_Event.h"
#include "kernel.h"
#include "soarversion.h"

#include "handlers.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif // !_WIN32

enum eTestOptions
{
	NONE,
	USE_CLIENT_THREAD,
	FULLY_OPTIMIZED,
	VERBOSE,
	REMOTE,
	AUTO_COMMIT_DISABLED,
	NUM_TEST_OPTIONS,
};

typedef std::bitset< NUM_TEST_OPTIONS > TestBitset;

#define TEST_DECLARATION( functionName ) void functionName(); void functionName##Body()
#define TEST_DEFINITION( functionName ) void FullTests::functionName() { m_pTestBody = &FullTests::functionName##Body; runAllTestTypes(); } void FullTests::functionName##Body()

class FullTests : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( FullTests );

	CPPUNIT_TEST( testInit );
	CPPUNIT_TEST( testProductions );
	CPPUNIT_TEST( testRHSHandler );
	CPPUNIT_TEST( testClientMessageHandler );
	CPPUNIT_TEST( testFilterHandler );
	CPPUNIT_TEST( testWMEs );
	CPPUNIT_TEST( testAlias );
	CPPUNIT_TEST( testXML );
	CPPUNIT_TEST( testAgent );
	CPPUNIT_TEST( testSimpleCopy );
	CPPUNIT_TEST( testSimpleReteNetLoader );
	CPPUNIT_TEST( testOSupportCopyDestroy );
	CPPUNIT_TEST( testOSupportCopyDestroyCircularParent );
	CPPUNIT_TEST( testOSupportCopyDestroyCircular );
	CPPUNIT_TEST( testSynchronize );
	CPPUNIT_TEST( testRunningAgentCreation );  // bug 952
	//CPPUNIT_TEST( testShutdownHandlerShutdown );
	CPPUNIT_TEST( testEventOrdering ); // bug 1100
	CPPUNIT_TEST( testStatusCompleteDuplication ); // bug 1042
	CPPUNIT_TEST( testStopSoarVsInterrupt ); // bug 782
	CPPUNIT_TEST( testSharedWmeSetViolation ); // bug 1060
	CPPUNIT_TEST( testEchoEquals ); // bug 1028
	CPPUNIT_TEST( testFindAttrPipes ); // bug 1138
	CPPUNIT_TEST( testTemplateVariableNameBug ); // bug 1121
	CPPUNIT_TEST( testNegatedConjunctiveChunkLoopBug510 ); // bug 510
	CPPUNIT_TEST( testGDSBug1144 ); // bug 1144
	CPPUNIT_TEST( testGDSBug1011 ); // bug 1011
	CPPUNIT_TEST( testLearn ); // bug 1145
	CPPUNIT_TEST( testPreferenceSemantics ); // bug 234
	CPPUNIT_TEST( testMatchTimeInterrupt ); // bug 873
	CPPUNIT_TEST( testNegatedConjunctiveTestReorder );
	CPPUNIT_TEST( testNegatedConjunctiveTestUnbound ); // bug 517
	CPPUNIT_TEST( testCommandToFile ); 

	CPPUNIT_TEST_SUITE_END();

public:
	TEST_DECLARATION( testInit );
	TEST_DECLARATION( testProductions );
	TEST_DECLARATION( testRHSHandler );
	TEST_DECLARATION( testClientMessageHandler );
	TEST_DECLARATION( testFilterHandler );
	TEST_DECLARATION( testWMEs );
	TEST_DECLARATION( testAlias );
	TEST_DECLARATION( testXML );
	TEST_DECLARATION( testAgent );
	TEST_DECLARATION( testSimpleCopy );
	TEST_DECLARATION( testSimpleReteNetLoader );
	TEST_DECLARATION( testOSupportCopyDestroy );
	TEST_DECLARATION( testOSupportCopyDestroyCircularParent );
	TEST_DECLARATION( testOSupportCopyDestroyCircular );
	TEST_DECLARATION( testSynchronize );
	TEST_DECLARATION( testRunningAgentCreation );
	TEST_DECLARATION( testEventOrdering );
	TEST_DECLARATION( testStatusCompleteDuplication );
	TEST_DECLARATION( testStopSoarVsInterrupt );
	TEST_DECLARATION( testSharedWmeSetViolation );
	TEST_DECLARATION( testEchoEquals );
	TEST_DECLARATION( testFindAttrPipes );
	TEST_DECLARATION( testTemplateVariableNameBug );
	TEST_DECLARATION( testNegatedConjunctiveChunkLoopBug510 );
	TEST_DECLARATION( testGDSBug1144 );
	TEST_DECLARATION( testGDSBug1011 );
	TEST_DECLARATION( testLearn );
	TEST_DECLARATION( testPreferenceSemantics );
	TEST_DECLARATION( testMatchTimeInterrupt );
	TEST_DECLARATION( testNegatedConjunctiveTestReorder );
	TEST_DECLARATION( testNegatedConjunctiveTestUnbound );
	TEST_DECLARATION( testCommandToFile );

	void testShutdownHandlerShutdown();

public:
	void setUp();
	void tearDown() {}

protected:
	void createSoar();
	void destroySoar();
	void runAllTestTypes();
	void runTest();
	void spawnListener();
	void cleanUpListener();

	void loadProductions( const char* productions );

	TestBitset m_Options;
	sml::Kernel* m_pKernel;
	sml::Agent* m_pAgent;
	void (FullTests::*m_pTestBody)();

	static const std::string kAgentName;

   int m_Port;
   static const int kPortBase;
   static const int kPortRange;

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
#else // _WIN32
    pid_t pid;
#endif // _WIN32
};

CPPUNIT_TEST_SUITE_REGISTRATION( FullTests ); // Registers the test so it will be used

const std::string FullTests::kAgentName( "full-tests-agent" );
const int FullTests::kPortBase = 12122;
const int FullTests::kPortRange = 1000;

void FullTests::setUp()
{
	assert( MAJOR_VERSION_NUMBER == SML_MAJOR_VERSION_NUMBER );
	assert( MINOR_VERSION_NUMBER == SML_MINOR_VERSION_NUMBER );
	assert( MICRO_VERSION_NUMBER == SML_MICRO_VERSION_NUMBER );
	assert( GREEK_VERSION_NUMBER == SML_GREEK_VERSION_NUMBER );
	assert( strcmp( VERSION_STRING, SML_VERSION_STRING ) == 0 );

	m_pKernel = 0;
	m_pAgent = 0;
	m_pTestBody = 0;

	m_Options.reset();
}

void FullTests::runAllTestTypes()
{
	// test 1
	// generate port
	m_Port = rand() % kPortRange;
	m_Port += kPortBase;

	m_Options.reset();
	m_Options.set( USE_CLIENT_THREAD );
	m_Options.set( FULLY_OPTIMIZED );
	runTest();
	std::cout << "1";
	std::cout.flush();

	// test 2
	// generate port
	m_Port = rand() % kPortRange;
	m_Port += kPortBase;

	m_Options.reset();
	m_Options.set( USE_CLIENT_THREAD );
	runTest();
	std::cout << "2";
	std::cout.flush();

	// test 3
	// generate port
	m_Port = rand() % kPortRange;
	m_Port += kPortBase;

	m_Options.reset();
	runTest();
	std::cout << "3";
	std::cout.flush();

	// test 4
	// generate port
	m_Port = rand() % kPortRange;
	m_Port += kPortBase;

	m_Options.reset();
	m_Options.set( REMOTE );
	runTest();
	std::cout << "4";
	std::cout.flush();
}

void FullTests::runTest()
{
	// create kernel and agent
	createSoar();
	
	// run test
	(this->*m_pTestBody)();

	// destroy agent and kernel
	destroySoar();
}

void FullTests::createSoar()
{
	CPPUNIT_ASSERT( m_pKernel == NULL );

	if ( m_Options.test( REMOTE ) )
	{
		spawnListener();
		m_pKernel = sml::Kernel::CreateRemoteConnection( true, 0, m_Port );
	}
	else
	{
		if ( m_Options.test( USE_CLIENT_THREAD ) )
		{
			bool optimized = m_Options.test( FULLY_OPTIMIZED );
			m_pKernel = sml::Kernel::CreateKernelInCurrentThread( sml::Kernel::GetDefaultLibraryName(), optimized, m_Port );
		}
		else
		{
			m_pKernel = sml::Kernel::CreateKernelInNewThread( sml::Kernel::GetDefaultLibraryName(), m_Port );
		}
	}

	CPPUNIT_ASSERT( m_pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( m_pKernel->GetLastErrorDescription(), !m_pKernel->HadError() );

	if ( m_Options.test( VERBOSE ) ) std::cout << "Soar kernel version " << m_pKernel->GetSoarKernelVersion() << std::endl ;
	if ( m_Options.test( VERBOSE ) ) std::cout << "SML version " << sml::sml_Names::kSMLVersionValue << std::endl ;

	CPPUNIT_ASSERT( std::string( m_pKernel->GetSoarKernelVersion() ) == std::string( sml::sml_Names::kSoarVersionValue ) );

	bool creationHandlerReceived( false );
	int agentCreationCallback = m_pKernel->RegisterForAgentEvent( sml::smlEVENT_AFTER_AGENT_CREATED, Handlers::MyCreationHandler, &creationHandlerReceived ) ;
	
	// Report the number of agents (always 0 unless this is a remote connection to a CLI or some such)
	CPPUNIT_ASSERT( m_pKernel->GetNumberAgents() == 0 );

	// NOTE: We don't delete the agent pointer.  It's owned by the kernel
	m_pAgent = m_pKernel->CreateAgent( kAgentName.c_str() ) ;
	CPPUNIT_ASSERT_MESSAGE( m_pKernel->GetLastErrorDescription(), !m_pKernel->HadError() );
	CPPUNIT_ASSERT( m_pAgent != NULL );
	CPPUNIT_ASSERT( creationHandlerReceived );

	CPPUNIT_ASSERT( m_pKernel->UnregisterForAgentEvent( agentCreationCallback ) );

	// a number of tests below depend on running full decision cycles.
	m_pAgent->ExecuteCommandLine( "set-stop-phase --before --input" ) ;
	CPPUNIT_ASSERT_MESSAGE( "set-stop-phase --before --input", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( m_pKernel->GetNumberAgents() == 1 );

	m_pKernel->SetAutoCommit( !m_Options.test( AUTO_COMMIT_DISABLED ) ) ;
}

void FullTests::destroySoar()
{
	// Agent deletion
	if ( m_Options.test( VERBOSE ) ) std::cout << "Destroy the agent now" << std::endl ;

	// The Before_Agent_Destroyed callback is a tricky one so we'll register for it to test it.
	// We need to get this callback just before the agentSML data is deleted (otherwise there'll be no way to send/receive the callback)
	// and then continue on to delete the agent after we've responded to the callback.
	// Interestingly, we don't explicitly unregister this callback because the agent has already been destroyed so
	// that's another test, that this callback is cleaned up correctly (and automatically).
	bool deletionHandlerReceived( false );
	m_pKernel->RegisterForAgentEvent( sml::smlEVENT_BEFORE_AGENT_DESTROYED, Handlers::MyDeletionHandler, &deletionHandlerReceived ) ;

	// Explicitly destroy our agent as a test, before we delete the kernel itself.
	// (Actually, if this is a remote connection we need to do this or the agent
	//  will remain alive).
	CPPUNIT_ASSERT( m_pKernel->DestroyAgent( m_pAgent ) );
	CPPUNIT_ASSERT( deletionHandlerReceived );
	deletionHandlerReceived = false;

	if ( m_Options.test( VERBOSE ) ) std::cout << "Calling shutdown on the kernel now" << std::endl ;

	if ( m_Options.test( REMOTE ) )
	{
		soar_thread::Event shutdownEvent;
		m_pKernel->RegisterForSystemEvent( sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyEventShutdownHandler, &shutdownEvent ) ;

		// BUGBUG
		// ClientSML thread dies inelegantly here spewing forth error messages
		// about sockets/pipes not being shut down correctly.
		std::string shutdownResponse = m_pKernel->SendClientMessage(0, "test-listener", "shutdown") ;
		CPPUNIT_ASSERT( shutdownResponse == "ok" );	

		CPPUNIT_ASSERT_MESSAGE( "Listener side kernel shutdown failed to fire smlEVENT_BEFORE_SHUTDOWN", shutdownEvent.WaitForEvent(5, 0) );

		// Note, in the remote case, this does not fire smlEVENT_BEFORE_SHUTDOWN
		// the listener side shutdown does trigger the event when it is deleted, see simplelistener.cpp
		m_pKernel->Shutdown() ;
		
	} else {
		bool shutdownHandlerReceived( false );
		m_pKernel->RegisterForSystemEvent( sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyBoolShutdownHandler, &shutdownHandlerReceived ) ;
		m_pKernel->Shutdown() ;
		CPPUNIT_ASSERT( shutdownHandlerReceived );
	}

	if ( m_Options.test( VERBOSE ) ) std::cout << "Shutdown completed now" << std::endl ;

	// Delete the kernel.  If this is an embedded connection this destroys the kernel.
	// If it's a remote connection we just disconnect.
	delete m_pKernel ;
	m_pKernel = NULL;

	if ( m_Options.test( REMOTE ) )
	{
		cleanUpListener();
		if ( m_Options.test( VERBOSE ) ) std::cout << "Cleaned up listener." << std::endl;
	}
}

void FullTests::spawnListener()
{
	// Spawning a new process is radically different on windows vs linux.
	// Instead of writing an abstraction layer, I'm just going to put platform-
	// specific code here.

#ifdef _WIN32
   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // Start the child process. 
   std::wstringstream commandLine;
   commandLine << L"Tests.exe --listener " << m_Port;
   BOOL success = CreateProcess( 
      L"Tests.exe",
      const_cast< LPWSTR >( commandLine.str().c_str() ),        // Command line
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
#else // _WIN32
	pid = fork();
	CPPUNIT_ASSERT_MESSAGE( "fork error", pid >= 0 );
	if ( pid == 0 )
	{
		// child
      std::stringstream portString;
      portString << m_Port;
		execl("Tests", "Tests", "--listener", portString.str().c_str(), static_cast< char* >( 0 ));
		// does not return on success
		CPPUNIT_ASSERT_MESSAGE( "execl failed", false );
	}
#endif // _WIN32

	sml::Sleep( 1, 0 );
}

void FullTests::cleanUpListener()
{
#ifdef _WIN32
	// Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else // _WIN32
	int status( 0 );
	wait( &status );
	if ( WIFEXITED( status ) )
	{
		CPPUNIT_ASSERT_MESSAGE( "listener terminated with nonzero status", WEXITSTATUS( status ) == 0 );
	}
	else
	{
		CPPUNIT_ASSERT_MESSAGE( "listener killed by signal", WIFSIGNALED( status ) );
		
		// not sure why signal 0 comes up but seems to fix things on Mac OS
		if ( !WIFSTOPPED( status ) && ( WSTOPSIG(status) != 0 ) )
		{
			CPPUNIT_ASSERT_MESSAGE( "listener stopped by signal", WIFSTOPPED( status ) );
		}
		else if ( WIFSTOPPED( status ) )
		{
			CPPUNIT_ASSERT_MESSAGE( "listener continued", WIFCONTINUED( status ) );
			CPPUNIT_ASSERT_MESSAGE( "listener died: unknown", false );
		}
	}
#endif // _WIN32
}

void FullTests::loadProductions( const char* productions )
{
	std::stringstream productionsPath;
	productionsPath << m_pKernel->GetLibraryLocation() << productions;

	m_pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "loadProductions", m_pAgent->GetLastCommandLineResult() );
}

TEST_DEFINITION( testInit )
{
	m_pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", m_pAgent->GetLastCommandLineResult() );
}

TEST_DEFINITION( testProductions )
{
	// Load and test productions
	loadProductions( "/Tests/testsml.soar" );

	CPPUNIT_ASSERT( m_pAgent->IsProductionLoaded( "apply*move" ) );
	CPPUNIT_ASSERT( !m_pAgent->IsProductionLoaded( "made*up*name" ) );

	int excisedCount( 0 );
	int prodCall = m_pAgent->RegisterForProductionEvent( sml::smlEVENT_BEFORE_PRODUCTION_REMOVED, Handlers::MyProductionHandler, &excisedCount ) ;

	m_pAgent->ExecuteCommandLine( "excise --all" ) ;
	CPPUNIT_ASSERT_MESSAGE( "excise --all", m_pAgent->GetLastCommandLineResult() );
	CPPUNIT_ASSERT( excisedCount > 0 );

	excisedCount = 0;
	loadProductions( "/Tests/testsml.soar" );
	CPPUNIT_ASSERT( excisedCount == 0 );

	CPPUNIT_ASSERT( m_pAgent->UnregisterForProductionEvent( prodCall ) );
}

TEST_DEFINITION( testRHSHandler )
{
	loadProductions( "/Tests/testsml.soar" );

	bool rhsFunctionHandlerReceived( false );

	// Record a RHS function
	int callback_rhs1 = m_pKernel->AddRhsFunction( "test-rhs", Handlers::MyRhsFunctionHandler, &rhsFunctionHandlerReceived ) ; 
	int callback_rhs_dup = m_pKernel->AddRhsFunction( "test-rhs", Handlers::MyRhsFunctionHandler, &rhsFunctionHandlerReceived ) ;
	//m_pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::DebugPrintEventHandler, 0) ;

	CPPUNIT_ASSERT_MESSAGE( "Duplicate RHS function registration should be detected and be ignored", callback_rhs_dup == callback_rhs1 );

	// need this to fire production that calls test-rhs
	sml::Identifier* pSquare = m_pAgent->GetInputLink()->CreateIdWME( "square" ) ;
	CPPUNIT_ASSERT( pSquare );
	sml::StringElement* pEmpty = pSquare->CreateStringWME( "content", "EMPTY" ) ;
	CPPUNIT_ASSERT( pEmpty );
	sml::IntElement* pRow = pSquare->CreateIntWME( "row", 1 ) ;
	CPPUNIT_ASSERT( pRow );
	sml::IntElement* pCol = pSquare->CreateIntWME( "col", 2 ) ;
	CPPUNIT_ASSERT( pCol );
	CPPUNIT_ASSERT( m_pAgent->Commit() );

	m_pKernel->RunAllAgents( 1 ) ;
	CPPUNIT_ASSERT_MESSAGE( "RunAllAgents", m_pAgent->GetLastCommandLineResult() );

	//std::cout << m_pAgent->ExecuteCommandLine("p i2 --depth 4") << std::endl;

	CPPUNIT_ASSERT( rhsFunctionHandlerReceived );

	CPPUNIT_ASSERT( m_pKernel->RemoveRhsFunction( callback_rhs1 ) );

	// Re-add it without the bool that is getting popped off the stack
	CPPUNIT_ASSERT( m_pKernel->AddRhsFunction( "test-rhs", Handlers::MyRhsFunctionHandler, 0 ) ); 

	CPPUNIT_ASSERT( pSquare->DestroyWME(  ) );
	CPPUNIT_ASSERT( m_pAgent->Commit() );
}

TEST_DEFINITION( testClientMessageHandler )
{
	// Record a client message handler
	bool clientHandlerReceived( false );
	int clientCallback = m_pKernel->RegisterForClientMessageEvent( "test-client", Handlers::MyClientMessageHandler, &clientHandlerReceived ) ;

	// This is a bit dopey--but we'll send a message to ourselves for this test
	std::string response = m_pKernel->SendClientMessage( m_pAgent, "test-client", "test-message" );
	CPPUNIT_ASSERT( clientHandlerReceived );
	clientHandlerReceived = false;
	CPPUNIT_ASSERT( response == "handler-messagetest-message" );

	CPPUNIT_ASSERT( m_pKernel->UnregisterForClientMessageEvent( clientCallback ) );
}

TEST_DEFINITION( testFilterHandler )
{
	// Record a filter
	bool filterHandlerReceived( false );
	int clientFilter = m_pKernel->RegisterForClientMessageEvent( sml::sml_Names::kFilterName, Handlers::MyFilterHandler, &filterHandlerReceived ) ;

	// Our filter adds "--depth 2" to all commands
	// so this should give us the result of "print s1 --depth 2"
	std::string output = m_pAgent->ExecuteCommandLine("print s1") ;
	CPPUNIT_ASSERT_MESSAGE( "print s1", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( filterHandlerReceived );
	filterHandlerReceived = false;

	// depth 2 should reveal I2
	CPPUNIT_ASSERT( output.find( "I2" ) != std::string::npos );

	// This is important -- if we don't unregister all subsequent commands will
	// come to our filter and promptly fail!
	CPPUNIT_ASSERT( m_pKernel->UnregisterForClientMessageEvent( clientFilter ) );
}

TEST_DEFINITION( testWMEs )
{
	sml::Identifier* pInputLink = m_pAgent->GetInputLink() ;
	CPPUNIT_ASSERT( pInputLink );

	// Some simple tests
	sml::StringElement* pWME = pInputLink->CreateStringWME( "my-att", "my-value" ) ;
	CPPUNIT_ASSERT( pWME );

	// This is to test a bug where an identifier isn't fully removed from working memory (you can still print it) after it is destroyed.
	sml::Identifier* pIDRemoveTest = pInputLink->CreateIdWME( "foo" ) ;
	CPPUNIT_ASSERT( pIDRemoveTest );
	CPPUNIT_ASSERT( pIDRemoveTest->CreateFloatWME( "bar", 1.23 ) );

	CPPUNIT_ASSERT( pIDRemoveTest->GetValueAsString() );

	sml::Identifier* pID = pInputLink->CreateIdWME( "plane" ) ;
	CPPUNIT_ASSERT( pID );

	// Trigger for inputWme update change problem
	sml::StringElement* pWMEtest = pID->CreateStringWME( "typeTest", "Boeing747" ) ;
	CPPUNIT_ASSERT( pWMEtest );

	CPPUNIT_ASSERT( m_pAgent->Commit() );

	m_pAgent->RunSelf(1) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pIDRemoveTest->DestroyWME(  ) );
	CPPUNIT_ASSERT( m_pAgent->Commit() );

	//m_pAgent->RunSelf(1) ;
	CPPUNIT_ASSERT( m_pAgent->ExecuteCommandLine("print i2 --depth 3") );
	CPPUNIT_ASSERT_MESSAGE( "print i2 --depth 3", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( m_pAgent->ExecuteCommandLine("print F1") );	// BUGBUG: This wme remains in memory even after we add the "RunSelf" at which point it should be gone.
	CPPUNIT_ASSERT_MESSAGE( "print F1", m_pAgent->GetLastCommandLineResult() );

	m_pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pID->CreateStringWME("type", "Boeing747") );

	sml::IntElement* pWME2    = pID->CreateIntWME("speed", 200) ;
	CPPUNIT_ASSERT( pWME2 );

	sml::FloatElement* pWME3  = pID->CreateFloatWME("direction", 50.5) ;
	CPPUNIT_ASSERT( pWME3 );

	CPPUNIT_ASSERT( m_pAgent->Commit() );

	m_pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", m_pAgent->GetLastCommandLineResult() );
	
	// Test the blink option
	m_pAgent->SetBlinkIfNoChange( false ) ;

	long timeTag1 = pWME3->GetTimeTag() ;
	m_pAgent->Update( pWME3, 50.5 ) ;	// Should not change the wme, so timetag should be the same
	
	long timeTag2 = pWME3->GetTimeTag() ;
	m_pAgent->SetBlinkIfNoChange( true ) ;	// Back to the default
	m_pAgent->Update( pWME3, 50.5 ) ;	// Should change the wme, so timetag should be different
	
	long timeTag3 = pWME3->GetTimeTag() ;

	CPPUNIT_ASSERT_MESSAGE( "Error in handling of SetBlinkIfNoChange flag", timeTag1 == timeTag2 );
	CPPUNIT_ASSERT_MESSAGE( "Error in handling of SetBlinkIfNoChange flag", timeTag2 != timeTag3 );

	// Remove a wme
	CPPUNIT_ASSERT( pWME3->DestroyWME(  ) );

	// Change the speed to 300
	m_pAgent->Update( pWME2, 300 ) ;

	// Create a new WME that shares the same id as plane
	// BUGBUG: This is triggering an assert and memory leak now after the changes
	// to InputWME not calling Update() immediately.  For now I've removed the test until
	// we have time to figure out what's going wrong.
	//Identifier* pID2 = m_pAgent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;
	//unused(pID2);

	CPPUNIT_ASSERT( m_pAgent->Commit() );

	/*
	printWMEs(m_pAgent->GetInputLink()) ;
	std::string printInput1 = m_pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput1 << endl ;
	cout << endl << "Now work with the input link" << endl ;
	*/

	// Delete one of the shared WMEs to make sure that's ok
	//m_pAgent->DestroyWME(pID) ;
	//m_pAgent->Commit() ;

	// Throw in a pattern as a test
	std::string pattern = m_pAgent->ExecuteCommandLine( "print -i (s1 ^* *)" ) ;
	CPPUNIT_ASSERT_MESSAGE( "print -i (s1 ^* *)", m_pAgent->GetLastCommandLineResult() );

}

TEST_DEFINITION( testAlias )
{
	CPPUNIT_ASSERT( m_pKernel->ExpandCommandLine( "p s1" ) );	// test for null first
	CPPUNIT_ASSERT( std::string( m_pKernel->ExpandCommandLine( "p s1" ) ) == "print s1" );

	CPPUNIT_ASSERT( m_pKernel->IsRunCommand( "d 3" ) );
	CPPUNIT_ASSERT( m_pKernel->IsRunCommand( "e 5" ) );
	CPPUNIT_ASSERT( m_pKernel->IsRunCommand( "run -d 10" ) );
}

TEST_DEFINITION( testXML )
{
	// Test calling CommandLineXML.
	sml::ClientAnalyzedXML xml ;
	CPPUNIT_ASSERT( m_pKernel->ExecuteCommandLineXML( "set-library-location", NULL, &xml ) );
	
	std::string path( xml.GetArgString( sml::sml_Names::kParamDirectory ) );

	// Check that we got some string back
	CPPUNIT_ASSERT( path.length() >= 3 );

	// 2nd Test calling CommandLineXML.
	sml::ClientAnalyzedXML xml2 ;
	CPPUNIT_ASSERT( m_pKernel->ExecuteCommandLineXML( "print -i --depth 3 s1", m_pAgent->GetAgentName(), &xml2 ) );
	
	char* xmlString = xml2.GenerateXMLString( true );
	CPPUNIT_ASSERT( xmlString );

	soarxml::ElementXML const* pResult = xml2.GetResultTag() ;
	CPPUNIT_ASSERT( pResult );

	// The XML format of "print" is a <trace> tag containing a series of
	// a) <wme> tags (if this is an --internal print) or
	// b) <id> tags that contain <wme> tags if this is not an --internal print.
	soarxml::ElementXML traceChild ;
	CPPUNIT_ASSERT( pResult->GetChild( &traceChild, 0 ) );

	int nChildren = traceChild.GetNumberChildren() ;
	soarxml::ElementXML wmeChild ;
	for (int i = 0 ; i < nChildren ; i++)
	{
		traceChild.GetChild( &wmeChild, i ) ;
		char* wmeString = wmeChild.GenerateXMLString( true ) ;
		CPPUNIT_ASSERT( wmeString );
		if ( m_Options.test( VERBOSE ) )
		{
			std::cout << wmeString << std::endl ;
		}
		wmeChild.DeleteString( wmeString ) ;
	}
	xml2.DeleteString(xmlString) ;
}

TEST_DEFINITION( testAgent )
{
	//m_pKernel->SetTraceCommunications( true );

	loadProductions( "/Tests/testsml.soar" );

	// Test that we get a callback after the decision cycle runs
	// We'll pass in an "int" and use it to count decisions (just as an example of passing user data around)
	int count( 0 );
	int callback1 = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &count );
	int callback_dup = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &count );

	CPPUNIT_ASSERT_MESSAGE( "Duplicate handler registration should be detected and be ignored", callback1 == callback_dup );

	// This callback unregisters itself in the callback -- as a test to see if we can do that safely.
	int selfRemovingCallback( -1 );
	selfRemovingCallback = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunSelfRemovingHandler, &selfRemovingCallback ) ;

	// Register for a String event
	bool stringEventHandlerReceived( false );
	int stringCall = m_pKernel->RegisterForStringEvent( sml::smlEVENT_EDIT_PRODUCTION, Handlers::MyStringEventHandler, &stringEventHandlerReceived ) ;
	CPPUNIT_ASSERT( m_pKernel->ExecuteCommandLine( "edit-production my*production", NULL ) );
	CPPUNIT_ASSERT_MESSAGE( "edit-production my*production", m_pAgent->GetLastCommandLineResult() );
	CPPUNIT_ASSERT( stringEventHandlerReceived );
	stringEventHandlerReceived = false;
	CPPUNIT_ASSERT( m_pKernel->UnregisterForStringEvent( stringCall ) );

	// Register another handler for the same event, to make sure we can do that.
	// Register this one ahead of the previous handler (so it will fire before MyRunEventHandler)
	bool addToBack = true ;
	int testData( 25 ) ;
	int callback2 = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyDuplicateRunEventHandler, &testData, !addToBack) ;

	// Run returns the result (succeeded, failed etc.)
	// To catch the trace output we have to register a print event listener
	std::stringstream trace ;	// We'll pass this into the handler and build up the output in it
	std::string structured ;	// Structured trace goes here
	int callbackp = m_pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

	sml::ClientXML* clientXMLStorage = 0;
	int callbackx = m_pAgent->RegisterForXMLEvent( sml::smlEVENT_XML_TRACE_OUTPUT, Handlers::MyXMLEventHandler, &clientXMLStorage ) ;

	int beforeCount( 0 );
	int afterCount( 0 );
	int callback_before = m_pAgent->RegisterForRunEvent( sml::smlEVENT_BEFORE_RUN_STARTS, Handlers::MyRunEventHandler, &beforeCount ) ;
	int callback_after = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_RUN_ENDS, Handlers::MyRunEventHandler, &afterCount ) ;

	//Some temp code to generate more complex watch traces.  Not usually part of the test
	/*
	Identifier* pSquare1 = m_pAgent->CreateIdWME(pInputLink, "square") ;
	StringElement* pEmpty1 = pSquare1->CreateStringWME("content", "RANDOM") ;
	IntElement* pRow1 = m_pAgent->CreateIntWME(pSquare1, "row", 1) ;
	IntElement* pCol1 = m_pAgent->CreateIntWME(pSquare1, "col", 2) ;
	m_pAgent->Update(pEmpty1, "EMPTY") ;
	ok = m_pAgent->Commit() ;
	m_pAgent->ExecuteCommandLine("watch 3") ;
	*/

	// Test that we get a callback after the all output phases complete
	// We'll pass in an "int" and use it to count output phases
	int outputPhases( 0 );
	int callback_u = m_pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, Handlers::MyUpdateEventHandler, &outputPhases ) ;

	int phaseCount( 0 );
	int callbackPhase = m_pAgent->RegisterForRunEvent( sml::smlEVENT_BEFORE_PHASE_EXECUTED, Handlers::MyRunEventHandler, &phaseCount ) ;

	// Nothing should match here
	m_pAgent->RunSelf(4) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", m_pAgent->GetLastCommandLineResult() );

	// Should be one output phase per decision
	CPPUNIT_ASSERT( outputPhases == 4 );

	CPPUNIT_ASSERT( m_pAgent->WasAgentOnRunList() );

	CPPUNIT_ASSERT( m_pAgent->GetResultOfLastRun() == sml::sml_RUN_COMPLETED );

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

	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callbackPhase) );

	// By this point the static variable ClientXMLStorage should have been filled in 
	// and it should be valid, even though the event handler for MyXMLEventHandler has completed.
	CPPUNIT_ASSERT_MESSAGE( "Error receiving XML trace events", clientXMLStorage != NULL );

	// If we crash on this access there's a problem with the ref-counting of
	// the XML message we're passed in MyXMLEventHandler.
	CPPUNIT_ASSERT( clientXMLStorage->ConvertToTraceXML()->IsTagTrace() );

	delete clientXMLStorage ;
	clientXMLStorage = NULL ;

	CPPUNIT_ASSERT( m_pAgent->UnregisterForXMLEvent(callbackx) );
	CPPUNIT_ASSERT( m_pAgent->UnregisterForPrintEvent(callbackp) );
	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callback_before) );
	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callback_after) );
	CPPUNIT_ASSERT( m_pKernel->UnregisterForUpdateEvent(callback_u) );

	// Print out the standard trace and the same thing as a structured XML trace
	if ( m_Options.test( VERBOSE ) )
	{
		std::cout << trace.str() << std::endl ;
	}
	trace.clear();
	if ( m_Options.test( VERBOSE ) )
	{
		std::cout << structured << std::endl ;
	}

	/*
	printWMEs(m_pAgent->GetInputLink()) ;
	std::string printInput = m_pAgent->ExecuteCommandLine("print --depth 2 I2") ;
	cout << printInput << endl ;
	*/

	// Then add some tic tac toe stuff which should trigger output
	sml::Identifier* pSquare = m_pAgent->GetInputLink()->CreateIdWME("square") ;
	CPPUNIT_ASSERT( pSquare );
	sml::StringElement* pEmpty = pSquare->CreateStringWME("content", "RANDOM") ;
	CPPUNIT_ASSERT( pEmpty );
	sml::IntElement* pRow = pSquare->CreateIntWME("row", 1) ;
	CPPUNIT_ASSERT( pRow );
	sml::IntElement* pCol = pSquare->CreateIntWME("col", 2) ;
	CPPUNIT_ASSERT( pCol );

	CPPUNIT_ASSERT( m_pAgent->Commit() );

	// Update the square's value to be empty.  This ensures that the update
	// call is doing something.  Otherwise, when we run we won't get a match.
	m_pAgent->Update(pEmpty, "EMPTY") ;
	CPPUNIT_ASSERT( m_pAgent->Commit() );

	int myCount( 0 );
	int callback_run_count = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &myCount) ;

	int outputsGenerated( 0 ) ;
	int callback_g = m_pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Handlers::MyUpdateEventHandler, &outputsGenerated) ;

	int outputNotifications( 0 ) ;
	int callback_notify = m_pAgent->RegisterForOutputNotification( Handlers::MyOutputNotificationHandler, &outputNotifications) ;

	// Can't test this at the same time as testing the getCommand() methods as registering for this clears the output link information
	//int outputHandler = m_pAgent->AddOutputHandler("move", MyOutputEventHandler, NULL) ;

	if ( m_Options.test( VERBOSE ) )
	{
		std::cout << "About to do first run-til-output" << std::endl ;
	}

	int callbackp1 = m_pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

	// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
	// We'll use RunAll just to test it out.  Could use RunSelf and get same result (presumably)
	m_pKernel->RunAllTilOutput() ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)
	CPPUNIT_ASSERT_MESSAGE( "RunAllTilOutput", m_pAgent->GetLastCommandLineResult() );

	// We should stop quickly (after a decision or two)
	CPPUNIT_ASSERT_MESSAGE( "Error in RunTilOutput -- it didn't stop on the output", myCount <= 10 );
	CPPUNIT_ASSERT_MESSAGE( "Error in callback handler for MyRunEventHandler -- failed to update count", myCount > 0 );

	if ( m_Options.test( VERBOSE ) )
	{
		std::cout << "Agent ran for " << myCount << " decisions before we got output" << std::endl ;
	}
	if ( m_Options.test( VERBOSE ) )
	{
		std::cout << trace.str() << std::endl ;
	}
	trace.clear();

	CPPUNIT_ASSERT_MESSAGE( "Error in AFTER_ALL_GENERATED event.", outputsGenerated == 1 );

	CPPUNIT_ASSERT_MESSAGE( "Error in OUTPUT_NOTIFICATION event.", outputNotifications == 1 );

	// Reset the agent and repeat the process to check whether init-soar works.
	m_pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", m_pAgent->GetLastCommandLineResult() );
	
	m_pAgent->RunSelfTilOutput() ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelfTilOutput", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( m_pAgent->UnregisterForOutputNotification(callback_notify) );
	CPPUNIT_ASSERT( m_pKernel->UnregisterForUpdateEvent(callback_g) );
	CPPUNIT_ASSERT( m_pAgent->UnregisterForPrintEvent(callbackp1) );

	//cout << "Time to dump output link" << endl ;

	CPPUNIT_ASSERT( m_pAgent->GetOutputLink() );
	//printWMEs(m_pAgent->GetOutputLink()) ;

	// Now update the output link with "status complete"
	sml::Identifier* pMove = static_cast< sml::Identifier* >( m_pAgent->GetOutputLink()->FindByAttribute("move", 0) );
	CPPUNIT_ASSERT( pMove );

	// Try to find an attribute that's missing to make sure we get null back
	sml::Identifier* pMissing = static_cast< sml::Identifier* >( m_pAgent->GetOutputLink()->FindByAttribute("not-there",0) );
	CPPUNIT_ASSERT( !pMissing );

	sml::Identifier* pMissingInput = static_cast< sml::Identifier* >( m_pAgent->GetInputLink()->FindByAttribute("not-there",0) );
	CPPUNIT_ASSERT( !pMissingInput );

	// We add an "alternative" to check that we handle shared WMEs correctly.
	// Look it up here.
	sml::Identifier* pAlt = static_cast< sml::Identifier* >( m_pAgent->GetOutputLink()->FindByAttribute("alternative", 0) );
	CPPUNIT_ASSERT( pAlt );

	// Should also be able to get the command through the "GetCommands" route which tests
	// whether we've flagged the right wmes as "just added" or not.
	int numberCommands = m_pAgent->GetNumberCommands() ;
	CPPUNIT_ASSERT( numberCommands == 3);

	// Get the first two commands (move and alternative and A)
	sml::Identifier* pCommand1 = m_pAgent->GetCommand(0) ;
	sml::Identifier* pCommand2 = m_pAgent->GetCommand(1) ;
	sml::Identifier* pCommand3 = m_pAgent->GetCommand(2) ;
	CPPUNIT_ASSERT( std::string( pCommand1->GetCommandName() ) == "move" 
		|| std::string( pCommand2->GetCommandName() ) == "move"  
		|| std::string( pCommand3->GetCommandName() ) == "move" );
	CPPUNIT_ASSERT( std::string( pCommand1->GetCommandName() ) == "alternative" 
		|| std::string( pCommand2->GetCommandName() ) == "alternative"  
		|| std::string( pCommand3->GetCommandName() ) == "alternative" );
	CPPUNIT_ASSERT( std::string( pCommand1->GetCommandName() ) == "A" 
		|| std::string( pCommand2->GetCommandName() ) == "A"  
		|| std::string( pCommand3->GetCommandName() ) == "A" );

	m_pAgent->ClearOutputLinkChanges() ;

	int clearedNumberCommands = m_pAgent->GetNumberCommands() ;
	CPPUNIT_ASSERT( clearedNumberCommands == 0);

	if ( m_Options.test( VERBOSE ) ) std::cout << "Marking command as completed." << std::endl ;
	pMove->AddStatusComplete();
	CPPUNIT_ASSERT( m_pAgent->Commit() );

	// The move command should be deleted in response to the
	// the status complete getting added
	m_pAgent->RunSelf(2) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", m_pAgent->GetLastCommandLineResult() );

	// Dump out the output link again.
	//if (m_pAgent->GetOutputLink())
	//{
	//	printWMEs(m_pAgent->GetOutputLink()) ;
	//}

	// Test that we can interrupt a run by registering a handler that
	// interrupts Soar immediately after a decision cycle.
	// Removed the test part for now. Stats doesn't report anything.
	bool interruptHandlerReceived( false );
	int callback3 = m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyInterruptHandler, &interruptHandlerReceived ) ;

	m_pAgent->InitSoar();
	CPPUNIT_ASSERT_MESSAGE( "init-soar", m_pAgent->GetLastCommandLineResult() );

	m_pAgent->RunSelf(20) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", m_pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( interruptHandlerReceived );
	interruptHandlerReceived = false;

	//CPPUNIT_ASSERT( m_pAgent->ExecuteCommandLine("stats") );
	//std::string stats( m_pAgent->ExecuteCommandLine("stats") );
	//CPPUNIT_ASSERT_MESSAGE( "stats", m_pAgent->GetLastCommandLineResult() );
	//size_t pos = stats.find( "1 decision cycles" ) ;

/*
	if (pos == std::string.npos)
	{
		cout << "*** ERROR: Failed to interrupt Soar during a run." << endl ;
		return false ;
	}
*/
	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callback3) );

	/* These comments haven't kept up with the test -- does a lot more now
	cout << endl << "If this test worked should see something like this (above here):" << endl ;
	cout << "Top Identifier I3" << endl << "(I3 ^move M1)" << endl << "(M1 ^row 1)" << endl ;
	cout << "(M1 ^col 1)" << endl << "(I3 ^alternative M1)" << endl ;
	cout << "And then after the command is marked as completed (during the test):" << endl ;
	cout << "Top Identifier I3" << endl ;
	cout << "Together with about 6 received events" << endl ;
	*/

	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callback1) );
	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callback2) );
	CPPUNIT_ASSERT( m_pAgent->UnregisterForRunEvent(callback_run_count) );
}

TEST_DEFINITION( testSimpleCopy )
{
	loadProductions( "/Tests/testcopy.soar" );

/* Input structure for the test
(S1 ^io I1)
  (I1 ^input-link I3)
    (I3 ^sentence S2)
      (S2 ^newest yes ^num-words 3 ^sentence-num 1 ^word W1 ^word W2 ^word W3)
        (W1 ^num-word 1 ^word the)
        (W2 ^num-word 2 ^word cat)
        (W3 ^num-word 3 ^word in)
*/

	sml::Identifier* map = m_pAgent->GetInputLink() ;

	sml::Identifier* square2 = map->CreateIdWME("square");
	CPPUNIT_ASSERT( std::string( square2->GetAttribute() ) == "square" );

	sml::Identifier* square5 = map->CreateIdWME("square");
	CPPUNIT_ASSERT( std::string( square5->GetAttribute() ) == "square" );

	square2->CreateSharedIdWME("north", square5) ;

	square5->CreateSharedIdWME("south", square2) ;

	sml::Identifier* pSentence = m_pAgent->GetInputLink()->CreateIdWME("sentence") ;
	CPPUNIT_ASSERT( std::string( pSentence->GetAttribute() ) == "sentence" );

	pSentence->CreateStringWME("newest", "ye s") ;

	pSentence->CreateIntWME("num-words", 3) ;

	sml::Identifier* pWord1 = pSentence->CreateIdWME("word") ;
	CPPUNIT_ASSERT( std::string( pWord1->GetAttribute() ) == "word" );

	sml::Identifier* pWord5 = pSentence->CreateSharedIdWME("word2", pWord1) ;
	CPPUNIT_ASSERT( std::string( pWord5->GetAttribute() ) == "word2" );

	sml::Identifier* pWord2 = pSentence->CreateIdWME("word") ;
	CPPUNIT_ASSERT( std::string( pWord2->GetAttribute() ) == "word" );

	sml::Identifier* pWord3 = pSentence->CreateIdWME("word") ;
	CPPUNIT_ASSERT( std::string( pWord3->GetAttribute() ) == "word" );

	pWord1->CreateIntWME("num-word", 1) ;
	pWord2->CreateIntWME("num-word", 2) ;
	pWord3->CreateIntWME("num-word", 3) ;
	pWord1->CreateStringWME("word", "the") ;
	pWord2->CreateStringWME("word", "cat") ;
	pWord3->CreateStringWME("word", "in") ;
	m_pAgent->Commit() ;

	// Register for the trace output
	std::stringstream trace ;	// We'll pass this into the handler and build up the output in it
	/*int callbackp = */m_pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

	// Set to true for more detail on this
	m_pKernel->SetTraceCommunications(false) ;

	std::string result = m_pAgent->RunSelf(3) ;

	//cout << result << endl ;
	//cout << trace << endl ;

	// TODO: check this output
	//std::cout << m_pAgent->ExecuteCommandLine("print --depth 5 s1 --tree") << std::endl;

	// Test the iterator
	sml::Identifier* pOutputLink = m_pAgent->GetOutputLink();
	sml::Identifier::ChildrenIter iter = pOutputLink->GetChildrenBegin();
	sml::WMElement* pTextOutputWME = 0;
	while( iter != pOutputLink->GetChildrenEnd() )
	{
		// There should be only one child
		CPPUNIT_ASSERT( pTextOutputWME == 0 );
		pTextOutputWME = *iter;
		CPPUNIT_ASSERT( pTextOutputWME );
		CPPUNIT_ASSERT( std::string( pTextOutputWME->GetAttribute() ) == "text-output" );
		++iter;
	}

	CPPUNIT_ASSERT( pTextOutputWME->IsIdentifier() );
	sml::Identifier* pTextOutput = pTextOutputWME->ConvertToIdentifier();
	CPPUNIT_ASSERT( pTextOutput );
	sml::Identifier::ChildrenIter textOutputIter = pTextOutput->GetChildrenBegin();
	int count = 0;
	while( textOutputIter != pTextOutput->GetChildrenEnd() )
	{
		//sml::WMElement* wme2 = *iter2;
		//std::cout << wme2->GetAttribute();
		++count;
		++textOutputIter;
	}
	CPPUNIT_ASSERT( count == 6 );

	sml::WMElement* pNewestWME = pTextOutput->FindByAttribute( "newest", 0 );
	CPPUNIT_ASSERT( pNewestWME );
	sml::StringElement* pNewest = pNewestWME->ConvertToStringElement();
	CPPUNIT_ASSERT( pNewest );
	CPPUNIT_ASSERT( std::string( pNewest->GetValue() ) == "ye s" );
	
	int changes = m_pAgent->GetNumberOutputLinkChanges() ;

	//std::cout << m_pAgent->ExecuteCommandLine("print i3 -d 100 -i --tree");

	/*
	Output:

	(28: I3 ^text-output S4)
	  (14: S4 ^newest yes)
	  (15: S4 ^num-words 3)
	  (16: S4 ^word W1)       <-------- Shared ID?
		(8: W1 ^num-word 1)
		(9: W1 ^word the)
	  (17: S4 ^word W1)       <-------- Shared ID?
	  (18: S4 ^word W2)
		(10: W2 ^num-word 2)
		(11: W2 ^word cat)
	  (19: S4 ^word W3)
		(12: W3 ^num-word 3)
		(13: W3 ^word in)
	*/

	// TODO: verify output
	//for (int i = 0 ; i < changes ; i++)
	//{
	//	sml::WMElement* pOutputWme = m_pAgent->GetOutputLinkChange(i) ;
	//	std::cout << pOutputWme->GetIdentifier()->GetIdentifierSymbol() << " ^ " << pOutputWme->GetAttribute() << " " << pOutputWme->GetValueAsString() << std::endl ;
	//}

	// We had a bug where some of these wmes would get dropped (the orphaned wme scheme didn't handle multiple levels)
	// so check now that we got the correct number of changes.
	std::stringstream changesString;
	//changesString << "Number of changes: " << changes << ", this failure is currently expected but needs to be addressed, see wiki gSKI removal page";
	changesString << "Number of changes: " << changes;
	CPPUNIT_ASSERT_MESSAGE( changesString.str().c_str(), changes == 13 );
}

TEST_DEFINITION( testSimpleReteNetLoader )
{
	std::string path = std::string(m_pKernel->GetLibraryLocation()) + "/Tests/test.soarx" ;
	std::string command = std::string("rete-net -l \"") + path + "\"";  // RPM: wrap path in quotes in case it contains a space
	std::string result = m_pAgent->ExecuteCommandLine(command.c_str()) ;
	CPPUNIT_ASSERT( m_pAgent->GetLastCommandLineResult() );

	// Get the latest id from the input link
	sml::Identifier* pID = m_pAgent->GetInputLink() ;
	//cout << "Input link id is " << pID->GetValueAsString() << endl ;

	CPPUNIT_ASSERT( pID );
}

TEST_DEFINITION( testOSupportCopyDestroy )
{
	loadProductions( "/Tests/testOSupportCopyDestroy.soar" );

	sml::Identifier* pInputLink = m_pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );

	sml::Identifier* pFoo = pInputLink->CreateIdWME( "foo" );
	CPPUNIT_ASSERT( pFoo );

	sml::Identifier* pBar = pFoo->CreateIdWME( "bar" );
	CPPUNIT_ASSERT( pBar );

	sml::StringElement* pToy = pBar->CreateStringWME( "toy", "jig" );
	CPPUNIT_ASSERT( pToy );

	bool badCopyExists( false );
	m_pKernel->AddRhsFunction( "bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists ) ; 

	m_pAgent->RunSelf(1);

	pToy->DestroyWME(  );
	pBar->DestroyWME(  );
	pFoo->DestroyWME(  );

	m_pAgent->RunSelf(1);

	CPPUNIT_ASSERT( !badCopyExists );
}

TEST_DEFINITION( testOSupportCopyDestroyCircularParent )
{
	loadProductions( "/Tests/testOSupportCopyDestroy.soar" );

	sml::Identifier* pInputLink = m_pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );

	sml::Identifier* pFoo = pInputLink->CreateIdWME( "foo" );
	CPPUNIT_ASSERT( pFoo );

	sml::Identifier* pBar = pFoo->CreateIdWME( "bar" );
	CPPUNIT_ASSERT( pBar );

	sml::Identifier* pToy = pBar->CreateSharedIdWME( "toy", pFoo );
	CPPUNIT_ASSERT( pToy );

	bool badCopyExists( false );
	m_pKernel->AddRhsFunction( "bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists ) ; 

	m_pAgent->RunSelf(1);

	pFoo->DestroyWME(  );

	m_pAgent->RunSelf(1);

	CPPUNIT_ASSERT( !badCopyExists );
}

TEST_DEFINITION( testOSupportCopyDestroyCircular )
{
	loadProductions( "/Tests/testOSupportCopyDestroy.soar" );

	sml::Identifier* pInputLink = m_pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );

	sml::Identifier* pFoo = pInputLink->CreateIdWME( "foo" );
	CPPUNIT_ASSERT( pFoo );

	sml::Identifier* pBar = pFoo->CreateIdWME( "bar" );
	CPPUNIT_ASSERT( pBar );

	sml::Identifier* pToy = pBar->CreateSharedIdWME( "toy", pFoo );
	CPPUNIT_ASSERT( pToy );

	bool badCopyExists( false );
	m_pKernel->AddRhsFunction( "bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists ) ; 

	m_pAgent->RunSelf(1);

	pToy->DestroyWME(  );
	pBar->DestroyWME(  );
	pFoo->DestroyWME(  );

	m_pAgent->RunSelf(1);

	CPPUNIT_ASSERT( !badCopyExists );
}

TEST_DEFINITION( testSynchronize )
{
    //m_pKernel->SetTraceCommunications( true ) ;

	// input link
	sml::Identifier* pInputLink = m_pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );
	CPPUNIT_ASSERT( pInputLink->GetNumberChildren() == 0 );
	std::string name = pInputLink->GetIdentifierName();

	CPPUNIT_ASSERT( m_pAgent->SynchronizeInputLink() );

	pInputLink = m_pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );
	CPPUNIT_ASSERT( pInputLink->GetNumberChildren() == 0 );
	std::string newName = pInputLink->GetIdentifierName();

	CPPUNIT_ASSERT( name == newName );

	// output link
	CPPUNIT_ASSERT( !m_pAgent->GetOutputLink() );

	CPPUNIT_ASSERT( m_pAgent->SynchronizeOutputLink() );

	sml::Identifier* pOutputLink = m_pAgent->GetOutputLink();
	CPPUNIT_ASSERT( pOutputLink );
	CPPUNIT_ASSERT( pOutputLink->GetNumberChildren() == 0 );
	std::string olName = pOutputLink->GetIdentifierName();

	CPPUNIT_ASSERT( m_pAgent->SynchronizeOutputLink() );

	pOutputLink = m_pAgent->GetOutputLink();
	CPPUNIT_ASSERT( pOutputLink );
	CPPUNIT_ASSERT( pOutputLink->GetNumberChildren() == 0 );
	std::string olNewName = pOutputLink->GetIdentifierName();

	CPPUNIT_ASSERT( olName == olNewName );
}

TEST_DEFINITION( testRunningAgentCreation )
{
	// SEE BUG 952
	RunningAgentData data;
	data.count = 0;
	data.pOnTheFly = 0;

	m_pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, Handlers::MyAgentCreationUpdateEventHandler, &data );
	m_pKernel->RunAllAgents( 10 );

	//std::cout << std::endl;
	//std::cout << "count: " << data.count;
	// FIXME: in a perfect world, this is 10 not 12 but since the run isn't forever, the newly created agent runs 10.
	CPPUNIT_ASSERT( data.count == 12 );	
	CPPUNIT_ASSERT( data.pOnTheFly );
	
	sml::ClientAnalyzedXML response1;
	m_pAgent->ExecuteCommandLineXML("stats", &response1);
	sml::ClientAnalyzedXML response2;
	data.pOnTheFly->ExecuteCommandLineXML("stats", &response2);
	//std::cout << "original: " << response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1);
	//std::cout << " " << response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1);
	//std::cout << " " << response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountInnerElaboration, -1);
	//std::cout << " onthefly: " << response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1);
	//std::cout << " " << response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1);
	//std::cout << " " << response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountInnerElaboration, -1) << std::endl;
	CPPUNIT_ASSERT(response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 10);
	CPPUNIT_ASSERT(response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 10);
}

void FullTests::testShutdownHandlerShutdown()
{
	m_Options.reset();
	m_Options.set( REMOTE );

	// create kernel and agent
	createSoar();

	// most of the following taken from destroySoar(), simplified
	CPPUNIT_ASSERT( m_pKernel->DestroyAgent( m_pAgent ) );

	CPPUNIT_ASSERT( m_Options.test( REMOTE ) );

	soar_thread::Event shutdownEvent;
	m_pKernel->RegisterForSystemEvent( sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyShutdownTestShutdownHandler, &shutdownEvent ) ;

	std::string shutdownResponse = m_pKernel->SendClientMessage(0, "test-listener", "shutdown") ;
	CPPUNIT_ASSERT( shutdownResponse == "ok" );	

	CPPUNIT_ASSERT_MESSAGE( "Listener side kernel shutdown failed to fire smlEVENT_BEFORE_SHUTDOWN", shutdownEvent.WaitForEvent(5, 0) );

	cleanUpListener();
}

TEST_DEFINITION( testEventOrdering )
{
	int count = 0;

	m_pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyOrderingPrintHandler, &count ) ;
	m_pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_OUTPUT_PHASE, Handlers::MyOrderingRunHandler, &count ) ;

	m_pAgent->RunSelf(2);
}

TEST_DEFINITION( testStatusCompleteDuplication )
{
	// Load and test productions
	loadProductions( "/Tests/teststatuscomplete.soar" );

	// step
	m_pAgent->RunSelf(1);

	// add status complete
	int numberCommands = m_pAgent->GetNumberCommands() ;
	CPPUNIT_ASSERT( numberCommands == 1);

	// Get the first two commands (move and alternate)
	{
		sml::Identifier* pCommandBefore = m_pAgent->GetCommand(0) ;
		pCommandBefore->AddStatusComplete();
	}

	// commit status complete
	CPPUNIT_ASSERT( m_pAgent->Commit() );

	// step
	m_pAgent->RunSelf(1);

	// count status complete instances
	{
		sml::Identifier* pCommandAfter = m_pAgent->GetCommand(0);
		sml::Identifier::ChildrenIter child = pCommandAfter->GetChildrenBegin();
		sml::Identifier::ChildrenIter end = pCommandAfter->GetChildrenEnd();

		int count = 0;
		while ( child != end )
		{
			if ( (*child)->GetAttribute() == std::string("status") )
			{
				++count;
			}
			++child;
		}

		// there should only be one
		CPPUNIT_ASSERT( count == 1 );
	}
}

TEST_DEFINITION( testStopSoarVsInterrupt )
{
	loadProductions( "/Tests/teststopsoar.soar" );

	m_pAgent->ExecuteCommandLine("run -o 3");
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());

	// expected: agent stops after 15 decision cycles
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 15);
	}

	m_pAgent->InitSoar();
	m_pAgent->ExecuteCommandLine("run 3");
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());

	// expected: agent stops after 1 decision cycle
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
	}

	m_pAgent->ExecuteCommandLine("ex -a"); // side effect: init-soar
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());

	loadProductions( "/Tests/testinterrupt.soar" );

	m_pAgent->ExecuteCommandLine("run -o 3");
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());

	// expected: agent stops after 1 elaboration
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 0);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 1);
	}

	m_pAgent->InitSoar();
	m_pAgent->ExecuteCommandLine("run 3");
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());

	// expected: agent stops after 1 elaboration
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 0);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 1);
	}

}

TEST_DEFINITION( testSharedWmeSetViolation )
{
	//io.input-link.foo <f>
	sml::Identifier* pFoo1 = m_pAgent->GetInputLink()->CreateIdWME("foo") ;
	CPPUNIT_ASSERT(pFoo1);

	// TODO: This is illegal, but is probably too expensive to test for in release.
	// See bug 1060
	sml::Identifier* pFoo2 = m_pAgent->GetInputLink()->CreateSharedIdWME("foo", pFoo1) ;
	CPPUNIT_ASSERT_MESSAGE("CreateSharedIdWME was able to create duplicate wme", pFoo2 == 0);

	CPPUNIT_ASSERT(m_pAgent->Commit());
}

TEST_DEFINITION( testEchoEquals )
{
	sml::ClientAnalyzedXML response;
	CPPUNIT_ASSERT(m_pAgent->ExecuteCommandLineXML("echo =", &response));
}

TEST_DEFINITION( testFindAttrPipes )
{
	sml::ClientAnalyzedXML response;
	CPPUNIT_ASSERT(m_pAgent->ExecuteCommandLineXML("add-wme I2 ^A a", &response));
	CPPUNIT_ASSERT(m_pAgent->SynchronizeInputLink());
	sml::WMElement* pWME = m_pAgent->GetInputLink()->FindByAttribute("A", 0);
	CPPUNIT_ASSERT(pWME);

	m_pAgent->GetInputLink()->CreateStringWME("B", "b") ;
	CPPUNIT_ASSERT(m_pAgent->Commit());
	pWME = 0;
	pWME = m_pAgent->GetInputLink()->FindByAttribute("B", 0);
	CPPUNIT_ASSERT(pWME);
}

TEST_DEFINITION( testTemplateVariableNameBug )
{
	loadProductions( "/Tests/test1121.soar" );
	m_pAgent->ExecuteCommandLine("run");
	sml::ClientAnalyzedXML response;
	m_pAgent->ExecuteCommandLineXML("stats", &response);
	CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
	CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 4);

}

TEST_DEFINITION( testNegatedConjunctiveChunkLoopBug510 )
{
	loadProductions( "/Tests/testNegatedConjunctiveChunkLoopBug510.soar" );
	m_pAgent->ExecuteCommandLine("run");
	sml::ClientAnalyzedXML response;
	m_pAgent->ExecuteCommandLineXML("stats", &response);
	CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
	CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
}

TEST_DEFINITION( testGDSBug1144 )
{
	loadProductions( "/Tests/testGDSBug1144.soar" );
	m_pAgent->ExecuteCommandLine("run");
}

TEST_DEFINITION( testGDSBug1011 )
{
	loadProductions( "/Tests/testGDSBug1011.soar" );
	m_pAgent->ExecuteCommandLine("run");
	sml::ClientAnalyzedXML response;
	m_pAgent->ExecuteCommandLineXML("stats", &response);
	CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 8);
	CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 19);

}

TEST_DEFINITION( testLearn )
{
	loadProductions( "/Tests/testLearn.soar" );
	m_pAgent->ExecuteCommandLine("learn --except");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// learning is off, same behavior expected
	m_pAgent->ExecuteCommandLine("init");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// turn learn except on
	m_pAgent->ExecuteCommandLine("init");
	m_pAgent->ExecuteCommandLine("learn --except");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// don't learn is active so same result expected
	m_pAgent->ExecuteCommandLine("init");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// get rid of dont learn
	m_pAgent->ExecuteCommandLine("init");
	m_pAgent->ExecuteCommandLine("excise dont*learn");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// expect improvement
	m_pAgent->ExecuteCommandLine("init");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
	}

	// go to only mode
	m_pAgent->ExecuteCommandLine("init");
	m_pAgent->ExecuteCommandLine("excise -c");
	m_pAgent->ExecuteCommandLine("learn --only");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// force learn is active, expect improvement
	m_pAgent->ExecuteCommandLine("init");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
	}

	// get rid of chunk and force learn
	m_pAgent->ExecuteCommandLine("init");
	m_pAgent->ExecuteCommandLine("excise -c");
	m_pAgent->ExecuteCommandLine("excise force*learn");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}

	// expect no improvement
	m_pAgent->ExecuteCommandLine("init");
	m_pKernel->RunAllAgentsForever();
	{
		sml::ClientAnalyzedXML response;
		m_pAgent->ExecuteCommandLineXML("stats", &response);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
		CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
	}
}

TEST_DEFINITION( testPreferenceSemantics )
{
	m_pKernel->AddRhsFunction( "test-failure", Handlers::MyRhsFunctionFailureHandler, 0 ) ; 
	loadProductions( "/Tests/pref-semantics-test.soar" );
	m_pAgent->ExecuteCommandLine("run");
}

TEST_DEFINITION( testMatchTimeInterrupt )
{
	m_pKernel->AddRhsFunction( "test-failure", Handlers::MyRhsFunctionFailureHandler, 0 ) ; 
	loadProductions( "/Tests/testMatchTimeInterrupt.soar" );
	m_pAgent->ExecuteCommandLine("run");
}
TEST_DEFINITION( testNegatedConjunctiveTestReorder )
{
	m_pAgent->ExecuteCommandLine("sp {test (state <s> ^a <val> -^a {<val> < 1}) --> }");
	std::string production(m_pAgent->ExecuteCommandLine("print test"));
	CPPUNIT_ASSERT(production == "sp {test\n    (state <s> ^a <val> -^a { <val> < 1 })\n    -->\n    \n}\n\n\n");
}

TEST_DEFINITION( testNegatedConjunctiveTestUnbound )
{
	// all of these should fail without crashing:
	m_pAgent->ExecuteCommandLine("sp {test (state <s> ^superstate nil -^foo { <> <bad> }) --> }");
	// <bad> is unbound referent in value test
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult() == false);

	m_pAgent->ExecuteCommandLine("sp {test (state <s> ^superstate nil -^{ <> <bad> } <s>) --> }");
	// <bad> is unbound referent in attr test
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult() == false);

	m_pAgent->ExecuteCommandLine("sp {test (state <s> ^superstate nil -^foo { <> <b> }) -{(<s> ^bar <b>) (<s> -^bar { <> <b>})} --> }");
	// <b> is unbound referent in test, defined in ncc out of scope
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult() == false);

	m_pAgent->ExecuteCommandLine("sp {test  (state <s> ^superstate <d> -^foo { <> <b> }) -{(<s> ^bar <b>) (<s> -^bar { <> <d>})} --> }");
	// <d> is unbound referent in value test in ncc
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult() == false);

	// these should succeed
	m_pAgent->ExecuteCommandLine("sp {test (state <s> ^superstate <d>) -{(<s> ^bar <b>) (<s> -^bar { <> <d>})} --> }");
	// <d> is bound outside of ncc but in scope
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());

	m_pAgent->ExecuteCommandLine("sp {test (state <s> ^superstate nil) -{(<s> ^bar <d>) (<s> -^bar { <> <d>})} --> }");
	// <d> is bound inside of ncc
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());
}

TEST_DEFINITION( testCommandToFile )
{
	loadProductions( "/Demos/water-jug/water-jug-rl.soar" );
	m_pKernel->RunAllAgentsForever();
	m_pAgent->ExecuteCommandLine("command-to-file testCommandToFile-output.soar print --rl --full");
	CPPUNIT_ASSERT(m_pAgent->GetLastCommandLineResult());
	const char* result = m_pAgent->ExecuteCommandLine( "source testCommandToFile-output.soar" );
	CPPUNIT_ASSERT(result);
	const std::string resultString("#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*\nTotal: 144 productions sourced. 144 productions excised.\nSource finished.\n");
	CPPUNIT_ASSERT(result == resultString);
	remove("testCommandToFile-output.soar");
}
