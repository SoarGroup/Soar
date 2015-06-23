//
//  FunctionalTests_EpMem.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FunctionalTests_EpMem.hpp"

#include <cassert>
#include <string>
#include <list>
#include <thread>

#include "SoarHelper.hpp"

void EpMemFunctionalTests::testCountEpMem()
{
	runTest("testCountEpMem", 1693+1);
}


void EpMemFunctionalTests::testHamilton()
{
	runTest("testHamilton", 2+1);
}


void EpMemFunctionalTests::testFilterEpMem()
{
	runTest("testFilterEpMem", 103+1);
}


void EpMemFunctionalTests::testAddCommand()
{
	runTest("testAddCommand", 27+1);
}


void EpMemFunctionalTests::testInclusions()
{
	runTest("testInclusions", 5+1);
}


void EpMemFunctionalTests::testDeliberateStorage()
{
	runTest("testDeliberateStorage", 7+1);
}


void EpMemFunctionalTests::testKB()
{
	runTest("testKB", 246+1);
}


void EpMemFunctionalTests::testSingleStoreRetrieve()
{
	runTest("testSingleStoreRetrieve", 2+1);
}


void EpMemFunctionalTests::testOddEven()
{
	runTest("testOddEven", 12+1);
}


void EpMemFunctionalTests::testBeforeEpMem()
{
	runTest("testBeforeEpMem", 12+1);
}


void EpMemFunctionalTests::testAfterEpMem()
{
	runTest("testAfterEpMem", 12+1);
}


void EpMemFunctionalTests::testAllNegQueriesEpMem()
{
	runTest("testAllNegQueriesEpMem", 12+1);
}


void EpMemFunctionalTests::testBeforeAfterProhibitEpMem()
{
	runTest("testBeforeAfterProhibitEpMem", 12+1);
}


void EpMemFunctionalTests::testMaxDoublePrecision_Irrational()
{
	runTest("testMaxDoublePrecision-Irrational", 4+1);
}


void EpMemFunctionalTests::testMaxDoublePrecisionEpMem()
{
	runTest("testMaxDoublePrecisionEpMem", 4+1);
}


void EpMemFunctionalTests::testNegativeEpisode()
{
	runTest("testNegativeEpisode", 12+1);
}


void EpMemFunctionalTests::testNonExistingEpisode()
{
	runTest("testNonExistingEpisode", 12+1);
}


void EpMemFunctionalTests::testSimpleFloatEpMem()
{
	runTest("testSimpleFloatEpMem", 4+1);
}


void EpMemFunctionalTests::testCyclicQuery()
{
	runTest("testCyclicQuery", 4+1);
}


void EpMemFunctionalTests::testWMELength_OneCycle()
{
	runTest("testWMELength_OneCycle", 4+1);
}


void EpMemFunctionalTests::testWMELength_FiveCycle()
{
	runTest("testWMELength_FiveCycle", 7+1);
}


void EpMemFunctionalTests::testWMELength_InfiniteCycle()
{
	runTest("testWMELength_InfiniteCycle", 12+1);
}


void EpMemFunctionalTests::testWMELength_MultiCycle()
{
	runTest("testWMELength_MultiCycle", 12+1);
}


void EpMemFunctionalTests::testWMActivation_Balance0()
{
	runTest("testWMActivation_Balance0", 5+1);
}


void EpMemFunctionalTests::testEpMemEncodeOutput_NoWMA()
{
	runTest("testEpMemEncodeOutput_NoWMA", 4+1);
}


void EpMemFunctionalTests::testEpMemEncodeOutput_WMA()
{
	runTest("testEpMemEncodeOutput_WMA", 4+1);
}


void EpMemFunctionalTests::testEpMemEncodeSelection_NoWMA()
{
	runTest("testEpMemEncodeSelection_NoWMA", 5+1);
}


void EpMemFunctionalTests::testEpMemEncodeSelection_WMA()
{
	runTest("testEpMemEncodeSelection_WMA", 5+1);
}


void EpMemFunctionalTests::testEpMemYRemoval()
{
	runTest("testYRemoval", 9+1);
}

void EpMemFunctionalTests::testEpMemSoarGroupTests()
{
	runTest("testEpMemSoarGroupTests", 140+1);
}


void EpMemFunctionalTests::readCSoarDB()
{
	agent->InitSoar();
	
	std::string db = SoarHelper::GetResource("epmem-csoar-db.sqlite");
	assertNonZeroSize("No CSoar db!", db);
	agent->ExecuteCommandLine((std::string("epmem --set path ") + db).c_str());
	agent->ExecuteCommandLine("epmem --set append-database on");
	agent->ExecuteCommandLine("epmem --reinit");
	
	std::string actualResult = agent->ExecuteCommandLine("epmem --print 4");
	
	std::string expectedResult = std::string("(<id0> ^counter 2 ^io <id1> ^name Factorization ^needs-factorization true ^number-to-factor 2 ^number-to-factor-int 2 ^operator <id2> ^operator* <id2> ^reward-link <id3> ^superstate nil ^type state ^using-epmem true)\n") +
	                         "(<id1> ^input-link <id5> ^output-link <id4>)\n" +
	                         "(<id2> ^name factor-number ^number-to-factor 2)\n";
	
	
	assertTrue("Unexpected output from CSoar database!", actualResult == expectedResult);
}


void EpMemFunctionalTests::testMultiAgent()
{
	std::vector<std::pair<sml::Agent*, std::thread*>> agents;
	
	auto threadedRun = [](sml::Agent* agent) {
		agent->RunSelf(3+1);
	};
	
	for (int i = 1;i <= 250;i++)
	{
		std::stringstream ss("Agent ");
		ss << i;
		
		sml::Agent* t = kernel->CreateAgent(ss.str().c_str());
		std::string sourceName = getCategoryName() + "_testMultiAgent.soar";
		std::string sourceUrl = SoarHelper::GetResource(sourceName);
		assertNonZeroSize("Could not find test file " + sourceName, sourceUrl);
		agent->ExecuteCommandLine(("source " + sourceUrl).c_str());

		agents.push_back(std::make_pair(t, nullptr));
	}
	
	for (auto p : agents)
	{
		p.second = new std::thread(threadedRun, p.first);
	}
	
	bool allStopped = false;
	while (!allStopped)
	{
		allStopped = true;
		
		for (auto a : agents)
		{
			if (!a.second->joinable())
			{
				allStopped = false;
				break;
			}
		}
	}
	
	for (auto a : agents)
	{
		auto cleanup = [this](std::pair<sml::Agent*, std::thread*> a) {
			a.second->join();
			delete a.second;
			
			kernel->DestroyAgent(a.first);
		};
		
		try
		{
			if (SoarHelper::getDecisionPhasesCount(a.first) != 3)
			{
				throw AssertException("Agent did not stop correctly! Ran too many cycles!");
			}
			
			std::string result = a.first->ExecuteCommandLine("epmem");
			
			if (result.find("native") == std::string::npos)
			{
				throw AssertException("Non Native Driver!");
			}
		}
		catch (AssertException& e)
		{
			cleanup(a);
			throw e;
		}
		
		cleanup(a);
	}
}