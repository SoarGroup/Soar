#include <portability.h>

#include <cppunit/extensions/HelperMacros.h>

#include "handlers.h"
#include "kernel.h"
#include "soar_rand.h"

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
    CPPUNIT_TEST( testSource );
    CPPUNIT_TEST( test_clog );
    CPPUNIT_TEST( test_gp );
    CPPUNIT_TEST( test_echo );
    CPPUNIT_TEST( test_ls );
	
	// too much pain to keep up-to-date: nixing for now
    //CPPUNIT_TEST( test_stats );

    CPPUNIT_TEST( testWrongAgentWmeFunctions );
    CPPUNIT_TEST( testRHSRand );
    CPPUNIT_TEST( testMultipleKernels );
    CPPUNIT_TEST( testSmemArithmetic );
    CPPUNIT_TEST( testSoarRand );
    CPPUNIT_TEST( testPreferenceDeallocation );

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();        
    void tearDown();    

protected:
    void testInstiationDeallocationStackOverflow();
    void test_clog();
    void test_gp();
    void test_echo();
    void test_ls();
    void test_stats();

    void testWrongAgentWmeFunctions();
    void testRHSRand();
    void testMultipleKernels();
    void testSmemArithmetic();

    void testSource();

    void testSoarRand();
    void testPreferenceDeallocation();

    void source(const std::string &path);

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

void MiscTest::source(const std::string &path)
{
    pAgent->LoadProductions((std::string("test_agents/") + path).c_str());
    CPPUNIT_ASSERT_MESSAGE( pAgent->GetLastErrorDescription(), pAgent->GetLastCommandLineResult() );
}

void MiscTest::setUp()
{
    pKernel = 0;
    pAgent = 0;
    pKernel = sml::Kernel::CreateKernelInNewThread() ;
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
    source("count-and-die.soar");
    pAgent->ExecuteCommandLine("w 0");
    pKernel->RunAllAgentsForever();
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

void MiscTest::test_gp()
{
	source("testgp.soar");

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
    pAgent->ExecuteCommandLine("echo sp \\{my*prod"); // bug 987
    CPPUNIT_ASSERT_MESSAGE("bug 987", pAgent->GetLastCommandLineResult());

    pAgent->ExecuteCommandLine("echo \"#########################################################\""); // bug 1013
    CPPUNIT_ASSERT_MESSAGE("bug 1013", pAgent->GetLastCommandLineResult());

    pAgent->ExecuteCommandLine("echo \\\"");
    CPPUNIT_ASSERT_MESSAGE("quote", pAgent->GetLastCommandLineResult());

    pAgent->ExecuteCommandLine("echo [");
    CPPUNIT_ASSERT_MESSAGE("left brace", pAgent->GetLastCommandLineResult());

    pAgent->ExecuteCommandLine("echo ]");
    CPPUNIT_ASSERT_MESSAGE("right brace", pAgent->GetLastCommandLineResult());

    pAgent->ExecuteCommandLine("echo \\{");
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

void MiscTest::test_ls()
{
    pAgent->ExecuteCommandLine("ls pci");
    CPPUNIT_ASSERT(!pAgent->GetLastCommandLineResult());
}

void MiscTest::test_stats()
{
    sml::ClientAnalyzedXML stats;
    pAgent->ExecuteCommandLineXML("stats", &stats);
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());

    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountDefault, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountUser, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountChunk, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountJustification, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountInnerElaboration, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsProductionFiringCount, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCountAddition, -1) == 13);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCountRemoval, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCount, -1) == 13);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsWmeCountAverage, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCountMax, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsKernelCPUTime, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsTotalCPUTime, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeInputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeProposePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeDecisionPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeApplyPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeOutputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimePreferencePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeWorkingMemoryPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeInputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeProposePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeDecisionPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeApplyPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeOutputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimePreferencePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeWorkingMemoryPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsInputFunctionTime, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOutputFunctionTime, -1) == 0);
#ifdef DETAILED_TIMING_STATS
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeInputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimePreferencePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeWorkingMemoryPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeOutputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeDecisionPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeProposePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeApplyPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeInputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimePreferencePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeOutputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeDecisionPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeProposePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeApplyPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeInputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimePreferencePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeOutputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeDecisionPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeProposePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeApplyPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeInputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimePreferencePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeWorkingMemoryPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeOutputPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeDecisionPhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeProposePhase, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeApplyPhase, -1) == 0);
#endif
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleTimeCycle, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgFloat(sml::sml_Names::kParamStatsMaxDecisionCycleTimeValueSec, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleTimeValueUSec, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleWMChangesCycle, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleWMChangesValue, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleFireCountCycle, -1) == 0);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleFireCountValue, -1) == 0);
#if defined(_WIN64) || !defined(_WIN32)
    if (sizeof(intptr_t) > 4)
    {
        // 64-bit any
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageMiscellaneous, -1) == 4376);
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageHash, -1) == 264032);
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsagePool, -1) == 687216);
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageStatsOverhead, -1) == 2136);
    }
    else
    {
        // 32-bit linux
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageMiscellaneous, -1) == 3612);
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageHash, -1) == 132232);
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsagePool, -1) == 327248);
        CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageStatsOverhead, -1) == 1036);
    }
#else // 32-bit windows
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageMiscellaneous, -1) == 3508);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageHash, -1) == 132232);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsagePool, -1) == 687240);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageStatsOverhead, -1) == 1068);
#endif
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsMemoryUsageString, -1) == 900);

    pAgent->ExecuteCommandLine("stats -t");
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
    pAgent->RunSelf(10);
    std::string res = pAgent->ExecuteCommandLine("stats -c");
    CPPUNIT_ASSERT(!res.empty());

    pAgent->ExecuteCommandLine("stats -t");
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
    pAgent->RunSelf(10);
    res = pAgent->ExecuteCommandLine("stats -c");
    CPPUNIT_ASSERT(!res.empty());

    pAgent->ExecuteCommandLine("stats -T");
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
    pAgent->RunSelf(10);
    res = pAgent->ExecuteCommandLine("stats -c");
    CPPUNIT_ASSERT(res.empty());

    pAgent->ExecuteCommandLine("stats -t");
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
    pAgent->RunSelf(10);
    res = pAgent->ExecuteCommandLine("stats -c");
    CPPUNIT_ASSERT(!res.empty());

    pAgent->ExecuteCommandLine("stats -T");
    CPPUNIT_ASSERT(pAgent->GetLastCommandLineResult());
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
    source("testRHSRand.soar");
    pAgent->RunSelf(5000);
}

void MiscTest::testMultipleKernels()
{
    sml::Kernel* pKernel2 = sml::Kernel::CreateKernelInNewThread(sml::Kernel::kDefaultSMLPort - 1);
    CPPUNIT_ASSERT( pKernel2 != NULL );
    CPPUNIT_ASSERT_MESSAGE( pKernel2->GetLastErrorDescription(), !pKernel2->HadError() );

    sml::Agent* pAgent2 = pKernel2->CreateAgent( "soar2" );
    CPPUNIT_ASSERT( pAgent2 != NULL );

    pKernel2->Shutdown();
    delete pKernel2;

    pAgent->ExecuteCommandLine("p s1");
    CPPUNIT_ASSERT( pAgent->GetLastCommandLineResult() );
}

void MiscTest::testSmemArithmetic()
{
    source("arithmetic/arithmetic.soar") ;
    pAgent->ExecuteCommandLine("watch 0");
    pAgent->ExecuteCommandLine("srand 1080");

    pAgent->RunSelfForever();

    sml::ClientAnalyzedXML stats;
    pAgent->ExecuteCommandLineXML("stats", &stats);
    CPPUNIT_ASSERT(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 46436);
}


void MiscTest::testSource()
{
    source("big.soar");
}

void MiscTest::testSoarRand()
{
    int halftrials = 50000000;
    double accum = 0;
    for (int i = 0; i < halftrials * 2; ++i)
        accum += SoarRand();
    double off = (accum - halftrials) / halftrials;
    CPPUNIT_ASSERT(off < 0.001);
}

void MiscTest::testPreferenceDeallocation()
{
    source("testPreferenceDeallocation.soar");
    pAgent->ExecuteCommandLine("run 10");

    sml::ClientAnalyzedXML response;
    pAgent->ExecuteCommandLineXML("stats", &response);
    CPPUNIT_ASSERT(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 6);
}

