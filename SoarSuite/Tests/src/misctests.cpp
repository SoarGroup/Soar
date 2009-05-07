#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

namespace sml
{
	class Kernel;
	class Agent;
};

class MiscTest : public CPPUNIT_NS::TestCase
{
	CPPUNIT_TEST_SUITE( MiscTest );	

#if !defined(_DEBUG)
	// this test takes forever in debug mode on windows (it needs to count high enough to overflow a 64-bit stack)
	CPPUNIT_TEST( testInstiationDeallocationStackOverflow );
#endif
	CPPUNIT_TEST( test_gp );
	CPPUNIT_TEST( test_echo );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

protected:
	void testInstiationDeallocationStackOverflow();
	void test_gp();
	void test_echo();

	sml::Kernel* pKernel;
	sml::Agent* pAgent;
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
	pKernel = 0;
	pAgent = 0;
	pKernel = sml::Kernel::CreateKernelInNewThread( "SoarKernelSML" ) ;
	CPPUNIT_ASSERT( pKernel != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel->GetLastErrorDescription(), !pKernel->HadError() );

	pAgent = pKernel->CreateAgent( "soar1" );
	CPPUNIT_ASSERT( pAgent != NULL );
}

void MiscTest::tearDown()
{
	pKernel->Shutdown();
	delete pKernel ;
	pKernel = 0;
	pAgent = 0;
}

void MiscTest::testInstiationDeallocationStackOverflow()
{
	std::stringstream productionsPath;
	productionsPath << pKernel->GetLibraryLocation() << "/Tests/count-and-die.soar";

	pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "loadProductions", pAgent->GetLastCommandLineResult() );

	pAgent->ExecuteCommandLine("w 0");

	pKernel->RunAllAgentsForever();
}

void MiscTest::test_gp()
{
	std::stringstream productionsPath;
	productionsPath << pKernel->GetLibraryLocation() << "/Tests/testgp.soar";

	pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "loadProductions", pAgent->GetLastCommandLineResult() );

	pAgent->ExecuteCommandLine("gp {gp*test10 (state <s> ^operator <o> + ^someflag [ <var> true false ] ^<< [ a1 a2 a3 a4 a5] [a6 a7 a8 a9 a10] >> << [v1 v2 v3] [v4 v5 v6] [v7 v8 v9 v10] >>) (<o> ^name foo ^att [ val1 1.3 |another val| |\\|another val\\|| ] ^[ att1 att2 att3 att4 att5] [val1 val2 val3 val4 <var>]) --> (<s> ^[<var> att] <var>) }");
	CPPUNIT_ASSERT_MESSAGE("valid but too large (540000) gp production didn't fail", pAgent->GetLastCommandLineResult() == false);

	pAgent->ExecuteCommandLine("gp {gp*fail1 (state <s> ^att []) --> (<s> ^operator <o> = 5) }");
	CPPUNIT_ASSERT_MESSAGE("'need at least one value in list' didn't fail", pAgent->GetLastCommandLineResult() == false);

	pAgent->ExecuteCommandLine("gp {gp*fail2 (state <s> ^[att1 att2][val1 val2]) --> (<s> ^operator <o> = 5) }");
	CPPUNIT_ASSERT_MESSAGE("'need space between value lists' didn't fail", pAgent->GetLastCommandLineResult() == false);

	pAgent->ExecuteCommandLine("gp {gp*fail3 (state <s> ^foo bar[) --> (<s> ^foo bar) }");
	CPPUNIT_ASSERT_MESSAGE("'unmatched [' didn't fail", pAgent->GetLastCommandLineResult() == false);
}

void MiscTest::test_echo()
{
	pAgent->ExecuteCommandLine("echo sp {my*prod"); // bug 987
	CPPUNIT_ASSERT_MESSAGE("bug 987", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo \"#########################################################\""); // bug 1013
	CPPUNIT_ASSERT_MESSAGE("bug 1013", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo \"");
	CPPUNIT_ASSERT_MESSAGE("quote", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo [");
	CPPUNIT_ASSERT_MESSAGE("left brace", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo ]");
	CPPUNIT_ASSERT_MESSAGE("right brace", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo {");
	CPPUNIT_ASSERT_MESSAGE("left bracket", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo }");
	CPPUNIT_ASSERT_MESSAGE("right bracket", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo <");
	CPPUNIT_ASSERT_MESSAGE("less than", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo >");
	CPPUNIT_ASSERT_MESSAGE("greater than", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo |");
	CPPUNIT_ASSERT_MESSAGE("pipe", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo |#");
	CPPUNIT_ASSERT_MESSAGE("pipe and pound", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo |#|");
	CPPUNIT_ASSERT_MESSAGE("pound in pipes", pAgent->GetLastCommandLineResult());

	pAgent->ExecuteCommandLine("echo ~!@#$%^&*()_+`1234567890-=\\:,.?/");
	CPPUNIT_ASSERT_MESSAGE("misc chars", pAgent->GetLastCommandLineResult());
}
