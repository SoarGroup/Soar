#include "SvsTests.hpp"
#include "soar_instance.h"

void SvsTests::setUp()
{
	kernel = sml::Kernel::CreateKernelInCurrentThread(true, sml::Kernel::kUseAnyPort);
    configure_for_unit_tests();
	agent = kernel->CreateAgent("soar1");
    configure_agent_for_unit_tests(NULL);
}

void SvsTests::tearDown(bool caught)
{
	kernel->DestroyAgent(agent);
	kernel->Shutdown();
	delete kernel;
	kernel = nullptr;
}

void SvsTests::testEnableFromTopState()
{
    // enable, step, check that ^svs exists everywhere

    agent->ExecuteCommandLine("svs --enable");
    assertTrue_msg("Failed to enable SVS.",
        agent->GetLastCommandLineResult());

    agent->ExecuteCommandLine("svs --enable-in-substates");
    assertTrue_msg("Failed to enable SVS in substates.",
        agent->GetLastCommandLineResult());

    std::string result = agent->ExecuteCommandLine("print S1");
    assertFalse_msg("Expected to find ^svs in S1; return was " + result,
        result.find("^svs") == std::string::npos);

    // no productions, so agent will subgoal each step
    agent->ExecuteCommandLine("step 2");

    result = agent->ExecuteCommandLine("print S2");
    assertFalse_msg("Expected to find ^svs in S2; return was: " + result,
        result.find("^svs") == std::string::npos);
    result = agent->ExecuteCommandLine("print S3");
    assertFalse_msg("Expected to find ^svs in S3",
        result.find("^svs") == std::string::npos);
}

void SvsTests::testEnableAndDisableInSubstatesFromTopState() {
    // disable-in-substates, step, ^svs should not exist in substates

    agent->ExecuteCommandLine("svs --enable");
    assertTrue_msg("Failed to enable SVS.",
        agent->GetLastCommandLineResult());

    agent->ExecuteCommandLine("svs --disable-in-substates");
    assertTrue_msg("Failed to enable SVS in substates.",
        agent->GetLastCommandLineResult());

    std::string result = agent->ExecuteCommandLine("print S1");
    assertFalse_msg("Expected to find ^svs in S1",
        result.find("^svs") == std::string::npos);

    // no productions, so agent will subgoal each step
    agent->ExecuteCommandLine("step 2");

    result = agent->ExecuteCommandLine("print S2");
    assertTrue_msg("Expected to not find ^svs in S2; return was: " + result,
        result.find("^svs") == std::string::npos);
    result = agent->ExecuteCommandLine("print S3");
    assertTrue_msg("Expected to not find ^svs in S3",
        result.find("^svs") == std::string::npos);
}

void SvsTests::testCannotDisableInSubstate()
{
    // enable, step, try to disable, should fail

    agent->ExecuteCommandLine("svs --enable");
    assertTrue_msg("Failed to enable SVS.",
        agent->GetLastCommandLineResult());

    // no productions, so agent will subgoal each step
    agent->ExecuteCommandLine("step 1");

    std::string result = agent->ExecuteCommandLine("svs --disable");
    assertFalse_msg("Expected not to be able to disable SVS from a substate; return was: " + result + " (" + (agent->GetLastCommandLineResult() ? "true" : "false") + ")",
        agent->GetLastCommandLineResult());

    result = agent->ExecuteCommandLine("svs --disable-in-substates");
    assertFalse_msg("Expected not to be able to disable SVS in substates from a substate; return was: " + result,
        agent->GetLastCommandLineResult());
}

void SvsTests::testEnableFromSubstate()
{
    // disable, step, enable, check that ^svs exists in all states



    agent->ExecuteCommandLine("svs --disable");
    assertTrue_msg("Failed to disable SVS.",
        agent->GetLastCommandLineResult());

    // no productions, so agent will subgoal each step
    agent->ExecuteCommandLine("step 2");
    agent->ExecuteCommandLine("svs --enable");
    assertTrue_msg("Failed to enable SVS.",
        agent->GetLastCommandLineResult());

    std::string result = agent->ExecuteCommandLine("print S1");
    assertFalse_msg("Expected to find ^svs in S1",
        result.find("^svs") == std::string::npos);
    result = agent->ExecuteCommandLine("print S2");
    assertFalse_msg("Expected to find ^svs in S2",
        result.find("^svs") == std::string::npos);
    result = agent->ExecuteCommandLine("print S3");
    assertFalse_msg("Expected to find ^svs in S3",
        result.find("^svs") == std::string::npos);

}

void SvsTests::testEnableInSubstatesFromSubstate()
{
    // disable-in-substates, step and then enable, check that ^svs exists only on top state

    agent->ExecuteCommandLine("svs --disable");
    assertTrue_msg("Failed to disable SVS.",
        agent->GetLastCommandLineResult());
    agent->ExecuteCommandLine("svs --disable-in-substates");
    assertTrue_msg("Failed to disable SVS in substates.",
        agent->GetLastCommandLineResult());

    // no productions, so agent will subgoal each step
    agent->ExecuteCommandLine("step 2");

    std::string result = agent->ExecuteCommandLine("print S1");
    assertTrue_msg("Expected to not find ^svs in S1; return was " + result,
        result.find("^svs") == std::string::npos);
    result = agent->ExecuteCommandLine("print S3");
    assertTrue_msg("Expected to not find ^svs in S3",
        result.find("^svs") == std::string::npos);

    agent->ExecuteCommandLine("svs --enable");
    assertTrue_msg("Failed to enable SVS.",
        agent->GetLastCommandLineResult());
    result = agent->ExecuteCommandLine("print S3");
    assertTrue_msg("Expected to not find ^svs in S3",
        result.find("^svs") == std::string::npos);

    // then enable-in-substates and check that ^svs exists on all states

    agent->ExecuteCommandLine("svs --enable-in-substates");
    assertTrue_msg("Failed to enable SVS in substates.",
        agent->GetLastCommandLineResult());
    result = agent->ExecuteCommandLine("print S3");
    assertFalse_msg("Expected to find ^svs in S3",
        result.find("^svs") == std::string::npos);
}

void SvsTests::testSubstateWhenDisabledInSubstates()
{
    // TODO
}

void SvsTests::testSvsSceneCaseInsensitivity()
{
    agent->ExecuteCommandLine("svs --enable");
    no_agent_assertTrue_msg("failed to enable SVS", agent->GetLastCommandLineResult());

    std::string result;
    result = agent->ExecuteCommandLine("svs S1.scene.world");
    no_agent_assertFalse_msg("could not find S1 scene", result == "path not found\n");

    result = agent->ExecuteCommandLine("svs s1.scene.world");
    no_agent_assertFalse_msg("lower-case s1 scene name not found", result == "path not found\n");

    result = agent->ExecuteCommandLine("svs D34.scene.world");
    no_agent_assertTrue_msg("D34 scene name found: " + result, result == "path not found\n");
}
