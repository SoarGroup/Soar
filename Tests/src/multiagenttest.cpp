#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>
#include <sstream>

#include "sml_Client.h"

class MultiAgentTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( MultiAgentTest );	

	CPPUNIT_TEST( testOneAgentForSanity );
	CPPUNIT_TEST( testTwoAgents );
	CPPUNIT_TEST( testTenAgents );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

	static void MyPrintEventHandler( sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage );
	static void MyUpdateEventHandler( sml::smlUpdateEventId id, void* pUserData, sml::Kernel* pKernel, sml::smlRunFlags runFlags );

protected:
	void testOneAgentForSanity();
	void testTwoAgents();
	void testTenAgents();

private:
	void doTest();
	void createInput( sml::Agent* pAgent, int value );
	void reportAgentStatus( sml::Kernel* pKernel, int numberAgents, std::vector< std::stringstream* >& trace );
	void initAll( sml::Kernel* pKernel );

	static const int MAX_AGENTS;
	int numberAgents;
};

CPPUNIT_TEST_SUITE_REGISTRATION( MultiAgentTest ); 

const int MultiAgentTest::MAX_AGENTS = 100;

void MultiAgentTest::setUp()
{
}

void MultiAgentTest::tearDown()
{
}

void MultiAgentTest::MyPrintEventHandler( sml::smlPrintEventId, void* pUserData, sml::Agent*, char const* pMessage )
{
	// In this case the user data is a string we're building up
	std::stringstream* pTrace = static_cast<std::stringstream*>( pUserData ) ;

	(*pTrace) << pMessage ;
}

void MultiAgentTest::createInput( sml::Agent* pAgent, int value )
{
	// This agent adds value1 to value2 inside the agent and puts the total on the output link.
	// We take it from the output link and puts it on the input link to generate a running total.
	sml::Identifier* pInputLink = pAgent->GetInputLink() ;
	CPPUNIT_ASSERT( pInputLink != NULL );

	sml::Identifier* pAdd = pInputLink->CreateIdWME( "add" ) ;
	CPPUNIT_ASSERT( pAdd != NULL );

	sml::WMElement* pValue1 = pAdd->CreateIntWME( "value1", 2 ) ;
	CPPUNIT_ASSERT( pValue1 != NULL );

	sml::WMElement* pValue2 = pAdd->CreateIntWME( "value2", value ) ;
	CPPUNIT_ASSERT( pValue2 != NULL );

	CPPUNIT_ASSERT( pAgent->Commit() );
}

void UpdateInput( sml::Agent* pAgent, int value )
{
	// Set value2 to a new value, triggering a new calculation
	sml::Identifier* pInputLink = pAgent->GetInputLink() ;
	CPPUNIT_ASSERT( pInputLink != NULL );

	sml::Identifier* pAdd = pInputLink->FindByAttribute( "add", 0 )->ConvertToIdentifier() ;
	CPPUNIT_ASSERT( pAdd != NULL );

	sml::IntElement* pValue2 = pAdd->FindByAttribute( "value2", 0 )->ConvertToIntElement() ;
	CPPUNIT_ASSERT( pValue2 != NULL );

	pAgent->Update( pValue2, value ) ;

	CPPUNIT_ASSERT( pAgent->Commit() );
}

void MultiAgentTest::MyUpdateEventHandler( sml::smlUpdateEventId, void*, sml::Kernel* pKernel, sml::smlRunFlags )
{
	int agents = pKernel->GetNumberAgents() ;
	for (int agentIndex = 0 ; agentIndex < agents ; ++agentIndex)
	{
		sml::Agent* pAgent = pKernel->GetAgentByIndex( agentIndex ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		char const* pIOString = pAgent->ExecuteCommandLine( "print --depth 4 i1" ) ;
		CPPUNIT_ASSERT( pIOString != NULL );
		//std::cout << pIOString << std::endl ;

		// Make sure we can get the output link (had a bug where this wouldn't always work)
		sml::Identifier* pOutputLink = pAgent->GetOutputLink() ;
		CPPUNIT_ASSERT( pOutputLink != NULL );

		// Read in the commands
		int numberCommands = pAgent->GetNumberCommands() ;
		for ( int i = 0 ; i < numberCommands ; ++i )
		{
			sml::Identifier* pCommand = pAgent->GetCommand( i ) ;			
			CPPUNIT_ASSERT( pCommand != NULL );

			char const* pName = pCommand->GetCommandName() ;
			CPPUNIT_ASSERT( pName != NULL );
			CPPUNIT_ASSERT( std::string( pName ) == "result" );

			// Receive the new total
			char const* pTotal = pCommand->GetParameterValue( "total" ) ;
			CPPUNIT_ASSERT( pTotal != NULL );
			std::stringstream paramValue( pTotal );
			int intTotal = 0;
			paramValue >> intTotal;

			// Mark command as completed in working memory
			pCommand->AddStatusComplete() ;

			// Place a new addition request on the input link
			UpdateInput( pAgent, intTotal );
		}

		pAgent->ClearOutputLinkChanges() ;
	}
}

void MultiAgentTest::reportAgentStatus( sml::Kernel* pKernel, int numberAgents, std::vector< std::stringstream* >& trace )
{
	for (int agentCounter = 0 ; agentCounter < numberAgents ; agentCounter++)
	{
		sml::Agent* pAgent = pKernel->GetAgentByIndex( agentCounter ) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		//std::cout << "Trace from agent " << pAgent->GetAgentName() << std::endl ;

		//std::cout << "Input link " << std::endl 
		//	<< pAgent->ExecuteCommandLine( "print --depth 3 i2" ) << std::endl ;

		//std::cout << trace[agentCounter]->str() << std::endl << std::endl ;

		// We need to clear this after it's been printed or the next time we print it
		// we'll get the entire trace from 0
		trace[agentCounter]->clear();
	}
}

void MultiAgentTest::initAll( sml::Kernel* pKernel )
{
	int agents = pKernel->GetNumberAgents() ;
	for (int i = 0 ; i < agents ; i++)
	{
		sml::Agent* pAgent = pKernel->GetAgentByIndex(i) ;
		CPPUNIT_ASSERT( pAgent != NULL );

		pAgent->InitSoar() ;
		//std::string initRes = pAgent->InitSoar() ;
		//cout << initRes << endl ;
	}
}

void MultiAgentTest::testOneAgentForSanity()
{
	numberAgents = 1;
	doTest();
}

void MultiAgentTest::testTwoAgents()
{
	numberAgents = 2;
	doTest();
}

void MultiAgentTest::testTenAgents()
{
	numberAgents = 10;
	doTest();
}

void MultiAgentTest::doTest()
{
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread( "SoarKernelSML" );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	// We'll require commits, just so we're testing that path
	pKernel->SetAutoCommit( false ) ;

	// Comment this in if you need to debug the messages going back and forth.
	//pKernel->SetTraceCommunications(true) ;

	CPPUNIT_ASSERT( numberAgents < MAX_AGENTS );
	
	std::vector< std::string > names;
	std::vector< sml::Agent* > agents;
	std::vector< std::stringstream* > trace;
	std::vector< int > callbackPrint;

	// Create the agents
	for ( int agentCounter = 0 ; agentCounter < numberAgents ; ++agentCounter )
	{
		std::stringstream name;
		name << "agent" << 1 + agentCounter;
		names.push_back( name.str() );

		sml::Agent* pAgent   = pKernel->CreateAgent( name.str().c_str() ) ;
		CPPUNIT_ASSERT( pAgent != NULL );
		CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

		agents.push_back( pAgent );

		std::stringstream path;
		// TODO: use boost filesystem
		path << pKernel->GetLibraryLocation() << "/share/soar/Tests/testmulti.soar";
		CPPUNIT_ASSERT( pAgent->LoadProductions( path.str().c_str() ) );
		createInput( pAgent, 0 );

		// Collect the trace output from the run
		trace.push_back( new std::stringstream() );
		callbackPrint.push_back( pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, MultiAgentTest::MyPrintEventHandler, trace[agentCounter] ) );
	}

	pKernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_GENERATED_OUTPUT, MultiAgentTest::MyUpdateEventHandler, NULL ) ;

	// Run for a first set of output, so we can see whether that worked
	pKernel->RunAllTilOutput() ;

	// Print out some information
	reportAgentStatus( pKernel, numberAgents, trace ) ;

	// Now get serious about a decent run
	const int kFirstRun = 5 ;
	for (int i = 0 ; i < kFirstRun ; i++)
	{
		// Run for a bit
		pKernel->RunAllTilOutput() ;
	}

	reportAgentStatus(pKernel, numberAgents, trace) ;

	// Toss in an init-soar and then go on a bit further
	initAll(pKernel) ;

	// Second run
	const int kSecondRun = 5 ;
	for (int i = 0 ; i < kSecondRun ; i++)
	{
		// Run for a bit
		pKernel->RunAllTilOutput() ;
	}

	reportAgentStatus(pKernel, numberAgents, trace) ;

	for ( std::vector< std::stringstream* >::iterator iter = trace.begin(); iter != trace.end(); ++iter )
	{
		delete *iter;
	}

	//cout << "Calling shutdown on the kernel now" << endl ;
	pKernel->Shutdown() ;
	//cout << "Shutdown completed now" << endl ;

	// Delete the kernel.  If this is an embedded connection this destroys the kernel.
	// If it's a remote connection we just disconnect.
	delete pKernel ;
}
