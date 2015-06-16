//
//  ExampleTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FunctionalTestHarness.hpp"

class ExampleTest : public FunctionalTestHarness
{
public:
	FUNCTIONAL_TEST_CATEGORY(ExampleTest)
	
	TEST(testWaterJug, 5000)
	void testWaterJug();
};
