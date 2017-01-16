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

void EpMemFunctionalTests::testEpMemSoarGroupTests()
{
	runTest("testEpMemSoarGroupTests", 140);
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
	
	std::string expectedResult =	std::string("========================================\n") +
									std::string("               Episode 4                \n") +
									std::string("========================================\n") +
									std::string("(<id0> ^counter 2 ^io <id1> ^name Factorization ^needs-factorization true ^number-to-factor 2 ^number-to-factor-int 2 ^operator <id2> ^operator* <id2> ^reward-link <id3> ^superstate nil ^type state ^using-epmem true)\n") +
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

void EpMemFunctionalTests::testEpMemUnit()
{
	runTest("epmem_unit", 140);
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
