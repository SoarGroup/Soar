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
	
    TEST(testReadCSoarDB, -1)
    void testReadCSoarDB();

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
    //TEST(testEpmemUnit_14, -1)  /* This test should no longer pass with the new 9.6.0 model of smem */

//	TEST(testCountEpMem, -1) // Crashing: Assertion failed: (temp_stmt->get_status() == ready), function structure, file Core/SoarKernel/src/shared/soar_db.h, line 520
	void testCountEpMem();
	
	TEST(testHamilton, -1)
	void testHamilton();
	
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
	
	TEST(testMultiAgent, -1)
	void testMultiAgent();
	
	TEST(testHamiltonian, -1)
	void testHamiltonian();
	
	TEST(testSVS, -1)
	void testSVS();
	
	TEST(testSVSHard, -1)
	void testSVSHard();

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
};

#endif /* FunctionalTests_EpMem_cpp */
