//
//  ExampleTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef ExampleTest_cpp
#define ExampleTest_cpp

#include "FunctionalTestHarness.hpp"

class ExampleTests : public FunctionalTestHarness
{
public:
	FUNCTIONAL_TEST_CATEGORY(ExampleTests)
	
	TEST(testWaterJug, 5000)
	void testWaterJug();
};

#endif
