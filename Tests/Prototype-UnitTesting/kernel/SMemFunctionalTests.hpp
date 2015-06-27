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
	FUNCTIONAL_TEST_CATEGORY(SMemFunctionalTests);
	
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
	
	TEST(testMirroring, -1)
	void testMirroring();
	
	TEST(testMergeAdd, -1)
	void testMergeAdd();
	
	TEST(testMergeNone, -1)
	void testMergeNone();
	
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

public:
	TEST(testSimpleNonCueBasedRetrieval_ActivationRecency, -1)
	void testSimpleNonCueBasedRetrieval_ActivationRecency();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery, -1)
	void testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationFrequency, -1)
	void testSimpleNonCueBasedRetrieval_ActivationFrequency();
	
private:
	bool checkActivationValues(std::string activationString, std::vector<double> lowEndExpectations, std::vector<double> highEndExpectations, const char* file, const int line);
	
public:
	TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable, -1)
	void testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive, -1)
	void testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive();
	
	TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental, -1)
	void testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental();
	
	TEST(testDbBackupAndLoadTests, -1)
	void testDbBackupAndLoadTests();
	
	TEST(testReadCSoarDB, -1)
	void testReadCSoarDB();
	
	// CSoar doesn't use Garbage Collection
//	TEST(testSimpleStoreGC, -1)
//	void testSimpleStoreGC();
	
	TEST(testMultiAgent, -1)
	void testMultiAgent();
};

#endif /* SMemFunctionalTests_cpp */
