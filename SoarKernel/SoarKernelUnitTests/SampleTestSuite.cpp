// Performs suite creation functions

#include "SampleTestSuite.h"

#include <cppunit/extensions/TestFactoryRegistry.h>

CppUnit::Test * getSampleSuite() {
   CppUnit::TestFactoryRegistry &registry = 
                      CppUnit::TestFactoryRegistry::getRegistry();

   registry.registerFactory( 
      &CppUnit::TestFactoryRegistry::getRegistry( getSampleSuiteName() ) );

   return registry.makeTest();
}
