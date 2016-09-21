//
//  testMain.cpp
//  Prototype-UnitTesting
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

#include <signal.h>

#include "portability.h"

// INCLUDE TEST HEADERS HERE

#include "AgentTest.hpp"
#include "BasicTests.hpp"
#include "FunctionalTests.hpp"
#include "EpMemFunctionalTests.hpp"
#include "SMemEpMemCombinedFunctionalTests.hpp"
#include "SMemFunctionalTests.hpp"
#include "WmaFunctionalTests.hpp"
#include "AliasTest.hpp"
#include "ElementXMLTest.hpp"
#include "FullTests.hpp"
#include "FullTestsClientThreadFullyOptimized.hpp"
#include "FullTestsClientThread.hpp"
#include "FullTestsRemote.hpp"
#include "IOTests.hpp"
#include "MiscTests.hpp"
#include "MultiAgentTest.hpp"
#include "TokenizerTest.hpp"
#include "ChunkingTests.hpp"

#include "SimpleListener.hpp"

void usage(std::string arg0)
{
    std::cout << "OVERVIEW: " << arg0 << ": Soar Unit Testing Framwork " << std::endl << std::endl;
    std::cout << "Usage: " << arg0 << " : [options]" << std::endl << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "\t" << "-c --category"                    << "\t\t\t\t" << "Run only these categories." << std::endl;
    std::cout << "\t" << "-t --test"                        << "\t\t\t\t" << "Run only these tests." << std::endl;
    std::cout << "\t" << "-E --exclude-category"            << "\t\t\t" << "Exclude this category." << std::endl;
    std::cout << "\t" << "-e --exclude-test"                << "\t\t\t" << "Exclude this test." << std::endl;
    std::cout << "\t" << "-F --expected-failure-category"   << "\t\t" << "Ignore failures in this category." << std::endl;
    std::cout << "\t" << "-f --expected-failure-test"       << "\t\t" << "Ignore this test failing." << std::endl;
    std::cout << "\t" << "-h --help"                        << "\t\t\t\t" << "This help message." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char** argv)
{
    std::vector<std::string> runCategories;
    std::vector<std::string> runTests;
    std::vector<std::string> excludeCategories;
    std::vector<std::string> excludeTests;
    std::vector<std::string> expectedFailureCategories;
    std::vector<std::string> expectedFailureTests;

	for (int index = 1; index < argc; ++index)
	{
		std::string argument(argv[index]);

        std::string parameter = "";

        if (index + 1 < argc)
            parameter = argv[index+1];

		if (argument == "--listener")
		{
			SimpleListener simpleListener(600);
			return simpleListener.run();
		}
        else if ((argument == "--category" || argument == "-c") && parameter.length() > 0)
        {
            runCategories.push_back(parameter);
            ++index;
        }
        else if ((argument == "--test" || argument == "-t") && parameter.length() > 0)
        {
            runTests.push_back(parameter);
            ++index;
        }
        else if ((argument == "--exclude-category" || argument == "-E") && parameter.length() > 0)
        {
            excludeCategories.push_back(parameter);
            ++index;
        }
        else if ((argument == "--exclude-test" || argument == "-e") && parameter.length() > 0)
        {
            excludeTests.push_back(parameter);
            ++index;
        }
        else if ((argument == "--expected-failure-category" || argument == "-F") && parameter.length() > 0)
        {
            expectedFailureCategories.push_back(parameter);
            ++index;
        }
        else if ((argument == "--expected-failure-test" || argument == "-f") && parameter.length() > 0)
        {
            expectedFailureTests.push_back(parameter);
            ++index;
        }
        else if ((argument == "--help" || argument == "-h"))
        {
            usage(argv[0]);
            exit(0);
        }
		else
		{
			std::cerr << "Unknown argument '" << argument << "'." << std::endl;
            usage(argv[0]);
            exit(1);
		}
	}

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
	TEST_DECLARATION(WmaFunctionalTests);
	TEST_DECLARATION(AliasTest);
	TEST_DECLARATION(ElementXMLTest);
	TEST_DECLARATION(FullTestsClientThreadFullyOptimized);
	TEST_DECLARATION(FullTestsClientThread);
	TEST_DECLARATION(FullTests);
	TEST_DECLARATION(FullTestsRemote);
	TEST_DECLARATION(IOTests);
	TEST_DECLARATION(MiscTests);
	TEST_DECLARATION(MultiAgentTest);
	TEST_DECLARATION(TokenizerTest);
	TEST_DECLARATION(ChunkingTests);

	size_t successCount = 0;
    size_t expectedFailureCount = 0;
	size_t testCount = 0;
    size_t skipCount = 0;
	
	std::vector<std::string> failedTests;
    std::vector<std::string> ignoredFailureTests;
	
	for (TestCategory* category : tests)
	{
        if (runCategories.size() > 0 && std::find(runCategories.begin(), runCategories.end(), category->getCategoryName()) == runCategories.end())
        {
            continue;
        }

        if (excludeCategories.size() > 0 && std::find(excludeCategories.begin(), excludeCategories.end(), category->getCategoryName()) != excludeCategories.end())
        {
            skipCount += category->getTests().size();
            continue;
        }

		std::cout << std::endl << "================================================" << std::endl << "Running " << category->getCategoryName() << std::endl << "================================================" << std::endl;

		for (TestCategory::TestCategory_test test : category->getTests())
        {
            if (runTests.size() > 0 && std::find(runTests.begin(), runTests.end(), category->getCategoryName() + "::" + std::get<2>(test)) == runTests.end())
            {
                continue;
            }

            if (excludeTests.size() > 0 && std::find(excludeTests.begin(), excludeTests.end(), category->getCategoryName() + "::" + std::get<2>(test)) != excludeTests.end())
            {
                ++skipCount;
                continue;
            }

			std::cout << std::get<2>(test) << ": ";
			std::cout.flush();
			
			std::function<void ()> function = std::get<0>(test);
			uint64_t timeout = std::get<1>(test) - 1000;

			TestRunner* runner = new TestRunner(category, function, &variable);
			
			std::thread (&TestRunner::run, runner).detach();
			
			uint64_t timeElapsed = 0;
			
			runner->ready.store(true);
			
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			
			variable.notify_one();
			while (!runner->done && variable.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout)
			{
				std::cout << '.';
				std::cout.flush();
				timeElapsed += 1000;
				
				if (timeElapsed > timeout)
					break;
			}

            bool unexpectedFailure = false;
			if (timeElapsed > timeout)
			{
				std::cout << "Timeout" << std::endl;
				std::cout.flush();
				
				runner->kill.store(true);
				
				failedTests.push_back(category->getCategoryName() + ": " + std::get<2>(test));
			}
			else if (!runner->failed)
			{
				std::cout << "Done" << std::endl;
				std::cout.flush();
			}
            else if (runner->failed && (std::find(expectedFailureCategories.begin(), expectedFailureCategories.end(), category->getCategoryName()) != expectedFailureCategories.end() || std::find(expectedFailureTests.begin(), expectedFailureTests.end(), category->getCategoryName() + "::" + std::get<2>(test)) != expectedFailureTests.end()))
            {
                std::cout << "Failed. Ignoring." << std::endl;
                std::cout.flush();
                ++expectedFailureCount;
            }
			else if (runner->failed)
			{
				std::cout << "Failed" << std::endl << "================================================" << std::endl << "Reason: ";
				std::cout << runner->failureMessage << std::endl << std::endl;
				std::cout.flush();
				
				failedTests.push_back(category->getCategoryName() + "::" + std::get<2>(test));
                unexpectedFailure = true;
			}
			
			std::mutex mutex;
			std::unique_lock<std::mutex> lock(mutex);
			variable.wait(lock, [runner]{ return runner->done == true; });
			
			if (runner->kill)
			{
				std::cout << "Killed" << std::endl;
				std::cout.flush();
			}
			
			if (ShowTestOutput || unexpectedFailure)
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
	
    std::cout << "Completed " << successCount << "/" << testCount << " successfully." << std::endl;
    std::cout << testCount - successCount - expectedFailureCount << " failed unexpectedly. " << expectedFailureCount << " failed as expected." << std::endl;
    std::cout << skipCount << " tests skipped." << std::endl;
    
	std::cout.flush();
	
	if (failedTests.size() > 0)
	{
		std::cout << "Failed Tests: " << std::endl;
		
		for (std::string test : failedTests)
		{
			std::cout << test << std::endl;
		}
	}

#ifdef _MSC_VER
	if (IsDebuggerPresent())
	{
		std::cout << "Press enter to continue..." << std::endl;
		std::cin.get();
	}
#endif
	
	if (failedTests.size() > 0)
		return 1;
	else
		return 0;
}
