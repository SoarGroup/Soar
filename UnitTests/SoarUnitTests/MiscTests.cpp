//
//  MiscTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "MiscTests.hpp"

#include "portability.h"
#include "Export.h"

#include "soar_rand.h"
#include "sml_Utils.h"
#include "sml_Client.h"
#include "sml_Names.h"

#include <string>
#include <iostream>

#include "SoarHelper.hpp"
#include "handlers.hpp"

void MiscTests::source(const std::string& file)
{
	agent->LoadProductions(SoarHelper::GetResource(file).c_str());
	assertTrue_msg(agent->GetLastErrorDescription(), agent->GetLastCommandLineResult());
}

void MiscTests::testInstiationDeallocationStackOverflow()
{
	source("count-and-die.soar");
	agent->ExecuteCommandLine("w 0");
	kernel->RunAllAgentsForever();
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::testGDS_Failed_Justification_Crash()
{
    source("testGDS_Failed_Justification_Crash.soar");
    agent->ExecuteCommandLine("w 0");
    kernel->RunAllAgentsForever();
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
void MiscTests::testIsupported_Smem_Chunk_Crash()
{
    source("testIsupported_Smem_Chunk_Crash.soar");
    agent->ExecuteCommandLine("w 0");
    kernel->RunAllAgentsForever();
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
void MiscTests::testNegated_Operator_Crash()
{
    source("testNegated_Operator_Crash.soar");
    agent->ExecuteCommandLine("w 0");
    kernel->RunAllAgentsForever();
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
void MiscTests::testOp_Augmentation_Crash()
{
    source("testOp_Augmentation_Crash.soar");
    agent->ExecuteCommandLine("w 0");
    kernel->RunAllAgentsForever();
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::test_clog()
{
	agent->ExecuteCommandLine("clog clog-test.txt");
	assertTrue_msg("clog clog-test.txt", agent->GetLastCommandLineResult());
	agent->ExecuteCommandLine("watch 5");
	assertTrue_msg("watch 5", agent->GetLastCommandLineResult());
	agent->RunSelf(5);
	agent->InitSoar();
	agent->RunSelf(5);

	tearDown(false);
	setUp();

	assertTrue(agent != NULL);
	agent->ExecuteCommandLine("clog clog-test.txt");
	assertTrue_msg("clog clog-test.txt", agent->GetLastCommandLineResult());
	agent->ExecuteCommandLine("watch 5");
	assertTrue_msg("watch 5", agent->GetLastCommandLineResult());
	agent->RunSelf(5);
	agent->InitSoar();
	agent->RunSelf(5);
	agent->ExecuteCommandLine("clog --close");
	remove("clog-test.txt");
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::test_gp()
{
	source("testgp.soar");

	agent->ExecuteCommandLine("gp {gp*test10 (state <s> ^operator <o> + ^someflag [ <var> true false ] ^<< [ a1 a2 a3 a4 a5] [a6 a7 a8 a9 a10] >> << [v1 v2 v3] [v4 v5 v6] [v7 v8 v9 v10] >>) (<o> ^name foo ^att [ val1 1.3 |another val| |\\|another val\\|| ] ^[ att1 att2 att3 att4 att5] [val1 val2 val3 val4 <var>]) --> (<s> ^[<var> att] <var>) }");
	assertTrue_msg("valid but too large (540000) gp production didn't fail", agent->GetLastCommandLineResult() == false);

	agent->ExecuteCommandLine("gp {gp*fail1 (state <s> ^att []) --> (<s> ^operator <o> = 5) }");
	assertTrue_msg("'need at least one value in list' didn't fail", agent->GetLastCommandLineResult() == false);

	agent->ExecuteCommandLine("gp {gp*fail2 (state <s> ^[att1 att2][val1 val2]) --> (<s> ^operator <o> = 5) }");
	assertTrue_msg("'need space between value lists' didn't fail", agent->GetLastCommandLineResult() == false);

	agent->ExecuteCommandLine("gp {gp*fail3 (state <s> ^foo bar[) --> (<s> ^foo bar) }");
	assertTrue_msg("'unmatched [' didn't fail", agent->GetLastCommandLineResult() == false);
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::test_echo()
{
	agent->ExecuteCommandLine("echo sp \\{my*prod"); // bug 987
	assertTrue_msg("bug 987", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo \"#########################################################\""); // bug 1013
	assertTrue_msg("bug 1013", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo \\\"");
	assertTrue_msg("quote", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo [");
	assertTrue_msg("left brace", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo ]");
	assertTrue_msg("right brace", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo \\{");
	assertTrue_msg("left bracket", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo }");
	assertTrue_msg("right bracket", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo <");
	assertTrue_msg("less than", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo >");
	assertTrue_msg("greater than", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo |#|");
	assertTrue_msg("pound in pipes", agent->GetLastCommandLineResult());

	agent->ExecuteCommandLine("echo ~!@#$%^&*()_+`1234567890-=\\:,.?/");
	assertTrue_msg("misc chars", agent->GetLastCommandLineResult());
}

void MiscTests::test_ls()
{
	agent->ExecuteCommandLine("ls pci");
	assertTrue(!agent->GetLastCommandLineResult());
}

void MiscTests::test_stats()
{
	sml::ClientAnalyzedXML stats;
	agent->ExecuteCommandLineXML("stats", &stats);
	assertTrue(agent->GetLastCommandLineResult());

	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountDefault, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountUser, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountChunk, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsProductionCountJustification, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountInnerElaboration, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsProductionFiringCount, -1), 0);
    assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCountAddition, -1), 13);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCount, -1), 13);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsWmeCountAverage, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsWmeCountMax, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsKernelCPUTime, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsTotalCPUTime, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeInputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeProposePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeDecisionPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeApplyPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeOutputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimePreferencePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsPhaseTimeWorkingMemoryPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeInputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeProposePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeDecisionPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeApplyPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeOutputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimePreferencePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMonitorTimeWorkingMemoryPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsInputFunctionTime, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOutputFunctionTime, -1), 0);
#ifdef DETAILED_TIMING_STATS
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeInputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimePreferencePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeWorkingMemoryPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeOutputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeDecisionPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeProposePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMatchTimeApplyPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeInputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimePreferencePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeOutputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeDecisionPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeProposePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsOwnershipTimeApplyPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeInputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimePreferencePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeOutputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeDecisionPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeProposePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsChunkingTimeApplyPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeInputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimePreferencePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeWorkingMemoryPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeOutputPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeDecisionPhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeProposePhase, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsGDSTimeApplyPhase, -1), 0);
#endif
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleTimeCycle, -1), 0);
	assertEquals(stats.GetArgFloat(sml::sml_Names::kParamStatsMaxDecisionCycleTimeValueSec, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleTimeValueUSec, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleWMChangesCycle, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleWMChangesValue, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleFireCountCycle, -1), 0);
	assertEquals(stats.GetArgInt(sml::sml_Names::kParamStatsMaxDecisionCycleFireCountValue, -1), 0);

	agent->ExecuteCommandLine("stats -t");
	assertTrue(agent->GetLastCommandLineResult());
	agent->RunSelf(10);
	std::string res = agent->ExecuteCommandLine("stats -c");
	assertTrue(!res.empty());

	agent->ExecuteCommandLine("stats -t");
	assertTrue(agent->GetLastCommandLineResult());
	agent->RunSelf(10);
	res = agent->ExecuteCommandLine("stats -c");
	assertTrue(!res.empty());

	agent->ExecuteCommandLine("stats -T");
	assertTrue(agent->GetLastCommandLineResult());
	agent->RunSelf(10);
	res = agent->ExecuteCommandLine("stats -c");
	assertTrue(res.empty());

	agent->ExecuteCommandLine("stats -t");
	assertTrue(agent->GetLastCommandLineResult());
	agent->RunSelf(10);
	res = agent->ExecuteCommandLine("stats -c");
	assertTrue(!res.empty());

	agent->ExecuteCommandLine("stats -T");
	assertTrue(agent->GetLastCommandLineResult());
}

void MiscTests::testWrongAgentWmeFunctions()
{
	sml::Agent* agent2 = 0;
	agent2 = kernel->CreateAgent("soar2");
	assertTrue(agent2 != NULL);

	sml::Identifier* il1 = agent->GetInputLink();
	sml::Identifier* il2 = agent2->GetInputLink();

	sml::Identifier* foo1 = il1->CreateIdWME("foo");
	sml::Identifier* foo2 = il2->CreateIdWME("foo");

	assertTrue(agent->CreateStringWME(foo2, "fail", "fail") == 0);
	assertTrue(agent->CreateIntWME(foo2, "fail", 1) == 0);
	assertTrue(agent->CreateFloatWME(foo2, "fail", 1.0f) == 0);
	assertTrue(agent->CreateIdWME(foo2, "fail") == 0);
	assertTrue(agent->CreateSharedIdWME(foo2, "fail", il1) == 0);
	assertTrue(agent->DestroyWME(foo2) == 0);

	assertTrue(agent2->CreateStringWME(foo1, "fail", "fail") == 0);
	assertTrue(agent2->CreateIntWME(foo1, "fail", 1) == 0);
	assertTrue(agent2->CreateFloatWME(foo1, "fail", 1.0f) == 0);
	assertTrue(agent2->CreateIdWME(foo1, "fail") == 0);
	assertTrue(agent2->CreateSharedIdWME(foo1, "fail", il2) == 0);
	assertTrue(agent2->DestroyWME(foo1) == 0);

	SoarHelper::init_check_to_find_refcount_leaks(agent);
	SoarHelper::init_check_to_find_refcount_leaks(agent2);
	kernel->DestroyAgent(agent2);

}

void MiscTests::testRegression370()
{
	source("testRegression370.soar");
	agent->RunSelf(5000);
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::testRHSRand()
{
	kernel->AddRhsFunction("failed", Handlers::MyRhsFunctionFailureHandler, 0) ;
	source("testRHSRand.soar");
	agent->RunSelf(5000);
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::testMultipleKernels()
{
	sml::Kernel* kernel2 = sml::Kernel::CreateKernelInNewThread(sml::Kernel::kDefaultSMLPort - 1);
	assertTrue(kernel2 != NULL);
	assertTrue_msg(kernel2->GetLastErrorDescription(), !kernel2->HadError());

	sml::Agent* agent2 = kernel2->CreateAgent("soar2");
	assertTrue(agent2 != NULL);

	kernel2->Shutdown();
	delete kernel2;

	agent->ExecuteCommandLine("p s1");
	assertTrue(agent->GetLastCommandLineResult());
}

void MiscTests::testSmemArithmetic()
{
	source("arithmetic.soar") ;
	agent->ExecuteCommandLine("watch 0");
	agent->ExecuteCommandLine("srand 1080");

	agent->RunSelfForever();

	sml::ClientAnalyzedXML stats;
	agent->ExecuteCommandLineXML("stats", &stats);
	assertTrue(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 46436);
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}


void MiscTests::testSource()
{
	source("big.soar");
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::testSoarRand()
{
	int halftrials = 50000000;
	double accum = 0;
	for (int i = 0; i < halftrials * 2; ++i)
	{
		accum += SoarRand();
	}
	double off = (accum - halftrials) / halftrials;
	assertTrue(off < 0.001);
}

void MiscTests::testPreferenceDeallocation()
{
	source("testPreferenceDeallocation.soar");
    SoarHelper::check_learning_override(agent);
	agent->ExecuteCommandLine("run 10");

	sml::ClientAnalyzedXML response;
	agent->ExecuteCommandLineXML("stats", &response);
	assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 6);
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::testStopPhaseRetrieval()
{
    // The debugger relies on this particular XML command return value
	sml::ClientAnalyzedXML response;
	agent->ExecuteCommandLineXML("soar stop-phase", &response);
	assertTrue(response.GetArgInt(sml::sml_Names::kParamPhase, -1) == sml::smlPhase::sml_APPLY_PHASE);
}

void MiscTests::testProductionPrinting()
{
    // Tries to fully exercise Symbol::to_string
    agent->ExecuteCommandLine("sp {foo (state <s> ^superstate nil) --> (<s> ^one 1 ^two 2.0 ^three 3.0e0 ^foo bar ^|| hi ^|<bar>| bar ^|^baz| baz ^1 one ^|2.0| two)}");
    no_agent_assertTrue(agent->GetLastCommandLineResult());
    std::string spMessage = agent->ExecuteCommandLine("print foo");
    no_agent_assertTrue(spMessage == "sp {foo\n    (state <s> ^superstate nil)\n    -->\n    (<s> ^one 1 ^two 2.000000 ^three 3.000000 ^foo bar ^|| hi ^|<bar>| bar\n           ^|^baz| baz ^1 one ^|2.0| two)\n}\n\n");
}

void MiscTests::testSvsSceneCaseInsensitivity()
{
    agent->ExecuteCommandLine("svs --enable");
    no_agent_assertTrue_msg("failed to enable SVS", agent->GetLastCommandLineResult());

    std::string result;
    result = agent->ExecuteCommandLine("svs S1.scene.world");
    no_agent_assertFalse_msg("could not find S1 scene", result == "path not found\n");

    result = agent->ExecuteCommandLine("svs s1.scene.world");
    no_agent_assertFalse_msg("lower-case s1 scene name not found", result == "path not found\n");

    result = agent->ExecuteCommandLine("svs D34.scene.world");
    no_agent_assertTrue_msg("D34 scene name found: " + result, result == "path not found\n");
}

void MiscTests::testLocationPredictionRhs()
{
	source("predict-location.soar");
    agent->RunSelf(1);
    std::string result = agent->ExecuteCommandLine("p s1");
    // Should contain "^new-x-position 17"
    no_agent_assertTrue_msg("predict-x failed: " + result, result.find("^new-x-position 17") != std::string::npos);
    // Should contain ^new-y-position 10
    no_agent_assertTrue_msg("predict-y failed: " + result, result.find("^new-y-position 10") != std::string::npos);
    // Should contain ^predicted-destination southeast-location
    no_agent_assertTrue_msg("compute-closest-intercept failed: " + result, result.find("^predicted-destination southeast-location") != std::string::npos);
}

//void MiscTests::testSoarDebugger()
//{
//	bool result = agent->SpawnDebugger();
//
//	assertTrue(result);
//
//#ifdef _MSC_VER
//	Sleep(10000);
//#else
//	sleep(10);
//#endif
//
//	agent->ExecuteCommandLine("run 10");
//
//	result = agent->KillDebugger();
//
//	assertTrue(result);
//}
