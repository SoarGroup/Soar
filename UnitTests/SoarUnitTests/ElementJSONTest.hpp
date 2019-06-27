//
//  ElementJSONTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef ElementJSONTest_cpp
#define ElementJSONTest_cpp

#include "portability.h"

#include <string>
#include "json.h"
#include "TestCategory.hpp"

class ElementJSONTest : public TestCategory
{
	sml::Agent* agent;
public:
	TEST_CATEGORY(ElementJSONTest);
	
	void before() { setUp(); }
	void setUp();
	
	void after(bool caught) { tearDown(caught); }
	void tearDown(bool caught);
	
	TEST(testSimple, -1);
	void testSimple();
	
	TEST(testChildren, -1);
	void testChildren();
	
	TEST(testParse, -1);
	void testParse();
	
	TEST(testBinaryData, -1);
	void testBinaryData();
	
	TEST(testEquals, -1);
	void testEquals();
	
private:
	soarjson::ElementJSON* createJSON1();
	soarjson::ElementJSON* createJSON2();
	soarjson::ElementJSON* createJSON3();
	soarjson::ElementJSON* createJSON4();
	soarjson::ElementJSON* createJSON5();
	
	bool verifyBuffer(const char* buffer) const;
	
	static const int BUFFER_LENGTH;
	char buffer[10] ;
};
#endif /* ElementJSONTest_cpp */
