#import <XCTest/XCTest.h>

#include "TestHelpers.hpp"
#include "FullTestsRemote.hpp"

#import "XCTestDefines.h"

TEST_SETUP(FullTestsRemote)

XC_TEST(testInit)
XC_TEST(testProductions)
XC_TEST(testRHSHandler)
XC_TEST(testClientMessageHandler)
XC_TEST(testFilterHandler)
XC_TEST(testWMEs)
XC_TEST(testXML)
XC_TEST(testAgent)
XC_TEST(testSimpleCopy)
XC_TEST(testSimpleReteNetLoader)
XC_TEST(test64BitReteNet)
XC_TEST(testOSupportCopyDestroy)
XC_TEST(testOSupportCopyDestroyCircularParent)
XC_TEST(testOSupportCopyDestroyCircular)
XC_TEST(testSynchronize)
XC_TEST(testRunningAgentCreation)
XC_TEST(testEventOrdering)
XC_TEST(testStatusCompleteDuplication)
XC_TEST(testStopSoarVsInterrupt)
XC_TEST(testSharedWmeSetViolation)
XC_TEST(testEchoEquals)
XC_TEST(testFindAttrPipes)
XC_TEST(testTemplateVariableNameBug)
XC_TEST(testNegatedConjunctiveChunkLoopBug510)
XC_TEST(testGDSBug1144)
XC_TEST(testGDSBug1011)
XC_TEST(testLearn)
XC_TEST(testSVS)
XC_TEST(testPreferenceSemantics)
XC_TEST(testMatchTimeInterrupt)
XC_TEST(testNegatedConjunctiveTestReorder)
XC_TEST(testNegatedConjunctiveTestUnbound)
XC_TEST(testCommandToFile)
XC_TEST(testConvertIdentifier)
XC_TEST(testOutputLinkRemovalOrdering)

@end
