//
//  FullTests.hpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FullTests_cpp
#define FullTests_cpp

#include "portability.h"

#include <string>
#include <vector>
#include <sstream>
#include <bitset>
#include <string>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_Utils.h"
#include "sml_ClientWMElement.h"
#include "sml_Names.h"
#include "thread_Event.h"
#include "soarversion.h"

#include "handlers.hpp"
#include "TestCategory.hpp"

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif // !_WIN32

struct OptionsStruct
{
	bool useClientThread;
	bool fullyOptimized;
	bool verbose;
	bool remote;
	bool autoCommitDisabled;

	void reset()
	{
		useClientThread = false;
		fullyOptimized = false;
		verbose = false;
		remote = false;
		autoCommitDisabled = false;
	}
};

class FullTests_Parent
{
public:
	void testInit();
	void testProductions();
    void testUngroundedLHS();
	void testRHSHandler();
	void testClientMessageHandler();
	void testFilterHandler();
	void testWMEs();
	void testXML();
	void testAgent();
	void testSimpleCopy();
	void testSimpleReteNetLoader();
	void test64BitReteNet();
	void testOSupportCopyDestroy();
	void testOSupportCopyDestroyCircularParent();
	void testOSupportCopyDestroyCircular();
	void testSynchronize();
	void testRunningAgentCreation();
	void testEventOrdering();
	void testStatusCompleteDuplication();
	void testStopSoarVsInterrupt();
	void testSharedWmeSetViolation();
	void testEchoEquals();
	void testFindAttrPipes();
	void testTemplateVariableNameBug();
	void testNegatedConjunctiveChunkLoopBug510();
	void testGDSBug1144();
	void testGDSBug1011();
	void testLearn();
	void testSVS();
	void testPreferenceSemantics();
	void testMatchTimeInterrupt();
	void testNegatedConjunctiveTestReorder();
	void testNegatedConjunctiveTestUnbound();
	void testCommandToFile();
	void testConvertIdentifier();
	void testOutputLinkRemovalOrdering();

	void before() { setUp(); }
	void after(bool caught) { tearDown(caught); }

	virtual void setUp();
	virtual void tearDown(bool caught);

	void createSoar();
	void destroySoar();
	int spawnListener();
	void cleanUpListener();

	void loadProductions(std::string productions);

	OptionsStruct m_Options;
	sml::Kernel* m_pKernel;
	sml::Agent* agent;

	static const std::string kAgentName;

#ifdef _WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#else // _WIN32
	pid_t pid;
#endif // _WIN32

protected:
	TestRunner* runner;
};

class FullTests : public FullTests_Parent, public TestCategory
{
public:
	TEST_CATEGORY(FullTests);

	TEST(testInit, -1);
	void testInit() { this->FullTests_Parent::testInit(); }

	TEST(testProductions, -1);
	void testProductions() { this->FullTests_Parent::testProductions(); }

    TEST(testUngroundedLHS, -1);
    void testUngroundedLHS() { this->FullTests_Parent::testUngroundedLHS(); }

	TEST(testRHSHandler, -1);
	void testRHSHandler() { this->FullTests_Parent::testRHSHandler(); }

	TEST(testClientMessageHandler, -1);
	void testClientMessageHandler() { this->FullTests_Parent::testClientMessageHandler(); }

	TEST(testFilterHandler, -1);
	void testFilterHandler() { this->FullTests_Parent::testFilterHandler(); }

	TEST(testWMEs, -1);
	void testWMEs() { this->FullTests_Parent::testWMEs(); }

	TEST(testXML, -1);
	void testXML() { this->FullTests_Parent::testXML(); }

	TEST(testAgent, -1);
	void testAgent() { this->FullTests_Parent::testAgent(); }

	TEST(testSimpleCopy, -1);
	void testSimpleCopy() { this->FullTests_Parent::testSimpleCopy(); }

	TEST(testSimpleReteNetLoader, -1);
	void testSimpleReteNetLoader() { this->FullTests_Parent::testSimpleReteNetLoader(); }

	TEST(test64BitReteNet, -1);
	void test64BitReteNet() { this->FullTests_Parent::test64BitReteNet(); }

	TEST(testOSupportCopyDestroy, -1);
	void testOSupportCopyDestroy() { this->FullTests_Parent::testOSupportCopyDestroy(); }

	TEST(testOSupportCopyDestroyCircularParent, -1);
	void testOSupportCopyDestroyCircularParent() { this->FullTests_Parent::testOSupportCopyDestroyCircularParent(); }

	TEST(testOSupportCopyDestroyCircular, -1);
	void testOSupportCopyDestroyCircular() { this->FullTests_Parent::testOSupportCopyDestroyCircular(); }

	TEST(testSynchronize, -1);
	void testSynchronize() { this->FullTests_Parent::testSynchronize(); }

	TEST(testRunningAgentCreation, -1);
	void testRunningAgentCreation() { this->FullTests_Parent::testRunningAgentCreation(); }

	TEST(testEventOrdering, -1);
	void testEventOrdering() { this->FullTests_Parent::testEventOrdering(); }

	TEST(testStatusCompleteDuplication, -1);
	void testStatusCompleteDuplication() { this->FullTests_Parent::testStatusCompleteDuplication(); }

	TEST(testStopSoarVsInterrupt, -1);
	void testStopSoarVsInterrupt() { this->FullTests_Parent::testStopSoarVsInterrupt(); }

	TEST(testSharedWmeSetViolation, -1);
	void testSharedWmeSetViolation() { this->FullTests_Parent::testSharedWmeSetViolation(); }

	TEST(testEchoEquals, -1);
	void testEchoEquals() { this->FullTests_Parent::testEchoEquals(); }

	TEST(testFindAttrPipes, -1);
	void testFindAttrPipes() { this->FullTests_Parent::testFindAttrPipes(); }

	TEST(testTemplateVariableNameBug, -1);
	void testTemplateVariableNameBug() { this->FullTests_Parent::testTemplateVariableNameBug(); }

	TEST(testNegatedConjunctiveChunkLoopBug510, -1);
	void testNegatedConjunctiveChunkLoopBug510() { this->FullTests_Parent::testNegatedConjunctiveChunkLoopBug510(); }

	TEST(testGDSBug1144, -1);
	void testGDSBug1144() { this->FullTests_Parent::testGDSBug1144(); }

	TEST(testGDSBug1011, -1);
	void testGDSBug1011() { this->FullTests_Parent::testGDSBug1011(); }

	TEST(testLearn, -1);
	void testLearn() { this->FullTests_Parent::testLearn(); }

    #ifndef NO_SVS
	TEST(testSVS, -1);
    #endif
	void testSVS() { this->FullTests_Parent::testSVS(); }

	TEST(testPreferenceSemantics, -1);
	void testPreferenceSemantics() { this->FullTests_Parent::testPreferenceSemantics(); }

	TEST(testMatchTimeInterrupt, -1);
	void testMatchTimeInterrupt() { this->FullTests_Parent::testMatchTimeInterrupt(); }

	TEST(testNegatedConjunctiveTestReorder, -1);
	void testNegatedConjunctiveTestReorder() { this->FullTests_Parent::testNegatedConjunctiveTestReorder(); }

	TEST(testNegatedConjunctiveTestUnbound, -1);
	void testNegatedConjunctiveTestUnbound() { this->FullTests_Parent::testNegatedConjunctiveTestUnbound(); }

	TEST(testCommandToFile, -1);
	void testCommandToFile() { this->FullTests_Parent::testCommandToFile(); }

	TEST(testConvertIdentifier, -1);
	void testConvertIdentifier() { this->FullTests_Parent::testConvertIdentifier(); }

	TEST(testOutputLinkRemovalOrdering, -1);
	void testOutputLinkRemovalOrdering() { this->FullTests_Parent::testOutputLinkRemovalOrdering(); }

	void before() { setUp(); }
	void after(bool caught) { tearDown(caught); }

	virtual void setUp();
	virtual void tearDown(bool caught) { FullTests_Parent::tearDown(caught); }
};

#endif /* FullTests_cpp */
