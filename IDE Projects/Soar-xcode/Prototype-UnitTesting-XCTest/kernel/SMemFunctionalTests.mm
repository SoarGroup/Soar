#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "SMemFunctionalTests.hpp"

#import "XCTestDefines.h"

TEST_SETUP(SMemFunctionalTests)

XC_TEST(testSimpleCueBasedRetrieval)
XC_TEST(testSimpleNonCueBasedRetrieval)
XC_TEST(testSimpleStore)
XC_TEST(testTrivialMathQuery)
XC_TEST(testBadMathQuery)
XC_TEST(testMaxQuery)
XC_TEST(testMaxMixedTypes)
XC_TEST(testMaxMultivalued)
XC_TEST(testMin)
XC_TEST(testMaxNegQuery)
XC_TEST(testGreater)
XC_TEST(testLess)
XC_TEST(testGreaterOrEqual)
XC_TEST(testLessOrEqual)
XC_TEST(testLessWithNeg)
XC_TEST(testLessNoSolution)
XC_TEST(testMirroring)
XC_TEST(testMergeAdd)
XC_TEST(testMergeNone)
XC_TEST(testSimpleStoreMultivaluedAttribute)
XC_TEST(testSimpleFloat)
XC_TEST(testMaxDoublePrecision_Irrational)
XC_TEST(testMaxDoublePrecision)
XC_TEST(testSimpleNonCueBasedRetrievalOfNonExistingLTI)
XC_TEST(testNegQuery)
XC_TEST(testNegStringFloat)
XC_TEST(testNegQueryNoHash)
XC_TEST(testCueSelection)
XC_TEST(testISupport)
XC_TEST(testISupportWithLearning)
XC_TEST(testSmemArithmetic)
XC_TEST(testSimpleNonCueBasedRetrieval_ActivationRecency)
XC_TEST(testSimpleNonCueBasedRetrieval_ActivationRecency_WithoutActivateOnQuery)
XC_TEST(testSimpleNonCueBasedRetrieval_ActivationFrequency)
XC_TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Stable)
XC_TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Naive)
XC_TEST(testSimpleNonCueBasedRetrieval_ActivationBaseLevel_Incremental)
XC_TEST(testDbBackupAndLoadTests)
XC_TEST(testReadCSoarDB)
XC_TEST(testMultiAgent)

@end
