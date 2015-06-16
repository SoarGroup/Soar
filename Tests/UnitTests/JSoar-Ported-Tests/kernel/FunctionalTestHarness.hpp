//
//  FunctionalTestHarness.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTestHarness_cpp
#define FunctionalTestHarness_cpp

#include "TestCategory.hpp"
#include "TestRunner.hpp"

#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"

#include <string>

class FunctionalTestHarness : public TestCategory
{
public:
	FunctionalTestHarness(std::string categoryName);
	
	sml::Agent* agent;
	sml::Kernel* kernel;
	
protected:
	bool halted = false;
	bool failed = false;
	
	static void afterDecisionCycleHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase);
	
public:
	/**
	 * Sources rules
	 * @param testName the test to perform
	 * @throws SoarException
	 */
	void runTestSetup(std::string testName);
	
	// this function assumes some other function has set up the agent (like runTestSetup)
	void runTestExecute(std::string testName, int expectedDecisions);
	
protected:
	void runTest(std::string testName, int expectedDecisions);
	
public:
	void before() { setUp(); }
	void setUp();
	
	void after() { tearDown(); }
	void tearDown();
	
	static std::string haltHandler(sml::smlRhsEventId id,
								   void* pUserData,
								   sml::Agent* pAgent,
								   char const* pFunctionName,
								   char const* pArgument);
	
	static std::string failedHandler(sml::smlRhsEventId id,
									 void* pUserData,
									 sml::Agent* pAgent,
									 char const* pFunctionName,
									 char const* pArgument);
	
	static std::string succeededHandler(sml::smlRhsEventId id,
										void* pUserData,
										sml::Agent* pAgent,
										char const* pFunctionName,
										char const* pArgument);
	
	/**
	 * Set up the agent with RHS functions common to these
	 * FunctionalTests.
	 */
	void installRHS(sml::Agent* agent);
};

#define FUNCTIONAL_TEST_CATEGORY(X) X() : FunctionalTestHarness(#X) {} \
	typedef X test_t; \
	class Member_Function : public TestFunction { \
        public: \
            Member_Function(test_t * const &this_, void (test_t::*fun_)()) : m_this(this_), m_fun(fun_) {} \
            void operator()() {(m_this->*m_fun)();} \
        private: \
            test_t * m_this; \
            void (test_t::*m_fun)(); \
    };

#endif /* FunctionalTestHarness_cpp */
