#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "EpMemFunctionalTests.hpp"

#import "XCTestDefines.h"

TEST_SETUP(EpMemFunctionalTests)

XC_TEST(testCountEpMem)
XC_TEST(testHamilton)
XC_TEST(testFilterEpMem)
XC_TEST(testAddCommand)
XC_TEST(testInclusions)
XC_TEST(testDeliberateStorage)
XC_TEST(testKB)
XC_TEST(testSingleStoreRetrieve)
XC_TEST(testOddEven)
XC_TEST(testBeforeEpMem)
XC_TEST(testAfterEpMem)
XC_TEST(testAllNegQueriesEpMem)
XC_TEST(testBeforeAfterProhibitEpMem)
XC_TEST(testMaxDoublePrecision_Irrational)
XC_TEST(testMaxDoublePrecisionEpMem)
XC_TEST(testNegativeEpisode)
XC_TEST(testNonExistingEpisode)
XC_TEST(testSimpleFloatEpMem)
XC_TEST(testCyclicQuery)
XC_TEST(testWMELength_OneCycle)
XC_TEST(testWMELength_FiveCycle)
XC_TEST(testWMELength_InfiniteCycle)
XC_TEST(testWMELength_MultiCycle)
XC_TEST(testWMActivation_Balance0)
XC_TEST(testEpMemEncodeOutput_NoWMA)
XC_TEST(testEpMemEncodeOutput_WMA)
XC_TEST(testEpMemEncodeSelection_NoWMA)
XC_TEST(testEpMemEncodeSelection_WMA)
XC_TEST(testEpMemYRemoval)
XC_TEST(testEpMemSoarGroupTests)
XC_TEST(testReadCSoarDB)
XC_TEST(testMultiAgent)

@end
