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
	TEST_CATEGORY(EpMemFunctionalTests);
	
	TEST(testAfterEpMem, -1)
	TEST(testAllNegQueriesEpMem, -1)
	TEST(testBeforeAfterProhibitEpMem, -1)
	TEST(testBeforeEpMem, -1)
	TEST(testCountEpMem, -1)
	TEST(testCyclicQuery, -1)
	TEST(testEpMemEncodeOutput_NoWMA, -1)
	TEST(testEpMemEncodeOutput_WMA, -1)
	TEST(testEpMemEncodeSelection_NoWMA, -1)
	TEST(testEpMemEncodeSelection_WMA, -1)
	TEST(testEpMemSmemFactorizationCombinationTest, -1)
	TEST(testEpmemUnit_1, -1)
	TEST(testEpmemUnit_2, -1)
	TEST(testEpmemUnit_3, -1)
	TEST(testEpmemUnit_4, -1)
	TEST(testEpmemUnit_5, -1)
	TEST(testEpmemUnit_6, -1)
	TEST(testEpmemUnit_7, -1)
	TEST(testEpmemUnit_8, -1)
	TEST(testEpmemUnit_9, -1)
	TEST(testEpmemUnit_10, -1)
	TEST(testEpmemUnit_11, -1)
	TEST(testEpmemUnit_12, -1)
	TEST(testEpmemUnit_13, -1)
//	TEST(testEpmemUnit_14, -1) /* This test should no longer pass with the new 9.6.0 model of smem */
	TEST(testEpMemYRemoval, -1)
	TEST(testHamilton, -1)
	TEST(testHamiltonian, -1)
	TEST(testKB, -1)
	TEST(testMaxDoublePrecision_Irrational, -1)
	TEST(testMaxDoublePrecisionEpMem, -1)
	TEST(testMultiAgent, -1)
	TEST(testNegativeEpisode, -1)
	TEST(testNonExistingEpisode, -1)
	TEST(testOddEven, -1)
	TEST(testReadCSoarDB, -1)
	TEST(testSimpleFloatEpMem, -1)
	TEST(testSingleStoreRetrieve, -1)
	TEST(testSVS, -1)
	TEST(testSVSHard, -1)
	TEST(testWMActivation_Balance0, -1)
	TEST(testWMELength_FiveCycle, -1)
	TEST(testWMELength_InfiniteCycle, -1)
	TEST(testWMELength_MultiCycle, -1)
	TEST(testWMELength_OneCycle, -1);
	
	void testAfterEpMem();
	void testAllNegQueriesEpMem();
	void testBeforeAfterProhibitEpMem();
	void testBeforeEpMem();
	void testCountEpMem();
	void testCyclicQuery();
	void testEpMemEncodeOutput_NoWMA();
	void testEpMemEncodeOutput_WMA();
	void testEpMemEncodeSelection_NoWMA();
	void testEpMemEncodeSelection_WMA();
	void testEpmemUnit_1();
	void testEpmemUnit_2();
	void testEpmemUnit_3();
	void testEpmemUnit_4();
	void testEpmemUnit_5();
	void testEpmemUnit_6();
	void testEpmemUnit_7();
	void testEpmemUnit_8();
	void testEpmemUnit_9();
	void testEpmemUnit_10();
	void testEpmemUnit_11();
	void testEpmemUnit_12();
	void testEpmemUnit_13();
	void testEpmemUnit_14();
    void testEpMemSmemFactorizationCombinationTest();
	void testEpMemYRemoval();
	void testHamilton();
	void testHamiltonian();
	void testKB();
	void testMaxDoublePrecision_Irrational();
	void testMaxDoublePrecisionEpMem();
	void testMultiAgent();
	void testNegativeEpisode();
	void testNonExistingEpisode();
	void testOddEven();
	void testReadCSoarDB();
	void testSimpleFloatEpMem();
	void testSingleStoreRetrieve();
	void testSVS();
	void testSVSHard();
	void testWMActivation_Balance0();
	void testWMELength_FiveCycle();
	void testWMELength_InfiniteCycle();
	void testWMELength_MultiCycle();
	void testWMELength_OneCycle();

};

#endif /* FunctionalTests_EpMem_cpp */
