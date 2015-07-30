#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "MultiAgentTest.hpp"

#import "XCTestDefines.h"

TEST_SETUP(MultiAgentTest)

XC_TEST(testOneAgentForSanity)
XC_TEST(testTwoAgents)
XC_TEST(testTenAgents)
XC_TEST(testMaxAgents)

@end
