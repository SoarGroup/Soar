// SoarKernelUnitTests.cpp : Defines the entry point for the console application.
//
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "AgentTestSuite.h"

#include <iostream>

int main(int argc, char* argv[])
{
   std::cout << "Running unit tests..." << std::endl;

   CppUnit::TextUi::TestRunner runner;
   runner.addTest( getAgentTestSuite() );   // Add the sample suite to the test runner

   // Run the test.
   bool wasSuccessful = runner.run( "" );

   if ( wasSuccessful ) {
      std::cout << std::endl << "All tests successful!" << std::endl; 
   } else {
      std::cout << std::endl << "Some tests failed!" << std::endl;
   }

   char temp;
   std::cin >> temp;
	return 0;
}

