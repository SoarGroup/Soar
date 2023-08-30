//
//  FullTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 6/26/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "FullTests.hpp"

#include "FullTestsClientThread.hpp"
#include "FullTestsClientThreadFullyOptimized.hpp"
#include "FullTestsRemote.hpp"

#include "SoarHelper.hpp"

#include "sml_AgentSML.h"
#include "sml_ClientKernel.h"
#include "soar_instance.h"

#include <functional>

bool g_Cancel = false;

#ifdef _WIN32
BOOL WINAPI handle_ctrlc(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        g_Cancel = true;
        return TRUE;
    }

    return FALSE;
}
#else // _WIN32
#include <spawn.h>
#endif // not _WIN32

const std::string FullTests_Parent::kAgentName("full-tests-agent");

void FullTests_Parent::setUp()
{
    no_agent_assertTrue(MAJOR_VERSION_NUMBER == SML_MAJOR_VERSION_NUMBER);
    no_agent_assertTrue(MINOR_VERSION_NUMBER == SML_MINOR_VERSION_NUMBER);
    no_agent_assertTrue(MICRO_VERSION_NUMBER == SML_MICRO_VERSION_NUMBER);
    no_agent_assertTrue(GREEK_VERSION_NUMBER == SML_GREEK_VERSION_NUMBER);
    no_agent_assertTrue(strcmp(VERSION_STRING(), SML_VERSION_STRING()) == 0);

    m_pKernel = 0;
    agent = 0;

    createSoar();
}

void FullTests_Parent::tearDown(bool caught)
{
    destroySoar();
}

void FullTestsClientThreadFullyOptimized::setUp()
{
    FullTests_Parent::runner = TestCategory::runner;

    m_Options.reset();
    m_Options.useClientThread = true;
    m_Options.fullyOptimized = true;

    FullTests_Parent::setUp();
}

void FullTestsClientThread::setUp()
{
    FullTests_Parent::runner = TestCategory::runner;

    m_Options.reset();
    m_Options.useClientThread = true;

    FullTests_Parent::setUp();
}

void FullTests::setUp()
{
    FullTests_Parent::runner = TestCategory::runner;

    m_Options.reset();

    FullTests_Parent::setUp();
}

void FullTestsRemote::setUp()
{
    FullTests_Parent::runner = TestCategory::runner;

    m_Options.reset();
    m_Options.remote = true;

    FullTests_Parent::setUp();
}

void FullTests_Parent::createSoar()
{
    no_agent_assertTrue(m_pKernel == NULL);

    if (m_Options.remote)
    {
        int targetPid = spawnListener();
        m_pKernel = sml::Kernel::CreateRemoteConnection(true, 0, targetPid);
    }
    else
    {
        if (m_Options.useClientThread)
        {
            bool optimized = m_Options.fullyOptimized;
            m_pKernel = sml::Kernel::CreateKernelInCurrentThread(optimized, sml::Kernel::kUseAnyPort);
        }
        else
        {
            m_pKernel = sml::Kernel::CreateKernelInNewThread(sml::Kernel::kUseAnyPort);
        }
        if (SoarHelper::run_as_unit_test)
        {
            configure_for_unit_tests();
        }
    }

    no_agent_assertTrue(m_pKernel != NULL);
    no_agent_assertTrue_msg(m_pKernel->GetLastErrorDescription(), !m_pKernel->HadError());

    if (m_Options.verbose)
    {
        std::cout << "Soar kernel version " << m_pKernel->GetSoarKernelVersion() << std::endl ;
    }
    if (m_Options.verbose)
    {
        std::cout << "SML version " << sml::sml_Names::kSMLVersionValue << std::endl ;
    }

    no_agent_assertTrue(std::string(m_pKernel->GetSoarKernelVersion()) == std::string(sml::sml_Names::kSoarVersionValue));

    bool creationHandlerReceived(false);
    int agentCreationCallback = m_pKernel->RegisterForAgentEvent(sml::smlEVENT_AFTER_AGENT_CREATED, Handlers::MyCreationHandler, &creationHandlerReceived) ;

    // Report the number of agents (always 0 unless this is a remote connection to a CLI or some such)
    no_agent_assertTrue(m_pKernel->GetNumberAgents() == 0);

    // NOTE: We don't delete the agent pointer.  It's owned by the kernel
    agent = m_pKernel->CreateAgent(kAgentName.c_str()) ;
    no_agent_assertTrue_msg(m_pKernel->GetLastErrorDescription(), !m_pKernel->HadError());
    no_agent_assertTrue(agent != NULL);
    no_agent_assertTrue(creationHandlerReceived);
    if (SoarHelper::run_as_unit_test)
    {
        agent_struct* lAgent = Soar_Instance::Get_Soar_Instance().Get_Agent_Info(kAgentName.c_str())->GetSoarAgent();
        configure_agent_for_unit_tests(lAgent);
    }

    no_agent_assertTrue(m_pKernel->UnregisterForAgentEvent(agentCreationCallback));

    // a number of tests below depend on running full decision cycles.
    agent->ExecuteCommandLine("soar stop-phase input") ;
    no_agent_assertTrue_msg("soar stop-phase input", agent->GetLastCommandLineResult());

    no_agent_assertTrue(m_pKernel->GetNumberAgents() == 1);

    m_pKernel->SetAutoCommit(!m_Options.autoCommitDisabled) ;
}

void FullTests_Parent::destroySoar()
{
    if (!m_pKernel)
        return;

    // Agent deletion
    if (m_Options.verbose)
    {
        std::cout << "Destroy the agent now" << std::endl ;
    }

    // The Before_Agent_Destroyed callback is a tricky one so we'll register for it to test it.
    // We need to get this callback just before the agentSML data is deleted (otherwise there'll be no way to send/receive the callback)
    // and then continue on to delete the agent after we've responded to the callback.
    // Interestingly, we don't explicitly unregister this callback because the agent has already been destroyed so
    // that's another test, that this callback is cleaned up correctly (and automatically).
    bool deletionHandlerReceived(false);
    m_pKernel->RegisterForAgentEvent(sml::smlEVENT_BEFORE_AGENT_DESTROYED, Handlers::MyDeletionHandler, &deletionHandlerReceived) ;

    // Explicitly destroy our agent as a test, before we delete the kernel itself.
    // (Actually, if this is a remote connection we need to do this or the agent
    //  will remain alive).
    no_agent_assertTrue(m_pKernel->DestroyAgent(agent));
    no_agent_assertTrue(deletionHandlerReceived);
    deletionHandlerReceived = false;

    if (m_Options.verbose)
    {
        std::cout << "Calling shutdown on the kernel now" << std::endl ;
    }

    if (m_Options.remote)
    {
        soar_thread::Event shutdownEvent;
        m_pKernel->RegisterForSystemEvent(sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyEventShutdownHandler, &shutdownEvent) ;

        // BUGBUG
        // ClientSML thread dies inelegantly here spewing forth error messages
        // about sockets/pipes not being shut down correctly.
        std::string shutdownResponse = m_pKernel->SendClientMessage(0, "test-listener", "shutdown") ;
        no_agent_assertTrue(shutdownResponse == "ok");

        no_agent_assertTrue_msg("Listener side kernel shutdown failed to fire smlEVENT_BEFORE_SHUTDOWN", shutdownEvent.WaitForEvent(5, 0));

        // Note, in the remote case, this does not fire smlEVENT_BEFORE_SHUTDOWN
        // the listener side shutdown does trigger the event when it is deleted, see simplelistener.cpp
        m_pKernel->Shutdown() ;

    }
    else
    {
        bool shutdownHandlerReceived(false);
        m_pKernel->RegisterForSystemEvent(sml::smlEVENT_BEFORE_SHUTDOWN, Handlers::MyBoolShutdownHandler, &shutdownHandlerReceived) ;
        m_pKernel->Shutdown() ;
        no_agent_assertTrue(shutdownHandlerReceived);
    }

    if (m_Options.verbose)
    {
        std::cout << "Shutdown completed now" << std::endl ;
    }

    // Delete the kernel.  If this is an embedded connection this destroys the kernel.
    // If it's a remote connection we just disconnect.
    delete m_pKernel ;
    m_pKernel = NULL;

    if (m_Options.remote)
    {
        cleanUpListener();
        if (m_Options.verbose)
        {
            std::cout << "Cleaned up listener." << std::endl;
        }
    }
}

int FullTests_Parent::spawnListener()
{
    // Spawning a new process is radically different on windows vs linux.
    // Instead of writing an abstraction layer, I'm just going to put platform-
    // specific code here.

    int targetPid = -1;

#ifdef _WIN32
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char executable[1024];
    GetModuleFileName(NULL, executable, 1024);

    // Start the child process.
    BOOL success = CreateProcess(executable,
        "Prototype-UnitTesting.exe --listener", // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi);           // Pointer to PROCESS_INFORMATION structure

    std::stringstream errorMessage;
    errorMessage << "CreateProcess error code: " << GetLastError();
    no_agent_assertTrue_msg(errorMessage.str().c_str(), success);

    targetPid = pi.dwProcessId;

#else // _WIN32
    std::string executable = SoarHelper::GetResource("Prototype-UnitTesting");

    char arg1[22] = {"Prototype-UnitTesting"};
    char arg2[11] = {"--listener"};
    char* argv[] = {arg1, arg2, NULL};
    int error = posix_spawn(&targetPid, executable.c_str(), NULL, NULL, argv, NULL);
    no_agent_assertTrue_msg("posix_spawn error", error == 0);

#endif // _WIN32

    sml::Sleep(1, 0);
    return targetPid;
}

void FullTests_Parent::cleanUpListener()
{
#ifdef _WIN32
    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else // _WIN32
    int status(0);
    wait(&status);
    if (WIFEXITED(status))
    {
        no_agent_assertTrue_msg("listener terminated with nonzero status", WEXITSTATUS(status) == 0);
    }
    else
    {
        no_agent_assertTrue_msg("listener killed by signal", WIFSIGNALED(status));

        // not sure why signal 0 comes up but seems to fix things on Mac OS
        if (!WIFSTOPPED(status) && (WSTOPSIG(status) != 0))
        {
            no_agent_assertTrue_msg("listener stopped by signal", WIFSTOPPED(status));
        }
        else if (WIFSTOPPED(status))
        {
#ifndef WIFCONTINUED
#define __W_CONTINUED 0xffff
#define __WIFCONTINUED(status) ((status) == __W_CONTINUED)
#define __WAIT_INT(status) (*(__const int *) &(status))
#define WIFCONTINUED(status) __WIFCONTINUED(__WAIT_INT(status))
#endif
            no_agent_assertTrue_msg("listener continued", WIFCONTINUED(status));
            no_agent_assertTrue_msg("listener died: unknown", false);
        }
    }
#endif // _WIN32
}

void FullTests_Parent::loadProductions(std::string productions)
{
    agent->LoadProductions(productions.c_str(), true) ;
    no_agent_assertTrue_msg("loadProductions", agent->GetLastCommandLineResult());
    SoarHelper::check_learning_override(agent);
}

void FullTests_Parent::testInit()
{
    agent->InitSoar();
    no_agent_assertTrue_msg("init-soar", agent->GetLastCommandLineResult());
}

void FullTests_Parent::testProductions()
{
    // Load and test productions
    loadProductions(SoarHelper::GetResource("testsml.soar"));

    no_agent_assertTrue(agent->IsProductionLoaded("apply*move"));
    no_agent_assertTrue(!agent->IsProductionLoaded("made*up*name"));

    int excisedCount(0);
    int prodCall = agent->RegisterForProductionEvent(sml::smlEVENT_BEFORE_PRODUCTION_REMOVED, Handlers::MyProductionHandler, &excisedCount) ;

    agent->ExecuteCommandLine("excise --all") ;
    no_agent_assertTrue_msg("excise --all", agent->GetLastCommandLineResult());
    no_agent_assertTrue(excisedCount > 0);

    excisedCount = 0;
    loadProductions(SoarHelper::GetResource("testsml.soar"));
    no_agent_assertTrue(excisedCount == 0);

    no_agent_assertTrue(agent->UnregisterForProductionEvent(prodCall));
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testRHSHandler()
{
    loadProductions(SoarHelper::GetResource("testsml.soar"));

    bool rhsFunctionHandlerReceived(false);

    // Record a RHS function
    int callback_rhs1 = m_pKernel->AddRhsFunction("test-rhs", Handlers::MyRhsFunctionHandler, &rhsFunctionHandlerReceived) ;
    int callback_rhs_dup = m_pKernel->AddRhsFunction("test-rhs", Handlers::MyRhsFunctionHandler, &rhsFunctionHandlerReceived) ;
    //agent->RegisterForPrintEvent( sml::smlEVENT_PRINT, Handlers::DebugPrintEventHandler, 0) ;
    no_agent_assertTrue_msg("Duplicate RHS function registration should be detected and be ignored", callback_rhs_dup == callback_rhs1);

    bool cppRhsHandlerReceived(false);
    int callback_rhs_cpp = m_pKernel->AddRhsFunction("test-rhs-cpp", Handlers::GetRhsFunctionHandlerCPP(&cppRhsHandlerReceived)) ;

    // need this to fire production that calls test-rhs
    sml::Identifier* pSquare = agent->GetInputLink()->CreateIdWME("square") ;
    no_agent_assertTrue(pSquare);
    sml::StringElement* pEmpty = pSquare->CreateStringWME("content", "EMPTY") ;
    no_agent_assertTrue(pEmpty);
    sml::IntElement* pRow = pSquare->CreateIntWME("row", 1) ;
    no_agent_assertTrue(pRow);
    sml::IntElement* pCol = pSquare->CreateIntWME("col", 2) ;
    no_agent_assertTrue(pCol);
    no_agent_assertTrue(agent->Commit());

    m_pKernel->RunAllAgents(1) ;
    no_agent_assertTrue_msg("RunAllAgents", agent->GetLastCommandLineResult());

    //std::cout << agent->ExecuteCommandLine("p i2 --depth 4") << std::endl;

    no_agent_assertTrue(rhsFunctionHandlerReceived);
    no_agent_assertTrue(cppRhsHandlerReceived);

    no_agent_assertTrue(m_pKernel->RemoveRhsFunction(callback_rhs1));

    // Re-add it without the bool that is getting popped off the stack
    no_agent_assertTrue(m_pKernel->AddRhsFunction("test-rhs", Handlers::MyRhsFunctionHandler, 0));

    no_agent_assertTrue(pSquare->DestroyWME());
    no_agent_assertTrue(agent->Commit());
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testClientMessageHandler()
{
    // Record a client message handler
    bool clientHandlerReceived(false);
    int clientCallback = m_pKernel->RegisterForClientMessageEvent("test-client", Handlers::MyClientMessageHandler, &clientHandlerReceived) ;

    // This is a bit dopey--but we'll send a message to ourselves for this test
    // use a large message to exercise buffer re-allocation logic
    std::string message(SML_INITIAL_BUFFER_SIZE + 100, 'x');
    std::string response = m_pKernel->SendClientMessage(agent, "test-client", message.c_str());
    no_agent_assertTrue(clientHandlerReceived);
    clientHandlerReceived = false;
    std::string expected = "handler-message" + message;
    no_agent_assertTrue(response == expected);

    no_agent_assertTrue(m_pKernel->UnregisterForClientMessageEvent(clientCallback));
}

void FullTests_Parent::testFilterHandler()
{
    // Record a filter
    bool filterHandlerReceived(false);
    int clientFilter = m_pKernel->RegisterForClientMessageEvent(sml::sml_Names::kFilterName, Handlers::MyFilterHandler, &filterHandlerReceived) ;

    // Our filter adds "--depth 2" to all commands
    // so this should give us the result of "print s1 --depth 2"
    std::string output = agent->ExecuteCommandLine("print <s>") ;
    no_agent_assertTrue_msg("print <s>", agent->GetLastCommandLineResult());

    no_agent_assertTrue(filterHandlerReceived);
    filterHandlerReceived = false;

    // depth 2 should reveal I2
    no_agent_assertTrue(output.find("input-link") != std::string::npos);

    // This is important -- if we don't unregister all subsequent commands will
    // come to our filter and promptly fail!
    no_agent_assertTrue(m_pKernel->UnregisterForClientMessageEvent(clientFilter));
}

void FullTests_Parent::testWMEs()
{
    sml::Identifier* pInputLink = agent->GetInputLink() ;
    no_agent_assertTrue(pInputLink);

    // Some simple tests
    sml::StringElement* pWME = pInputLink->CreateStringWME("my-att", "my-value") ;
    no_agent_assertTrue(pWME);

    // This is to test a bug where an identifier isn't fully removed from working memory (you can still print it) after it is destroyed.
    sml::Identifier* pIDRemoveTest = pInputLink->CreateIdWME("foo") ;
    no_agent_assertTrue(pIDRemoveTest);
    no_agent_assertTrue(pIDRemoveTest->CreateFloatWME("bar", 1.23));

    no_agent_assertTrue(pIDRemoveTest->GetValueAsString());

    sml::Identifier* pID = pInputLink->CreateIdWME("plane") ;
    no_agent_assertTrue(pID);

    // Trigger for inputWme update change problem
    sml::StringElement* pWMEtest = pID->CreateStringWME("typeTest", "Boeing747") ;
    no_agent_assertTrue(pWMEtest);

    no_agent_assertTrue(agent->Commit());

    agent->RunSelf(1) ;
    no_agent_assertTrue_msg("RunSelf", agent->GetLastCommandLineResult());

    no_agent_assertTrue(pIDRemoveTest->DestroyWME());
    no_agent_assertTrue(agent->Commit());

    //agent->RunSelf(1) ;
    no_agent_assertTrue(agent->ExecuteCommandLine("print i2 --depth 3"));
    no_agent_assertTrue_msg("print i2 --depth 3", agent->GetLastCommandLineResult());

    no_agent_assertTrue(agent->ExecuteCommandLine("print F1"));    // BUGBUG: This wme remains in memory even after we add the "RunSelf" at which point it should be gone.
    no_agent_assertTrue_msg("print F1", agent->GetLastCommandLineResult());

    agent->InitSoar();
    no_agent_assertTrue_msg("init-soar", agent->GetLastCommandLineResult());

    no_agent_assertTrue(pID->CreateStringWME("type", "Boeing747"));

    sml::IntElement* pWME2    = pID->CreateIntWME("speed", 200) ;
    no_agent_assertTrue(pWME2);

    sml::FloatElement* pWME3  = pID->CreateFloatWME("direction", 50.5) ;
    no_agent_assertTrue(pWME3);

    no_agent_assertTrue(agent->Commit());

    agent->InitSoar();
    no_agent_assertTrue_msg("init-soar", agent->GetLastCommandLineResult());

    // Test the blink option
    agent->SetBlinkIfNoChange(false) ;

    int64_t timeTag1 = pWME3->GetTimeTag() ;
    agent->Update(pWME3, 50.5) ;      // Should not change the wme, so timetag should be the same

    int64_t timeTag2 = pWME3->GetTimeTag() ;
    agent->SetBlinkIfNoChange(true) ;     // Back to the default
    agent->Update(pWME3, 50.5) ;      // Should change the wme, so timetag should be different

    int64_t timeTag3 = pWME3->GetTimeTag() ;

    no_agent_assertTrue_msg("Error in handling of SetBlinkIfNoChange flag", timeTag1 == timeTag2);
    no_agent_assertTrue_msg("Error in handling of SetBlinkIfNoChange flag", timeTag2 != timeTag3);

    // Remove a wme
    no_agent_assertTrue(pWME3->DestroyWME());

    // Change the speed to 300
    agent->Update(pWME2, 300) ;

    // Create a new WME that shares the same id as plane
    // BUGBUG: This is triggering an no_agent_assert and memory leak now after the changes
    // to InputWME not calling Update() immediately.  For now I've removed the test until
    // we have time to figure out what's going wrong.
    //Identifier* pID2 = agent->CreateSharedIdWME(pInputLink, "all-planes", pID) ;
    //unused(pID2);

    no_agent_assertTrue(agent->Commit());

    /*
	 printWMEs(agent->GetInputLink()) ;
	 std::string printInput1 = agent->ExecuteCommandLine("print --depth 2 I2") ;
	 std::cout << printInput1 << std::endl ;
	 std::cout << std::endl << "Now work with the input link" << std::endl ;
     */

    // Delete one of the shared WMEs to make sure that's ok
    //agent->DestroyWME(pID) ;
    //agent->Commit() ;

    // Throw in a pattern as a test
    std::string pattern = agent->ExecuteCommandLine("print -i (s1 ^* *)") ;
    no_agent_assertTrue_msg("print -i (s1 ^* *)", agent->GetLastCommandLineResult());

    SoarHelper::init_check_to_find_refcount_leaks(agent);

}

void FullTests_Parent::testXML()
{
    // Test calling CommandLineXML.
    sml::ClientAnalyzedXML xml2 ;
    no_agent_assertTrue(m_pKernel->ExecuteCommandLineXML("print -i --depth 3 s1", agent->GetAgentName(), &xml2));

    char* xmlString = xml2.GenerateXMLString(true);
    no_agent_assertTrue(xmlString);

    soarxml::ElementXML const* pResult = xml2.GetResultTag() ;
    no_agent_assertTrue(pResult);

    // The XML format of "print" is a <trace> tag containing a series of
    // a) <wme> tags (if this is an --internal print) or
    // b) <id> tags that contain <wme> tags if this is not an --internal print.
    soarxml::ElementXML traceChild ;
    no_agent_assertTrue(pResult->GetChild(&traceChild, 0));

    int nChildren = traceChild.GetNumberChildren() ;
    soarxml::ElementXML wmeChild ;
    for (int i = 0 ; i < nChildren ; i++)
    {
        traceChild.GetChild(&wmeChild, i) ;
        char* wmeString = wmeChild.GenerateXMLString(true) ;
        no_agent_assertTrue(wmeString);
        if (m_Options.verbose)
        {
            std::cout << wmeString << std::endl ;
        }
        wmeChild.DeleteString(wmeString) ;
    }
    xml2.DeleteString(xmlString) ;

}

void FullTests_Parent::testAgent()
{
    //m_pKernel->SetTraceCommunications( true );
    agent->SetOutputLinkChangeTracking(true);
    loadProductions(SoarHelper::GetResource("testsml.soar"));

    // Test that we get a callback after the decision cycle runs
    // We'll pass in an "int" and use it to count decisions (just as an example of passing user data around)
    int count(0);
    int callback1 = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &count);
    int callback_dup = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &count);

    no_agent_assertTrue_msg("Duplicate handler registration should be detected and be ignored", callback1 == callback_dup);

    // This callback unregisters itself in the callback -- as a test to see if we can do that safely.
    int selfRemovingCallback(-1);
    selfRemovingCallback = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunSelfRemovingHandler, &selfRemovingCallback) ;

    // Register for a String event
    //	bool stringEventHandlerReceived(false);
    //	int stringCall = m_pKernel->RegisterForStringEvent(sml::smlEVENT_LOAD_LIBRARY, Handlers::MyStringEventHandler, &stringEventHandlerReceived) ;
    //	no_agent_assertTrue(m_pKernel->ExecuteCommandLine("load-library TestExternalLibraryLib", NULL));
    //	no_agent_assertTrue_msg("echo hello world", agent->GetLastCommandLineResult());
    //	no_agent_assertTrue(stringEventHandlerReceived);
    //	stringEventHandlerReceived = false;
    //	no_agent_assertTrue(m_pKernel->UnregisterForStringEvent(stringCall));

    // Register another handler for the same event, to make sure we can do that.
    // Register this one ahead of the previous handler (so it will fire before MyRunEventHandler)
    bool addToBack = true ;
    int testData(25) ;
    int callback2 = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyDuplicateRunEventHandler, &testData, !addToBack) ;

    // Run returns the result (succeeded, failed etc.)
    // To catch the trace output we have to register a print event listener
    std::stringstream trace ;   // We'll pass this into the handler and build up the output in it
    std::string structured ;    // Structured trace goes here
    int callbackp = agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

    sml::ClientXML* clientXMLStorage = 0;
    int callbackx = agent->RegisterForXMLEvent(sml::smlEVENT_XML_TRACE_OUTPUT, Handlers::MyXMLEventHandler, &clientXMLStorage) ;

    int beforeCount(0);
    int afterCount(0);
    int callback_before = agent->RegisterForRunEvent(sml::smlEVENT_BEFORE_RUN_STARTS, Handlers::MyRunEventHandler, &beforeCount) ;
    int callback_after = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_RUN_ENDS, Handlers::MyRunEventHandler, &afterCount) ;

    //Some temp code to generate more complex watch traces.  Not usually part of the test
    /*
	 Identifier* pSquare1 = agent->CreateIdWME(pInputLink, "square") ;
	 StringElement* pEmpty1 = pSquare1->CreateStringWME("content", "RANDOM") ;
	 IntElement* pRow1 = agent->CreateIntWME(pSquare1, "row", 1) ;
	 IntElement* pCol1 = agent->CreateIntWME(pSquare1, "col", 2) ;
	 agent->Update(pEmpty1, "EMPTY") ;
	 ok = agent->Commit() ;
	 agent->ExecuteCommandLine("watch 3") ;
     */

    // Test that we get a callback after the all output phases complete
    // We'll pass in an "int" and use it to count output phases
    int outputPhases(0);
    int callback_u = m_pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, Handlers::MyUpdateEventHandler, &outputPhases) ;

    int phaseCount(0);
    int callbackPhase = agent->RegisterForRunEvent(sml::smlEVENT_BEFORE_PHASE_EXECUTED, Handlers::MyRunEventHandler, &phaseCount) ;

    // Nothing should match here
    agent->RunSelf(4) ;
    no_agent_assertTrue_msg("RunSelf", agent->GetLastCommandLineResult());

    // Should be one output phase per decision
    no_agent_assertTrue(outputPhases == 4);

    no_agent_assertTrue(agent->WasAgentOnRunList());

    no_agent_assertTrue(agent->GetResultOfLastRun() == sml::sml_RUN_COMPLETED);

    // Should be 5 phases per decision
    /* Not true now we support stopping before/after phases when running by decision.
	 if (phaseCount != 20)
	 {
	 std::cout << "Error receiving phase events" << std::endl ;
	 return false ;
	 }
     */

    no_agent_assertTrue(beforeCount == 1);
    no_agent_assertTrue(afterCount == 1);

    no_agent_assertTrue(agent->UnregisterForRunEvent(callbackPhase));

    // By this point the static variable ClientXMLStorage should have been filled in
    // and it should be valid, even though the event handler for MyXMLEventHandler has completed.
    no_agent_assertTrue_msg("Error receiving XML trace events", clientXMLStorage != NULL);

    // If we crash on this access there's a problem with the ref-counting of
    // the XML message we're passed in MyXMLEventHandler.
    no_agent_assertTrue(clientXMLStorage->ConvertToTraceXML()->IsTagTrace());

    delete clientXMLStorage ;
    clientXMLStorage = NULL ;

    no_agent_assertTrue(agent->UnregisterForXMLEvent(callbackx));
    no_agent_assertTrue(agent->UnregisterForPrintEvent(callbackp));
    no_agent_assertTrue(agent->UnregisterForRunEvent(callback_before));
    no_agent_assertTrue(agent->UnregisterForRunEvent(callback_after));
    no_agent_assertTrue(m_pKernel->UnregisterForUpdateEvent(callback_u));

    // Print out the standard trace and the same thing as a structured XML trace
    if (m_Options.verbose)
    {
        std::cout << trace.str() << std::endl ;
    }
    trace.clear();
    if (m_Options.verbose)
    {
        std::cout << structured << std::endl ;
    }

    /*
	 printWMEs(agent->GetInputLink()) ;
	 std::string printInput = agent->ExecuteCommandLine("print --depth 2 I2") ;
	 std::cout << printInput << std::endl ;
     */

    // Then add some tic tac toe stuff which should trigger output
    sml::Identifier* pSquare = agent->GetInputLink()->CreateIdWME("square") ;
    no_agent_assertTrue(pSquare);
    sml::StringElement* pEmpty = pSquare->CreateStringWME("content", "RANDOM") ;
    no_agent_assertTrue(pEmpty);
    sml::IntElement* pRow = pSquare->CreateIntWME("row", 1) ;
    no_agent_assertTrue(pRow);
    sml::IntElement* pCol = pSquare->CreateIntWME("col", 2) ;
    no_agent_assertTrue(pCol);

    no_agent_assertTrue(agent->Commit());

    // Update the square's value to be empty.  This ensures that the update
    // call is doing something.  Otherwise, when we run we won't get a match.
    agent->Update(pEmpty, "EMPTY") ;
    no_agent_assertTrue(agent->Commit());

    int myCount(0);
    int callback_run_count = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyRunEventHandler, &myCount) ;

    int outputsGenerated(0) ;
    int callback_g = m_pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_GENERATED_OUTPUT, Handlers::MyUpdateEventHandler, &outputsGenerated) ;

    int outputNotifications(0) ;
    int callback_notify = agent->RegisterForOutputNotification(Handlers::MyOutputNotificationHandler, &outputNotifications) ;

    // Can't test this at the same time as testing the getCommand() methods as registering for this clears the output link information
    //int outputHandler = agent->AddOutputHandler("move", MyOutputEventHandler, NULL) ;

    if (m_Options.verbose)
    {
        std::cout << "About to do first run-til-output" << std::endl ;
    }

    int callbackp1 = agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

    // Now we should match (if we really loaded the tictactoe example rules) and so generate some real output
    // We'll use RunAll just to test it out.  Could use RunSelf and get same result (presumably)
    m_pKernel->RunAllTilOutput() ;  // Should just cause Soar to run a decision or two (this is a test that run til output works stops at output)
    no_agent_assertTrue_msg("RunAllTilOutput", agent->GetLastCommandLineResult());

    // We should stop quickly (after a decision or two)
    no_agent_assertTrue_msg("Error in RunTilOutput -- it didn't stop on the output", myCount <= 10);
    no_agent_assertTrue_msg("Error in callback handler for MyRunEventHandler -- failed to update count", myCount > 0);

    if (m_Options.verbose)
    {
        std::cout << "Agent ran for " << myCount << " decisions before we got output" << std::endl ;
    }
    if (m_Options.verbose)
    {
        std::cout << trace.str() << std::endl ;
    }
    trace.clear();

    no_agent_assertTrue_msg("Error in AFTER_ALL_GENERATED event.", outputsGenerated == 1);

    no_agent_assertTrue_msg("Error in OUTPUT_NOTIFICATION event.", outputNotifications == 1);

    // Reset the agent and repeat the process to check whether init-soar works.
    agent->InitSoar();
    no_agent_assertTrue_msg("init-soar", agent->GetLastCommandLineResult());

    agent->RunSelfTilOutput() ;
    no_agent_assertTrue_msg("RunSelfTilOutput", agent->GetLastCommandLineResult());

    no_agent_assertTrue(agent->UnregisterForOutputNotification(callback_notify));
    no_agent_assertTrue(m_pKernel->UnregisterForUpdateEvent(callback_g));
    no_agent_assertTrue(agent->UnregisterForPrintEvent(callbackp1));

    //cout << "Time to dump output link" << std::endl ;

    no_agent_assertTrue(agent->GetOutputLink());
    //printWMEs(agent->GetOutputLink()) ;

    // Now update the output link with "status complete"
    sml::Identifier* pMove = static_cast< sml::Identifier* >(agent->GetOutputLink()->FindByAttribute("move", 0));
    no_agent_assertTrue(pMove);

    // Try to find an attribute that's missing to make sure we get null back
    sml::Identifier* pMissing = static_cast< sml::Identifier* >(agent->GetOutputLink()->FindByAttribute("not-there", 0));
    no_agent_assertTrue(!pMissing);

    sml::Identifier* pMissingInput = static_cast< sml::Identifier* >(agent->GetInputLink()->FindByAttribute("not-there", 0));
    no_agent_assertTrue(!pMissingInput);

    // We add an "alternative" to check that we handle shared WMEs correctly.
    // Look it up here.
    sml::Identifier* pAlt = static_cast< sml::Identifier* >(agent->GetOutputLink()->FindByAttribute("alternative", 0));
    no_agent_assertTrue(pAlt);

    // Should also be able to get the command through the "GetCommands" route which tests
    // whether we've flagged the right wmes as "just added" or not.
    int numberCommands = agent->GetNumberCommands() ;
    no_agent_assertTrue(numberCommands == 3);

    // Get the first two commands (move and alternative and A)
    sml::Identifier* pCommand1 = agent->GetCommand(0) ;
    sml::Identifier* pCommand2 = agent->GetCommand(1) ;
    sml::Identifier* pCommand3 = agent->GetCommand(2) ;
    no_agent_assertTrue(std::string(pCommand1->GetCommandName()) == "move"
        || std::string(pCommand2->GetCommandName()) == "move"
            || std::string(pCommand3->GetCommandName()) == "move");
    no_agent_assertTrue(std::string(pCommand1->GetCommandName()) == "alternative"
        || std::string(pCommand2->GetCommandName()) == "alternative"
            || std::string(pCommand3->GetCommandName()) == "alternative");
    no_agent_assertTrue(std::string(pCommand1->GetCommandName()) == "A"
        || std::string(pCommand2->GetCommandName()) == "A"
            || std::string(pCommand3->GetCommandName()) == "A");

    if (m_Options.verbose)
    {
        std::cout << "Marking command as completed." << std::endl ;
    }
    pMove->AddStatusComplete();
    no_agent_assertTrue(agent->Commit());

    // The move command should be deleted in response to the
    // the status complete getting added
    agent->RunSelf(2) ;
    no_agent_assertTrue_msg("RunSelf", agent->GetLastCommandLineResult());

    // Dump out the output link again.
    //if (agent->GetOutputLink())
    //{
    //  printWMEs(agent->GetOutputLink()) ;
    //}

    // Test that we can interrupt a run by registering a handler that
    // interrupts Soar immediately after a decision cycle.
    // Removed the test part for now. Stats doesn't report anything.
    bool interruptHandlerReceived(false);
    int callback3 = agent->RegisterForRunEvent(sml::smlEVENT_AFTER_DECISION_CYCLE, Handlers::MyInterruptHandler, &interruptHandlerReceived) ;

    agent->InitSoar();
    no_agent_assertTrue_msg("init-soar", agent->GetLastCommandLineResult());

    agent->RunSelf(20) ;
    no_agent_assertTrue_msg("RunSelf", agent->GetLastCommandLineResult());

    no_agent_assertTrue(interruptHandlerReceived);
    interruptHandlerReceived = false;

    //no_agent_assertTrue( agent->ExecuteCommandLine("stats") );
    //std::string stats( agent->ExecuteCommandLine("stats") );
    //no_agent_assertTrue_msg( "stats", agent->GetLastCommandLineResult() );
    //size_t pos = stats.find( "1 decision cycles" ) ;

    /*
	 if (pos == std::string.npos)
	 {
	 std::cout << "*** ERROR: Failed to interrupt Soar during a run." << std::endl ;
	 return false ;
	 }
     */
    no_agent_assertTrue(agent->UnregisterForRunEvent(callback3));

    /* These comments haven't kept up with the test -- does a lot more now
	 std::cout << std::endl << "If this test worked should see something like this (above here):" << std::endl ;
	 std::cout << "Top Identifier I3" << std::endl << "(I3 ^move M1)" << std::endl << "(M1 ^row 1)" << std::endl ;
	 std::cout << "(M1 ^col 1)" << std::endl << "(I3 ^alternative M1)" << std::endl ;
	 std::cout << "And then after the command is marked as completed (during the test):" << std::endl ;
	 std::cout << "Top Identifier I3" << std::endl ;
	 std::cout << "Together with about 6 received events" << std::endl ;
     */

    no_agent_assertTrue(agent->UnregisterForRunEvent(callback1));
    no_agent_assertTrue(agent->UnregisterForRunEvent(callback2));
    no_agent_assertTrue(agent->UnregisterForRunEvent(callback_run_count));

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testSimpleCopy()
{
    agent->SetOutputLinkChangeTracking(true);
    loadProductions(SoarHelper::GetResource("testcopy.soar"));

    /* Input structure for the test
	 (S1 ^io I1)
	 (I1 ^input-link I3)
	 (I3 ^sentence S2)
	 (S2 ^newest yes ^num-words 3 ^sentence-num 1 ^word W1 ^word W2 ^word W3)
	 (W1 ^num-word 1 ^word the)
	 (W2 ^num-word 2 ^word cat)
	 (W3 ^num-word 3 ^word in)
     */

    sml::Identifier* map = agent->GetInputLink() ;

    sml::Identifier* square2 = map->CreateIdWME("square");
    no_agent_assertTrue(std::string(square2->GetAttribute()) == "square");

    sml::Identifier* square5 = map->CreateIdWME("square");
    no_agent_assertTrue(std::string(square5->GetAttribute()) == "square");

    sml::Identifier* north = square2->CreateSharedIdWME("north", square5) ;

    sml::Identifier* south = square5->CreateSharedIdWME("south", square2) ;

    sml::Identifier* pSentence = agent->GetInputLink()->CreateIdWME("sentence") ;
    no_agent_assertTrue(std::string(pSentence->GetAttribute()) == "sentence");

    pSentence->CreateStringWME("newest", "ye s") ;

    pSentence->CreateIntWME("num-words", 3) ;

    sml::Identifier* pWord1 = pSentence->CreateIdWME("word") ;
    no_agent_assertTrue(std::string(pWord1->GetAttribute()) == "word");

    sml::Identifier* pWord5 = pSentence->CreateSharedIdWME("word2", pWord1) ;
    no_agent_assertTrue(std::string(pWord5->GetAttribute()) == "word2");

    sml::Identifier* pWord2 = pSentence->CreateIdWME("word") ;
    no_agent_assertTrue(std::string(pWord2->GetAttribute()) == "word");

    sml::Identifier* pWord3 = pSentence->CreateIdWME("word") ;
    no_agent_assertTrue(std::string(pWord3->GetAttribute()) == "word");

    pWord1->CreateIntWME("num-word", 1) ;
    pWord2->CreateIntWME("num-word", 2) ;
    pWord3->CreateIntWME("num-word", 3) ;
    pWord1->CreateStringWME("word", "the") ;
    pWord2->CreateStringWME("word", "cat") ;
    pWord3->CreateStringWME("word", "in") ;
    agent->Commit() ;

    // Register for the trace output
    std::stringstream trace ;   // We'll pass this into the handler and build up the output in it
    /*int callbackp = */agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, Handlers::MyPrintEventHandler, &trace) ;

    // Set to true for more detail on this
    m_pKernel->SetTraceCommunications(false) ;

    std::string result = agent->RunSelf(1) ;

    //cout << result << std::endl ;
    //cout << trace << std::endl ;

    // TODO: check this output
    //std::cout << agent->ExecuteCommandLine("print --depth 5 s1 --tree") << std::endl;

    // Test the iterator
    sml::Identifier* pOutputLink = agent->GetOutputLink();
    sml::Identifier::ChildrenIter iter = pOutputLink->GetChildrenBegin();
    sml::WMElement* pTextOutputWME = 0;
    while (iter != pOutputLink->GetChildrenEnd())
    {
        // There should be only one child
        no_agent_assertTrue(pTextOutputWME == 0);
        pTextOutputWME = *iter;
        no_agent_assertTrue(pTextOutputWME);
        no_agent_assertTrue(std::string(pTextOutputWME->GetAttribute()) == "text-output");
        ++iter;
    }

    no_agent_assertTrue(pTextOutputWME->IsIdentifier());
    sml::Identifier* pTextOutput = pTextOutputWME->ConvertToIdentifier();
    no_agent_assertTrue(pTextOutput);
    sml::Identifier::ChildrenIter textOutputIter = pTextOutput->GetChildrenBegin();
    int count = 0;
    while (textOutputIter != pTextOutput->GetChildrenEnd())
    {
        //sml::WMElement* wme2 = *iter2;
        //std::cout << wme2->GetAttribute();
        ++count;
        ++textOutputIter;
    }
    no_agent_assertTrue(count == 6);

    sml::WMElement* pNewestWME = pTextOutput->FindByAttribute("newest", 0);
    no_agent_assertTrue(pNewestWME);
    sml::StringElement* pNewest = pNewestWME->ConvertToStringElement();
    no_agent_assertTrue(pNewest);
    no_agent_assertTrue(std::string(pNewest->GetValue()) == "ye s");

    int changes = agent->GetNumberOutputLinkChanges() ;

    //std::cout << agent->ExecuteCommandLine("print i3 -d 100 -i --tree");

    /*
	 Output:

	 (28: I3 ^text-output S4)
	 (14: S4 ^newest yes)
	 (15: S4 ^num-words 3)
	 (16: S4 ^word W1)       <-------- Shared ID?
	 (8: W1 ^num-word 1)
	 (9: W1 ^word the)
	 (17: S4 ^word W1)       <-------- Shared ID?
	 (18: S4 ^word W2)
	 (10: W2 ^num-word 2)
	 (11: W2 ^word cat)
	 (19: S4 ^word W3)
	 (12: W3 ^num-word 3)
	 (13: W3 ^word in)
     */

    // TODO: verify output
    //for (int i = 0 ; i < changes ; i++)
    //{
    //  sml::WMElement* pOutputWme = agent->GetOutputLinkChange(i) ;
    //  std::cout << pOutputWme->GetIdentifier()->GetIdentifierSymbol() << " ^ " << pOutputWme->GetAttribute() << " " << pOutputWme->GetValueAsString() << std::endl ;
    //}

    // We had a bug where some of these wmes would get dropped (the orphaned wme scheme didn't handle multiple levels)
    // so check now that we got the correct number of changes.
    std::stringstream changesString;
    //changesString << "Number of changes: " << changes << ", this failure is currently expected but needs to be addressed, see wiki gSKI removal page";
    changesString << "Number of changes: " << changes;
    no_agent_assertTrue_msg(changesString.str().c_str(), changes == 13);

    SoarHelper::init_check_to_find_refcount_leaks(agent);

    // FIXME: leaks without this
    north->DestroyWME();
    south->DestroyWME();
}

void FullTests_Parent::testSimpleReteNetLoader()
{
    std::string result = agent->ExecuteCommandLine(("rete-net -l \"" + SoarHelper::GetResource("test.soarx") + "\"").c_str()) ;
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    // Get the latest id from the input link
    sml::Identifier* pID = agent->GetInputLink() ;
    //cout << "Input link id is " << pID->GetValueAsString() << std::endl ;

    no_agent_assertTrue(pID);
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::test64BitReteNet()
{
    std::string result = agent->ExecuteCommandLine(("rete-net -l \"" + SoarHelper::GetResource("test64.soarx") + "\"").c_str()) ;
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    // Get the latest id from the input link
    sml::Identifier* pID = agent->GetInputLink() ;
    //cout << "Input link id is " << pID->GetValueAsString() << std::endl ;

    no_agent_assertTrue(pID);
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testOSupportCopyDestroy()
{
    loadProductions(SoarHelper::GetResource("testOSupportCopyDestroy.soar"));

    sml::Identifier* pInputLink = agent->GetInputLink();
    no_agent_assertTrue(pInputLink);

    sml::Identifier* pFoo = pInputLink->CreateIdWME("foo");
    no_agent_assertTrue(pFoo);

    sml::Identifier* pBar = pFoo->CreateIdWME("bar");
    no_agent_assertTrue(pBar);

    sml::StringElement* pToy = pBar->CreateStringWME("toy", "jig");
    no_agent_assertTrue(pToy);

    bool badCopyExists(false);
    m_pKernel->AddRhsFunction("bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists) ;

    agent->RunSelf(1);

    pToy->DestroyWME();
    pBar->DestroyWME();
    pFoo->DestroyWME();

    agent->RunSelf(1);

    no_agent_assertTrue(!badCopyExists);
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testOSupportCopyDestroyCircularParent()
{
    loadProductions(SoarHelper::GetResource("testOSupportCopyDestroy.soar"));

    sml::Identifier* pInputLink = agent->GetInputLink();
    no_agent_assertTrue(pInputLink);

    sml::Identifier* pFoo = pInputLink->CreateIdWME("foo");
    no_agent_assertTrue(pFoo);

    sml::Identifier* pBar = pFoo->CreateIdWME("bar");
    no_agent_assertTrue(pBar);

    sml::Identifier* pToy = pBar->CreateSharedIdWME("toy", pFoo);
    no_agent_assertTrue(pToy);

    bool badCopyExists(false);
    m_pKernel->AddRhsFunction("bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists) ;

    agent->RunSelf(1);

    pFoo->DestroyWME();

    agent->RunSelf(1);

    no_agent_assertTrue(!badCopyExists);

    // FIXME: leaks without this.
    pBar->DestroyWME();
    SoarHelper::init_check_to_find_refcount_leaks(agent);

}

void FullTests_Parent::testOSupportCopyDestroyCircular()
{
    loadProductions(SoarHelper::GetResource("testOSupportCopyDestroy.soar"));

    sml::Identifier* pInputLink = agent->GetInputLink();
    no_agent_assertTrue(pInputLink);

    sml::Identifier* pFoo = pInputLink->CreateIdWME("foo");
    no_agent_assertTrue(pFoo);

    sml::Identifier* pBar = pFoo->CreateIdWME("bar");
    no_agent_assertTrue(pBar);

    sml::Identifier* pToy = pBar->CreateSharedIdWME("toy", pFoo);
    no_agent_assertTrue(pToy);

    bool badCopyExists(false);
    m_pKernel->AddRhsFunction("bad-copy-exists", Handlers::MyRhsFunctionHandler, &badCopyExists) ;

    agent->RunSelf(1);

    pToy->DestroyWME();
    pBar->DestroyWME();
    pFoo->DestroyWME();

    agent->RunSelf(1);

    no_agent_assertTrue(!badCopyExists);

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testSynchronize()
{
    //m_pKernel->SetTraceCommunications( true ) ;

    // input link
    sml::Identifier* pInputLink = agent->GetInputLink();
    no_agent_assertTrue(pInputLink);
    no_agent_assertTrue(pInputLink->GetNumberChildren() == 0);
    std::string name = pInputLink->GetIdentifierName();

    no_agent_assertTrue(agent->SynchronizeInputLink());

    pInputLink = agent->GetInputLink();
    no_agent_assertTrue(pInputLink);
    no_agent_assertTrue(pInputLink->GetNumberChildren() == 0);
    std::string newName = pInputLink->GetIdentifierName();

    no_agent_assertTrue(name == newName);

    // output link
    no_agent_assertTrue(!agent->GetOutputLink());

    no_agent_assertTrue(agent->SynchronizeOutputLink());

    sml::Identifier* pOutputLink = agent->GetOutputLink();
    no_agent_assertTrue(pOutputLink);
    no_agent_assertTrue(pOutputLink->GetNumberChildren() == 0);
    std::string olName = pOutputLink->GetIdentifierName();

    no_agent_assertTrue(agent->SynchronizeOutputLink());

    pOutputLink = agent->GetOutputLink();
    no_agent_assertTrue(pOutputLink);
    no_agent_assertTrue(pOutputLink->GetNumberChildren() == 0);
    std::string olNewName = pOutputLink->GetIdentifierName();

    no_agent_assertTrue(olName == olNewName);

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testRunningAgentCreation()
{
    // SEE BUG 952
    RunningAgentData data;
    data.count = 0;
    data.pOnTheFly = 0;

    m_pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, Handlers::MyAgentCreationUpdateEventHandler, &data);
    m_pKernel->RunAllAgents(10);

    //std::cout << std::endl;
    //std::cout << "count: " << data.count;
    // FIXME: in a perfect world, this is 10 not 12 but since the run isn't forever, the newly created agent runs 10.
    no_agent_assertTrue(data.count == 12);
    no_agent_assertTrue(data.pOnTheFly);

    sml::ClientAnalyzedXML response1;
    agent->ExecuteCommandLineXML("stats", &response1);
    sml::ClientAnalyzedXML response2;
    data.pOnTheFly->ExecuteCommandLineXML("stats", &response2);
    //std::cout << "original: " << response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1);
    //std::cout << " " << response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1);
    //std::cout << " " << response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountInnerElaboration, -1);
    //std::cout << " onthefly: " << response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1);
    //std::cout << " " << response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1);
    //std::cout << " " << response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountInnerElaboration, -1) << std::endl;
    no_agent_assertTrue(response1.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 10);
    no_agent_assertTrue(response2.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 10);

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testEventOrdering()
{
    int count = 0;

    agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, Handlers::MyOrderingPrintHandler, &count) ;
    agent->RegisterForRunEvent(sml::smlEVENT_AFTER_OUTPUT_PHASE, Handlers::MyOrderingRunHandler, &count) ;

    agent->RunSelf(2);
}

void FullTests_Parent::testStatusCompleteDuplication()
{
    // Load and test productions
    loadProductions(SoarHelper::GetResource("teststatuscomplete.soar"));

    // step
    agent->RunSelf(1);

    // add status complete
    int numberCommands = agent->GetNumberCommands() ;
    no_agent_assertTrue(numberCommands == 1);

    // Get the first two commands (move and alternate)
    sml::Identifier* pCommand = agent->GetCommand(0) ;
    pCommand->AddStatusComplete();

    // commit status complete
    no_agent_assertTrue(agent->Commit());

    // step
    agent->RunSelf(1);

    // count status complete instances
    sml::Identifier::ChildrenIter child = pCommand->GetChildrenBegin();
    sml::Identifier::ChildrenIter end = pCommand->GetChildrenEnd();

    int count = 0;
    while (child != end)
    {
        if ((*child)->GetAttribute() == std::string("status"))
        {
            ++count;
        }
        ++child;
    }

    // there should only be one
    no_agent_assertTrue(count == 1);

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testStopSoarVsInterrupt()
{
    loadProductions(SoarHelper::GetResource("teststopsoar.soar"));

    agent->ExecuteCommandLine("run -o 3");
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    // expected: agent stops after 15 decision cycles
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 15);
    }

    agent->InitSoar();
    agent->ExecuteCommandLine("run 3");
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    // expected: agent stops after 1 decision cycle
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
    }

    agent->ExecuteCommandLine("ex -a");
    agent->ExecuteCommandLine("soar init");
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    loadProductions(SoarHelper::GetResource("testinterrupt.soar"));

    agent->ExecuteCommandLine("run -o 3");
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    // expected: agent stops after 1 elaboration
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 0);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 1);
    }

    agent->InitSoar();
    agent->ExecuteCommandLine("run 3");
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    // expected: agent stops after 1 elaboration
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 0);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 1);
    }

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testSharedWmeSetViolation()
{
    //io.input-link.foo <f>
    sml::Identifier* pFoo1 = agent->GetInputLink()->CreateIdWME("foo") ;
    no_agent_assertTrue(pFoo1);

    // TODO: This is illegal, but is probably too expensive to test for in release.
    // See bug 1060
    sml::Identifier* pFoo2 = agent->GetInputLink()->CreateSharedIdWME("foo", pFoo1) ;
    no_agent_assertTrue_msg("CreateSharedIdWME was able to create duplicate wme", pFoo2 == 0);

    no_agent_assertTrue(agent->Commit());

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testEchoEquals()
{
    sml::ClientAnalyzedXML response;
    no_agent_assertTrue(agent->ExecuteCommandLineXML("echo =", &response));
}

void FullTests_Parent::testFindAttrPipes()
{
    sml::ClientAnalyzedXML response;
    no_agent_assertTrue(agent->ExecuteCommandLineXML("add-wme I2 ^A a", &response));
    no_agent_assertTrue(agent->SynchronizeInputLink());
    sml::WMElement* pWME = agent->GetInputLink()->FindByAttribute("A", 0);
    no_agent_assertTrue(pWME);

    agent->GetInputLink()->CreateStringWME("B", "b") ;
    no_agent_assertTrue(agent->Commit());
    pWME = 0;
    pWME = agent->GetInputLink()->FindByAttribute("B", 0);
    no_agent_assertTrue(pWME);
}

void FullTests_Parent::testTemplateVariableNameBug()
{
    loadProductions(SoarHelper::GetResource("test1121.soar"));
    agent->ExecuteCommandLine("run");
    sml::ClientAnalyzedXML response;
    agent->ExecuteCommandLineXML("stats", &response);
    no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
    no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 4);

    SoarHelper::init_check_to_find_refcount_leaks(agent);

}

void FullTests_Parent::testNegatedConjunctiveChunkLoopBug510()
{
    loadProductions(SoarHelper::GetResource("testNegatedConjunctiveChunkLoopBug510.soar"));
    agent->ExecuteCommandLine("run");
    sml::ClientAnalyzedXML response;
    agent->ExecuteCommandLineXML("stats", &response);
    no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
    no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testGDSBug1144()
{
    loadProductions(SoarHelper::GetResource("testGDSBug1144.soar"));
    agent->ExecuteCommandLine("run");

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testGDSBug1011()
{
    loadProductions(SoarHelper::GetResource("testGDSBug1011.soar"));
    agent->ExecuteCommandLine("run");
    sml::ClientAnalyzedXML response;
    agent->ExecuteCommandLineXML("stats", &response);
    no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 8);
    no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 19);

    SoarHelper::init_check_to_find_refcount_leaks(agent);

}

void FullTests_Parent::testLearn()
{
    loadProductions(SoarHelper::GetResource("testLearn.soar"));
    agent->ExecuteCommandLine("chunk unflagged");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // learning is off, same behavior expected
    agent->ExecuteCommandLine("init");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // turn learn except on
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("chunk unflagged");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // don't learn is active so same result expected
    agent->ExecuteCommandLine("init");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // get rid of dont learn
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("excise dont*learn");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // expect improvement
    agent->ExecuteCommandLine("init");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
    }

    // go to only mode
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("excise -c");
    agent->ExecuteCommandLine("chunk only");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // force learn is active, expect improvement
    agent->ExecuteCommandLine("init");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 1);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 3);
    }

    // get rid of chunk and force learn
    agent->ExecuteCommandLine("init");
    agent->ExecuteCommandLine("excise -c");
    agent->ExecuteCommandLine("excise force*learn");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }

    // expect no improvement
    agent->ExecuteCommandLine("init");
    m_pKernel->RunAllAgentsForever();
    {
        sml::ClientAnalyzedXML response;
        agent->ExecuteCommandLineXML("stats", &response);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountDecision, -1) == 3);
        no_agent_assertTrue(response.GetArgInt(sml::sml_Names::kParamStatsCycleCountElaboration, -1) == 5);
    }
}

void FullTests_Parent::testSVS()
{
    m_pKernel->AddRhsFunction("failed", Handlers::MyRhsFunctionFailureHandler, 0) ;
    loadProductions(SoarHelper::GetResource("testSVS.soar"));
    agent->ExecuteCommandLine("run");
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testPreferenceSemantics()
{
    m_pKernel->AddRhsFunction("failed", Handlers::MyRhsFunctionFailureHandler, 0) ;
    loadProductions(SoarHelper::GetResource("pref-semantics-test.soar"));
    agent->ExecuteCommandLine("run");
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testMatchTimeInterrupt()
{
    m_pKernel->AddRhsFunction("failed", Handlers::MyRhsFunctionFailureHandler, 0) ;
    loadProductions(SoarHelper::GetResource("testMatchTimeInterrupt.soar"));
    agent->ExecuteCommandLine("run");
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
void FullTests_Parent::testNegatedConjunctiveTestReorder()
{
    agent->ExecuteCommandLine("sp {test (state <s> ^a <val> -^a {<val> < 1}) --> }");
    std::string production(agent->ExecuteCommandLine("print test"));
    no_agent_assertTrue(production == "sp {test\n    (state <s> ^a <val> -^a { <val> < 1 })\n    -->\n    \n}\n\n");
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testNegatedConjunctiveTestUnbound()
{
    // all of these should fail without crashing:
    agent->ExecuteCommandLine("sp {test (state <s> ^superstate nil -^foo { <> <bad> }) --> }");
    // <bad> is unbound referent in value test
    no_agent_assertTrue(agent->GetLastCommandLineResult() == false);

    agent->ExecuteCommandLine("sp {test (state <s> ^superstate nil -^{ <> <bad> } <s>) --> }");
    // <bad> is unbound referent in attr test
    no_agent_assertTrue(agent->GetLastCommandLineResult() == false);

    agent->ExecuteCommandLine("sp {test (state <s> ^superstate nil -^foo { <> <b> }) -{(<s> ^bar <b>) (<s> -^bar { <> <b>})} --> }");
    // <b> is unbound referent in test, defined in ncc out of scope
    no_agent_assertTrue(agent->GetLastCommandLineResult() == false);

    agent->ExecuteCommandLine("sp {test  (state <s> ^superstate <d> -^foo { <> <b> }) -{(<s> ^bar <b>) (<s> -^bar { <> <d>})} --> }");
    // <d> is unbound referent in value test in ncc
    no_agent_assertTrue(agent->GetLastCommandLineResult() == false);

    // these should succeed
    agent->ExecuteCommandLine("sp {test (state <s> ^superstate <d>) -{(<s> ^bar <b>) (<s> -^bar { <> <d>})} --> }");
    // <d> is bound outside of ncc but in scope
    no_agent_assertTrue(agent->GetLastCommandLineResult());

    agent->ExecuteCommandLine("sp {test (state <s> ^superstate nil) -{(<s> ^bar <d>) (<s> -^bar { <> <d>})} --> }");
    // <d> is bound inside of ncc
    no_agent_assertTrue(agent->GetLastCommandLineResult());
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

// Maintainer note: this test fails if the WORKING_DIRECTORY env var is not set, and
// the current working directory is not the same as the one that the UnitTests exe is in.
// If you noticed it failing, try `cd out` and run UnitTests again.
void FullTests_Parent::testCommandToFile()
{
    loadProductions(SoarHelper::GetResource("water-jug-rl.soar"));
    m_pKernel->RunAllAgentsForever();

    const char* workingDirectory = getenv("WORKING_DIRECTORY");

    std::string resourceDirectory = SoarHelper::ResourceDirectory;

    if (workingDirectory)
        resourceDirectory = workingDirectory;

    agent->ExecuteCommandLine(("command-to-file \"" + resourceDirectory + "testCommandToFile-output.soar\" print --rl --full").c_str());
    no_agent_assertTrue(agent->GetLastCommandLineResult());
    const char* result = agent->ExecuteCommandLine(("source \"" + resourceDirectory + "/" + "testCommandToFile-output.soar\"").c_str());
    no_agent_assertTrue(result);
    const std::string resultString("#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*\nTotal: 144 productions sourced. 144 productions excised.\n");
    no_agent_assertTrue(result == resultString);
    remove(("\"" + resourceDirectory + "/" + "testCommandToFile-output.soar\"").c_str());
    SoarHelper::init_check_to_find_refcount_leaks(agent);
}

void FullTests_Parent::testConvertIdentifier()
{
    agent->ExecuteCommandLine("sp {test (state <s> ^io.output-link <out>) --> (<out> ^a b)}");
    agent->ExecuteCommandLine("sp {fnord (state <s> ^io.output-link <out>) (<out> ^foo <f>) --> (<out> ^bar <f>)}");
    m_pKernel->RunAllAgents(2);

    sml::Identifier* pOutputLink = agent->GetOutputLink();
    sml::Identifier* pFoo = pOutputLink->CreateIdWME("foo");

    char const* pConvertedId = agent->ConvertIdentifier(pFoo->GetValueAsString());
    no_agent_assertTrue(pConvertedId == pFoo->GetValueAsString());

    agent->Commit();
    m_pKernel->RunAllAgents(2);

    sml::WMElement* pBarWme = pOutputLink->FindByAttribute("bar", 0);

    pConvertedId = agent->ConvertIdentifier(pFoo->GetValueAsString());
    no_agent_assertTrue(pConvertedId);
    std::string convertedId(pConvertedId);
    no_agent_assertTrue(convertedId == pBarWme->GetValueAsString());

    SoarHelper::init_check_to_find_refcount_leaks(agent);

}

void FullTests_Parent::testOutputLinkRemovalOrdering()
{
    agent->SetOutputLinkChangeTracking(false);
    // The following would crash with output-link change tracking false

    // Creates an output link command O3 ^one.two 2 and then a shared wme with the same child O3 ^three.two 2
    agent->ExecuteCommandLine("sp {one (state <s> ^superstate nil -^phase) --> (<s> ^operator.name one) }");
    agent->ExecuteCommandLine("sp {one-apply (state <s> ^operator.name one ^io.output-link <ol>) --> (<ol> ^one.two 2) (<s> ^phase 1) }");
    agent->ExecuteCommandLine("sp {two (state <s> ^superstate nil ^phase 1) --> (<s> ^operator.name two) }");
    agent->ExecuteCommandLine("sp {two-apply (state <s> ^operator.name two ^io.output-link <ol>) (<ol> ^one <one>) --> (<ol> ^three <one>) (<s> ^phase 1 - 2) }");

    // Removes O3 ^one <one> leaving a parent with a higher timetag than it's children, and then removes the other, then halts
    agent->ExecuteCommandLine("sp {three (state <s> ^superstate nil ^phase 2) --> (<s> ^operator.name three) }");
    agent->ExecuteCommandLine("sp {three-apply (state <s> ^operator.name three ^io.output-link <ol>) (<ol> ^one <one>) --> (<ol> ^one <one> -) (<s> ^phase 2 - 3) }");
    agent->ExecuteCommandLine("sp {four (state <s> ^superstate nil ^phase 3) --> (<s> ^operator.name four) }");
    agent->ExecuteCommandLine("sp {four-apply (state <s> ^operator.name four ^io.output-link <ol>) (<ol> ^<asdf> <one>) --> (<ol> ^<asdf> <one> -) (<s> ^phase 3 - 4) }");
    agent->ExecuteCommandLine("sp {five (state <s> ^superstate nil ^phase 4) --> (<s> ^operator.name five) }");
    agent->ExecuteCommandLine("sp {five-apply (state <s> ^operator.name five) --> (halt) }");

    SoarHelper::init_check_to_find_refcount_leaks(agent);
}
