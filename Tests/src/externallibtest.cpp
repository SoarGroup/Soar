#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

class ExternalLibTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( ExternalLibTest );	

	CPPUNIT_TEST( testExternalLib );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

	static bool bSuccess;

protected:
	void testExternalLib();
};

CPPUNIT_TEST_SUITE_REGISTRATION( ExternalLibTest );

#include "sml_Utils.h"
#include "sml_Client.h"
#include "sml_Names.h"
#include "Export.h"

#include <string>
#include <iostream>

bool ExternalLibTest::bSuccess = false;

void ExternalLibTest::setUp()
{
}

void ExternalLibTest::tearDown()
{
}

void PrintCallbackHandler(sml::smlPrintEventId, void*, sml::Agent*, char const* pMessage) {
	if(std::string( pMessage ) == "Success!") {
		ExternalLibTest::bSuccess = true;
	}
}

void ExternalLibTest::testExternalLib()
{
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread( "SoarKernelSML" ) ;
	CPPUNIT_ASSERT( pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	//std::cout << "load-library command returned: \"" 
	//	<< pKernel->LoadExternalLibrary("TestExternalLibraryLib \"Parsing and load-library error message test successful!\"") 
	//	<< "\"" << std::endl;

	sml::Agent* pAgent = pKernel->CreateAgent( "soar1" );
	CPPUNIT_ASSERT( pAgent != NULL );

	pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, PrintCallbackHandler, 0 );

        std::stringstream productionsPath;
        productionsPath << pKernel->GetLibraryLocation() << "/share/soar/Tests/TestExternalLibrary.soar";

        pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
        CPPUNIT_ASSERT_MESSAGE( "loadProductions", pAgent->GetLastCommandLineResult() );

	pKernel->RunAllAgents(1);
	CPPUNIT_ASSERT_MESSAGE( "RHS function did not fire.", bSuccess );

	pKernel->Shutdown();
	delete pKernel ;
}
