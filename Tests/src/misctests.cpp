#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

class MiscTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( MiscTest );	

#if !defined(_DEBUG)
	// this test takes forever in debug mode on windows (it needs to count high enough to overflow a 64-bit stack)
	CPPUNIT_TEST( testInstiationDeallocationStackOverflow );
	CPPUNIT_TEST( testGP );
#endif

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

protected:
	void testInstiationDeallocationStackOverflow();
	void testGP();
};

CPPUNIT_TEST_SUITE_REGISTRATION( MiscTest );

#include "sml_Utils.h"
#include "sml_Client.h"
#include "sml_Names.h"
#include "Export.h"

#include <string>
#include <iostream>

void MiscTest::setUp()
{
}

void MiscTest::tearDown()
{
}

void MiscTest::testInstiationDeallocationStackOverflow()
{
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread( "SoarKernelSML" ) ;
	CPPUNIT_ASSERT( pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	sml::Agent* pAgent = pKernel->CreateAgent( "soar1" );
	CPPUNIT_ASSERT( pAgent != NULL );

	std::stringstream productionsPath;
	productionsPath << pKernel->GetLibraryLocation() << "/Tests/count-and-die.soar";

	pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "loadProductions", pAgent->GetLastCommandLineResult() );

	pAgent->ExecuteCommandLine("w 0");

	pKernel->RunAllAgentsForever();
	pKernel->Shutdown();
	delete pKernel ;
}

void MiscTest::testGP()
{
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread( "SoarKernelSML" ) ;
	CPPUNIT_ASSERT( pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	sml::Agent* pAgent = pKernel->CreateAgent( "soar1" );
	CPPUNIT_ASSERT( pAgent != NULL );

	std::stringstream productionsPath;
	productionsPath << pKernel->GetLibraryLocation() << "/Tests/testgp.soar";

	pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "loadProductions", pAgent->GetLastCommandLineResult() );

	pKernel->Shutdown();
	delete pKernel ;
}