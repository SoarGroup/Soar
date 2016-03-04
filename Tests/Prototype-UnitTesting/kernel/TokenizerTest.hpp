//
//  TokenizerTest.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef TokenizerTest_cpp
#define TokenizerTest_cpp

#include "portability.h"

#include "TestCategory.hpp"

#include "tokenizer.h"
#include <stdarg.h>
#include <queue>

struct CallData
{
	CallData(const char* input)
	: input(input)
	{
	}
	
	CallData(const char* input, int expected, ...)
	: input(input)
	{
		va_list args;
		va_start(args, expected);
		std::vector<std::string> argv;
		for (int i = 0; i < expected; ++i)
		{
			argv.push_back(va_arg(args, const char*));
		}
		va_end(args);
		
		q.push(argv);
	}
	
	void addResult(int expected, ...)
	{
		va_list args;
		va_start(args, expected);
		std::vector<std::string> argv;
		for (int i = 0; i < expected; ++i)
		{
			argv.push_back(va_arg(args, const char*));
		}
		va_end(args);
		q.push(argv);
	}
	
	const char* input;
	std::queue< std::vector<std::string> > q;
};

class TokenizerTest : public TestCategory, public soar::tokenizer_callback
{
public:
	TEST_CATEGORY(TokenizerTest)
	
	virtual ~TokenizerTest() {}
	virtual bool handle_command(std::vector<std::string>& argv);
	
	void before() { setUp(); }
	void after(bool caught) { tearDown(caught); }
	
	void setUp();       // Called before each function outlined by CPPUNIT_TEST
	void tearDown(bool caught);    // Called after each function outlined by CPPUNIT_TEST
	
	TEST(testTokenizer01, -1)
	void testTokenizer01();
	
	TEST(testTokenizer02, -1)
	void testTokenizer02();
	
	TEST(testTokenizer03, -1)
	void testTokenizer03();
	
	TEST(testTokenizer04, -1)
	void testTokenizer04();
	
	TEST(testTokenizer05, -1)
	void testTokenizer05();
	
	TEST(testTokenizer06, -1)
	void testTokenizer06();
	
	TEST(testTokenizer07, -1)
	void testTokenizer07();
	
	TEST(testTokenizer08, -1)
	void testTokenizer08();
	
	TEST(testTokenizer09, -1)
	void testTokenizer09();
	
	TEST(testTokenizer10, -1)
	void testTokenizer10();
	
	TEST(testTokenizer11, -1)
	void testTokenizer11();
	
	TEST(testTokenizer12, -1)
	void testTokenizer12();
	
	TEST(testTokenizer13, -1)
	void testTokenizer13();
	
	TEST(testTokenizer14, -1)
	void testTokenizer14();
	
	TEST(testTokenizer15, -1)
	void testTokenizer15();
	
	TEST(testTokenizer16, -1)
	void testTokenizer16();
	
	TEST(testTokenizer17, -1)
	void testTokenizer17();
	
	TEST(testTokenizer18, -1)
	void testTokenizer18();
	
	TEST(testTokenizer19, -1)
	void testTokenizer19();
	
	TEST(testTokenizer20, -1)
	void testTokenizer20();
	
	TEST(testTokenizer21, -1)
	void testTokenizer21();
	
	TEST(testTokenizer22, -1)
	void testTokenizer22();
	
	TEST(testTokenizer23, -1)
	void testTokenizer23();
	
	TEST(testTokenizer24, -1)
	void testTokenizer24();
	
	TEST(testTokenizer25, -1)
	void testTokenizer25();
	
	TEST(testTokenizer26, -1)
	void testTokenizer26();
	
	TEST(testTokenizer27, -1)
	void testTokenizer27();
	
	TEST(testTokenizer28, -1)
	void testTokenizer28();
	
	TEST(testTokenizer29, -1)
	void testTokenizer29();
	
	TEST(testTokenizer30, -1)
	void testTokenizer30();
	
	TEST(testTokenizer31, -1)
	void testTokenizer31();
	
	TEST(testTokenizer32, -1)
	void testTokenizer32();
	
	TEST(testTokenizer33, -1)
	void testTokenizer33();
	
	TEST(testTokenizer34, -1)
	void testTokenizer34();
	
	TEST(testTokenizer35, -1)
	void testTokenizer35();
	
	TEST(testTokenizer36, -1)
	void testTokenizer36();
	
	TEST(testTokenizer37, -1)
	void testTokenizer37();
	
	TEST(testTokenizer38, -1)
	void testTokenizer38();
	
	TEST(testTokenizer39, -1)
	void testTokenizer39();
	
	TEST(testTokenizer40, -1)
	void testTokenizer40();
	
	TEST(testTokenizer41, -1)
	void testTokenizer41();
	
	TEST(testTokenizer42, -1)
	void testTokenizer42();
	
	TEST(testTokenizer43, -1)
	void testTokenizer43();
	
	TEST(testTokenizer44, -1)
	void testTokenizer44();
	
	TEST(testTokenizer45, -1)
	void testTokenizer45();
	
	TEST(testTokenizer46, -1)
	void testTokenizer46();
	
	TEST(testTokenizer47, -1)
	void testTokenizer47();
	
	TEST(testTokenizer48, -1)
	void testTokenizer48();
	
	void evaluate(CallData& cd);
	
	soar::tokenizer* tokenizer;
	CallData* cd;
};

#endif /* TokenizerTest_cpp */
