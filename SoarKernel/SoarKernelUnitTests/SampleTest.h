// A simple sample test 

#ifndef SAMPLETEST_H
#define SAMPLETEST_H

#include "kernel_struct.h"
#include "agent.h"
#include <cppunit/extensions/HelperMacros.h>

// The Test Fixture is just a simple test with the addition
// of the setUp and tearDown functions creating necessary 
// support objects for each individual test.
class SampleTest : public CppUnit::TestFixture
{
  // Helper macros for creating the tests
  CPPUNIT_TEST_SUITE( SampleTest );
  CPPUNIT_TEST( testAgentName );
  CPPUNIT_TEST_SUITE_END();

public:
  SampleTest();
  virtual ~SampleTest();

  // These functions handle creating and destroying any helper
  // objects needed by the individual unit tests.
  virtual void setUp();
  virtual void tearDown();

  // A simple test that verifies that a created agent has the
  // correct name.
  void testAgentName();

private:
  // To avoid the compiler creating these by default
  SampleTest( const SampleTest &copy );
  void operator =( const SampleTest &copy );

private:
   // Support objects for the individual tests
   Kernel* m_kernel;
   agent* m_agent;
};



#endif  // SAMPLETEST_H
