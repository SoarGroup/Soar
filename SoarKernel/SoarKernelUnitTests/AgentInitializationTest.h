// A simple sample test 

#ifndef AGENTINITIALIZATIONTEST_H
#define AGENTINITIALIZATIONTEST_H

#include "kernel_struct.h"
#include "agent.h"
#include <cppunit/extensions/HelperMacros.h>

//
// This class should test anything related to the creation of an agent.
//
class AgentInitializationTest : public CppUnit::TestFixture
{
  // Helper macros for creating the tests
  CPPUNIT_TEST_SUITE( AgentInitializationTest );
  CPPUNIT_TEST( testAgentName );
  CPPUNIT_TEST( testAgentWMState );
  CPPUNIT_TEST( testAgentProductions );
  CPPUNIT_TEST_SUITE_END();

public:
  AgentInitializationTest();
  virtual ~AgentInitializationTest();

  // These functions handle creating and destroying any helper
  // objects needed by the individual unit tests.
  virtual void setUp();
  virtual void tearDown();

  //
  // These tests aren't intended to test every aspect of the agent
  // structure to make sure that they are initialized correctly.
  // Only those elements that are typically part of the "external"
  // interface of the struct.
  //

  // A simple test that verifies that a created agent has the
  // correct name.
  void testAgentName();

  // A simple test that verifies the initial production memory state
  // of the agent
  void testAgentProductions();

  // A simple test that verifies the initial WM state of the agent
  // structure
  void testAgentWMState();

private:
  // To avoid the compiler creating these by default
  AgentInitializationTest( const AgentInitializationTest &copy );
  void operator =( const AgentInitializationTest &copy );

private:
   // Support objects for the individual tests
   Kernel* m_kernel;
   agent* m_agent;
};



#endif  // AGENTINITIALIZATIONTEST_H
