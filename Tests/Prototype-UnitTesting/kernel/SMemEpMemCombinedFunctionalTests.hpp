//
//  FunctionalTests_SMemEpMemCombined.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTests_SMemEpMemCombined_cpp
#define FunctionalTests_SMemEpMemCombined_cpp

#include "FunctionalTestHarness.hpp"

class SMemEpMemCombinedFunctionalTests : public FunctionalTestHarness
{
public:
	FUNCTIONAL_TEST_CATEGORY(SMemEpMemCombinedFunctionalTests);
	
	// TODO: Fix 'Assert: Unexpected output from SMem!'
	TEST(smemEpMemFactorizationCombinationTest, -1)
	void smemEpMemFactorizationCombinationTest();
};

#endif /* FunctionalTests_SMemEpMemCombined_cpp */
