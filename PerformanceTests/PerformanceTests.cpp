#include "portability.h"

#include <stdlib.h>

#include <assert.h>
#include <string>
#include <iostream>
#include <iomanip>
#include "sml_Client.h"
#include "sml_Connection.h"

#define QUIET_MODE
//#define BRIEF_MODE
#define DEFAULT_TRIALS 3
#define DEFAULT_DCS -1
#define DEFAULT_AGENT "count-test-5000";

using namespace std;
using namespace sml;

class StatsTracker
{
    public:
        vector<double> realtimes;
        vector<double> kerneltimes;
        vector<double> totaltimes;

        double GetAverage(vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetAverage: Size of set must be non-zero");

            double total = 0.0;
            for (unsigned int i = 0; i < numbers.size(); i++)
            {
                total += numbers[i];
            }
            return total / static_cast<double>(numbers.size());
        }

        double GetHigh(vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetHigh: Size of set must be non-zero");

            double high = numbers[0];
            for (unsigned int i = 0; i < numbers.size(); i++)
            {
                if (numbers[i] > high)
                {
                    high = numbers[i];
                }
            }
            return high;
        }

        double GetLow(vector<double> numbers)
        {
            assert(numbers.size() > 0 && "GetLow: Size of set must be non-zero");

            double low = numbers[0];
            for (unsigned int i = 0; i < numbers.size(); i++)
            {
                if (numbers[i] < low)
                {
                    low = numbers[i];
                }
            }
            return low;
        }

        void PrintResults()
        {
            cout << endl << "Test Results:" << endl;
            cout << resetiosflags(ios::right) << setiosflags(ios::left);
            cout << setw(12) << " ";
            cout << " ";
            cout << resetiosflags(ios::left) << setiosflags(ios::right);
            cout << setw(10) << "Avg";
#ifndef BRIEF_MODE
            cout << setw(10) << "Low";
            cout << setw(10) << "High" << endl;
#else
            cout << endl;
#endif
            PrintResultsHelper("OS Real", GetAverage(realtimes), GetLow(realtimes), GetHigh(realtimes));
            PrintResultsHelper("Soar Kernel", GetAverage(kerneltimes), GetLow(kerneltimes), GetHigh(kerneltimes));
            PrintResultsHelper("Soar Total", GetAverage(totaltimes), GetLow(totaltimes), GetHigh(totaltimes));
        }

        void PrintResultsHelper(string label, double avg, double low, double high)
        {
            cout.precision(3);
            cout << resetiosflags(ios::right) << setiosflags(ios::left);
            cout << setw(12) << label;
            cout << ":";
            cout << resetiosflags(ios::left);
            cout << setiosflags(ios::right);
            cout << setw(10) << setiosflags(ios::fixed) << setprecision(3) << avg;
#ifndef BRIEF_MODE
            cout << setw(10) << setiosflags(ios::fixed) << setprecision(3) << low;
            cout << setw(10) << setiosflags(ios::fixed) << setprecision(3) << high;
#endif
            cout << endl;
        }
};

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
    cout << pMessage << endl;
}

void Run_PerformanceTest(int numTrials, int numDecisions, StatsTracker* pSt, const vector<string>& commands)
{

    for (int i = 0; i < numTrials; i++)
    {
        Kernel* kernel = Kernel::CreateKernelInNewThread();
        Agent* agent = kernel->CreateAgent("Soar1");

#ifndef QUIET_MODE
        agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);
#endif
        agent->SetOutputLinkChangeTracking(false);

        for (int j = 0; j < commands.size(); ++j)
        {
            agent->ExecuteCommandLine(commands[j].c_str());
        }

        ClientAnalyzedXML response;
        string runCmd = "time run ";
        if (numDecisions > 0) runCmd += to_string(numDecisions);
        agent->ExecuteCommandLineXML(runCmd.c_str(), &response);

        pSt->realtimes.push_back(response.GetArgFloat(sml_Names::kParamRealSeconds, 0.0));

        agent->ExecuteCommandLineXML("stats", &response);

        pSt->kerneltimes.push_back(response.GetArgFloat(sml_Names::kParamStatsKernelCPUTime, 0.0));
        pSt->totaltimes.push_back(response.GetArgFloat(sml_Names::kParamStatsTotalCPUTime, 0.0));

        agent->ExecuteCommandLine("stats");

        kernel->Shutdown();
        delete kernel;

        cout << response.GetArgFloat(sml_Names::kParamStatsKernelCPUTime, 0.0) << " ✅  ";
        cout.flush();
    }

    cout << endl;
    cout.flush();
}

int main(int argc, char* argv[])
{

#ifdef _DEBUG
    // When we have a memory leak, set this variable to
    // the allocation number (e.g. 122) and then we'll break
    // when that allocation occurs.
    //_crtBreakAlloc = 1565 ;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

    set_working_directory_to_executable_path();

    const char* agentname ;
    int numTrials = DEFAULT_TRIALS;
    int numDCs = DEFAULT_DCS;

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
        stringstream(argv[2]) >> numTrials;
    }
    else if (argc == 4)
    {
        agentname = argv[1];
        stringstream(argv[2]) >> numTrials;
        stringstream(argv[3]) >> numDCs;
    }
    else
    {
        cout << "Usage: " << argv[0] << " [default | <agent name>] [<numtrials>] [<num_decisions>]" << endl;
        return 1;
    }
    if (!strcmp(agentname, "default"))
    {
        agentname = DEFAULT_AGENT;
    }

    cout << "Soar Performance Tests " << endl << endl;

    cout << "Measuring performance of " << agentname << " agent " << numTrials << " times: ";
    cout.flush();

    {
        // create local scope to allow for local memory cleanup before we check at end

        StatsTracker l_testStats;
        vector<string> commands;

        commands.push_back("pushd SoarPerformanceTests");
        string srccmd = "source ";
        srccmd += agentname;
        srccmd += ".soar";
        commands.push_back(srccmd.c_str());
        commands.push_back("output console off");
        commands.push_back("output callbacks off");
        commands.push_back("output agent-writes off");
        commands.push_back("watch 0");
        commands.push_back("srand 233391");

        Run_PerformanceTest(numTrials, numDCs, &l_testStats, commands);
        l_testStats.PrintResults();
    }

//    cout << endl << "Usage: " << argv[0] << " [default | <agent path>] [<numtrials>]  [<num_decisions>]" << endl << endl;

    return 0;
}

