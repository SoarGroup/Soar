//
//  WmaFunctionalTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/24/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef WmaFunctionalTests_cpp
#define WmaFunctionalTests_cpp

#include "FunctionalTestHarness.hpp"

class WmaFunctionalTests : public FunctionalTestHarness
{
public:
	FUNCTIONAL_TEST_CATEGORY(WmaFunctionalTests);
	
	TEST(testSimpleActivation, -1);
	void testSimpleActivation();
};

#endif /* WmaFunctionalTests_cpp */
