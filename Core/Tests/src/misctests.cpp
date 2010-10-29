#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "handlers.h"

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
#endif // _DEBUG
	CPPUNIT_TEST( test_clog );
	CPPUNIT_TEST( test_gp );
	CPPUNIT_TEST( test_echo );

	CPPUNIT_TEST( testWrongAgentWmeFunctions );
	CPPUNIT_TEST( testRHSRand );
	CPPUNIT_TEST( testMultipleKernels );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();		
	void tearDown();	

protected:
	void testInstiationDeallocationStackOverflow();
	void test_clog();
	void test_gp();
	void test_echo();

	void testWrongAgentWmeFunctions();
	void testRHSRand();
	void testMultipleKernels();

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
	productionsPath << pKernel->GetLibraryLocation() << "share/soar/Tests/count-and-die.soar";

	pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( "loadProductions", pAgent->GetLastCommandLineResult() );

	pAgent->ExecuteCommandLine("w 0");

	pKernel->RunAllAgentsForever();
}

void MiscTest::test_gp()
{
	std::stringstream productionsPath;
	productionsPath << pKernel->GetLibraryLocation() << "share/soar/Tests/testgp.soar";

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

void MiscTest::test_clog()
{
	pAgent->ExecuteCommandLine("clog clog-test.txt");
	CPPUNIT_ASSERT_MESSAGE("clog clog-test.txt", pAgent->GetLastCommandLineResult());
	pAgent->ExecuteCommandLine("watch 5");
	CPPUNIT_ASSERT_MESSAGE("watch 5", pAgent->GetLastCommandLineResult());
	pAgent->RunSelf(5);
	pAgent->InitSoar();
	pAgent->RunSelf(5);
	pKernel->DestroyAgent(pAgent);
	pAgent = pKernel->CreateAgent( "soar1" );
	CPPUNIT_ASSERT( pAgent != NULL );
	pAgent->ExecuteCommandLine("clog clog-test.txt");
	CPPUNIT_ASSERT_MESSAGE("clog clog-test.txt", pAgent->GetLastCommandLineResult());
	pAgent->ExecuteCommandLine("watch 5");
	CPPUNIT_ASSERT_MESSAGE("watch 5", pAgent->GetLastCommandLineResult());
	pAgent->RunSelf(5);
	pAgent->InitSoar();
	pAgent->RunSelf(5);
	pAgent->ExecuteCommandLine("clog --close");
	remove("clog-test.txt");
}

void MiscTest::testWrongAgentWmeFunctions()
{
	sml::Agent* pAgent2 = 0;
	pAgent2 = pKernel->CreateAgent( "soar2" );
	CPPUNIT_ASSERT( pAgent2 != NULL );

	sml::Identifier* il1 = pAgent->GetInputLink();
	sml::Identifier* il2 = pAgent2->GetInputLink();

	sml::Identifier* foo1 = il1->CreateIdWME("foo");
	sml::Identifier* foo2 = il2->CreateIdWME("foo");

	CPPUNIT_ASSERT(pAgent->CreateStringWME(foo2, "fail", "fail") == 0);
	CPPUNIT_ASSERT(pAgent->CreateIntWME(foo2, "fail", 1) == 0);
	CPPUNIT_ASSERT(pAgent->CreateFloatWME(foo2, "fail", 1.0f) == 0);
	CPPUNIT_ASSERT(pAgent->CreateIdWME(foo2, "fail") == 0);
	CPPUNIT_ASSERT(pAgent->CreateSharedIdWME(foo2, "fail", il1) == 0);
	CPPUNIT_ASSERT(pAgent->DestroyWME(foo2) == 0);

	CPPUNIT_ASSERT(pAgent2->CreateStringWME(foo1, "fail", "fail") == 0);
	CPPUNIT_ASSERT(pAgent2->CreateIntWME(foo1, "fail", 1) == 0);
	CPPUNIT_ASSERT(pAgent2->CreateFloatWME(foo1, "fail", 1.0f) == 0);
	CPPUNIT_ASSERT(pAgent2->CreateIdWME(foo1, "fail") == 0);
	CPPUNIT_ASSERT(pAgent2->CreateSharedIdWME(foo1, "fail", il2) == 0);
	CPPUNIT_ASSERT(pAgent2->DestroyWME(foo1) == 0);
}

void MiscTest::testRHSRand()
{
	pKernel->AddRhsFunction( "test-failure", Handlers::MyRhsFunctionFailureHandler, 0 ) ; 

	std::stringstream productionsPath;
	productionsPath << pKernel->GetLibraryLocation() << "share/soar/Tests/testRHSRand.soar";

	pAgent->LoadProductions( productionsPath.str().c_str(), true ) ;
	CPPUNIT_ASSERT_MESSAGE( pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult() );

	pAgent->RunSelf(5000);
}

void MiscTest::testMultipleKernels()
{
	sml::Kernel* pKernel2 = sml::Kernel::CreateKernelInNewThread(sml::Kernel::kDefaultLibraryName, sml::Kernel::kDefaultSMLPort - 1);
	CPPUNIT_ASSERT( pKernel2 != NULL );
	CPPUNIT_ASSERT_MESSAGE( pKernel2->GetLastErrorDescription(), !pKernel2->HadError() );

	sml::Agent* pAgent2 = pKernel2->CreateAgent( "soar2" );
	CPPUNIT_ASSERT( pAgent2 != NULL );

	pKernel2->Shutdown();
	delete pKernel2;

	pAgent->ExecuteCommandLine("p s1");
	CPPUNIT_ASSERT( pAgent->GetLastCommandLineResult() );
}
