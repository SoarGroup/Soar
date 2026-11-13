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
#include <fstream>

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
    // Local promoted o-support is no longer an external GDS ground. Removing
    // F1 ^bar therefore preserves S2 instead of forcing two rebuild decisions.
    assertEquals(4, response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1));
    std::string top = agent->ExecuteCommandLine("print S1 -d 3");
    assertTrue(top.find("^count 2") != std::string::npos);
    assertTrue(top.find("^bar bar") == std::string::npos);
    std::string substate = agent->ExecuteCommandLine("print S2 -d 1");
    assertTrue(substate.find("^hello world") != std::string::npos);
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

void MiscTests::testLocationPredictionRhs()
{
	source("predict-location.soar");
    agent->RunSelf(1);
    std::string result = agent->ExecuteCommandLine("p s1");
    // Should contain "^new-x-position 17"
    no_agent_assertTrue_msg("extrapolate-x-position failed: " + result, result.find("^new-x-position 17") != std::string::npos);
    // Should contain ^new-y-position 10
    no_agent_assertTrue_msg("extrapolate-y-position failed: " + result, result.find("^new-y-position 10") != std::string::npos);
    // Should contain ^predicted-destination southeast-location
    no_agent_assertTrue_msg("select-point-closest-to-vector failed: " + result, result.find("^predicted-destination southeast-location") != std::string::npos);
}


void MiscTests::test529JustificationBug()
{
	std::string testName = "529-justification-bug";
	runTestSetup(testName);
	runTestExecute(testName, 11);

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::test494Cd()
{
    agent->ExecuteCommandLine("cd");
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

void MiscTests::testPerceptReplayEscape()
{
	// Test percept replay with escaped spaces and regex patterns
	// Verifies GitHub issue fix where findDelimReplaceEscape function
	// was incorrectly removing backslashes from regex patterns
	
	// Test 1: Simple case
	{
		// Create file just before use
		std::ofstream testFile1("testPerceptReplayEscape_Simple.spr");
		testFile1 << "12345" << std::endl;
		testFile1 << "1 -1 add-wme I2 sensor reading string" << std::endl;
		testFile1 << "1 -2 add-wme I2 value 42 int" << std::endl;
		testFile1.close();
		
		// Load the file
		agent->ExecuteCommandLine("load percepts -o testPerceptReplayEscape_Simple.spr");
		
		// Delete file immediately after loading
		std::remove("testPerceptReplayEscape_Simple.spr");
		
		agent->Commit();
		agent->RunSelf(2);
		
		// Synchronize input link so SML can see the loaded percepts
		agent->SynchronizeInputLink();
		
		// Check input link WMEs directly via SML
		sml::Identifier* pInputLink = agent->GetInputLink();
		
		sml::WMElement* pSensor = pInputLink->FindByAttribute("sensor", 0);
		sml::WMElement* pValue = pInputLink->FindByAttribute("value", 0);
		assertTrue_msg("Should find sensor WME", pSensor != nullptr);
		assertTrue_msg("Should find value WME", pValue != nullptr);
		if (pSensor && pValue) {
			assertTrue_msg("Sensor value should be 'reading'", std::string(pSensor->GetValueAsString()) == "reading");
			assertTrue_msg("Value should be 42", std::string(pValue->GetValueAsString()) == "42");
		}
		
		agent->ExecuteCommandLine("load percepts --close", false, false);
		tearDown(false);
		setUp();
	}
	
	// Test 2: Escaped spaces
	{
		// Create file just before use
		std::ofstream testFile2("testPerceptReplayEscape_Escaped.spr");
		testFile2 << "67890" << std::endl;
		testFile2 << "1 -1 add-wme I2 name\\ with\\ spaces test-value string" << std::endl;
		testFile2 << "1 -2 add-wme I2 status active string" << std::endl;
		testFile2.close();
		
		// Load the file
		agent->ExecuteCommandLine("load percepts --open testPerceptReplayEscape_Escaped.spr");
		
		// Delete file immediately after loading
		std::remove("testPerceptReplayEscape_Escaped.spr");
		
		agent->RunSelf(2, sml::sml_DECIDE);
		agent->Commit();
		
		// Synchronize input link so SML can see the loaded percepts
		agent->SynchronizeInputLink();
		
		// Check that escaped spaces were converted to regular spaces
		sml::Identifier* pInputLink2 = agent->GetInputLink();
		sml::WMElement* pNameSpaces = pInputLink2->FindByAttribute("name with spaces", 0);
		sml::WMElement* pStatus = pInputLink2->FindByAttribute("status", 0);
		assertTrue_msg("Should find 'name with spaces' WME (escaped spaces converted)", pNameSpaces != nullptr);
		assertTrue_msg("Should find status WME", pStatus != nullptr);
		if (pNameSpaces && pStatus) {
			assertTrue_msg("Name with spaces value should be 'test-value'", std::string(pNameSpaces->GetValueAsString()) == "test-value");
			assertTrue_msg("Status should be 'active'", std::string(pStatus->GetValueAsString()) == "active");
		}
		
		agent->ExecuteCommandLine("load percepts --close", false, false);
		tearDown(false);
		setUp();
	}
	
	// Test 3: Regex patterns with backslashes
	{
		// Create file just before use
		std::ofstream testFile3("testPerceptReplayEscape_Regex.spr");
		testFile3 << "11111" << std::endl;
		testFile3 << "1 -1 add-wme I2 pattern \\d+\\w+ string" << std::endl;
		testFile3 << "1 -2 add-wme I2 regex-test \\s*\\w+ string" << std::endl;
		testFile3.close();
		
		// Load the file
		agent->ExecuteCommandLine("load percepts --open testPerceptReplayEscape_Regex.spr");
		
		// Delete file immediately after loading
		std::remove("testPerceptReplayEscape_Regex.spr");
		
		agent->RunSelf(2, sml::sml_DECIDE);
		agent->Commit();
		
		// Synchronize input link so SML can see the loaded percepts
		agent->SynchronizeInputLink();
		
		// Check that regex patterns with backslashes were preserved
		sml::Identifier* pInputLink3 = agent->GetInputLink();
		sml::WMElement* pPattern = pInputLink3->FindByAttribute("pattern", 0);
		sml::WMElement* pRegexTest = pInputLink3->FindByAttribute("regex-test", 0);
		assertTrue_msg("Should find pattern WME", pPattern != nullptr);
		assertTrue_msg("Should find regex-test WME", pRegexTest != nullptr);
		if (pPattern && pRegexTest) {
			std::string patternValue = pPattern->GetValueAsString();
			std::string regexValue = pRegexTest->GetValueAsString();
			// Regex patterns should preserve backslashes
			assertTrue_msg("Pattern should contain backslashes", patternValue.find("\\") != std::string::npos);
			assertTrue_msg("Regex-test should contain backslashes", regexValue.find("\\") != std::string::npos);
			// Specifically check for the regex patterns (they should contain d+ and s*)
			assertTrue_msg("Pattern should contain 'd+'", patternValue.find("d+") != std::string::npos);
			assertTrue_msg("Regex-test should contain 's*'", regexValue.find("s*") != std::string::npos);
		}
		
		agent->ExecuteCommandLine("load percepts --close", false, false);
	}
	
	SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void MiscTests::testGDSCloneSupport()
{
    for (const char* mode : {"never", "always"})
    {
        tearDown(false);
        setUp();
        agent->ExecuteCommandLine("excise --all");
        source("gds-clone-support.soar");
        agent->ExecuteCommandLine((std::string("chunk ") + mode).c_str());
        agent->ExecuteCommandLine("smem --add { (<c> ^handle sample ^item-type predicate ^argnum 1 ^primary-rtype single ^predicate-structure <p>) (<p> ^feature value) }");
        sml::Identifier* query = agent->GetInputLink()->CreateIdWME("concept-query");
        query->CreateStringWME("name", "sample");
        query->CreateStringWME("type", "unary");
        agent->Commit();
        agent->RunSelf(40);
        std::string wm;
        for (const std::string& goal : SoarHelper::getGoalStack(agent))
        {
            wm += agent->ExecuteCommandLine(("print " + goal + " -d 20").c_str());
        }
        for (const char* token : {"^formated predicate", "^handle sample", "^feature value", "^gtype predicate"})
        {
            assertTrue_msg(std::string(mode) + ": retrieval missing " + token, wm.find(token) != std::string::npos);
        }
    }
}

void MiscTests::testGDSIndependentDependencies()
{
    for (const char* mode : {"never", "always"})
    {
        // 18 order/withdrawal controls and 4 shared-result controls.
        for (int order = 0; order < 4; ++order)
        {
            for (int change = 0; change < (order == 3 ? 2 : 3); ++change)
            {
                tearDown(false);
                setUp();
                agent->ExecuteCommandLine("excise --all");
                agent->ExecuteCommandLine((std::string("chunk ") + mode).c_str());
                source(order == 3 ? "gds-shared-support.soar" : "gds-independent-dependencies.soar");
                sml::Identifier* il = agent->GetInputLink();
                sml::StringElement* a = il->CreateStringWME("a", order == 2 ? "no" : "yes");
                sml::StringElement* b = il->CreateStringWME("b", order == 1 ? "no" : "yes");
                il->CreateStringWME("local-gate", "yes");
                agent->Commit();
                agent->RunSelf(3);
                if (order == 1 || order == 2)
                {
                    (order == 1 ? b : a)->Update("yes");
                    agent->Commit();
                    agent->RunSelf(4);
                }
                std::string wm = agent->ExecuteCommandLine("print S2 -d 1");
                if (order == 3)
                {
                    assertTrue(wm.find("^shared yes") != std::string::npos);
                }
                else
                {
                    for (const char* attr : {"^branch-a yes", "^chain-a yes", "^branch-b yes", "^chain-b yes"})
                    {
                        assertTrue_msg(attr, wm.find(attr) != std::string::npos);
                    }
                }
                std::string gds = agent->ExecuteCommandLine("print --gds");
                assertTrue(gds.find("^a yes") != std::string::npos);
                assertTrue(gds.find("^b yes") != std::string::npos);
                if (change < 2)
                {
                    (change == 0 ? a : b)->Update("no");
                }
                else
                {
                    il->CreateStringWME("unrelated", "new");
                }
                agent->Commit();
                agent->RunSelf(1);
                std::string stack = agent->ExecuteCommandLine("print --stack");
                assertTrue_msg(std::string(mode) + ": wrong retraction for order " + std::to_string(order) + ", change " + std::to_string(change),
                    (stack.find("==>S: S2 ") == std::string::npos) == (change < 2));
            }
        }
    }
}

void MiscTests::testGDSPromotedSupport()
{
    for (const char* mode : {"never", "always"})
    {
        tearDown(false);
        setUp();
        agent->ExecuteCommandLine("excise --all");
        agent->ExecuteCommandLine((std::string("chunk ") + mode).c_str());
        source("gds-promoted-o-support.soar");
        sml::StringElement* external = agent->GetInputLink()->CreateStringWME("external", "yes");
        agent->Commit();
        bool cleaned = false;
        for (int i = 0; i < 18 && !cleaned; ++i)
        {
            agent->RunSelf(1);
            std::string wm = agent->ExecuteCommandLine("print S2 -d 2");
            cleaned = wm.find("^cleaned yes") != std::string::npos;
        }
        assertTrue_msg(std::string(mode) + ": local effect cleanup removed S2", cleaned);
        external->Update("no");
        agent->Commit();
        agent->RunSelf(1);
        std::string stack = agent->ExecuteCommandLine("print --stack");
        assertTrue(stack.find("==>S: S2 ") == std::string::npos);

        tearDown(false);
        setUp();
        agent->ExecuteCommandLine("excise --all");
        agent->ExecuteCommandLine((std::string("chunk ") + mode).c_str());
        source("gds-promoted-i-support.soar");
        sml::StringElement* premise = agent->GetInputLink()->CreateStringWME("a", "yes");
        agent->GetInputLink()->CreateStringWME("b", "yes");
        agent->Commit();
        agent->RunSelf(4);
        std::string wm = agent->ExecuteCommandLine("print S2 -d 3");
        assertTrue(wm.find("^answer yes") != std::string::npos);
        premise->Update("no");
        agent->Commit();
        agent->RunSelf(1);
        stack = agent->ExecuteCommandLine("print --stack");
        assertTrue(stack.find("==>S: S2 ") == std::string::npos);
    }
}

void MiscTests::testReteAndSupportDefects()
{
    // #6: Cross-type relational comparisons fail antisymmetrically; 64-bit integer comparisons do not overflow
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("sp {init (state <s> ^superstate nil) --> (<s> ^str foo ^num 5 ^big 9223372036854775807 ^neg -2)}");
    agent->ExecuteCommandLine("sp {bad-lt1 (state <s> ^str { < 5 }) --> (<s> ^cross-lt1 yes)}");
    agent->ExecuteCommandLine("sp {bad-lt2 (state <s> ^num { < foo }) --> (<s> ^cross-lt2 yes)}");
    agent->ExecuteCommandLine("sp {bad-gt (state <s> ^str <x> ^num { > <x> }) --> (<s> ^cross-gt yes)}");
    agent->ExecuteCommandLine("sp {no-overflow (state <s> ^big <b1> ^neg { < <b1> }) --> (<s> ^overflow-fixed yes)}");
    agent->ExecuteCommandLine("sp {overflow-bad (state <s> ^big { < -2 }) --> (<s> ^overflow-bug yes)}");
    agent->RunSelf(1);
    std::string wm = agent->ExecuteCommandLine("print S1 -d 1");
    assertTrue_msg("#6 cross-lt1 matched", wm.find("^cross-lt1") == std::string::npos);
    assertTrue_msg("#6 cross-lt2 matched", wm.find("^cross-lt2") == std::string::npos);
    assertTrue_msg("#6 cross-gt matched", wm.find("^cross-gt") == std::string::npos);
    assertTrue_msg("#6 overflow-fixed missing", wm.find("^overflow-fixed yes") != std::string::npos);
    assertTrue_msg("#6 overflow-bug matched", wm.find("^overflow-bug") == std::string::npos);

    // #10: Mixed operator elaboration + application gets i-support even when warnings are disabled;
    //      operator elaboration in a substate stays i-supported when matched across nested states
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("output warnings --disable");
    agent->ExecuteCommandLine("sp {propose-go (state <s> ^superstate nil) --> (<s> ^operator <o> +) (<o> ^name go)}");
    agent->ExecuteCommandLine("sp {mixed-rule (state <s> ^operator <o>) (<o> ^name go) --> (<o> ^elab 1) (<s> ^mixed-flag yes)}");
    agent->RunSelf(2);
    std::string prefs = agent->ExecuteCommandLine("preferences S1 mixed-flag --names");
    assertTrue_msg("#10 mixed rule got o-support with warnings off: " + prefs, prefs.find(":I") != std::string::npos);

    // #11: User RL rules differing only in numeric preference value are not rejected as duplicates;
    //      rules differing after a leading funcall or in funcall arg count are not rejected
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("sp {rl-a (state <s> ^superstate nil ^operator <o> +) --> (<s> ^operator <o> = 1.0)}");
    std::string res_rl = agent->ExecuteCommandLine("sp {rl-b (state <s> ^superstate nil ^operator <o> +) --> (<s> ^operator <o> = 2.0)}");
    assertTrue_msg("#11 second RL rule rejected as duplicate", res_rl.find("Ignoring") == std::string::npos);
    agent->ExecuteCommandLine("sp {fn-a (state <s> ^superstate nil) --> (write |hi|) (<s> ^a 1)}");
    std::string res_fn = agent->ExecuteCommandLine("sp {fn-b (state <s> ^superstate nil) --> (write |hi|) (<s> ^b 2)}");
    assertTrue_msg("#11 rule after funcall rejected as duplicate", res_fn.find("Ignoring") == std::string::npos);
    agent->ExecuteCommandLine("sp {plus-2 (state <s> ^superstate nil) --> (<s> ^val (+ 1 2))}");
    std::string res_plus = agent->ExecuteCommandLine("sp {plus-3 (state <s> ^superstate nil) --> (<s> ^val (+ 1 2 3))}");
    assertTrue_msg("#11 funcall with extra arg rejected as duplicate", res_plus.find("Ignoring") == std::string::npos);
}

void MiscTests::testRHSMathAndSymbolDefects()
{
    // #12, #13, #21: sum/product with floats, set_reduce error string without bars,
    // select-point-closest-to-vector across multiple slots/incomplete points, and canonical +0.0
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("sp {init (state <s> ^superstate nil) --> "
                              "(<s> ^nums <n> ^pts <p> ^neg-zero (* -1.0 0.0)) "
                              "(<n> ^a 0.5 ^b 2 ^c 1.5) "
                              "(<p> ^pt1 <p1> ^pt2 <p2> ^incomplete <p3>) "
                              "(<p1> ^x 10 ^y 0 ^id east) "
                              "(<p2> ^x 0 ^y 10 ^id north) "
                              "(<p3> ^x 1)}");
    agent->ExecuteCommandLine("sp {compute (state <s> ^superstate nil ^nums <n> ^pts <p>) --> "
                              "(<s> ^total (sum <n>) ^prod (product <n>) "
                              "^bad-set (set-count 123) "
                              "^closest (select-point-closest-to-vector 0 0 90 <p>))}");
    agent->RunSelf(1);
    std::string wm = agent->ExecuteCommandLine("print S1 -d 1");
    assertTrue_msg("#12 sum float failed: " + wm, wm.find("^total 4.") != std::string::npos);
    assertTrue_msg("#12 product float failed: " + wm, wm.find("^prod 1.5") != std::string::npos);
    assertTrue_msg("#13 set-count error has escaped bars: " + wm, wm.find("^bad-set |ERROR: First parameter needs to be an identifier.|") != std::string::npos);
    assertTrue_msg("#13 select-point failed: " + wm, wm.find("^closest east") != std::string::npos);
    assertTrue_msg("#21 negative zero not folded: " + wm, wm.find("-0.0") == std::string::npos);
}

void MiscTests::testIdentifierAttrAndEBCDefects()
{
    // #30: Identifier used only as an attribute is kept while linked and GC'd when unlinked
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("sp {propose-make (state <s> ^superstate nil -^step) --> (<s> ^operator <o> +) (<o> ^name make)}");
    agent->ExecuteCommandLine("sp {apply-make (state <s> ^operator.name make) --> (<s> ^<x> yes ^step 1) (<x> ^tag alive)}");
    agent->ExecuteCommandLine("sp {read-attr-id (state <s> ^<x> yes) (<x> ^tag alive) --> (<s> ^saw-attr-id yes)}");
    agent->RunSelf(2);
    std::string wm = agent->ExecuteCommandLine("print S1 -d 2");
    assertTrue_msg("#30 identifier attribute not reachable: " + wm, wm.find("^saw-attr-id yes") != std::string::npos);

    // #5 & #8: State no-change substate returning a deep-copied structure promotes it and names the chunk *snc* (rule-based naming)
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("chunk always");
    agent->ExecuteCommandLine("chunk naming-style rule");
    agent->ExecuteCommandLine("sp {init (state <s> ^superstate nil) --> (<s> ^src <x>) (<x> ^v 42)}");
    agent->ExecuteCommandLine("sp {sub-propose (state <s> ^superstate <ss> ^impasse no-change ^attribute state) (<ss> ^superstate nil ^src <x>) --> (<s> ^operator <o> +) (<o> ^name copy-it ^src <x>)}");
    agent->ExecuteCommandLine("sp {sub-apply (state <s> ^superstate <ss> ^operator <o> ^impasse no-change ^attribute state) (<o> ^name copy-it ^src <x>) --> (<ss> ^copied (deep-copy <x>))}");
    agent->RunSelf(3);
    std::string chunks = agent->ExecuteCommandLine("print -cf");
    assertTrue_msg("#5 StateNoChange naming missing: " + chunks, chunks.find("*StateNoChange*") != std::string::npos);
    assertTrue_msg("#8 deep-copy child not in chunk: " + chunks, chunks.find("^v 42") != std::string::npos);

    // #20: Top-level OSK preference on a substate returned as a result learns a justification, not a literal-ID chunk
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("chunk always");
    agent->ExecuteCommandLine("sp {return-sub (state <s> ^superstate <ss>) (<ss> ^superstate nil -^sub) --> (<ss> ^sub <s>)}");
    agent->ExecuteCommandLine("sp {sub-propose (state <s> ^superstate <ss>) (<ss> ^superstate nil) --> (<s> ^operator <o1> + <o2> +) (<o1> ^name a) (<o2> ^name b)}");
    agent->ExecuteCommandLine("sp {top-prefer (state <s> ^superstate nil ^sub <sub-s>) (<sub-s> ^operator <o1> + ^operator <o2> +) (<o1> ^name a) (<o2> ^name b) --> (<sub-s> ^operator <o1> > <o2>)}");
    agent->ExecuteCommandLine("sp {sub-apply (state <s> ^superstate <ss> ^operator.name a) --> (<ss> ^done yes)}");
    agent->RunSelf(4);
    std::string learned = agent->ExecuteCommandLine("print -c");
    assertTrue_msg("#20 learned chunk from top-level OSK: " + learned, learned.find("chunk*sub-apply") == std::string::npos);
}

void MiscTests::testRLUpdateAndTemplateDefects()
{
    // #14 & #15: Multiple RL rules supporting an operator each retain their own distinct live preference values;
    //            GQ(lambda) updates the rule's RHS action referent so print --rl reflects the correction
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("rl --set learning on");
    agent->ExecuteCommandLine("rl --set learning-rate 0.5");
    agent->ExecuteCommandLine("rl --set discount-rate 0.9");
    agent->ExecuteCommandLine("sp {propose-op (state <s> ^superstate nil) --> (<s> ^operator <o> +) (<o> ^name act)}");
    agent->ExecuteCommandLine("sp {rl-1 (state <s> ^superstate nil ^operator <o> +) (<o> ^name act) --> (<s> ^operator <o> = 2.0)}");
    agent->ExecuteCommandLine("sp {rl-2 (state <s> ^superstate nil ^operator <o> + -^never) (<o> ^name act) --> (<s> ^operator <o> = 10.0)}");
    agent->ExecuteCommandLine("sp {apply-op (state <s> ^operator.name act ^reward-link <r>) --> (<s> ^done yes) (<r> ^reward.value 0.0)}");
    agent->ExecuteCommandLine("sp {propose-op2 (state <s> ^superstate nil ^done yes) --> (<s> ^operator <o> + >) (<o> ^name next)}");
    agent->ExecuteCommandLine("sp {rl-next (state <s> ^superstate nil ^operator <o> +) (<o> ^name next) --> (<s> ^operator <o> = 0.0)}");
    agent->RunSelf(2);
    std::string prefs = agent->ExecuteCommandLine("preferences S1 operator --names");
    assertTrue_msg("#14 rl-1 and rl-2 live preferences were clobbered to identical values: " + prefs,
                   prefs.find("-1.000000") != std::string::npos && prefs.find("7.000000") != std::string::npos);

    // #28: RL template rules clear m_sym_to_var_map between instantiations so variables do not collide
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("rl --set learning on");
    agent->ExecuteCommandLine("sp {tpl :template (state <s> ^operator <o> +) (<o> ^tag <t>) --> (<s> ^operator <o> = 0.0)}");
    agent->ExecuteCommandLine("sp {top-propose (state <s> ^superstate nil) --> (<s> ^operator <o> +) (<o> ^name top-op ^tag t1)}");
    agent->ExecuteCommandLine("sp {sub-propose (state <s> ^superstate <ss>) (<ss> ^operator.name top-op) --> (<s> ^operator <o> +) (<o> ^name sub-op ^tag t2)}");
    agent->RunSelf(3);
    std::string rules = agent->ExecuteCommandLine("print --rl");
    assertTrue_msg("#28 second template rule not generated: " + rules, rules.find("rl*tpl*2") != std::string::npos);
    std::string tpl2 = agent->ExecuteCommandLine("print rl*tpl*2");
    assertTrue_msg("#28 template rule merged operator variables: " + tpl2, tpl2.find("^tag t1") == std::string::npos);
}

void MiscTests::testSMemEpMemWmaDefects()
{
    // #2 & #3: Two states querying SMem in one output phase and rejecting a status WME do not crash or UAF
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("smem --enable");
    agent->ExecuteCommandLine("smem --add { (<a1> ^type item ^name alpha) (<a2> ^type item ^name beta) }");
    agent->ExecuteCommandLine("sp {top-propose (state <s> ^superstate nil) --> (<s> ^operator <o> +) (<o> ^name wait)}");
    agent->ExecuteCommandLine("sp {both-query (state <s> ^smem.command <c>) --> (<c> ^query <q>) (<q> ^type item)}");
    agent->ExecuteCommandLine("sp {reject-status (state <s> ^smem.result <r>) (<r> ^success <q>) --> (<r> ^success <q> -)}");
    agent->RunSelf(3);
    std::string s1 = agent->ExecuteCommandLine("print S1 -d 3");
    assertTrue_msg("#2/#3 SMem dual-state query failed", s1.find("^retrieved") != std::string::npos);

    // #7 & #17 & S1: Deterministic smem --add LTI numbering, comma-separated base-incremental-threshes,
    //                and store-new with link-to-new-ltm yes allocating a fresh LTI
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("smem --enable");
    agent->ExecuteCommandLine("smem --set base-incremental-threshes 1,3,7");
    std::string threshes = agent->ExecuteCommandLine("smem --get base-incremental-threshes");
    assertTrue_msg("#17 base-incremental-threshes list parse failed: " + threshes,
                   threshes.find("1") != std::string::npos && threshes.find("3") != std::string::npos && threshes.find("7") != std::string::npos && threshes.find("10") == std::string::npos);
    agent->ExecuteCommandLine("sp {init-store (state <s> ^superstate nil ^smem.command <c>) -{(<s> ^stored)} --> (<s> ^item <x> ^stored 1) (<x> ^val 1) (<c> ^store <x>)}");
    agent->ExecuteCommandLine("sp {store-new-relink (state <s> ^superstate nil ^stored 1 -^stored2 ^item <x> ^smem.command <c> ^smem.result.success) --> "
                              "(<s> ^stored2 1) (<x> ^val 1 - ^val 2) (<c> ^store <x> - ^store-new <x> ^link-to-new-ltm yes)}");
    agent->RunSelf(3);
    std::string ltm = agent->ExecuteCommandLine("print @");
    assertTrue_msg("S1 store-new with link-to-new-ltm yes overwrote @1 instead of creating @2: " + ltm,
                   ltm.find("@1") != std::string::npos && ltm.find("@2") != std::string::npos && ltm.find("^val 1") != std::string::npos && ltm.find("^val 2") != std::string::npos);

    // #23: WMA with petrov-approx on and decay-rate 1 does not produce NaN / immediate forgetting
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("wma --set activation on");
    agent->ExecuteCommandLine("wma --set petrov-approx on");
    agent->ExecuteCommandLine("wma --set decay-rate 1");
    agent->ExecuteCommandLine("sp {propose-init (state <s> ^superstate nil -^item) --> (<s> ^operator <o> +) (<o> ^name init-item)}");
    agent->ExecuteCommandLine("sp {apply-init (state <s> ^operator.name init-item) --> (<s> ^item <x>) (<x> ^v 1)}");
    agent->ExecuteCommandLine("sp {touch-item (state <s> ^item <x>) (<x> ^v 1) --> (<s> ^operator <o> + =) (<o> ^name step)}");
    agent->RunSelf(15);
    std::string wm_wma = agent->ExecuteCommandLine("print S1 -d 2");
    assertTrue_msg("#23 WMA decay-rate 1 lost item: " + wm_wma, wm_wma.find("^v 1") != std::string::npos);

    // #18 & #31: EpMem preserves STI status on earlier episodes after later SMem store,
    //            and graph-match backtracking restores shared bound_nodes
    tearDown(false);
    setUp();
    agent->ExecuteCommandLine("excise --all");
    agent->ExecuteCommandLine("smem --enable");
    agent->ExecuteCommandLine("epmem --set learning on");
    agent->ExecuteCommandLine("epmem --set trigger dc");
    agent->ExecuteCommandLine("sp {s1-create (state <s> ^superstate nil -^step) --> (<s> ^step 1 ^holder <h>) (<h> ^tag pre-lti)}");
    agent->ExecuteCommandLine("sp {s2-store (state <s> ^step 1 ^holder <h> ^smem.command <sc>) --> (<s> ^step 1 - ^step 2) (<sc> ^store <h>)}");
    agent->ExecuteCommandLine("sp {s3-remove (state <s> ^step 2 ^holder <h> ^smem.command <sc>) --> (<s> ^step 2 - ^step 3 ^holder <h> -) (<sc> ^store <h> -)}");
    agent->ExecuteCommandLine("sp {s4-retrieve (state <s> ^step 3 ^epmem.command <ec>) --> (<s> ^step 3 - ^step 4) (<ec> ^retrieve 2)}");
    agent->RunSelf(5);
    std::string ep_ret = agent->ExecuteCommandLine("print S1 -d 4");
    assertTrue_msg("#18 EpMem rewrote episode 2 STI into @1: " + ep_ret, ep_ret.find("(@1)") == std::string::npos);
}

void MiscTests::testSanitizerAndPushArchitecture()
{
    // Finding #3: CLI settings/stats/timers get_cstring() allocator consistency
    std::string wma_out = agent->ExecuteCommandLine("wm activation");
    assertTrue_msg("wm activation summary failed", wma_out.find("Activation") != std::string::npos);
    std::string wma_stats = agent->ExecuteCommandLine("wm activation --stats");
    assertTrue_msg("wm activation --stats failed", wma_stats.find("Forgotten WMEs") != std::string::npos);
    std::string wma_timers = agent->ExecuteCommandLine("wm activation --timers");
    assertTrue_msg("wm activation --timers failed", !wma_timers.empty());
    std::string rl_out = agent->ExecuteCommandLine("rl");
    assertTrue_msg("rl summary failed", rl_out.find("Soar-RL learning") != std::string::npos);
    std::string smem_out = agent->ExecuteCommandLine("smem ?");
    assertTrue_msg("smem ? failed", smem_out.find("Semantic Memory") != std::string::npos);

    // Issue #582: Multi-agent output log does not deny XML/command output on second agent
    sml::Agent* agent2 = kernel->CreateAgent("second-agent");
    assertTrue_msg("Failed to create second-agent", agent2 != nullptr);
    agent->ExecuteCommandLine("output log test_agent1_log.txt");
    std::string s1_agent1 = agent->ExecuteCommandLine("print S1 -d 1");
    std::string s1_agent2 = agent2->ExecuteCommandLine("print S1 -d 1");
    agent->ExecuteCommandLine("output log --close");
    std::remove("test_agent1_log.txt");
    assertTrue_msg("#582 agent1 print S1 was empty when output log active", s1_agent1.find("S1") != std::string::npos);
    assertTrue_msg("#582 agent2 print S1 was denied when agent1 had output log open: " + s1_agent2,
                   s1_agent2.find("S1") != std::string::npos);
    kernel->DestroyAgent(agent2);

    // Part II: CreateKernelInNewThread + CreateRemoteConnection push/wait wakeup
    sml::Kernel* asyncKernel = sml::Kernel::CreateKernelInNewThread(12345);
    assertTrue_msg("Failed to create asyncKernel on port 12345", asyncKernel != nullptr);
    assertFalse_msg("asyncKernel had creation error", asyncKernel->HadError());
    sml::Agent* asyncAgent = asyncKernel->CreateAgent("async-agent");
    assertTrue_msg("Failed to create async-agent", asyncAgent != nullptr);
    std::string echoEmbedded = asyncAgent->ExecuteCommandLine("echo hello-embedded");
    assertTrue_msg("Async embedded command failed: " + echoEmbedded, echoEmbedded.find("hello-embedded") != std::string::npos);

    sml::Kernel* remoteClient = sml::Kernel::CreateRemoteConnection(true, "127.0.0.1", 12345);
    assertTrue_msg("Failed to connect remoteClient to port 12345", remoteClient != nullptr);
    assertFalse_msg("remoteClient had connection error: " + std::string(remoteClient->GetLastErrorDescription()),
                    remoteClient->HadError());
    sml::Agent* remoteAgent = remoteClient->GetAgent("async-agent");
    assertTrue_msg("remoteClient could not find async-agent", remoteAgent != nullptr);
    std::string echoRemote = remoteAgent->ExecuteCommandLine("print S1 -d 1");
    assertTrue_msg("Remote debugger-style print S1 failed: " + echoRemote, echoRemote.find("S1") != std::string::npos);

    remoteClient->Shutdown();
    delete remoteClient;
    asyncKernel->Shutdown();
    delete asyncKernel;
}

