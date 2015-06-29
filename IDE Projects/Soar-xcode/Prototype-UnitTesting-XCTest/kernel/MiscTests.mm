#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "MiscTests.hpp"

#import "XCTestDefines.h"

TEST_SETUP(MiscTests)

XC_TEST(testInstiationDeallocationStackOverflow)
XC_TEST(test_clog)
XC_TEST(test_gp)
XC_TEST(test_echo)
XC_TEST(test_ls)
XC_TEST(test_stats)
XC_TEST(testWrongAgentWmeFunctions)
XC_TEST(testRHSRand)
XC_TEST(testMultipleKernels)
XC_TEST(testSmemArithmetic)
XC_TEST(testSource)
XC_TEST(testSoarRand)
XC_TEST(testPreferenceDeallocation)
XC_TEST(testSoarDebugger)

@end
