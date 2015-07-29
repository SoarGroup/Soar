#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "AgentTest.hpp"

#import "XCTestDefines.h"

TEST_SETUP(AgentTest)

XC_TEST(testDefaultStopPhaseIsApply)
XC_TEST(testSetStopPhaseSetsTheStopPhaseProperty)
XC_TEST(testGetGoalStack)

@end
