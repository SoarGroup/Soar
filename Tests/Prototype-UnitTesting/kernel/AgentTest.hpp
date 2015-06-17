//
//  AgentTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef AgentTest_cpp
#define AgentTest_cpp

#include "TestCategory.hpp"

#include "SoarHelper.hpp"

#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"

class AgentTest : public TestCategory
{
private:
	sml::Agent* agent;
	sml::Kernel* kernel;
	
public:
	TEST_CATEGORY(AgentTest);
	
	void before() { setUp(); }
	void setUp();
	
	void after(bool caught) { tearDown(caught); }
	void tearDown(bool caught);
	
	TEST(testDefaultStopPhaseIsApply, -1)
	void testDefaultStopPhaseIsApply();
	
	TEST(testSetStopPhaseSetsTheStopPhaseProperty, -1)
	void testSetStopPhaseSetsTheStopPhaseProperty();
	
	TEST(testGetGoalStack, -1)
	void testGetGoalStack();
};

#endif /* AgentTest_cpp */
