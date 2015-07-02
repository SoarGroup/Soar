//
//  AliasTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef AliasTest_cpp
#define AliasTest_cpp

#include "portability.h"

#include "cli_Aliases.h"
#include "cli_Parser.h"

#include "TestCategory.hpp"

class AliasTest : public TestCategory
{
private:
	sml::Agent* agent;
	cli::Aliases* aliases;
	
public:
	TEST_CATEGORY(AliasTest);
	
	void setUp() { before(); }
	virtual void before();
	
	void tearDown(bool caught) { after(caught); }
	virtual void after(bool caught);
	
	TEST(testOne, -1)
	void testOne();
	
	TEST(testTwo, -1)
	void testTwo();
	
	TEST(testThree, -1)
	void testThree();
	
	TEST(testRemove, -1)
	void testRemove();
	
	TEST(testDefaults, -1)
	void testDefaults();
	
	TEST(testSimpleCommand, -1)
	void testSimpleCommand();
};

#endif /* AliasTest_cpp */
