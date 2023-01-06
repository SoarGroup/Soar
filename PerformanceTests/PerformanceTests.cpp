
#include "PerformanceTests.h"

#include "sml_Client.h"
#include "sml_Connection.h"

using namespace sml;

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
    std::cout << pMessage << std::endl;
}

void Run_PerformanceTest(int numTrials, int numDecisions, StatsTracker* pSt, const std::vector<std::string>& commands, int numInits, const std::vector<std::string>& init_commands)
{

    for (int i = 0; i < numTrials; i++)
    {
        Kernel* kernel = Kernel::CreateKernelInNewThread();
        Agent* agent = kernel->CreateAgent("Soar1");
        std::string runCmd = "time run ";
        if (numDecisions > 0) runCmd += std::to_string(numDecisions);
        std::cout << (i+1) << " ";
        std::cout.flush();

        #ifndef QUIET_MODE
        agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);
        #endif
        agent->SetOutputLinkChangeTracking(false);

        for (int pass = 0; pass <= numInits; ++pass)
        {
            if (pass > 0)
            {
                for (int j = 0; j < init_commands.size(); ++j)
                {
                    agent->ExecuteCommandLine(init_commands[j].c_str());
                }
                std::cout << ".";
                std::cout.flush();
            } else {
                for (int j = 0; j < commands.size(); ++j)
                {
                    agent->ExecuteCommandLine(commands[j].c_str());
                }
            }
            {
                ClientAnalyzedXML response;
                agent->ExecuteCommandLineXML(runCmd.c_str(), &response);

                pSt->realtimes.push_back(response.GetArgFloat(sml_Names::kParamRealSeconds, 0.0));

                agent->ExecuteCommandLineXML("stats", &response);
                pSt->kerneltimes.push_back(response.GetArgFloat(sml_Names::kParamStatsKernelCPUTime, 0.0));
                pSt->totaltimes.push_back(response.GetArgFloat(sml_Names::kParamStatsTotalCPUTime, 0.0));
            }
        }
        kernel->Shutdown();
        delete kernel;

        if (!numInits) std::cout << "✅  "; else std::cout << "✅\n";
        std::cout.flush();
    }

    std::cout << std::endl;
    std::cout.flush();
}

int main(int argc, char* argv[])
{
    set_working_directory_to_executable_path();

    const char* agentname ;
    int numTrials = DEFAULT_TRIALS;
    int numDCs = DEFAULT_DCS;
    int numInits = DEFAULT_INITS;

    if (argc == 1)
    {
        agentname = DEFAULT_AGENT;
    }
    else if (argc == 2)
    {
        agentname = argv[1];
    }
    else if (argc == 3)
    {
        agentname = argv[1];
        std::stringstream(argv[2]) >> numTrials;
    }
    else if (argc == 4)
    {
        agentname = argv[1];
        std::stringstream(argv[2]) >> numTrials;
        std::stringstream(argv[3]) >> numDCs;
    }
    else if (argc == 5)
    {
        agentname = argv[1];
        std::stringstream(argv[2]) >> numTrials;
        std::stringstream(argv[3]) >> numDCs;
        std::stringstream(argv[4]) >> numInits;
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " [default | <agent name>] [<numtrials>] [<num_decisions>] [<num_init_and_rerun>]" << std::endl;
        return 1;
    }
    if (!strcmp(agentname, "default"))
    {
        agentname = DEFAULT_AGENT;
    }
    if (!numDCs) numDCs = DEFAULT_DCS;

    std::cout << "\033[1;31m" << agentname << "\033[0;37m" << ": ";
    if (numTrials > 1) std::cout << numTrials << " trials"; else std::cout << "single run";
    if (numDCs > 0) std::cout << ", " << numDCs << " DCs"; else std::cout << ", run forever";
    if (numInits > 0) std::cout << ", " << numInits << " extra init-soar/runs\n"; else std::cout << std::endl;
    std::cout.flush();

    {
        StatsTracker l_testStats;
        std::vector<std::string> commands, init_commands;

        commands.push_back("pushd SoarPerformanceTests");
        std::string srccmd = "source ";
        srccmd += agentname;
        srccmd += ".soar";
        commands.push_back(srccmd.c_str());
        commands.push_back("output console off");
        commands.push_back("output callbacks off");
        commands.push_back("output agent-writes off");
        commands.push_back("output enable off");
        commands.push_back("watch 0");
        commands.push_back("srand 3");

        init_commands.push_back("init-soar");
        init_commands.push_back("excise -c");
        init_commands.push_back("srand 3");

        Run_PerformanceTest(numTrials, numDCs, &l_testStats, commands, numInits, init_commands);

        l_testStats.PrintResults(agentname);
    }

    return 0;
}

