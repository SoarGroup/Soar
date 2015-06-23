//
//  FunctionalTests_EpMem.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTests_EpMem_cpp
#define FunctionalTests_EpMem_cpp

/*
 * Copyright (c) 2012 Soar Technology, Inc.
 *
 * Created on Jan 18, 2013
 */
// package org.jsoar.kernel.epmem;

// import static org.junit.Assert.assertNotNull;
// import static org.junit.Assert.assertTrue;

// import java.net.URL;
// import java.util.ArrayList;
// import java.util.List;

// import org.jsoar.kernel.FunctionalTestHarness;
#include "FunctionalTestHarness.hpp"

// import org.jsoar.kernel.RunType;
// import org.jsoar.kernel.SoarProperties;
// import org.jsoar.runtime.ThreadedAgent;
// import org.junit.Test;


/**
 * @author bob.marinier
 */
// public class EpMemFunctionalTests extends FunctionalTestHarness

class EpMemFunctionalTests : public FunctionalTestHarness {
public:
	FUNCTIONAL_TEST_CATEGORY(EpMemFunctionalTests);
	
	TEST(testCountEpMem, -1)
	void testCountEpMem();
	
	// TODO: Fix 'Assert: Expected equal values (3, 4) but was unequal.'
	TEST(testHamilton, -1)
	void testHamilton();
	
	// TODO: Fix 'Assert: testFilterEpMem functional test did not halt'
	TEST(testFilterEpMem, -1)
	void testFilterEpMem();
	
	// TODO: Fix 'Assert: testAddCommand functional test did not halt'
	TEST(testAddCommand, -1)
	void testAddCommand();
	
	// TODO: Fix 'Assert: testInclusions functional test did not halt'
	TEST(testInclusions, -1)
	void testInclusions();
	
	// TODO: Fix 'testDeliberateStorage functional test did not halt'
	TEST(testDeliberateStorage, -1)
	void testDeliberateStorage();
	
	TEST(testKB, -1)
	void testKB();
	
	// TODO: Fix 'Assert: Expected equal values (3, 4) but was unequal.'
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
	
	// TODO: Fix 'Assert: Unexpected output from CSoar database!'
	TEST(readCSoarDB, -1)
	void readCSoarDB();
	
//	TEST(testMultiAgent, -1)
	void testMultiAgent();
};

#endif /* FunctionalTests_EpMem_cpp */
