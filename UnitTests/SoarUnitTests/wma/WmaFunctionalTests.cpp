//
//  WmaFunctionalTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/24/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "WmaFunctionalTests.hpp"

void WmaFunctionalTests::testSimpleActivation()
{
	runTest("testSimpleActivation", 2679);
	std::string result = agent->ExecuteCommandLine("print s1 -i");
	
	assertTrue(result.find("S1 ^o-from-a true [-1.5]") != std::string::npos);
	assertTrue(result.find("S1 ^o-from-o true [-1.9]") != std::string::npos);
	assertTrue(result.find("S1 ^i-from-i true [1]") != std::string::npos);
	assertFalse(result.find("S1 ^o-from-i2") != std::string::npos);
}
