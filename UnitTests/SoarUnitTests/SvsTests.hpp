//
//  SvsTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef SvsTests_cpp
#define SvsTests_cpp

#include "portability.h"

#include <string>
#include <vector>
#include <sstream>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "thread_Event.h"
#include "sml_Names.h"

#include "FunctionalTestHarness.hpp"

class SvsTests : public FunctionalTestHarness
{
private:
    sml::Agent* agent;
    sml::Kernel* kernel;

public:
    TEST_CATEGORY(SvsTests);
    void before() { setUp(); }
    void setUp();

	void after(bool caught) { tearDown(caught); }
	void tearDown(bool caught);

    TEST(testEnableFromTopState, -1);
    void testEnableFromTopState();

    TEST(testEnableAndDisableInSubstatesFromTopState, -1);
    void testEnableAndDisableInSubstatesFromTopState();

    TEST(testCannotDisableInSubstate, -1);
    void testCannotDisableInSubstate();

    TEST(testEnableFromSubstate, -1);
    void testEnableFromSubstate();

    TEST(testEnableInSubstatesFromSubstate, -1);
    void testEnableInSubstatesFromSubstate();

    TEST(testSubstateWhenDisabledInSubstates, -1);
    void testSubstateWhenDisabledInSubstates();

    TEST(testSvsSceneCaseInsensitivity, -1);
    void testSvsSceneCaseInsensitivity();
};

#endif /* SvsTests_cpp */
