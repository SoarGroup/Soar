#ifndef FullTests_USE_CLIENT_THREAD
#define FullTests_USE_CLIENT_THREAD

#include "FullTests.hpp"
#include "TestCategory.hpp"

class FullTestsClientThread : public FullTests_Parent, public TestCategory
{
public:
	TEST_CATEGORY(FullTestsClientThread)
	
	TEST(testInit, -1)
	void testInit() { this->FullTests_Parent::testInit(); }
	
	TEST(testProductions, -1)
	void testProductions() { this->FullTests_Parent::testProductions(); }
	
	TEST(testRHSHandler, -1)
	void testRHSHandler() { this->FullTests_Parent::testRHSHandler(); }
	
	TEST(testClientMessageHandler, -1)
	void testClientMessageHandler() { this->FullTests_Parent::testClientMessageHandler(); }
	
	TEST(testFilterHandler, -1)
	void testFilterHandler() { this->FullTests_Parent::testFilterHandler(); }
	
	TEST(testWMEs, -1)
	void testWMEs() { this->FullTests_Parent::testWMEs(); }
	
	TEST(testXML, -1)
	void testXML() { this->FullTests_Parent::testXML(); }
	
	TEST(testAgent, -1)
	void testAgent() { this->FullTests_Parent::testAgent(); }
	
	TEST(testSimpleCopy, -1)
	void testSimpleCopy() { this->FullTests_Parent::testSimpleCopy(); }
	
	TEST(testSimpleReteNetLoader, -1)
	void testSimpleReteNetLoader() { this->FullTests_Parent::testSimpleReteNetLoader(); }
	
	TEST(test64BitReteNet, -1)
	void test64BitReteNet() { this->FullTests_Parent::test64BitReteNet(); }
	
	TEST(testOSupportCopyDestroy, -1)
	void testOSupportCopyDestroy() { this->FullTests_Parent::testOSupportCopyDestroy(); }
	
	TEST(testOSupportCopyDestroyCircularParent, -1)
	void testOSupportCopyDestroyCircularParent() { this->FullTests_Parent::testOSupportCopyDestroyCircularParent(); }
	
	TEST(testOSupportCopyDestroyCircular, -1)
	void testOSupportCopyDestroyCircular() { this->FullTests_Parent::testOSupportCopyDestroyCircular(); }
	
	TEST(testSynchronize, -1)
	void testSynchronize() { this->FullTests_Parent::testSynchronize(); }
	
	TEST(testRunningAgentCreation, -1)
	void testRunningAgentCreation() { this->FullTests_Parent::testRunningAgentCreation(); }
	
	TEST(testEventOrdering, -1)
	void testEventOrdering() { this->FullTests_Parent::testEventOrdering(); }
	
	TEST(testStatusCompleteDuplication, -1)
	void testStatusCompleteDuplication() { this->FullTests_Parent::testStatusCompleteDuplication(); }
	
	TEST(testStopSoarVsInterrupt, -1)
	void testStopSoarVsInterrupt() { this->FullTests_Parent::testStopSoarVsInterrupt(); }
	
	TEST(testSharedWmeSetViolation, -1)
	void testSharedWmeSetViolation() { this->FullTests_Parent::testSharedWmeSetViolation(); }
	
	TEST(testEchoEquals, -1)
	void testEchoEquals() { this->FullTests_Parent::testEchoEquals(); }
	
	TEST(testFindAttrPipes, -1)
	void testFindAttrPipes() { this->FullTests_Parent::testFindAttrPipes(); }
	
	TEST(testTemplateVariableNameBug, -1)
	void testTemplateVariableNameBug() { this->FullTests_Parent::testTemplateVariableNameBug(); }
	
	TEST(testNegatedConjunctiveChunkLoopBug510, -1)
	void testNegatedConjunctiveChunkLoopBug510() { this->FullTests_Parent::testNegatedConjunctiveChunkLoopBug510(); }
	
	TEST(testGDSBug1144, -1)
	void testGDSBug1144() { this->FullTests_Parent::testGDSBug1144(); }
	
	TEST(testGDSBug1011, -1)
	void testGDSBug1011() { this->FullTests_Parent::testGDSBug1011(); }
	
	TEST(testLearn, -1)
	void testLearn() { this->FullTests_Parent::testLearn(); }
	
#ifndef NO_SVS
	TEST(testSVS, -1)
#endif
	void testSVS() { this->FullTests_Parent::testSVS(); }
	
	TEST(testPreferenceSemantics, -1)
	void testPreferenceSemantics() { this->FullTests_Parent::testPreferenceSemantics(); }
	
	TEST(testMatchTimeInterrupt, -1)
	void testMatchTimeInterrupt() { this->FullTests_Parent::testMatchTimeInterrupt(); }
	
	TEST(testNegatedConjunctiveTestReorder, -1)
	void testNegatedConjunctiveTestReorder() { this->FullTests_Parent::testNegatedConjunctiveTestReorder(); }
	
	TEST(testNegatedConjunctiveTestUnbound, -1)
	void testNegatedConjunctiveTestUnbound() { this->FullTests_Parent::testNegatedConjunctiveTestUnbound(); }
	
	TEST(testCommandToFile, -1)
	void testCommandToFile() { this->FullTests_Parent::testCommandToFile(); }
	
	TEST(testConvertIdentifier, -1)
	void testConvertIdentifier() { this->FullTests_Parent::testConvertIdentifier(); }
	
	TEST(testOutputLinkRemovalOrdering, -1)
	void testOutputLinkRemovalOrdering() { this->FullTests_Parent::testOutputLinkRemovalOrdering(); }
	
	void before() { setUp(); }
	void after(bool caught) { tearDown(caught); }
	
	virtual void setUp();
	virtual void tearDown(bool caught) { FullTests_Parent::tearDown(caught); }
};

#endif
