// Performs suite creation functions

#include "AgentTestSuite.h"

#include <cppunit/extensions/TestFactoryRegistry.h>

CppUnit::Test * getAgentTestSuite() {
   CppUnit::TestFactoryRegistry &registry = 
                      CppUnit::TestFactoryRegistry::getRegistry();

   registry.registerFactory( 
      &CppUnit::TestFactoryRegistry::getRegistry( getAgentTestSuiteName() ) );

   return registry.makeTest();
}
