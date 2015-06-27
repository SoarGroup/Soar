//
//  IOTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef IOTests_cpp
#define IOTests_cpp

#include "portability.h"

#include "FunctionalTestHarness.hpp"

#include <string>
#include <vector>
#include <sstream>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "thread_Event.h"
#include "sml_Names.h"

class IOTests : public FunctionalTestHarness
{
public:
	FUNCTIONAL_TEST_CATEGORY(IOTests);

	TEST(testInputLeak, -1);
	void testInputLeak();  // only string
	
	TEST(testInputLeak2, -1);
	void testInputLeak2(); // explicitly delete both
	
	TEST(testInputLeak3, -1);
	void testInputLeak3(); // only delete identifier
	
	TEST(testInputLeak4, -1);
	void testInputLeak4(); // do something with shared ids
	
	TEST(testOutputLeak1, -1);
	void testOutputLeak1(); // output input wme created but not destroyed
};

#endif /* IOTests_cpp */
