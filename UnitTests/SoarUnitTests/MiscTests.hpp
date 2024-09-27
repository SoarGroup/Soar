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

//	TEST(testInstiationDeallocationStackOverflow, -1)
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
	TEST(testRegression370, -1)
	void testRegression370();
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

	TEST(testStopPhaseRetrieval, -1)
    void testStopPhaseRetrieval();

	TEST(testGDS_Failed_Justification_Crash, -1);
	TEST(testIsupported_Smem_Chunk_Crash, -1);
	TEST(testNegated_Operator_Crash, -1);
	TEST(testOp_Augmentation_Crash, -1);

	void testGDS_Failed_Justification_Crash();
	void testIsupported_Smem_Chunk_Crash();
	void testNegated_Operator_Crash();
	void testOp_Augmentation_Crash();

	TEST(testProductionPrinting, -1)
    void testProductionPrinting();

	TEST(testSvsSceneCaseInsensitivity, -1)
    void testSvsSceneCaseInsensitivity();

    TEST(testLocationPredictionRhs, -1)
    void testLocationPredictionRhs();

	// If you would like to test the Soar Debugger Spawning, uncomment below.
	// It may or may not work but should unless you're running without a GUI.
//	TEST(testSoarDebugger, -1)
//	void testSoarDebugger();

	void source(const std::string& path);
};

#endif /* MiscTests_cpp */
