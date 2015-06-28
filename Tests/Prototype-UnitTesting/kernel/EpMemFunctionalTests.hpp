//
//  FunctionalTests_EpMem.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTests_EpMem_cpp
#define FunctionalTests_EpMem_cpp

#include "FunctionalTestHarness.hpp"

class EpMemFunctionalTests : public FunctionalTestHarness {
public:
	FUNCTIONAL_TEST_CATEGORY(EpMemFunctionalTests);
	
	TEST(testCountEpMem, -1)
	void testCountEpMem();
	
	TEST(testHamilton, -1)
	void testHamilton();
	
	// CSoar does not have a filter command for epmem
//	TEST(testFilterEpMem, -1)
	void testFilterEpMem();
	
	// CSoar does not have an add command for epmem
//	TEST(testAddCommand, -1)
	void testAddCommand();
	
	// CSoar does not have inclusions for epmem
//	TEST(testInclusions, -1)
	void testInclusions();
	
	// CSoar does not have deliberate storage for epmem
//	TEST(testDeliberateStorage, -1)
	void testDeliberateStorage();
	
	TEST(testKB, -1)
	void testKB();
	
	TEST(testSingleStoreRetrieve, -1)
	void testSingleStoreRetrieve();
	
	TEST(testOddEven, -1)
	void testOddEven();
	
	TEST(testBeforeEpMem, -1)
	void testBeforeEpMem();
	
	TEST(testAfterEpMem, -1)
	void testAfterEpMem();
	
	TEST(testAllNegQueriesEpMem, -1)
	void testAllNegQueriesEpMem();
	
	TEST(testBeforeAfterProhibitEpMem, -1)
	void testBeforeAfterProhibitEpMem();
	
	TEST(testMaxDoublePrecision_Irrational, -1)
	void testMaxDoublePrecision_Irrational();
	
	TEST(testMaxDoublePrecisionEpMem, -1)
	void testMaxDoublePrecisionEpMem();
	
	TEST(testNegativeEpisode, -1)
	void testNegativeEpisode();
	
	TEST(testNonExistingEpisode, -1)
	void testNonExistingEpisode();
	
	TEST(testSimpleFloatEpMem, -1)
	void testSimpleFloatEpMem();
	
	TEST(testCyclicQuery, -1)
	void testCyclicQuery();
	
	TEST(testWMELength_OneCycle, -1);
	void testWMELength_OneCycle();
	
	TEST(testWMELength_FiveCycle, -1)
	void testWMELength_FiveCycle();
	
	TEST(testWMELength_InfiniteCycle, -1)
	void testWMELength_InfiniteCycle();
	
	TEST(testWMELength_MultiCycle, -1)
	void testWMELength_MultiCycle();
	
	TEST(testWMActivation_Balance0, -1)
	void testWMActivation_Balance0();
	
	TEST(testEpMemEncodeOutput_NoWMA, -1)
	void testEpMemEncodeOutput_NoWMA();
	
	TEST(testEpMemEncodeOutput_WMA, -1)
	void testEpMemEncodeOutput_WMA();
	
	TEST(testEpMemEncodeSelection_NoWMA, -1)
	void testEpMemEncodeSelection_NoWMA();
	
	TEST(testEpMemEncodeSelection_WMA, -1)
	void testEpMemEncodeSelection_WMA();
	
	TEST(testEpMemYRemoval, -1)
	void testEpMemYRemoval();
	
	// TODO: Fix 'Assert: testEpMemSoarGroupTests functional test failed'
	TEST(testEpMemSoarGroupTests, -1);
	void testEpMemSoarGroupTests();
	
	TEST(testReadCSoarDB, -1)
	void testReadCSoarDB();
	
	TEST(testMultiAgent, -1)
	void testMultiAgent();
};

#endif /* FunctionalTests_EpMem_cpp */
