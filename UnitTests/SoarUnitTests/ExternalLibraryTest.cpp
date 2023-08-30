
#include "ExternalLibraryTest.hpp"

#include "SoarHelper.hpp"

bool success = false;

void PrintCallbackHandler(sml::smlPrintEventId, void*, sml::Agent*, char const* pMessage)
{
    if (std::string(pMessage) == "myLibTest")
    {
        success = true;
    }
}

void ExternalLibraryTest::testLoadLibrary()
{
    // External library registers a single RHS function that returns "myRHSTest"; exec the
    // RHS and check the returned string. If it's correct, then the lib was indeed loaded.
    const std::string loadResult = kernel->LoadExternalLibrary("TestExternalLibraryLib");
    assertTrue_msg(loadResult, loadResult.empty());

    agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);
    std::string spMessage = agent->ExecuteCommandLine("sp {test (state <s> ^superstate nil) --> (write (exec test))}");
    assertTrue_msg(spMessage, agent->GetLastCommandLineResult());

    kernel->RunAllAgents(1);
    assertTrue_msg("Library RHS function did not fire", success);

	// SoarHelper::init_check_to_find_refcount_leaks(agent);
}
