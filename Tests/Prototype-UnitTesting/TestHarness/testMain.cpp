//
//  testMain.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "TestCategory.hpp"
#include "TestRunner.hpp"

#include <functional>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <iostream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

// INCLUDE TEST HEADERS HERE

#include "AgentTest.hpp"
#include "BasicTests.hpp"
#include "FunctionalTests.hpp"
#include "EpMemFunctionalTests.hpp"
#include "SMemEpMemCombinedFunctionalTests.hpp"
#include "SMemFunctionalTests.hpp"

int main(int argc, char** argv)
{	
	const bool ShowTestOutput = false;
	
	std::condition_variable_any variable;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	
	std::vector<TestCategory*> tests;
	
	// DEFINE ALL TESTS HERE
	
	TEST_DECLARATION(AgentTest);
	TEST_DECLARATION(BasicTests);
	TEST_DECLARATION(FunctionalTests);
	TEST_DECLARATION(EpMemFunctionalTests);
	TEST_DECLARATION(SMemEpMemCombinedFunctionalTests);
	TEST_DECLARATION(SMemFunctionalTests);
	
	size_t successCount = 0;
	size_t testCount = 0;
	
	for (TestCategory* category : tests)
	{
		std::cout << std::endl << "================================================" << std::endl << "Running " << category->getCategoryName() << std::endl << "================================================" << std::endl;
		
		for (TestCategory::TestCategory_test test : category->getTests())
		{
			std::cout << std::get<2>(test) << ": ";
			std::cout.flush();
			
			TestFunction* function = std::get<0>(test);
			uint64_t timeout = std::get<1>(test) - 1000;

			TestRunner* runner = new TestRunner(category, function, &variable);
			
			std::thread (&TestRunner::run, runner).detach();
			
			uint64_t timeElapsed = 0;
			
			runner->ready.store(true);
			
			variable.notify_one();
			while (variable.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout || !runner->done)
			{
				std::cout << '.';
				std::cout.flush();
				timeElapsed += 1000;
				
				if (timeElapsed > timeout)
					break;
			}
			
			if (timeElapsed > timeout)
			{
				std::cout << "Timeout" << std::endl;
				std::cout.flush();
				
				runner->kill.store(true);
			}
			else if (!runner->failed)
			{
				std::cout << "Done" << std::endl;
				std::cout.flush();
			}
			else if (runner->failed)
			{
				std::cout << "Failed" << std::endl << "================================================" << std::endl << "Reason: ";
				std::cout << runner->failureMessage << std::endl << std::endl;
				std::cout.flush();
			}
			
			std::mutex mutex;
			std::unique_lock<std::mutex> lock(mutex);
			variable.wait(lock, [runner]{ return runner->done == true; });
			
			if (runner->kill)
			{
				std::cout << "Killed" << std::endl;
				std::cout.flush();
			}
			
			if (ShowTestOutput || runner->failed)
			{
				std::cout << std::get<2>(test) << " Output:" << std::endl;
				std::cout << runner->output.str() << "================================================" << std::endl << std::endl;
				std::cout.flush();
			}
			
			if (!runner->failed)
			{
				++successCount;
			}
			
			++testCount;
			
			delete runner;
		}
	}
	
	std::cout << "Completed " << successCount << "/" << testCount << " successfully. " << testCount - successCount << " failed." << std::endl;
	std::cout.flush();

#ifdef _MSC_VER
	if (IsDebuggerPresent())
	{
		std::cout << "Press enter to continue..." << std::endl;
		std::cin.get();
	}
#endif
}
