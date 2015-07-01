//
//  MiscTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/27/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef MiscTests_cpp
#define MiscTests_cpp

#include "FunctionalTestHarness.hpp"

class MiscTests : public FunctionalTestHarness
{
public:
	TEST_CATEGORY(MiscTests);
	
	TEST(testInstiationDeallocationStackOverflow, -1)
	void testInstiationDeallocationStackOverflow();
	TEST(test_clog, -1)
	void test_clog();
	TEST(test_gp, -1)
	void test_gp();
	TEST(test_echo, -1)
	void test_echo();
	TEST(test_ls, -1)
	void test_ls();
	
	// TODO: Update for linux & Windows (32 + 64)
	TEST(test_stats, -1)
	void test_stats();
	
	TEST(testWrongAgentWmeFunctions, -1)
	void testWrongAgentWmeFunctions();
	TEST(testRHSRand, -1)
	void testRHSRand();
	TEST(testMultipleKernels, -1)
	void testMultipleKernels();
	TEST(testSmemArithmetic, -1)
	void testSmemArithmetic();
	
	TEST(testSource, -1)
	void testSource();
	
	TEST(testSoarRand, -1)
	void testSoarRand();
	TEST(testPreferenceDeallocation, -1)
	void testPreferenceDeallocation();
	
	// If you would like to test the Soar Debugger Spawning, uncomment below.
	// It may or may not work but should unless you're running without a GUI.
//	TEST(testSoarDebugger, -1)
	void testSoarDebugger();
	
	void source(const std::string& path);
};

#endif /* MiscTests_cpp */
