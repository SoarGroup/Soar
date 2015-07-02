//
//  ElementXMLTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef ElementXMLTest_cpp
#define ElementXMLTest_cpp

#include "portability.h"

#include "TestCategory.hpp"

#include <string>

class ElementXMLTest : public TestCategory
{
	sml::Agent* agent;
public:
	TEST_CATEGORY(ElementXMLTest);
	
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
	soarxml::ElementXML* createXML1();
	soarxml::ElementXML* createXML2();
	soarxml::ElementXML* createXML3();
	soarxml::ElementXML* createXML4();
	soarxml::ElementXML* createXML5();
	
	bool verifyBuffer(const char* buffer) const;
	
	static const int BUFFER_LENGTH;
	char buffer[10] ;
};
#endif /* ElementXMLTest_cpp */
