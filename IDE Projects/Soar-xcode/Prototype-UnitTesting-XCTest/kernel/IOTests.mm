#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "IOTests.hpp"

#import "XCTestDefines.h"

TEST_SETUP(IOTests)

XC_TEST(testInputLeak)
XC_TEST(testInputLeak2)
XC_TEST(testInputLeak3)
XC_TEST(testInputLeak4)
XC_TEST(testOutputLeak1)

@end
