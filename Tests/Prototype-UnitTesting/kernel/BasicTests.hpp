//
//  BasicTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef BasicTests_cpp
#define BasicTests_cpp

#include "FunctionalTestHarness.hpp"

class BasicTests : public FunctionalTestHarness
{
public:
	FUNCTIONAL_TEST_CATEGORY(BasicTests);
	
	// TODO: Fix 'testBasicElaborationAndMatch: Failed: Assert: Expected equal values (0, 1) but was unequal.'
	TEST(testBasicElaborationAndMatch, -1)
	void testBasicElaborationAndMatch();
	
	// TODO: Fix 'testInitialState: Failed: Assert: testInitialState functional test did not halt'
	TEST(testInitialState, -1)
	void testInitialState();
};

#endif /* BasicTests_cpp */
