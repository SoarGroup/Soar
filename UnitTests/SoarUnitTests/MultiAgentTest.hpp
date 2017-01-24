//
//  MultiAgentTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef MultiAgentTest_cpp
#define MultiAgentTest_cpp

#include <string>
#include <vector>
#include <sstream>

#include "sml_Client.h"
#include "TestCategory.hpp"

class MultiAgentTest : public TestCategory
{
public:
	TEST_CATEGORY(MultiAgentTest);
	
	void before() { setUp(); }
	void after(bool caught) { tearDown(caught); }
	
	void setUp();
	void tearDown(bool caught);
	
	void MyUpdateEventHandler();
	
	TEST(testOneAgentForSanity, -1)
	void testOneAgentForSanity();
	
	TEST(testTwoAgents, -1)
	void testTwoAgents();
	
	TEST(testTenAgents, -1)
	void testTenAgents();
	
	TEST(testMaxAgents, -1)
	void testMaxAgents();
	
private:
	struct user_data_struct
	{
		user_data_struct(std::function<void ()> routine)
		: function(routine)
		{}
		
		user_data_struct()
		{}
		
		std::function<void ()> function;
	};

	user_data_struct updateEventHandler;
	
	void doTest();
	void createInput(sml::Agent* agent, int value);
	void reportAgentStatus(sml::Kernel* pKernel, int numberAgents, std::vector< std::stringstream* >& trace);
	void initAll(sml::Kernel* pKernel);
	void UpdateInput(sml::Agent* agent, int value);
	
	static const int MAX_AGENTS;
	int numberAgents;
	sml::Kernel* pKernel;
};

#endif /* MultiAgentTest_cpp */
