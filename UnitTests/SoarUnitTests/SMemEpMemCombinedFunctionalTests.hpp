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
	TEST_CATEGORY(SMemEpMemCombinedFunctionalTests);
	
	TEST(testSmemEpMemFactorizationCombinationTest, -1)
	void testSmemEpMemFactorizationCombinationTest();
};

#endif /* FunctionalTests_SMemEpMemCombined_cpp */
