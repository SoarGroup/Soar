//
//  SMemFunctionalTests.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "SMemFunctionalTests.hpp"

#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"
#include "SoarHelper.hpp"
#include "sml_Utils.h"
#include "sml_Client.h"
#include "sml_Names.h"

void SMemFunctionalTests::setUp()
{
    FunctionalTestHarness::setUp();
}

void SMemFunctionalTests::tearDown(bool caught)
{
    SoarHelper::init_check_to_find_refcount_leaks(agent);
    FunctionalTestHarness::tearDown(caught);
}

void SMemFunctionalTests::testSimpleCueBasedRetrieval()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleCueBasedRetrieval", 1);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleNonCueBasedRetrieval", 2);
}

void SMemFunctionalTests::testSimpleStore()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleStore", 2);
}

void SMemFunctionalTests::testTrivialMathQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testTrivialMathQuery", 2);
}

void SMemFunctionalTests::testBadMathQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testBadMathQuery", 2);
}

void SMemFunctionalTests::testMaxQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMax", 1);
}

void SMemFunctionalTests::testMaxMixedTypes()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMaxMixedTypes", 1);
}

void SMemFunctionalTests::testMaxMultivalued()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMaxMultivalued", 1);
}

void SMemFunctionalTests::testMin()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMin", 1);
}

void SMemFunctionalTests::testMaxNegQuery()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testMaxNegation", 1);
}

void SMemFunctionalTests::testGreater()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testGreater", 1);
}

void SMemFunctionalTests::testLess()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLess", 1);
}

void SMemFunctionalTests::testGreaterOrEqual()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testGreaterOrEqual", 1);
}

void SMemFunctionalTests::testLessOrEqual()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLessOrEqual", 1);
}

void SMemFunctionalTests::testLessWithNeg()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLessWithNeg", 1);
}

void SMemFunctionalTests::testLessNoSolution()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testLessNoSolution", 1);
}

void SMemFunctionalTests::testSimpleStoreMultivaluedAttribute()
{
	SoarHelper::setStopPhase(agent, SoarHelper::StopPhase::OUTPUT);
	runTest("testSimpleStoreMultivaluedAttribute", 2);
}

void SMemFunctionalTests::testSimpleFloat()
{
	runTest("testSimpleFloat", 5);
}

void SMemFunctionalTests::testMaxDoublePrecision_Irrational()
{
	runTest("testMaxDoublePrecision-Irrational", 5);
}

void SMemFunctionalTests::testMaxDoublePrecision()
{
	runTest("testMaxDoublePrecision", 5);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrievalOfNonExistingLTI()
{
	runTest("testSimpleNonCueBasedRetrievalOfNonExistingLTI", 1);
}

void SMemFunctionalTests::testNegQuery()
{
	runTest("testNegQuery", 248);
}

void SMemFunctionalTests::testNegStringFloat()
{
	runTest("testNegStringFloat", 5);
}

void SMemFunctionalTests::testNegQueryNoHash()
{
	runTest("testNegQueryNoHash", 1);
}

void SMemFunctionalTests::testCueSelection()
{
	runTestSetup("testCueSelection");

	std::string result = agent->RunSelf(2);

	agent::BasicWeightedCue* bwc = internal_agent->lastCue;
	assertTrue_msg("Incorrect cue selected",
			   bwc &&
			   (std::string(bwc->cue->attr->to_string()) == "name") &&
			   (std::string(bwc->cue->value->to_string()) == "val") &&
			   bwc->weight == 4);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationRecency()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationRecency");
	std::string result, expected;
	//result = agent->ExecuteCommandLine("srand 23");
	result  = agent->RunSelf(3);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationRecency functional test did not halt", halted);

    result = agent->ExecuteCommandLine("print @1 -d 1");
    expected = "(@1 ^location @2 ^name foo [+1.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @2 -d 1");
    expected = "(@2 ^x 1 ^y 2 ^z 3 [+2.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @3 -d 1");
    expected = "(@3 ^location @4 ^name bar [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @4 -d 1");
    expected = "(@4 ^x 2 ^y 3 ^z 1 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery");

    std::string result, expected;
	result = agent->RunSelf(3);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery functional test did not halt", halted);

    result = agent->ExecuteCommandLine("print @1 -d 1");
    expected = "(@1 ^location @2 ^name foo [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @2 -d 1");
    expected = "(@2 ^x 1 ^y 2 ^z 3 [+1.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @3 -d 1");
    expected = "(@3 ^location @4 ^name bar [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @4 -d 1");
    expected = "(@4 ^x 2 ^y 3 ^z 1 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);

}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationFrequency()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationFrequency");

	std::string result, expected;
	result = agent->RunSelf(3);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationFrequency functional test did not halt", halted);

    result = agent->ExecuteCommandLine("print @1 -d 1");
    expected = "(@1 ^location @2 ^name foo [+1.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @2 -d 1");
    expected = "(@2 ^x 1 ^y 2 ^z 3 [+1.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @3 -d 1");
    expected = "(@3 ^location @4 ^name bar [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @4 -d 1");
    expected = "(@4 ^x 2 ^y 3 ^z 1 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable");

	//agent->RunSelf(6);
    //std::string test_smem = agent->ExecuteCommandLine("soar init");
    agent->RunSelf(6);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable functional test did not halt", halted);

    std::string result, expected;
	result = agent->ExecuteCommandLine("print @1 -d 1");
    expected = "(@1 ^location @2 ^name foo [-0.461])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @2 -d 1");
    expected = "(@2 ^x 1 ^y 2 ^z 3 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @3 -d 1");
    expected = "(@3 ^location @4 ^name bar [-0.461])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @4 -d 1");
    expected = "(@4 ^x 2 ^y 3 ^z 1 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive");

	agent->RunSelf(6);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive functional test did not halt", halted);

    std::string result, expected;
    result = agent->ExecuteCommandLine("print @1 -d 1");
    expected = "(@1 ^location @2 ^name foo [-0.374])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @2 -d 1");
    expected = "(@2 ^x 1 ^y 2 ^z 3 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @3 -d 1");
    expected = "(@3 ^location @4 ^name bar [-0.881])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @4 -d 1");
    expected = "(@4 ^x 2 ^y 3 ^z 1 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
}

void SMemFunctionalTests::testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental()
{
	runTestSetup("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental");

	std::string result, expected;
	//result = agent->ExecuteCommandLine("srand 23");
	result = agent->RunSelf(6);

	assertTrue_msg("testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental functional test did not halt", halted);

    result = agent->ExecuteCommandLine("print @1 -d 1");
    expected = "(@1 ^location @2 ^name foo [-0.374])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @2 -d 1");
    expected = "(@2 ^x 1 ^y 2 ^z 3 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @3 -d 1");
    expected = "(@3 ^location @4 ^name bar [-1.005])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
    result = agent->ExecuteCommandLine("print @4 -d 1");
    expected = "(@4 ^x 2 ^y 3 ^z 1 [+0.000])\n";
    assertTrue_msg(std::string("Activation value ") + expected + std::string(" != " + result), result == expected);
}

void SMemFunctionalTests::testSpreadingActivation_AlphabetAgentAllOn()
{
    SoarHelper::start_log(agent, "testSpreadingActivation_AlphabetAgentAllOn");
    runTestSetup("testSpreadingActivation_AlphabetAgentAllOn");
    agent->ExecuteCommandLine("srand 480");
    agent->RunSelf(1650);
    sml::ClientAnalyzedXML stats;
    agent->ExecuteCommandLineXML("stats", &stats);
    std::string dc_count(std::to_string(stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1)));
    std::string msg;
    SoarHelper::close_log(agent);
    assertTrue_msg(msg.append("testSpreadingActivation_AlphabetAgentAllOn halted too early. Letters were likely skipped. DC = ").append(dc_count).c_str(),stats.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1649);
    assertTrue_msg(msg.append("testSpreadingActivation_AlphabetAgentAllOn functional test did not halt. DC = ").append(dc_count).c_str(), halted);
}

void SMemFunctionalTests::testDbBackupAndLoadTests()
{
	runTestSetup("testFactorization");

	std::string result, expected;
	agent->RunSelf(1223);
	result = agent->ExecuteCommandLine("p s1 -i -d 1");

	expected =  std::string("(9639: S1 ^counter 50)\n") +
                std::string("(4: S1 ^epmem E1)\n") +
                std::string("(11: S1 ^io I1)\n") +
                std::string("(17: S1 ^name Factorization)\n") +
                std::string("(9809: S1 ^operator O1385)\n") +
                std::string("(9808: S1 ^operator O1385 +)\n") +
                std::string("(3: S1 ^reward-link R1)\n") +
                std::string("(8: S1 ^smem L1)\n") +
                std::string("(2: S1 ^superstate nil)\n") +
                std::string("(1: S1 ^type state)\n") +
                std::string("(20: S1 ^using-smem true)\n");

#ifndef _WIN32
    assertTrue_msg("Didn't stop where expected!", result == expected);
#endif

    agent->ExecuteCommandLine("smem --backup backup.sqlite");
	agent->ExecuteCommandLine("smem --clear");

	result = agent->ExecuteCommandLine("print @");
	assertTrue_msg("smem --clear didn't empty long-term memory!", result.length() == 0);

    agent->ExecuteCommandLine("soar init");
	result = agent->ExecuteCommandLine("p s1 -i -d 1");
    expected =  std::string("(4: S1 ^epmem E1)\n") +
                std::string("(11: S1 ^io I1)\n") +
                std::string("(3: S1 ^reward-link R1)\n") +
                std::string("(8: S1 ^smem L1)\n") +
                std::string("(2: S1 ^superstate nil)\n") +
                std::string("(1: S1 ^type state)\n");
#ifndef _WIN32
	assertTrue_msg("soar init didn't reinit WM!", result == expected);
#endif

	agent->ExecuteCommandLine("smem --set database file");
    agent->ExecuteCommandLine("smem --set path backup.sqlite");
    agent->ExecuteCommandLine("smem --init");

	result = agent->RunSelf(2278);
	assertTrue_msg(std::string("After loading backup db and running, agent did not halt."), halted);

	result = agent->ExecuteCommandLine("p @197 -d 1");
    expected = "(@197 ^complete true ^factor @48 @198 ^number 100 [+573.000])\n";
	assertTrue_msg("testFactorization: Test did not get the correct result!", result == result);
	
	std::string pwd = agent->ExecuteCommandLine("pwd");
	remove((pwd + "/backup.sqlite").c_str());
}

void SMemFunctionalTests::testReadCSoarDB()
{
	agent->InitSoar();

	std::string db = SoarHelper::GetResource("smem-csoar-db.sqlite");
	assertNonZeroSize_msg("No CSoar db!", db);
	agent->ExecuteCommandLine(std::string("smem --set path \"" + db + "\"").c_str());
	agent->ExecuteCommandLine("smem --set database file");
	agent->ExecuteCommandLine("smem --set append on");
	agent->ExecuteCommandLine("smem --init");

	std::string actualResult = agent->ExecuteCommandLine("print @");
	//std::string actualResult = agent->ExecuteCommandLine("print @1 -d 1");

	std::string expectedResult = "(@1 ^complete true ^factor @2 ^number 2 [+0.000])\n(@2 ^multiplicity 1 ^value 2 [+0.000])\n(@3 ^complete true ^factor @4 ^number 3 [+0.000])\n(@4 ^multiplicity 1 ^value 3 [+0.000])\n(@5 ^complete true ^factor @6 ^number 4 [+0.000])\n(@6 ^multiplicity 2 ^value 2 [+0.000])\n";

	assertTrue_msg("Unexpected output from CSoar database!", actualResult == expectedResult);
}

void SMemFunctionalTests::testMultiAgent()
{
	std::vector<sml::Agent*> agents;

	for (int i = 1;i <= 250;i++)
	{
		std::stringstream ss("Agent ");
		ss << i;

		sml::Agent* t = kernel->CreateAgent(ss.str().c_str());
		std::string sourceName = getCategoryName() + "_testMultiAgent.soar";
		std::string sourceUrl = SoarHelper::GetResource(sourceName);
		assertNonZeroSize_msg("Could not find test file " + sourceName, sourceUrl);
		agent->ExecuteCommandLine(("source " + sourceUrl).c_str());

		agents.push_back(t);
	}

	kernel->RunAllAgents(3);

	for (auto a : agents)
	{
		try
		{
			if (SoarHelper::getDecisionPhasesCount(a) != 3)
			{
				throw SoarAssertionException("Agent did not stop correctly! Ran too many cycles!", __FILE__, __LINE__);
			}

			std::string result = a->ExecuteCommandLine("epmem");

			if (result.find("memory") == std::string::npos)
			{
				throw SoarAssertionException("Non Memory Driver!", __FILE__, __LINE__);
			}
		}
		catch (SoarAssertionException& e)
		{
			kernel->DestroyAgent(a);
			throw e;
		}

		kernel->DestroyAgent(a);
	}
}

void SMemFunctionalTests::testISupport()
{
	runTest("smem-i-support", 6);
}

void SMemFunctionalTests::testISupportWithLearning()
{
	std::string result = agent->ExecuteCommandLine("chunk always") ;
	runTest("smem-i-support", 6);
}
