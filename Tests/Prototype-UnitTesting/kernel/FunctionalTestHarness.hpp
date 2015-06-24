//
//  FunctionalTestHarness.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTestHarness_cpp
#define FunctionalTestHarness_cpp

#include "TestCategory.hpp"
#include "TestRunner.hpp"

#include "portability.h"

#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_RhsFunction.h"
#include "agent.h"
#include "rhs_functions.h"

#include <string>

class FunctionalTestHarness : public TestCategory
{
public:
	FunctionalTestHarness(std::string categoryName);
	
protected:
	sml::Agent* agent;
	sml::Kernel* kernel;
	sml::KernelSML* internal_kernel;
	::agent* internal_agent;
	
	bool halted = false;
	bool failed = false;
	
	::rhs_function_routine halt_routine;
	
	void afterDecisionCycleHandler();
	
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
	
	void after(bool caught) { tearDown(caught); }
	void tearDown(bool caught);
	
	::Symbol* haltHandler();
	::Symbol* failedHandler();
	::Symbol* succeededHandler();
	
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
            Member_Function(test_t * const this_, void (test_t::*fun_)()) : m_this(this_), m_fun(fun_) {} \
            void operator()() {(m_this->*m_fun)();} \
        private: \
            test_t * m_this; \
            void (test_t::*m_fun)(); \
    };

#endif /* FunctionalTestHarness_cpp */
