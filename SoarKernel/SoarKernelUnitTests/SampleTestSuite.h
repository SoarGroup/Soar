// A simple sample test suite

#include <cppunit/TestSuite.h>

// Just keeps the name of the test suite confined to one location
inline std::string getSampleSuiteName(){
   return "SampleSuite";
}

// Returns the top level test
CppUnit::Test * getTopLevelSuite();
