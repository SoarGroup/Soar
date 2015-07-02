#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "FunctionalTests.hpp"

#import "XCTestDefines.h"

TEST_SETUP(FunctionalTests)

XC_TEST(testWaterJug)
XC_TEST(testWaterJugLookAhead)
XC_TEST(testWaterJugHierarchy)
XC_TEST(testTowersOfHanoi)
XC_TEST(testTowersOfHanoiFast)
XC_TEST(testEightPuzzle)
XC_TEST(testBlocksWorld)
XC_TEST(testBlocksWorldOperatorSubgoaling)
XC_TEST(testBlocksWorldLookAhead)
XC_TEST(testBlocksWorldLookAhead2)
XC_TEST(testBlocksWorldLookAheadRandom)
XC_TEST(testArithmetic)
XC_TEST(testCountTest)

@end
