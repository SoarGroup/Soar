//
//  AgentTest.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "AgentTest.hpp"

void AgentTest::setUp()
{
	kernel = sml::Kernel::CreateKernelInCurrentThread(true);
	agent = kernel->CreateAgent("soar1");
}

void AgentTest::tearDown(bool caught)
{
	kernel->DestroyAgent(agent);
	kernel->Shutdown();
	delete kernel;
	kernel = nullptr;
}

void AgentTest::testDefaultStopPhaseIsApply()
{
	assertEquals(std::get<0>(SoarHelper::getStopPhase(agent)), SoarHelper::StopPhase::APPLY);
}

void AgentTest::testSetStopPhaseSetsTheStopPhaseProperty()
{
	agent->ExecuteCommandLine("set-stop-phase --before --decision");
	assertEquals(std::get<0>(SoarHelper::getStopPhase(agent)), SoarHelper::StopPhase::DECISION);
}

void AgentTest::testGetGoalStack()
{
	agent->RunSelf(3);
	// We start with S1. Running three steps, gives three new states, S2, S3, S4
	const std::vector<std::string> gs = SoarHelper::getGoalStack(agent);
	assertEquals(size_t(4), gs.size());
	
	assertEquals_vector(gs, std::vector<std::string>({"S1", "S5", "S9", "S13"}));
}