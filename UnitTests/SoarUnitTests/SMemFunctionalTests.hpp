//
//  SMemFunctionalTests.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/23/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef SMemFunctionalTests_cpp
#define SMemFunctionalTests_cpp

#include "FunctionalTestHarness.hpp"

class SMemFunctionalTests : public FunctionalTestHarness
{
public:
	TEST_CATEGORY(SMemFunctionalTests);

    void before() { setUp(); }
    void setUp();

    void after(bool caught) { tearDown(caught); }
    void tearDown(bool caught);

    TEST(testSimpleCueBasedRetrieval, -1)
	void testSimpleCueBasedRetrieval();
	
	TEST(testSimpleNonCueBasedRetrieval, -1)
	void testSimpleNonCueBasedRetrieval();
	
	TEST(testSimpleStore, -1)
	void testSimpleStore();
	
	TEST(testTrivialMathQuery, -1)
	void testTrivialMathQuery();
	
	TEST(testBadMathQuery, -1)
	void testBadMathQuery();
	
	TEST(testMaxQuery, -1)
	void testMaxQuery();
	
	TEST(testMaxMixedTypes, -1)
	void testMaxMixedTypes();
	
	TEST(testMaxMultivalued, -1)
	void testMaxMultivalued();
	
	TEST(testMin, -1)
	void testMin();
	
	TEST(testMaxNegQuery, -1)
	void testMaxNegQuery();
	
	TEST(testGreater, -1)
	void testGreater();
	
	TEST(testLess, -1)
	void testLess();
	
	TEST(testGreaterOrEqual, -1)
	void testGreaterOrEqual();
	
	TEST(testLessOrEqual, -1)
	void testLessOrEqual();
	
	TEST(testLessWithNeg, -1)
	void testLessWithNeg();
	
	TEST(testLessNoSolution, -1)
	void testLessNoSolution();
	
    TEST(testSimpleStoreMultivaluedAttribute, -1)
	void testSimpleStoreMultivaluedAttribute();
	
	TEST(testSimpleFloat, -1)
	void testSimpleFloat();
	
	TEST(testMaxDoublePrecision_Irrational, -1)
	void testMaxDoublePrecision_Irrational();
	
	TEST(testMaxDoublePrecision, -1)
	void testMaxDoublePrecision();
	
	TEST(testSimpleNonCueBasedRetrievalOfNonExistingLTI, -1)
	void testSimpleNonCueBasedRetrievalOfNonExistingLTI();
	
	TEST(testNegQuery, -1)
	void testNegQuery();
	
	TEST(testNegStringFloat, -1)
	void testNegStringFloat();
	
	TEST(testNegQueryNoHash, -1)
	void testNegQueryNoHash();
	
	TEST(testCueSelection, -1)
	void testCueSelection();
	
	TEST(testISupport, -1)
	void testISupport();
	
	TEST(testISupportWithLearning, -1)
	void testISupportWithLearning();

	TEST(testSimpleNonCueBasedRetrieval_ActivationRecency, -1)
	void testSimpleNonCueBasedRetrieval_ActivationRecency();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery, -1)
	void testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationFrequency, -1)
	void testSimpleNonCueBasedRetrieval_ActivationFrequency();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable, -1)
	void testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive, -1)
	void testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental, -1)
	void testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental();

	TEST(testSpreadingActivation_AlphabetAgentAllOn, -1)
    void testSpreadingActivation_AlphabetAgentAllOn();

	TEST(testDbBackupAndLoadTests, -1)
	void testDbBackupAndLoadTests();
	
	TEST(testReadCSoarDB, -1)
	void testReadCSoarDB();

	TEST(testMultiAgent, -1)
	void testMultiAgent();

    // Tests for LTI Aliases (LTI string constants in CLI commands)
	TEST(testLTIAlias_SameRoot, -1)
	void testLTIAlias_SameRoot();

	TEST(testLTIAlias_RootAndValue, -1)
	void testLTIAlias_RootAndValue();

	TEST(testLTIAlias_CLICommands, -1)
	void testLTIAlias_CLICommands();
};

#endif /* SMemFunctionalTests_cpp */
