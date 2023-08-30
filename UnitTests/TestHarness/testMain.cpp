//
//  testMain.cpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include <functional>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <fstream>

#include <signal.h>

#include "portability.h"

// INCLUDE TEST HEADERS HERE

#include "AgentTest.hpp"
#include "AliasTest.hpp"
#include "BasicTests.hpp"
#include "BuiltinRHSTests.hpp"
#include "ChunkingTests.hpp"
#include "ChunkingDemoTests.hpp"
#include "ElementXMLTest.hpp"
#include "EpMemFunctionalTests.hpp"
#include "ExternalLibraryTest.hpp"
#include "FullTests.hpp"
#include "FullTestsClientThread.hpp"
#include "FullTestsClientThreadFullyOptimized.hpp"
#include "FullTestsRemote.hpp"
#include "FunctionalTests.hpp"
#include "IOTests.hpp"
#include "MiscTests.hpp"
#include "MultiAgentTest.hpp"
#include "SimpleListener.hpp"
#include "SMemFunctionalTests.hpp"
#include "TokenizerTest.hpp"
#include "wma/WmaFunctionalTests.hpp"
#include "TestCategory.hpp"
#include "TestRunner.hpp"

#if defined(_WIN32) || defined(WIN32)
#include <sstream>
std::string OS = static_cast<std::ostringstream&>(std::ostringstream() << "Microsoft C/C++ " << _MSC_FULL_VER).str();
#elif defined(__APPLE__)
std::string OS = "OS X";
#elif defined(__linux__)
std::string OS = "Linux";
#elif defined(__unix__)
std::string OS = "Unknown Unix";
#else
std::string OS = "Unknown OS";
#endif

void usage(std::string arg0)
{
    std::cout << "Soar Unit Tests" << std::endl << std::endl;
    std::cout << "Usage: " << arg0 << " [options]" << std::endl << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "\t" << "-c --category"                    << "\t\t\t\t" << "Run only these categories." << std::endl;
    std::cout << "\t" << "-t --test"                        << "\t\t\t\t" << "Run only these tests." << std::endl;
    std::cout << "\t" << "-E --exclude-category"            << "\t\t\t" << "Exclude this category." << std::endl;
    std::cout << "\t" << "-e --exclude-test"                << "\t\t\t" << "Exclude this test." << std::endl;
    std::cout << "\t" << "-F --expected-failure-category"   << "\t\t" << "Ignore failures in this category." << std::endl;
    std::cout << "\t" << "-f --expected-failure-test"       << "\t\t" << "Ignore this test failing." << std::endl;
    std::cout << "\t" << "-n --no-refcount-leak-check"      << "\t\t" << "Do not init-soar and check for refcount leaks." << std::endl;
    std::cout << "\t" << "-a --after-action-reports"        << "\t\t" << "Generate reports for each agent trial." << std::endl;
    std::cout << "\t" << "-l --logs"                        << "\t\t\t\t" << "Record logs of agent trials." << std::endl;
    std::cout << "\t" << "-x --no-explainer"                << "\t\t\t" << "Run learning agents with explainer on." << std::endl;
    std::cout << "\t" << "-r --run-debug-mode"              << "\t\t\t" << "Don't force strict unit test settings." << std::endl;
    std::cout << "\t" << "-h --help"                        << "\t\t\t\t" << "This help message." << std::endl;
    std::cout << "\t" << "-s --silent"                      << "\t\t\t\t" << "Always return 0.  Read Test.xml for results." << std::endl;
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
    bool silent = false;

    #if defined(_WIN32) || defined(WIN32)
    // Allows printing emoji ✅
    SetConsoleOutputCP(CP_UTF8);
    #endif

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
        else if ((argument == "--run-debug-mode" || argument == "-r"))
        {
            SoarHelper::run_as_unit_test = false;
        }
        else if ((argument == "--no-explainer" || argument == "-x"))
        {
            SoarHelper::no_explainer = true;
        }
        else if ((argument == "--after-action-reports" || argument == "-a"))
        {
            SoarHelper::no_explainer = false;
            SoarHelper::save_after_action_report = true;
        }
        else if ((argument == "--logs" || argument == "-l"))
        {
            SoarHelper::save_logs = true;
        }
        else if ((argument == "--no-refcount-leak-check" || argument == "-n"))
        {
            SoarHelper::no_init_soar = true;
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
        else if ((argument == "--silent" || argument == "-s"))
        {
            silent = true;
        }
        else
        {
            std::cerr << "Unknown argument '" << argument << "'." << std::endl;
            usage(argv[0]);
            exit(1);
        }
    }

    usage(argv[0]);

    const bool ShowTestOutput = false;

    std::condition_variable_any variable;
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

    std::vector<TestCategory*> tests;

    TEST_DECLARATION(AgentTest);
    TEST_DECLARATION(AliasTest);
    TEST_DECLARATION(BasicTests);
    TEST_DECLARATION(BuiltinRHSTests);
    TEST_DECLARATION(ChunkingDemoTests);
    TEST_DECLARATION(ChunkingTests);
    TEST_DECLARATION(EpMemFunctionalTests);
    TEST_DECLARATION(ElementXMLTest);
    TEST_DECLARATION(ExternalLibraryTest);
    TEST_DECLARATION(FullTests);
    TEST_DECLARATION(FullTestsClientThreadFullyOptimized);
    TEST_DECLARATION(FullTestsClientThread);
    //	TEST_DECLARATION(FullTestsRemote);
    TEST_DECLARATION(FunctionalTests);
    TEST_DECLARATION(IOTests);
    TEST_DECLARATION(MiscTests);
    TEST_DECLARATION(MultiAgentTest);
    TEST_DECLARATION(SMemFunctionalTests);
    TEST_DECLARATION(TokenizerTest);
    TEST_DECLARATION(WmaFunctionalTests);

    size_t successCount = 0;
    size_t expectedFailureCount = 0;
    size_t testCount = 0;
    size_t skipCount = 0;

    struct failure {
            std::string name;
            std::string output;
    };

    std::vector<failure> failedTests;
    std::vector<std::string> ignoredFailureTests;

    std::ofstream xml("TestResults.xml");
    xml << "<testsuite tests=\"" << tests.size() << "\">" << std::endl;

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

        std::cout << "======== " << category->getCategoryName() << " ========" << std::endl;

        for (TestCategory::TestCategory_test test : category->getTests())
        {
            if (runTests.size() > 0 && std::find(runTests.begin(), runTests.end(), std::get<2>(test)) == runTests.end())
            {
                continue;
            }

            if (excludeTests.size() > 0 && std::find(excludeTests.begin(), excludeTests.end(), std::get<2>(test)) != excludeTests.end())
            {
                ++skipCount;
                continue;
            }

            std::cout << std::get<2>(test) << ": ";
            std::cout.flush();

            std::function<void ()> function = std::get<0>(test);
            uint64_t timeout = std::get<1>(test) - 1000;

            TestRunner* runner = new TestRunner(category, function, &variable);
            xml << "\t<testcase classname=\"" << category->getCategoryName() << " - " << OS << "\" name=\"" << std::get<2>(test) << "\"";

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

                failedTests.push_back({category->getCategoryName() + "::" + std::get<2>(test), "Test timed out."});

                xml << " >" << std::endl
                    << "\t\t" << "<failure type=\"Timeout\">" << runner->output.str() << "</failure>" << std::endl
                    << "\t</testcase>" << std::endl;
            }
            else if (!runner->failed)
            {
                std::cout << "✅" << std::endl;
                std::cout.flush();
                xml << " />" << std::endl;
            }
            else if (runner->failed && (std::find(expectedFailureCategories.begin(), expectedFailureCategories.end(), category->getCategoryName()) != expectedFailureCategories.end() || std::find(expectedFailureTests.begin(), expectedFailureTests.end(), std::get<2>(test)) != expectedFailureTests.end()))
            {
                std::cout << "Failed. Ignoring." << std::endl;
                std::cout.flush();
                ++expectedFailureCount;

                // status="ignored" with the failure message would be more correct,
                //but our reporting tool can't currently handle this.
                xml << " status=\"disabled\" />" << std::endl;
                // << "\t\t" << "<failure type=\"Test Failure\">" << runner->failureMessage << "</failure>" << std::endl
                // << "\t</testcase>" << std::endl;
            }
            else if (runner->failed)
            {
                std::cout << "😈  Ha! " << runner->failureMessage << std::endl;
                std::cout.flush();

                failedTests.push_back({category->getCategoryName() + "::" + std::get<2>(test), runner->failureMessage + "\n\n" + runner->output.str()});
                unexpectedFailure = true;

                xml << " >" << std::endl
                    << "\t\t" << "<failure type=\"Test Failure\">" << runner->failureMessage << "</failure>" << std::endl
                    << "\t</testcase>" << std::endl;
            }

            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            variable.wait(lock, [runner]{ return runner->done == true; });

            if (runner->kill)
            {
                std::cout << "Killed" << std::endl;
                std::cout.flush();
            }

            if (ShowTestOutput && runner->failed)
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

    xml << "</testsuite>" << std::endl;
    xml.close();

    std::cout << "================================================" << std::endl << std::endl;
    std::cout << "Completed " << successCount << "/" << testCount << " successfully. " << testCount - successCount - expectedFailureCount << " failed unexpectedly. " << expectedFailureCount << " failed as expected. " << skipCount << " tests skipped." << std::endl;
    std::cout.flush();

    if (failedTests.size() > 0)
    {
        std::cout << "Failed Tests: " << std::endl << std::endl;

        for (auto test : failedTests)
        {
            std::cout << test.name << std::endl;
        }
    }

#ifdef _MSC_VER
    if (IsDebuggerPresent())
    {
        std::cout << "Press enter to continue..." << std::endl;
        std::cin.get();
    }
#endif

    if (failedTests.size() > 0 && !silent)
        return 1;
    else
        return 0;
}
