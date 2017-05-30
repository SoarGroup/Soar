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
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
void FunctionalTests::testWaterJugHierarchy()
{
    runTest("testWaterJugHierarchy", -1);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testTowersOfHanoi()
{
    runTest("testTowersOfHanoi", 2048);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testTowersOfHanoiFast()
{
    runTest("testTowersOfHanoiFast", 2047);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testEightPuzzle()
{
    runTest("testEightPuzzle", -1);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testBlocksWorld()
{
	runTest("testBlocksWorld", -1);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testBlocksWorldOperatorSubgoaling()
{
	runTest("testBlocksWorldOperatorSubgoaling", 5);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testBlocksWorldLookAhead()
{
	std::string testName = "testBlocksWorldLookAhead";
	runTestSetup(testName);
	agent->ExecuteCommandLine("srand 1");
	runTestExecute(testName, -1);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testArithmetic()
{
	runTest("testArithmetic", -1);
	assertTrue(SoarHelper::getD_CYCLE_COUNT(agent) > 40000);
    if (SoarHelper::save_after_action_report)
    {
        SoarHelper::agent_command(agent,"explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FunctionalTests::testCountTest()
{
	runTest("testCountTest", 25042);
	assertEquals(42, SoarHelper::getUserProductionCount(agent));
	assertEquals(6, SoarHelper::getChunkProductionCount(agent));
	assertEquals(85133, SoarHelper::getE_CYCLE_COUNT(agent));
	assertEquals(40038, SoarHelper::getPE_CYCLE_COUNT(agent));
	assertEquals(95151, SoarHelper::getINNER_E_CYCLE_COUNT(agent));
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
