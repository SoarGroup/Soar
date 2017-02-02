//
//  EpMemFunctionalTests.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "EpMemFunctionalTests.hpp"

#include <cassert>
#include <string>
#include <list>
#include <thread>

#include "SoarHelper.hpp"

void EpMemFunctionalTests::tearDown(bool caught)
{
    SoarHelper::init_check_to_find_refcount_leaks(agent);
    FunctionalTestHarness::tearDown(caught);
}

void EpMemFunctionalTests::testCountEpMem()
{
	runTest("testCountEpMem", 1693);
}

void EpMemFunctionalTests::testHamilton()
{
	runTest("testHamilton", 2);
}

void EpMemFunctionalTests::testKB()
{
	runTest("testKB", 246);
}

void EpMemFunctionalTests::testSingleStoreRetrieve()
{
	runTest("testSingleStoreRetrieve", 2);
}

void EpMemFunctionalTests::testOddEven()
{
	runTest("testOddEven", 12);
}

void EpMemFunctionalTests::testBeforeEpMem()
{
	runTest("testBeforeEpMem", 12);
}

void EpMemFunctionalTests::testAfterEpMem()
{
	runTest("testAfterEpMem", 12);
}

void EpMemFunctionalTests::testAllNegQueriesEpMem()
{
	runTest("testAllNegQueriesEpMem", 12);
}

void EpMemFunctionalTests::testBeforeAfterProhibitEpMem()
{
	runTest("testBeforeAfterProhibitEpMem", 12);
}

void EpMemFunctionalTests::testMaxDoublePrecision_Irrational()
{
	runTest("testMaxDoublePrecision-Irrational", 4);
}

void EpMemFunctionalTests::testMaxDoublePrecisionEpMem()
{
	runTest("testMaxDoublePrecisionEpMem", 4);
}

void EpMemFunctionalTests::testNegativeEpisode()
{
	runTest("testNegativeEpisode", 12);
}

void EpMemFunctionalTests::testNonExistingEpisode()
{
	runTest("testNonExistingEpisode", 12);
}

void EpMemFunctionalTests::testSimpleFloatEpMem()
{
	runTest("testSimpleFloatEpMem", 4);
}

void EpMemFunctionalTests::testCyclicQuery()
{
	runTest("testCyclicQuery", 4);
}

void EpMemFunctionalTests::testWMELength_OneCycle()
{
	runTest("testWMELength_OneCycle", 4);
}

void EpMemFunctionalTests::testWMELength_FiveCycle()
{
	runTest("testWMELength_FiveCycle", 7);
}

void EpMemFunctionalTests::testWMELength_InfiniteCycle()
{
	runTest("testWMELength_InfiniteCycle", 12);
}

void EpMemFunctionalTests::testWMELength_MultiCycle()
{
	runTest("testWMELength_MultiCycle", 12);
}

void EpMemFunctionalTests::testWMActivation_Balance0()
{
	runTest("testWMActivation_Balance0", 5);
}

void EpMemFunctionalTests::testEpMemEncodeOutput_NoWMA()
{
	runTest("testEpMemEncodeOutput_NoWMA", 4);
}

void EpMemFunctionalTests::testEpMemEncodeOutput_WMA()
{
	runTest("testEpMemEncodeOutput_WMA", 4);
}

void EpMemFunctionalTests::testEpMemEncodeSelection_NoWMA()
{
	runTest("testEpMemEncodeSelection_NoWMA", 5);
}

void EpMemFunctionalTests::testEpMemEncodeSelection_WMA()
{
	runTest("testEpMemEncodeSelection_WMA", 5);
}

void EpMemFunctionalTests::testEpMemYRemoval()
{
	runTest("testYRemoval", 9);
}

void EpMemFunctionalTests::testReadCSoarDB()
{
	agent->InitSoar();
	
	std::string db = SoarHelper::GetResource("epmem-csoar-db.sqlite");
	assertNonZeroSize_msg("No CSoar db!", db);
	std::string pathCommand = agent->ExecuteCommandLine(std::string("epmem --set path \"" + db + "\"").c_str());
	std::string memoryCommand = agent->ExecuteCommandLine("epmem --set database file");
	std::string appendCommand = agent->ExecuteCommandLine("epmem --set append on");
	std::string initCommand = agent->ExecuteCommandLine("epmem --init");
	
	std::string actualResult = agent->ExecuteCommandLine("epmem --print 4");
	
	std::string expectedResult =
	    std::string("========================================\n") +
	    std::string("               Episode 4                \n") +
	    std::string("========================================\n") +
	    std::string("(<id0> ^counter 2 ^io <id1> ^name Factorization ^needs-factorization true ^number-to-factor 2 ^number-to-factor-int 2 ^operator <id2> ^operator* <id2> ^reward-link <id3> ^superstate nil ^type state)\n") +
	    std::string("(<id1> ^input-link <id5> ^output-link <id4>)\n") +
	    std::string("(<id2> ^name factor-number ^number-to-factor 2)\n\n");
	assertTrue_msg("Unexpected output from CSoar database!", actualResult == expectedResult);
}


void EpMemFunctionalTests::testMultiAgent()
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

void EpMemFunctionalTests::testHamiltonian()
{
	runTest("hamiltonian", 2);
}

void EpMemFunctionalTests::testSVS()
{
	runTest("svs", 2);
}

void EpMemFunctionalTests::testSVSHard()
{
	runTest("svs_hard", 2);
}

void EpMemFunctionalTests::testEpmemUnit_1()
{
    runTest("epmem_unit_test_1", 113);
}

void EpMemFunctionalTests::testEpmemUnit_2()
{
    runTest("epmem_unit_test_2", 113);
}

void EpMemFunctionalTests::testEpmemUnit_3()
{
    runTest("epmem_unit_test_3", 113);
}

void EpMemFunctionalTests::testEpmemUnit_4()
{
    runTest("epmem_unit_test_4", 113);
}

void EpMemFunctionalTests::testEpmemUnit_5()
{
    runTest("epmem_unit_test_5", 113);
}

void EpMemFunctionalTests::testEpmemUnit_6()
{
    runTest("epmem_unit_test_6", 113);
}

void EpMemFunctionalTests::testEpmemUnit_7()
{
    runTest("epmem_unit_test_7", 113);
}

void EpMemFunctionalTests::testEpmemUnit_8()
{
    runTest("epmem_unit_test_8", 113);
}

void EpMemFunctionalTests::testEpmemUnit_9()
{
    runTest("epmem_unit_test_9", 113);
}

void EpMemFunctionalTests::testEpmemUnit_10()
{
    runTest("epmem_unit_test_10", 113);
}

void EpMemFunctionalTests::testEpmemUnit_11()
{
    runTest("epmem_unit_test_11", 113);
}

void EpMemFunctionalTests::testEpmemUnit_12()
{
    runTest("epmem_unit_test_12", 113);
}

void EpMemFunctionalTests::testEpmemUnit_13()
{
    runTest("epmem_unit_test_13", 113);
}

void EpMemFunctionalTests::testEpmemUnit_14()
{
    runTest("epmem_unit_test_14", 113);
}

void EpMemFunctionalTests::testEpMemSmemFactorizationCombinationTest()
{
    runTestSetup("testSMemEpMemFactorization");
    agent->RunSelf(100);
    std::string actualResultSMem = agent->ExecuteCommandLine("p @");
    std::string expectedResultSMem = "(@1 ^complete true ^factor @2 ^number 3 [+5.000])\n(@2 ^multiplicity 1 ^value 3 [+6.000])\n(@3 ^complete true ^factor @4 ^number 5 [+3.000])\n(@4 ^multiplicity 1 ^value 5 [+4.000])\n(@5 ^complete true ^factor @6 ^number 7 [+7.000])\n(@6 ^multiplicity 1 ^value 7 [+8.000])\n";

    assertTrue_msg(std::string("Unexpected output from SMem:\n") + actualResultSMem, actualResultSMem == expectedResultSMem);

#ifndef NO_SVS
    std::string actualResultEpMem = agent->ExecuteCommandLine("epmem --print 97");
    std::string expectedResultEpMem = "========================================\n               Episode 97               \n========================================\n(<id0> ^counter 7 ^factorization-object @F17 ^has-factorization-object true ^has-factorization-object-complete true ^io <id1> ^name Factorization ^needs-factorization true ^number-to-factor 7 ^number-to-factor-int 7 ^operator* <id3> <id10> ^reward-link <id4> ^superstate nil ^svs <id2> ^type state)\n(<id1> ^input-link <id6> ^output-link <id5>)\n(<id2> ^command <id8> ^spatial-scene <id7>)\n(<id3> ^name factor-number ^number-to-factor 7)\n(<id7> ^id world)\n(<id10> ^factorization-object @F17 ^name check)\n(@F17 ^complete true ^factor @F18 ^number 7)\n(@F18 ^multiplicity 1 ^value 7)\n\n";
    assertTrue_msg(std::string("Unexpected output from EpMem:\n") + actualResultEpMem, actualResultEpMem == expectedResultEpMem);
#endif
}

