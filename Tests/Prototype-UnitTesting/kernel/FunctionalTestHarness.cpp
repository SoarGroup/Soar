//
//  FunctionalTestHarness.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FunctionalTestHarness.hpp"

#include "SoarHelper.hpp"

FunctionalTestHarness::FunctionalTestHarness(std::string categoryName)
: TestCategory(categoryName)
{}

void FunctionalTestHarness::runTestSetup(std::string testName)
{
	std::string sourceName = this->getCategoryName() + "_" + testName + ".soar";
	
	assertTrue("Could not find test file " + sourceName, isfile(("Tests/" + sourceName).c_str()));
	const char* result = agent->ExecuteCommandLine(("source Tests/" + sourceName).c_str());
	
	runner->output << "Loaded Productions for " << sourceName << ":" << std::endl;
	runner->output << result << std::endl;
	
	result = agent->ExecuteCommandLine("set-stop-phase --apply");
	runner->output << "Set Stop Phase: " << result << std::endl;
}

// this function assumes some other function has set up the agent (like runTestSetup)
void FunctionalTestHarness::runTestExecute(std::string testName, int expectedDecisions)
{
	const char* result = nullptr;
	
	if(expectedDecisions >= 0)
	{
		result = agent->RunSelf(expectedDecisions + 1);
	}
	else
	{
		result = agent->RunSelfForever();
	}

	runner->output << result << std::endl;
	
	assertTrue(testName + " functional test did not halt", halted);
	assertFalse(testName + " functional test failed", failed);
	if(expectedDecisions >= 0)
	{
		assertEquals(expectedDecisions, SoarHelper::getDecisionPhasesCount(agent)); // deterministic!
	}
	
	agent->ExecuteCommandLine("stats");
}

void FunctionalTestHarness::runTest(std::string testName, int expectedDecisions)
{
	runTestSetup(testName);
	runTestExecute(testName, expectedDecisions);
}

static int count = 0;

void FunctionalTestHarness::afterDecisionCycleHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase)
{
	FunctionalTestHarness* _this = (FunctionalTestHarness*)pUserData;
	
	++count;
	
	// Called during smlEVENT_AFTER_DECISION_CYCLE
	if (_this->runner->kill == true)
	{
		_this->halted = true;
		pAgent->StopSelf();
	}
}

void FunctionalTestHarness::setUp()
{
	halted = false;
	failed = false;
	
	kernel = sml::Kernel::CreateKernelInCurrentThread(true);
	agent = kernel->CreateAgent("soar1");
	
	agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, &FunctionalTestHarness::afterDecisionCycleHandler, this);
	
	installRHS(agent);
}

void FunctionalTestHarness::tearDown(bool caught)
{
	kernel->DestroyAgent(agent);
	kernel->Shutdown();
	delete kernel;
	kernel = nullptr;
}

std::string FunctionalTestHarness::haltHandler(sml::smlRhsEventId id,
													  void* pUserData,
													  sml::Agent* pAgent,
													  char const* pFunctionName,
													  char const* pArgument)
{
	FunctionalTestHarness* _this = (FunctionalTestHarness*)pUserData;
	
	_this->halted = true;
	_this->runner->failed = false;

	return pAgent->StopSelf();
}

std::string FunctionalTestHarness::failedHandler(sml::smlRhsEventId id,
														void* pUserData,
														sml::Agent* pAgent,
														char const* pFunctionName,
														char const* pArgument)
{
	FunctionalTestHarness* _this = (FunctionalTestHarness*)pUserData;
	
	_this->halted = true;
	_this->failed = true;
	
	_this->runner->failed = true;
	
	return pAgent->StopSelf();
}

std::string FunctionalTestHarness::succeededHandler(sml::smlRhsEventId id,
														   void* pUserData,
														   sml::Agent* pAgent,
														   char const* pFunctionName,
														   char const* pArgument)
{
	FunctionalTestHarness* _this = (FunctionalTestHarness*)pUserData;
	
	_this->halted = true;
	_this->failed = false;
	_this->runner->failed = false;
	
	return pAgent->StopSelf();
}


/**
 * Set up the agent with RHS functions common to these
 * FunctionalTests.
 */
void FunctionalTestHarness::installRHS(sml::Agent* agent)
{
	// set up the agent with common RHS functions
	agent->GetKernel()->AddRhsFunction("halt", FunctionalTestHarness::haltHandler, this);
	assertFalse(agent->GetLastErrorDescription(), agent->GetLastCommandLineResult());

	agent->GetKernel()->AddRhsFunction("failed", FunctionalTestHarness::failedHandler, this);
	assertFalse(agent->GetLastErrorDescription(), agent->GetLastCommandLineResult());

	agent->GetKernel()->AddRhsFunction("succeeded", FunctionalTestHarness::succeededHandler, this);
	assertFalse(agent->GetLastErrorDescription(), agent->GetLastCommandLineResult());
}

