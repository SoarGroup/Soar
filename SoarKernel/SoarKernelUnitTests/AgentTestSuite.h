// A test case for various things related to the agent structure
#include <cppunit/TestSuite.h>

// The name of the agent test suite
inline std::string getAgentTestSuiteName() {
   return "AgentTestSuite";
}

// Handles creating the agent test suite
CppUnit::Test* getAgentTestSuite();
