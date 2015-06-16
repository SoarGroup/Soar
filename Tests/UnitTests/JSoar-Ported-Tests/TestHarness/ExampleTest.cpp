//
//  ExampleTest.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ExampleTest.hpp"

#include <iostream>

void ExampleTest::testWaterJug()
{
	agent->ExecuteCommandLine("srand 1");
	
	runTest("testWaterJug", -1);
}
