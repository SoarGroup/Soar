#include "SampleTestSuite.h"
#include "SampleTest.h"
#include <cppunit/extensions/Orthodox.h>
#include <cppunit/TestResult.h>

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SampleTest,
                                       getSampleSuiteName() );

SampleTest::SampleTest():m_kernel(0),m_agent(0) {}
SampleTest::~SampleTest() {}

// Setting up a kernel and agent for all the tests in this section
void SampleTest::setUp() {
   m_kernel = create_kernel();
   m_agent = create_soar_agent(m_kernel, "SampleTestAgent");
   initialize_soar_agent(m_kernel, m_agent);
}

// Cleaning up after the tests have completed
void SampleTest::tearDown() {
   destroy_soar_agent(m_kernel, m_agent);
   destroy_kernel(m_kernel);
}

void SampleTest::testAgentName() {
   // Some very simple agent initialization tests.
   CPPUNIT_ASSERT(!strcmp(m_agent->name,"SampleTestAgent"));

   // A sample failure.
   //CPPUNIT_ASSERT(false);
}

