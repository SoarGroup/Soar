//
//  FunctionalTests.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FunctionalTests.hpp"

#include "SoarHelper.hpp"

void FunctionalTests::testWaterJug()
{
    runTest("testWaterJug", -1);
}
void FunctionalTests::testWaterJugLookAhead()
{
    runTest("testWaterJugLookAhead", -1);
}
void FunctionalTests::testWaterJugHierarchy()
{
    runTest("testWaterJugHierarchy", -1);
}

void FunctionalTests::testTowersOfHanoi()
{
    runTest("testTowersOfHanoi", 2048);
}

void FunctionalTests::testTowersOfHanoiFast()
{
    runTest("testTowersOfHanoiFast", 2047);
}

void FunctionalTests::testEightPuzzle()
{
    runTest("testEightPuzzle", -1);
}

void FunctionalTests::testBlocksWorld()
{
	runTest("testBlocksWorld", -1);
}

void FunctionalTests::testBlocksWorldOperatorSubgoaling()
{
	runTest("testBlocksWorldOperatorSubgoaling", 5);
}

void FunctionalTests::testBlocksWorldLookAhead()
{
	std::string testName = "testBlocksWorldLookAhead";
	runTestSetup(testName);
	agent->ExecuteCommandLine("srand 1");
	runTestExecute(testName, 27);
}

void FunctionalTests::testBlocksWorldLookAhead2()
{
	std::string testName = "testBlocksWorldLookAhead";
	runTestSetup(testName);
	agent->ExecuteCommandLine("srand 100000000002");
	runTestExecute(testName, 29);
}

void FunctionalTests::testBlocksWorldLookAheadRandom()
{
	runTest("testBlocksWorldLookAhead", -1);
}

void FunctionalTests::testArithmetic()
{
	runTest("testArithmetic", -1);
	assertTrue(SoarHelper::getD_CYCLE_COUNT(agent) > 40000);
}

void FunctionalTests::testCountTest()
{
	runTest("testCountTest", 45047);
	assertEquals(42, SoarHelper::getUserProductionCount(agent));
	assertEquals(15012, SoarHelper::getChunkProductionCount(agent));
	assertEquals(115136, SoarHelper::getE_CYCLE_COUNT(agent));
	assertEquals(40039, SoarHelper::getPE_CYCLE_COUNT(agent));
	assertEquals(120146, SoarHelper::getINNER_E_CYCLE_COUNT(agent));
}
