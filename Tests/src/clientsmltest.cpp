#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>
#include <sstream>
#include <bitset>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "thread_Event.h"
#include "kernel.h"
#include "soarversion.h"

#include "handlers.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif // !_WIN32

enum eKernelOptions
{
	NONE,
	EMBEDDED,
	USE_CLIENT_THREAD,
	FULLY_OPTIMIZED,
	AUTO_COMMIT_ENABLED,
	NUM_KERNEL_OPTIONS,
};

typedef std::bitset<NUM_KERNEL_OPTIONS> KernelBitset;

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
	CPPUNIT_TEST( testSimpleCopy );
	CPPUNIT_TEST( testSimpleReteNetLoader );
	CPPUNIT_TEST( testSimpleStopUpdate );
	CPPUNIT_TEST( testSimpleSNCBreak );
	CPPUNIT_TEST( testWMEMemoryLeakDestroyChildren );	// see bugzilla bug 1034
	CPPUNIT_TEST( testWMEMemoryLeak );					// see bugzilla bug 1035
	CPPUNIT_TEST( testWMEMemoryLeakNotOptimized );		// see bugzilla bug 1035
	CPPUNIT_TEST( testWMEMemoryLeakNoAutoCommit );		// see bugzilla bug 1035
	CPPUNIT_TEST( testWMEMemoryLeakRemote );			// see bugzilla bug 1035
	CPPUNIT_TEST( testNonAlphaAttrs );
	CPPUNIT_TEST( testOSupportCopyDestroy );			// see bugzilla bug 515
	CPPUNIT_TEST( testOSupportCopyDestroyCircular );	// see bugzilla bug 515
	CPPUNIT_TEST( testOSupportCopyDestroyCircularParent );	// see bugzilla bug 515
	CPPUNIT_TEST( testEmbeddedDirectOutputLinkExists );	// see bugzilla bug 1025
	CPPUNIT_TEST( testEmbeddedOutputLinkExists );		// see bugzilla bug 1025

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

protected:

	void testWMEMemoryLeakDestroyChildren();
	void testWMEMemoryLeak();
	void testWMEMemoryLeakNotOptimized();
	void testWMEMemoryLeakNoAutoCommit();
	void testWMEMemoryLeakRemote();
	void testEmbeddedDirectInit();
	void testEmbeddedDirect();
	void testEmbedded();
	void testNewThread();
	void testNewThreadNoAutoCommit();
	void testRemote();
	void testRemoteNoAutoCommit();
	void testSimpleCopy();
	void testSimpleReteNetLoader();
	void testSimpleStopUpdate();
	void testSimpleSNCBreak();
	void testNonAlphaAttrs();
	void testOSupportCopyDestroy();
	void testOSupportCopyDestroyCircularParent();
	void testOSupportCopyDestroyCircular();
	void testEmbeddedDirectOutputLinkExists();
	void testEmbeddedOutputLinkExists();

private:
	void createKernelAndAgents( const KernelBitset& options, int port = 12121 );

	void doFullTest();

	void testWMEMemoryLeakInternal( sml::UpdateEventHandler handler, const KernelBitset& options );

	std::string getAgentName( int agentIndex );
	void initAgent( sml::Agent* pAgent );
	void loadProductions( sml::Agent* pAgent, const std::string& productions );
	void checkProductions( sml::Agent* pAgent, const std::string& productions );
	void doRHSHandlerTest( sml::Agent* pAgent );
	void doClientMessageHandlerTest( sml::Agent* pAgent );
	void doFilterHandlerTest( sml::Agent* pAgent );
	void doWMETests( sml::Agent* pAgent );
	void doPatternTest( sml::Agent* pAgent );
	void doAliasTest( sml::Agent* pAgent );
	void doXMLTest( sml::Agent* pAgent );
	void doAgentTest( sml::Agent* pAgent );

	void doSimpleCopy();
	void doSimpleReteNetLoader();

	void spawnListener();
	void cleanUpListener();

	static const std::string kBaseName;
	static sml::ClientXML* clientXMLStorage;
	static bool verbose;

	sml::Kernel* pKernel;
	int numberAgents; // This number determines how many agents are created.  We create <n>, test <n> and then delete <n>
	bool remote;

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
#else // _WIN32
    pid_t pid;
#endif // _WIN32

};

CPPUNIT_TEST_SUITE_REGISTRATION( ClientSMLTest ); 

const std::string ClientSMLTest::kBaseName( "test-client-sml" );
sml::ClientXML* ClientSMLTest::clientXMLStorage = 0;
bool ClientSMLTest::verbose = false;

void ClientSMLTest::setUp()
{
	assert( MAJOR_VERSION_NUMBER == SML_MAJOR_VERSION_NUMBER );
	assert( MINOR_VERSION_NUMBER == SML_MINOR_VERSION_NUMBER );
	assert( MICRO_VERSION_NUMBER == SML_MICRO_VERSION_NUMBER );
	assert( GREEK_VERSION_NUMBER == SML_GREEK_VERSION_NUMBER );
	assert( strcmp( VERSION_STRING, SML_VERSION_STRING ) == 0 );

	pKernel = NULL;
	numberAgents = 1;
	clientXMLStorage = NULL;
	remote = false;

	// kernel initialized in test
}

void ClientSMLTest::tearDown()
{
	if ( !pKernel )
	{
		return;
	}

	// Agent deletion
	if ( verbose ) std::cout << "Destroy the agent now" << std::endl ;
	for (int agentDeletions = 0 ; agentDeletions < numberAgents ; agentDeletions++)
	{
		sml::Agent* pAgent = pKernel->GetAgent( getAgentName( agentDeletions ).c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		// The Before_Agent_Destroyed callback is a tricky one so we'll register for it to test it.
		// We need to get this callback just before the agentSML data is deleted (otherwise there'll be no way to send/receive the callback)
		// and then continue on to delete the agent after we've responded to the callback.
		// Interestingly, we don't explicitly unregister this callback because the agent has already been destroyed so
		// that's another test, that this callback is cleaned up correctly (and automatically).
		bool deletionHandlerReceived( false );
		pKernel->RegisterForAgentEvent( sml::smlEVENT_BEFORE_AGENT_DESTROYED, Handlers::MyDeletionHandler, &deletionHandlerReceived ) ;

		// Explicitly destroy our agent as a test, before we delete the kernel itself.
		// (Actually, if this is a remote connection we need to do this or the agent
		//  will remain alive).
		CPPUNIT_ASSERT( pKernel->DestroyAgent( pAgent ) );
		CPPUNIT_ASSERT( deletionHandlerReceived );
		deletionHandlerReceived = false;
	}

	if ( verbose ) std::cout << "Calling shutdown on the kernel now" << std::endl ;

	if ( remote )
	{
		soar_thread::Event shutdownEvent;
		pKernel->RegisterForSystemEvent( sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyEventShutdownHandler, &shutdownEvent ) ;

		// BUGBUG
		// ClientSML thread dies inelegantly here spewing forth error messages
		// about sockets/pipes not being shut down correctly.
		std::string shutdownResponse = pKernel->SendClientMessage(0, "test-listener", "shutdown") ;
		CPPUNIT_ASSERT( shutdownResponse == "ok" );	

		CPPUNIT_ASSERT_MESSAGE( "Listener side kernel shutdown failed to fire smlEVENT_BEFORE_SHUTDOWN", shutdownEvent.WaitForEvent(5, 0) );

		// Note, in the remote case, this does not fire smlEVENT_BEFORE_SHUTDOWN
		// the listener side shutdown does trigger the event when it is deleted, see simplelistener.cpp
		pKernel->Shutdown() ;
		
	} else {
		bool shutdownHandlerReceived( false );
		pKernel->RegisterForSystemEvent( sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyBoolShutdownHandler, &shutdownHandlerReceived ) ;
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
		cleanUpListener();
		if ( verbose ) std::cout << "Cleaned up listener." << std::endl;
	}
}

void ClientSMLTest::doFullTest()
{
	for ( int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter )
	{
		sml::Agent* pAgent = pKernel->GetAgent( getAgentName( agentCounter ).c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		loadProductions( pAgent, "/Tests/testsml.soar" );
		checkProductions( pAgent, "/Tests/testsml.soar" );
		doRHSHandlerTest( pAgent );
		initAgent( pAgent );
		doClientMessageHandlerTest( pAgent );
		doFilterHandlerTest( pAgent );
		initAgent( pAgent );
		doWMETests( pAgent );
		initAgent( pAgent );
		doPatternTest( pAgent );
		doAliasTest( pAgent ) ;
		doXMLTest( pAgent ) ;
		doAgentTest( pAgent ) ;
	}
}

void ClientSMLTest::testEmbeddedDirectInit()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	for ( int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter )
	{
		sml::Agent* pAgent = pKernel->GetAgent( getAgentName( agentCounter ).c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		initAgent( pAgent );
	}
}

void ClientSMLTest::testEmbeddedDirect()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	doFullTest();
}

void ClientSMLTest::testEmbedded()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	doFullTest();
}

void ClientSMLTest::testEmbeddedDirectOutputLinkExists()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	for ( int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter )
	{
		sml::Agent* pAgent = pKernel->GetAgent( getAgentName( agentCounter ).c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		CPPUNIT_ASSERT( pAgent->SynchronizeOutputLink() );
		CPPUNIT_ASSERT( pAgent->GetOutputLink() );
	}	
}

void ClientSMLTest::testEmbeddedOutputLinkExists()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	for ( int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter )
	{
		sml::Agent* pAgent = pKernel->GetAgent( getAgentName( agentCounter ).c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		CPPUNIT_ASSERT( pAgent->SynchronizeOutputLink() );
		CPPUNIT_ASSERT( pAgent->GetOutputLink() );
	}	
}

void ClientSMLTest::testNewThread()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	doFullTest();
}

void ClientSMLTest::testNewThreadNoAutoCommit()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	createKernelAndAgents( options );

	doFullTest();
}

void ClientSMLTest::testRemote()
{
	remote = true;
	spawnListener();

	if ( verbose )
	{
		std::cout << "Spawned listener." << std::endl;
	}

	numberAgents = 1;
	KernelBitset options(0);
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	doFullTest();

	if ( verbose )
	{
		std::cout << "Test complete." << std::endl;
	}
}

void ClientSMLTest::testRemoteNoAutoCommit()
{
	remote = true;
	spawnListener();

	if ( verbose )
	{
		std::cout << "Spawned listener." << std::endl;
	}

	numberAgents = 1;
	KernelBitset options(0);
	createKernelAndAgents( options );

	doFullTest();

	if ( verbose )
	{
		std::cout << "Test complete." << std::endl;
	}
}

void ClientSMLTest::testWMEMemoryLeakDestroyChildren()
{
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	options.set( USE_CLIENT_THREAD );
	testWMEMemoryLeakInternal( Handlers::MyMemoryLeakUpdateHandlerDestroyChildren, options );
}

void ClientSMLTest::testWMEMemoryLeak()
{
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	options.set( USE_CLIENT_THREAD );
	testWMEMemoryLeakInternal( Handlers::MyMemoryLeakUpdateHandler, options );
}

void ClientSMLTest::testWMEMemoryLeakNotOptimized()
{
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( AUTO_COMMIT_ENABLED );
	options.set( USE_CLIENT_THREAD );
	testWMEMemoryLeakInternal( Handlers::MyMemoryLeakUpdateHandler, options );
}

void ClientSMLTest::testWMEMemoryLeakNoAutoCommit()
{
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( FULLY_OPTIMIZED );
	options.set( USE_CLIENT_THREAD );
	testWMEMemoryLeakInternal( Handlers::MyMemoryLeakUpdateHandler, options );
}

void ClientSMLTest::testWMEMemoryLeakRemote()
{
	remote = true;
	spawnListener();

	KernelBitset options(0);
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	options.set( USE_CLIENT_THREAD );
	testWMEMemoryLeakInternal( Handlers::MyMemoryLeakUpdateHandler, options );
}

void ClientSMLTest::testWMEMemoryLeakInternal( sml::UpdateEventHandler handler, const KernelBitset& options )
{
	// see bugzilla bug 1034, 1035

	numberAgents = 1;
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, handler, pAgent ) ;

	pAgent->ExecuteCommandLine("waitsnc --on");
	
#ifdef _WIN32
	_CrtMemState memState;
	_CrtMemCheckpoint( &memState );
	//_CrtSetBreakAlloc( 2975 );
#endif

	pAgent->RunSelf(3); // 3 cycles to allow one additional clean-up phase after identifier destruction
	//pAgent->RunSelf(6000); 
	//pAgent->RunSelfForever();

#ifdef WIN32
	_CrtMemDumpAllObjectsSince( &memState );
#endif
}

void ClientSMLTest::testSimpleCopy()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	doSimpleCopy();
}

void ClientSMLTest::testNonAlphaAttrs()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	sml::Identifier* il = pAgent->GetInputLink() ;
	sml::Identifier* one = pAgent->CreateIdWME(il, "1");
	sml::Identifier* two = pAgent->CreateSharedIdWME(il, "2", one) ;
	sml::StringElement* three = pAgent->CreateStringWME(il, "3", "4") ;
	sml::IntElement* four = pAgent->CreateIntWME(il, "5", 6) ;
	sml::FloatElement* five = pAgent->CreateFloatWME(il, "7", 8.0) ;
	pAgent->Commit() ;

	std::string result = pAgent->RunSelf(3) ;

	// TODO: Test that things are there
}

void ClientSMLTest::testSimpleReteNetLoader()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	doSimpleReteNetLoader();
}

void ClientSMLTest::testSimpleStopUpdate()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	int callback_u = pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, Handlers::MyCallStopOnUpdateEventHandler, 0 ) ;

	pAgent->RunSelf(4) ;
	CPPUNIT_ASSERT_MESSAGE( "RunSelf", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT_MESSAGE( "Should be 1", pAgent->GetDecisionCycleCounter() == 1);
}

void ClientSMLTest::testSimpleSNCBreak()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	pAgent->RunSelfForever();
	CPPUNIT_ASSERT_MESSAGE( "RunSelfForever", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT_MESSAGE( "Should be 100", pAgent->GetDecisionCycleCounter() == 100);
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
#else // _WIN32
	pid = fork();
	CPPUNIT_ASSERT_MESSAGE( "fork error", pid >= 0 );
	if ( pid == 0 )
	{
		// child
		execl("Tests", "Tests", "--listener", static_cast< char* >( 0 ));
		// does not return on success
		CPPUNIT_ASSERT_MESSAGE( "execl failed", false );
	}
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
#else // _WIN32
	int status( 0 );
	wait( &status );
	CPPUNIT_ASSERT_MESSAGE( "listener didn't terminate properly", WIFEXITED( status ) );
	CPPUNIT_ASSERT_MESSAGE( "listener terminated with nonzero status", WEXITSTATUS( status ) == 0 );
#endif // _WIN32
}

void ClientSMLTest::createKernelAndAgents( const KernelBitset& options, int port )
{
	CPPUNIT_ASSERT( pKernel == NULL );

	if ( options.test( EMBEDDED ) )
	{
		CPPUNIT_ASSERT( !remote );
		if ( options.test( USE_CLIENT_THREAD ) )
		{
			pKernel = sml::Kernel::CreateKernelInCurrentThread( sml::Kernel::GetDefaultLibraryName(), options.test( FULLY_OPTIMIZED ), sml::Kernel::GetDefaultPort());
		}
		else
		{
			pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML", sml::Kernel::GetDefaultPort());
		}
	}
	else
	{
		CPPUNIT_ASSERT( remote );
		pKernel = sml::Kernel::CreateRemoteConnection(true, 0, port);
	}

	CPPUNIT_ASSERT( pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	pKernel->SetAutoCommit( options.test( AUTO_COMMIT_ENABLED ) ) ;

	// Set this to true to give us lots of extra debug information on remote clients
	// (useful in a test app like this).
    // pKernel->SetTraceCommunications(true) ;

	if ( verbose ) std::cout << "Soar kernel version " << pKernel->GetSoarKernelVersion() << std::endl ;
	if ( verbose ) std::cout << "SML version " << sml::sml_Names::kSMLVersionValue << std::endl ;

	CPPUNIT_ASSERT( std::string( pKernel->GetSoarKernelVersion() ) == std::string( sml::sml_Names::kSoarVersionValue ) );

	bool creationHandlerReceived( false );
	pKernel->RegisterForAgentEvent( sml::smlEVENT_AFTER_AGENT_CREATED, Handlers::MyCreationHandler, &creationHandlerReceived ) ;
	
	// Report the number of agents (always 0 unless this is a remote connection to a CLI or some such)
	CPPUNIT_ASSERT( pKernel->GetNumberAgents() == 0 );

	for ( int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter )
	{
		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		sml::Agent* pAgent = pKernel->CreateAgent( getAgentName( agentCounter ).c_str() ) ;
		CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );
		CPPUNIT_ASSERT( pAgent != NULL );
		CPPUNIT_ASSERT( creationHandlerReceived );
		creationHandlerReceived = false;

		// a number of tests below depend on running full decision cycles.
		pAgent->ExecuteCommandLine( "set-stop-phase --before --input" ) ;
		CPPUNIT_ASSERT_MESSAGE( "set-stop-phase --before --input", pAgent->GetLastCommandLineResult() );
	}

	CPPUNIT_ASSERT( pKernel->GetNumberAgents() == numberAgents );
}

void ClientSMLTest::initAgent( sml::Agent* pAgent )
{
	if ( verbose )
	{
		std::cout << "Performing simple init-soar..." << std::endl;
	}
	pAgent->InitSoar() ;
	CPPUNIT_ASSERT_MESSAGE( "init-soar", pAgent->GetLastCommandLineResult() );
}

void ClientSMLTest::loadProductions( sml::Agent* pAgent, const std::string& productions )
{
	// TODO boost filesystem
	std::stringstream path;
	path << pKernel->GetLibraryLocation() << productions ;

	pAgent->LoadProductions( path.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "LoadProductions", pAgent->GetLastCommandLineResult() );

	if ( verbose )
	{
		std::cout << "Loaded productions" << std::endl ;
	}
}

void ClientSMLTest::checkProductions( sml::Agent* pAgent, const std::string& productions )
{
	// TODO boost filesystem
	std::stringstream path;
	path << pKernel->GetLibraryLocation() << productions ;

	CPPUNIT_ASSERT( pAgent->IsProductionLoaded( "apply*move" ) );
	CPPUNIT_ASSERT( !pAgent->IsProductionLoaded( "made*up*name" ) );

	int excisedCount( 0 );
	int prodCall = pAgent->RegisterForProductionEvent( sml::smlEVENT_BEFORE_PRODUCTION_REMOVED, Handlers::MyProductionHandler, &excisedCount ) ;

	pAgent->ExecuteCommandLine( "excise --all" ) ;
	CPPUNIT_ASSERT_MESSAGE( "excise --all", pAgent->GetLastCommandLineResult() );
	CPPUNIT_ASSERT( excisedCount > 0 );

	pAgent->LoadProductions( path.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "LoadProductions", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( pAgent->UnregisterForProductionEvent( prodCall ) );
}

std::string ClientSMLTest::getAgentName( int agentIndex ) 
{
	std::stringstream name;
	name << kBaseName << 1 + agentIndex;
	return name.str();
}

void ClientSMLTest::doRHSHandlerTest( sml::Agent* pAgent )
{
	bool rhsFunctionHandlerReceived( false );

	// Record a RHS function
	int callback_rhs1 = pKernel->AddRhsFunction( "test-rhs", Handlers::MyRhsFunctionHandler, &rhsFunctionHandlerReceived ) ; 
	int callback_rhs_dup = pKernel->AddRhsFunction( "test-rhs", Handlers::MyRhsFunctionHandler, &rhsFunctionHandlerReceived ) ;

	CPPUNIT_ASSERT_MESSAGE( "Duplicate RHS function registration should be detected and be ignored", callback_rhs_dup == callback_rhs1 );

	// need this to fire production that calls test-rhs
	sml::Identifier* pSquare = pAgent->CreateIdWME(pAgent->GetInputLink(), "square") ;
	CPPUNIT_ASSERT( pSquare );
	sml::StringElement* pEmpty = pAgent->CreateStringWME(pSquare, "content", "EMPTY") ;
	CPPUNIT_ASSERT( pEmpty );
	sml::IntElement* pRow = pAgent->CreateIntWME(pSquare, "row", 1) ;
	CPPUNIT_ASSERT( pRow );
	sml::IntElement* pCol = pAgent->CreateIntWME(pSquare, "col", 2) ;
	CPPUNIT_ASSERT( pCol );
	CPPUNIT_ASSERT( pAgent->Commit() );

	pKernel->RunAllAgents( 1 ) ;
	CPPUNIT_ASSERT_MESSAGE( "RunAllAgents", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( rhsFunctionHandlerReceived );

	CPPUNIT_ASSERT( pKernel->RemoveRhsFunction(callback_rhs1) );

	// Re-add it without the bool that is getting popped off the stack
	CPPUNIT_ASSERT( pKernel->AddRhsFunction( "test-rhs", Handlers::MyRhsFunctionHandler, 0 ) ); 

	CPPUNIT_ASSERT( pAgent->DestroyWME( pSquare ) );
	CPPUNIT_ASSERT( pAgent->Commit() );
}

void ClientSMLTest::doClientMessageHandlerTest( sml::Agent* pAgent )
{
	// Record a client message handler
	bool clientHandlerReceived( false );
	int clientCallback = pKernel->RegisterForClientMessageEvent( "test-client", Handlers::MyClientMessageHandler, &clientHandlerReceived ) ;

	// This is a bit dopey--but we'll send a message to ourselves for this test
	std::string response = pKernel->SendClientMessage(pAgent, "test-client", "test-message") ;
	CPPUNIT_ASSERT( clientHandlerReceived );
	clientHandlerReceived = false;
	CPPUNIT_ASSERT( response == "handler-messagetest-message" );

	CPPUNIT_ASSERT( pKernel->UnregisterForClientMessageEvent( clientCallback ) );
}

void ClientSMLTest::doFilterHandlerTest( sml::Agent* pAgent )
{
	// Record a filter
	bool filterHandlerReceived( false );
	int clientFilter = pKernel->RegisterForClientMessageEvent( sml::sml_Names::kFilterName, Handlers::MyFilterHandler, &filterHandlerReceived ) ;

	// Our filter adds "--depth 2" to all commands
	// so this should give us the result of "print s1 --depth 2"
	std::string output = pAgent->ExecuteCommandLine("print s1") ;
	CPPUNIT_ASSERT_MESSAGE( "print s1", pAgent->GetLastCommandLineResult() );

	CPPUNIT_ASSERT( filterHandlerReceived );
	filterHandlerReceived = false;

	// depth 2 should reveal I2
	CPPUNIT_ASSERT( output.find( "I2" ) != std::string::npos );

	// This is important -- if we don't unregister all subsequent commands will
	// come to our filter and promptly fail!
	CPPUNIT_ASSERT( pKernel->UnregisterForClientMessageEvent( clientFilter ) );
}

void ClientSMLTest::doWMETests( sml::Agent* pAgent )
{
	sml::Identifier* pInputLink = pAgent->GetInputLink() ;
	CPPUNIT_ASSERT( pInputLink );

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

	initAgent( pAgent );

	CPPUNIT_ASSERT( pAgent->CreateStringWME(pID, "type", "Boeing747") );

	sml::IntElement* pWME2    = pAgent->CreateIntWME(pID, "speed", 200) ;
	CPPUNIT_ASSERT( pWME2 );

	sml::FloatElement* pWME3  = pAgent->CreateFloatWME(pID, "direction", 50.5) ;
	CPPUNIT_ASSERT( pWME3 );

	CPPUNIT_ASSERT( pAgent->Commit() );

	initAgent( pAgent );
	
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
}

void ClientSMLTest::doPatternTest( sml::Agent* pAgent )
{
	// Throw in a pattern as a test
	std::string pattern = pAgent->ExecuteCommandLine( "print -i (s1 ^* *)" ) ;
	CPPUNIT_ASSERT_MESSAGE( "print -i (s1 ^* *)", pAgent->GetLastCommandLineResult() );
}

void ClientSMLTest::doAliasTest( sml::Agent* pAgent )
{
	CPPUNIT_ASSERT( pKernel->ExpandCommandLine( "p s1" ) );	// test for null first
	CPPUNIT_ASSERT( std::string( pKernel->ExpandCommandLine( "p s1" ) ) == "print s1" );

	CPPUNIT_ASSERT( pKernel->IsRunCommand( "d 3" ) );
	CPPUNIT_ASSERT( pKernel->IsRunCommand( "e 5" ) );
	CPPUNIT_ASSERT( pKernel->IsRunCommand( "run -d 10" ) );
}

void ClientSMLTest::doXMLTest( sml::Agent* pAgent )
{
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
		if ( verbose )
		{
			std::cout << wmeString << std::endl ;
		}
		wmeChild.DeleteString( wmeString ) ;
	}
	xml2.DeleteString(xmlString) ;
}

void ClientSMLTest::doAgentTest( sml::Agent* pAgent )
{
	// Test that we get a callback after the decision cycle runs
	// We'll pass in an "int" and use it to count decisions (just as an example of passing user data around)
	int count( 0 );
	int callback1 = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &count );
	int callback_dup = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &count );

	CPPUNIT_ASSERT_MESSAGE( "Duplicate handler registration should be detected and be ignored", callback1 == callback_dup );

	// This callback unregisters itself in the callback -- as a test to see if we can do that safely.
	int selfRemovingCallback( -1 );
	selfRemovingCallback = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunSelfRemovingHandler, &selfRemovingCallback ) ;

	// Register for a String event
	bool stringEventHandlerReceived( false );
	int stringCall = pKernel->RegisterForStringEvent( sml::smlEVENT_EDIT_PRODUCTION, Handlers::MyStringEventHandler, &stringEventHandlerReceived ) ;
	CPPUNIT_ASSERT( pKernel->ExecuteCommandLine( "edit-production my*production", NULL ) );
	CPPUNIT_ASSERT_MESSAGE( "edit-production my*production", pAgent->GetLastCommandLineResult() );
	CPPUNIT_ASSERT( stringEventHandlerReceived );
	stringEventHandlerReceived = false;
	CPPUNIT_ASSERT( pKernel->UnregisterForStringEvent( stringCall ) );

	// Register another handler for the same event, to make sure we can do that.
	// Register this one ahead of the previous handler (so it will fire before MyRunEventHandler)
	bool addToBack = true ;
	int testData( 25 ) ;
	int callback2 = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyDuplicateRunEventHandler, &testData, !addToBack) ;

	// Run returns the result (succeeded, failed etc.)
	// To catch the trace output we have to register a print event listener
	std::stringstream trace ;	// We'll pass this into the handler and build up the output in it
	std::string structured ;	// Structured trace goes here
	int callbackp = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

	int callbackx = pAgent->RegisterForXMLEvent( sml::smlEVENT_XML_TRACE_OUTPUT, Handlers::MyXMLEventHandler, &clientXMLStorage ) ;

	int beforeCount( 0 );
	int afterCount( 0 );
	int callback_before = pAgent->RegisterForRunEvent( sml::smlEVENT_BEFORE_RUN_STARTS, Handlers::MyRunEventHandler, &beforeCount ) ;
	int callback_after = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_RUN_ENDS, Handlers::MyRunEventHandler, &afterCount ) ;

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
	int callback_u = pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, Handlers::MyUpdateEventHandler, &outputPhases ) ;

	int phaseCount( 0 );
	int callbackPhase = pAgent->RegisterForRunEvent( sml::smlEVENT_BEFORE_PHASE_EXECUTED, Handlers::MyRunEventHandler, &phaseCount ) ;

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
	if ( verbose )
	{
		std::cout << trace.str() << std::endl ;
	}
	trace.clear();
	if ( verbose )
	{
		std::cout << structured << std::endl ;
	}

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

	// TODO: SynchronizeInputLink is only valid during certain kinds of connections
	//if (synch)
	//{
		////cout << "Results of synchronizing the input link:" << endl ;
		////printWMEs(pAgent->GetInputLink()) ;
		////cout << endl ;

		//CPPUNIT_ASSERT( pAgent->GetInputLink()->GetNumberChildren() != 0 );
	//}

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
	int callback_run_count = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &myCount) ;

	int outputsGenerated( 0 ) ;
	int callback_g = pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Handlers::MyUpdateEventHandler, &outputsGenerated) ;

	int outputNotifications( 0 ) ;
	int callback_notify = pAgent->RegisterForOutputNotification( Handlers::MyOutputNotificationHandler, &outputNotifications) ;

	// Can't test this at the same time as testing the getCommand() methods as registering for this clears the output link information
	//int outputHandler = pAgent->AddOutputHandler("move", MyOutputEventHandler, NULL) ;

	if ( verbose )
	{
		std::cout << "About to do first run-til-output" << std::endl ;
	}

	int callbackp1 = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

	// Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
	// We'll use RunAll just to test it out.  Could use RunSelf and get same result (presumably)
	pKernel->RunAllTilOutput() ;	// Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)
	CPPUNIT_ASSERT_MESSAGE( "RunAllTilOutput", pAgent->GetLastCommandLineResult() );

	// We should stop quickly (after a decision or two)
	CPPUNIT_ASSERT_MESSAGE( "Error in RunTilOutput -- it didn't stop on the output", myCount <= 10 );
	CPPUNIT_ASSERT_MESSAGE( "Error in callback handler for MyRunEventHandler -- failed to update count", myCount > 0 );

	if ( verbose )
	{
		std::cout << "Agent ran for " << myCount << " decisions before we got output" << std::endl ;
	}
	if ( verbose )
	{
		std::cout << trace.str() << std::endl ;
	}
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
	pMove->AddStatusComplete();
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
	int callback3 = pAgent->RegisterForRunEvent( sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyInterruptHandler, &interruptHandlerReceived ) ;

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

// Creates an agent that copies values from input link to output link
// so we can test that this is OK.
void ClientSMLTest::doSimpleCopy()
{
	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	loadProductions( pAgent, "/Tests/testcopy.soar" );

/* Input structure for the test
(S1 ^io I1)
  (I1 ^input-link I3)
    (I3 ^sentence S2)
      (S2 ^newest yes ^num-words 3 ^sentence-num 1 ^word W1 ^word W2 ^word W3)
        (W1 ^num-word 1 ^word the)
        (W2 ^num-word 2 ^word cat)
        (W3 ^num-word 3 ^word in)
*/

	sml::Identifier* map = pAgent->GetInputLink() ;
	sml::Identifier* square2 = pAgent->CreateIdWME(map, "square");
	sml::Identifier* square5 = pAgent->CreateIdWME(map, "square");
	pAgent->CreateSharedIdWME(square2, "north", square5) ;
	pAgent->CreateSharedIdWME(square5, "south", square2) ;

	sml::Identifier* pSentence = pAgent->CreateIdWME(pAgent->GetInputLink(), "sentence") ;
	pAgent->CreateStringWME(pSentence, "newest", "yes") ;
	pAgent->CreateIntWME(pSentence, "num-words", 3) ;
	sml::Identifier* pWord1 = pAgent->CreateIdWME(pSentence, "word") ;

	// BADBAD: This should be illegal, but is not!
	//sml::Identifier* pWord5 = pAgent->CreateSharedIdWME(pSentence, "word", pWord1) ;
	sml::Identifier* pWord5 = pAgent->CreateSharedIdWME(pSentence, "word2", pWord1) ;

	sml::Identifier* pWord2 = pAgent->CreateIdWME(pSentence, "word") ;
	sml::Identifier* pWord3 = pAgent->CreateIdWME(pSentence, "word") ;
	pAgent->CreateIntWME(pWord1, "num-word", 1) ;
	pAgent->CreateIntWME(pWord2, "num-word", 2) ;
	pAgent->CreateIntWME(pWord3, "num-word", 3) ;
	pAgent->CreateStringWME(pWord1, "word", "the") ;
	pAgent->CreateStringWME(pWord2, "word", "cat") ;
	pAgent->CreateStringWME(pWord3, "word", "in") ;
	pAgent->Commit() ;

	// Register for the trace output
	std::stringstream trace ;	// We'll pass this into the handler and build up the output in it
	int callbackp = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

	// Set to true for more detail on this
	pKernel->SetTraceCommunications(false) ;

	std::string result = pAgent->RunSelf(3) ;

	//cout << result << endl ;
	//cout << trace << endl ;

	// TODO: check this output
	//pAgent->ExecuteCommandLine("print --depth 5 s1");

	int changes = pAgent->GetNumberOutputLinkChanges() ;

	//std::cout << pAgent->ExecuteCommandLine("print i3 -d 100 -i --tree");

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
	//	sml::WMElement* pOutputWme = pAgent->GetOutputLinkChange(i) ;
	//	std::cout << pOutputWme->GetIdentifier()->GetIdentifierSymbol() << " ^ " << pOutputWme->GetAttribute() << " " << pOutputWme->GetValueAsString() << std::endl ;
	//}

	// We had a bug where some of these wmes would get dropped (the orphaned wme scheme didn't handle multiple levels)
	// so check now that we got the correct number of changes.
	std::stringstream changesString;
	//changesString << "Number of changes: " << changes << ", this failure is currently expected but needs to be addressed, see wiki gSKI removal page";
	changesString << "Number of changes: " << changes;
	CPPUNIT_ASSERT_MESSAGE( changesString.str().c_str(), changes == 13 );
}

void ClientSMLTest::doSimpleReteNetLoader()
{
	// Creates an agent that loads a rete net and then works with it a little.
	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	std::string path = std::string(pKernel->GetLibraryLocation()) + "/Tests/test.soarx" ;
	std::string command = std::string("rete-net -l ") + path ;
	std::string result = pAgent->ExecuteCommandLine(command.c_str()) ;
	CPPUNIT_ASSERT( pAgent->GetLastCommandLineResult() );

	// Make us match the current input link values
	pAgent->SynchronizeInputLink();

	// Get the latest id from the input link
	sml::Identifier* pID = pAgent->GetInputLink() ;
	//cout << "Input link id is " << pID->GetValueAsString() << endl ;

	CPPUNIT_ASSERT( pID );
}

void ClientSMLTest::testOSupportCopyDestroy()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	pAgent->LoadProductions( "/Tests/testOSupportCopyDestroy.soar" );

	sml::Identifier* pInputLink = pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );

	sml::Identifier* pFoo = pAgent->CreateIdWME( pInputLink, "foo" );
	CPPUNIT_ASSERT( pFoo );

	sml::Identifier* pBar = pAgent->CreateIdWME( pFoo, "bar" );
	CPPUNIT_ASSERT( pBar );

	sml::StringElement* pToy = pAgent->CreateStringWME( pBar, "toy", "jig" );
	CPPUNIT_ASSERT( pToy );

	bool badCopyExists( false );
	pKernel->AddRhsFunction( "bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists ) ; 

	pAgent->RunSelf(1);

	pAgent->DestroyWME( pToy );
	pAgent->DestroyWME( pBar );
	pAgent->DestroyWME( pFoo );

	pAgent->RunSelf(1);

	CPPUNIT_ASSERT( !badCopyExists );
}

void ClientSMLTest::testOSupportCopyDestroyCircularParent()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	pAgent->LoadProductions( "/Tests/testOSupportCopyDestroy.soar" );

	sml::Identifier* pInputLink = pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );

	sml::Identifier* pFoo = pAgent->CreateIdWME( pInputLink, "foo" );
	CPPUNIT_ASSERT( pFoo );

	sml::Identifier* pBar = pAgent->CreateIdWME( pFoo, "bar" );
	CPPUNIT_ASSERT( pBar );

	sml::Identifier* pToy = pAgent->CreateSharedIdWME( pBar, "toy", pFoo );
	CPPUNIT_ASSERT( pToy );

	bool badCopyExists( false );
	pKernel->AddRhsFunction( "bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists ) ; 

	pAgent->RunSelf(1);

	pAgent->DestroyWME( pFoo );

	pAgent->RunSelf(1);

	CPPUNIT_ASSERT( !badCopyExists );
}

void ClientSMLTest::testOSupportCopyDestroyCircular()
{
	numberAgents = 1;
	KernelBitset options(0);
	options.set( EMBEDDED );
	options.set( USE_CLIENT_THREAD );
	options.set( FULLY_OPTIMIZED );
	options.set( AUTO_COMMIT_ENABLED );
	createKernelAndAgents( options );

	sml::Agent* pAgent = pKernel->GetAgent( getAgentName( 0 ).c_str() ) ;
	CPPUNIT_ASSERT( pAgent != NULL );

	pAgent->LoadProductions( "/Tests/testOSupportCopyDestroy.soar" );

	sml::Identifier* pInputLink = pAgent->GetInputLink();
	CPPUNIT_ASSERT( pInputLink );

	sml::Identifier* pFoo = pAgent->CreateIdWME( pInputLink, "foo" );
	CPPUNIT_ASSERT( pFoo );

	sml::Identifier* pBar = pAgent->CreateIdWME( pFoo, "bar" );
	CPPUNIT_ASSERT( pBar );

	sml::Identifier* pToy = pAgent->CreateSharedIdWME( pBar, "toy", pFoo );
	CPPUNIT_ASSERT( pToy );

	bool badCopyExists( false );
	pKernel->AddRhsFunction( "bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists ) ; 

	pAgent->RunSelf(1);

	pAgent->DestroyWME( pToy );
	pAgent->DestroyWME( pBar );
	pAgent->DestroyWME( pFoo );

	pAgent->RunSelf(1);

	CPPUNIT_ASSERT( !badCopyExists );
}

