//
//  FunctionalTestHarness.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FunctionalTestHarness.hpp"

#include "sml_EmbeddedConnectionSynch.h"
#include "sml_AgentSML.h"
#include "symbol_manager.h"
#include "SoarHelper.hpp"

#define CONFIGURE_SOAR_FOR_UNIT_TESTS

FunctionalTestHarness::FunctionalTestHarness()
: haltData(std::bind(&FunctionalTestHarness::haltHandler, this)),
failedData(std::bind(&FunctionalTestHarness::failedHandler, this)),
succeededData(std::bind(&FunctionalTestHarness::succeededHandler, this))
{}

void FunctionalTestHarness::runTestSetup(std::string testName)
{
	std::string sourceName = this->getCategoryName() + "_" + testName + ".soar";
	
	std::string path = SoarHelper::GetResource(sourceName);

    if (path.size() == 0)
    {
        sourceName = testName + ".soar";
        path = SoarHelper::GetResource(sourceName);
    }

    if (path.size() == 0 && testName.find("test") == 0)
    {
        sourceName = testName.substr(std::string("test").size(), -1) + ".soar";
        path = SoarHelper::GetResource(sourceName);
    }

	assertNonZeroSize_msg("Could not find test file '" + sourceName + "'", path);
	
	const char* result = agent->ExecuteCommandLine(("source \"" + path + "\"").c_str());
	
	runner->output << "Loaded Productions for " << sourceName << ":" << std::endl;
	runner->output << result << std::endl;
	
	result = agent->ExecuteCommandLine("soar stop-phase apply");
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
	
	runner->output << std::endl << result << std::endl;
	
	assertTrue_msg(testName + " functional test did not halt", halted);
	assertFalse_msg(testName + " functional test failed", failed);
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

void FunctionalTestHarness::afterDecisionCycleHandler()
{
	++count;
	
	// Called during smlEVENT_AFTER_DECISION_CYCLE
	if (runner->kill == true)
	{
		halted = true;
		halt_routine(internal_agent, nullptr, nullptr);
	}
}

void FunctionalTestHarness::printHandler(const char* msg)
{
	runner->output << msg;
}

void FunctionalTestHarness::setUp()
{
	halted = false;
	failed = false;
	
	kernel = sml::Kernel::CreateKernelInCurrentThread(true, sml::Kernel::kUseAnyPort);
    #ifdef CONFIGURE_SOAR_FOR_UNIT_TESTS
	    configure_for_unit_tests();
    #endif
	agent = kernel->CreateAgent("soar1");
	
	// /BEGIN WARN WARN:
	//
	// This works because we're using 'CreateKernelInCurrentThread(true)'
	// THIS WILL BREAK OTHERWISE.  WE DO NOT NEED FANCINESS HERE SO THIS IS GOOD
	// BECAUSE WE CAN TEST INTERNAL AGENT WITH THIS.
	//
	// /END WARN WARN
	
	sml::EmbeddedConnectionSynch* connection = dynamic_cast<sml::EmbeddedConnectionSynch*>(kernel->GetConnection());
	
	internal_kernel = connection->GetKernelSML();
	internal_agent = internal_kernel->GetAgentSML("soar1")->GetSoarAgent();
	configure_agent_for_unit_tests(internal_agent);
	
	soar_add_callback(internal_agent,
					  AFTER_DECISION_CYCLE_CALLBACK,
					  [](::agent*, soar_callback_event_id, soar_callback_data data, soar_call_data) {
						  static_cast<FunctionalTestHarness*>(data)->afterDecisionCycleHandler();
					  },
					  AFTER_DECISION_CYCLE_CALLBACK,
					  this,
					  [](soar_callback_data){},
					  "Prototype-UnitTesting AFTER_DECISION_CYCLE_CALLBACK");
	
	soar_add_callback(internal_agent,
					  PRINT_CALLBACK,
					  [](::agent*, soar_callback_event_id, soar_callback_data data, soar_call_data msg) {
						  static_cast<FunctionalTestHarness*>(data)->printHandler(static_cast<const char*>(msg));
					  },
					  PRINT_CALLBACK,
					  this,
					  [](soar_callback_data){},
					  "Prototype-UnitTesting PRINT_CALLBACK");
	
	installRHS();
}

void FunctionalTestHarness::tearDown(bool caught)
{
	removeRHS();
	
	halt_routine = nullptr;

    if (agent != nullptr)
        kernel->DestroyAgent(agent);

	kernel->Shutdown();
    
	delete kernel;
	kernel = nullptr;
	agent = nullptr;
	internal_agent = nullptr;
	internal_kernel = nullptr;
}

Symbol* FunctionalTestHarness::haltHandler()
{
	halted = true;
	failed = false;
	
	runner->failed = false;
	
	return halt_routine(internal_agent, nullptr, nullptr);
}

Symbol* FunctionalTestHarness::failedHandler()
{
	halted = true;
	failed = true;
	
	runner->failed = true;
	
	return halt_routine(internal_agent, nullptr, nullptr);
}

Symbol* FunctionalTestHarness::succeededHandler()
{
	halted = true;
	failed = false;
	
	runner->failed = false;
	
	return halt_routine(internal_agent, nullptr, nullptr);
}

/**
 * Set up the agent with RHS functions common to these
 * FunctionalTests.
 */
void FunctionalTestHarness::installRHS()
{
	// set up the agent with common RHS functions
	::rhs_function* halt_function = lookup_rhs_function(internal_agent, internal_agent->symbolManager->make_str_constant("halt"));
	halt_routine = halt_function->f;
	
	auto call_routine = [](::agent* thisAgent, cons* args, void* user_data) -> Symbol* {
		return static_cast<user_data_struct*>(user_data)->function();
	};
	
	halt_function->user_data = &haltData;
	halt_function->f = call_routine;
	
	add_rhs_function(internal_agent,
					 internal_agent->symbolManager->make_str_constant("failed"),
					 call_routine,
					 0, false, true,
					 &failedData);
	
	add_rhs_function(internal_agent,
					 internal_agent->symbolManager->make_str_constant("succeeded"),
					 call_routine,
					 0, false, true,
					 &succeededData);
}

void FunctionalTestHarness::removeRHS()
{
    if (internal_agent != nullptr)
    {
        remove_rhs_function(internal_agent, internal_agent->symbolManager->find_str_constant("failed"));
        remove_rhs_function(internal_agent, internal_agent->symbolManager->find_str_constant("succeeded"));
        
        ::rhs_function* halt_function = lookup_rhs_function(internal_agent, internal_agent->symbolManager->make_str_constant("halt"));
        
        halt_function->user_data = NULL;
        halt_function->f = halt_routine;
    }
}

