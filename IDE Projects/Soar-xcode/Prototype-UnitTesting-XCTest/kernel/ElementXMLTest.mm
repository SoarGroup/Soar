#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "ElementXMLTest.hpp"

#import "XCTestDefines.h"

TEST_SETUP(ElementXMLTest)

XC_TEST(testSimple)
XC_TEST(testChildren)
XC_TEST(testParse)
XC_TEST(testBinaryData)
XC_TEST(testEquals)

@end
