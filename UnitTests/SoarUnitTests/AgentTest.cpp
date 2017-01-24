//
//  AgentTest.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "AgentTest.hpp"
#include "soar_instance.h"

void AgentTest::setUp()
{
	kernel = sml::Kernel::CreateKernelInCurrentThread(true, sml::Kernel::kUseAnyPort);
    configure_for_unit_tests();
	agent = kernel->CreateAgent("soar1");
    configure_agent_for_unit_tests(NULL);
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
	agent->ExecuteCommandLine("soar stop-phase decision");
	assertEquals(std::get<0>(SoarHelper::getStopPhase(agent)), SoarHelper::StopPhase::DECISION);
}

void AgentTest::testGetGoalStack()
{
	agent->RunSelf(3);
	// We start with S1. Running three steps, gives three new states, S2, S3, S4
	const std::vector<std::string> gs = SoarHelper::getGoalStack(agent);
	assertEquals(size_t(4), gs.size());
	
	assertEquals_vector(gs, std::vector<std::string>({"S1", "S2", "S3", "S4"}));
}
