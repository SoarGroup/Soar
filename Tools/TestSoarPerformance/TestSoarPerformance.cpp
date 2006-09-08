#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#ifdef _MSC_VER
// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <assert.h>
#include <string>
#include <iostream>
#include <iomanip>
#include "sml_Client.h"
#include "sml_Connection.h"

using namespace std;
using namespace sml;

class StatsTracker {
public:
	vector<double> realtimes;
	vector<double> proctimes;
	vector<double> kerneltimes;
	vector<double> totaltimes;

	double GetAverage(vector<double> numbers) {
		assert(numbers.size() > 0 && "GetAverage: Size of set must be non-zero");

		double total = 0.0;
		for(unsigned int i=0; i<numbers.size(); i++) {
			total += numbers[i];
		}
		return total/(double)numbers.size();
	}

	double GetHigh(vector<double> numbers) {
		assert(numbers.size() > 0 && "GetHigh: Size of set must be non-zero");

		double high = numbers[0];
		for(unsigned int i=0; i<numbers.size(); i++) {
			if(numbers[i] > high) high = numbers[i];
		}
		return high;
	}

	double GetLow(vector<double> numbers) {
		assert(numbers.size() > 0 && "GetLow: Size of set must be non-zero");

		double low = numbers[0];
		for(unsigned int i=0; i<numbers.size(); i++) {
			if(numbers[i] < low) low = numbers[i];
		}
		return low;
	}

	void PrintResults() {
		cout << resetiosflags(ios::right) << setiosflags(ios::left);
		cout << setw(12) << " ";
		cout << " ";
		cout << resetiosflags(ios::left) << setiosflags(ios::right);
		cout << setw(10) << "Avg";
		cout << setw(10) << "Low";
		cout << setw(10) << "High" << endl;
		PrintResultsHelper("OS Real", GetAverage(realtimes), GetLow(realtimes), GetHigh(realtimes));
		PrintResultsHelper("OS Proc", GetAverage(proctimes), GetLow(proctimes), GetHigh(proctimes));
		PrintResultsHelper("Soar Kernel", GetAverage(kerneltimes), GetLow(kerneltimes), GetHigh(kerneltimes));
		PrintResultsHelper("Soar Total", GetAverage(totaltimes), GetLow(totaltimes), GetHigh(totaltimes));
	}

	void PrintResultsHelper(string label, double avg, double low, double high) {
		cout.precision(3);
		cout << resetiosflags(ios::right) << setiosflags(ios::left);
		cout << setw(12) << label;
		cout << ":";
		cout << resetiosflags(ios::left);
		cout << setiosflags(ios::right);
		cout << setw(10) << setiosflags(ios::fixed) << setprecision(3) << avg;
		cout << setw(10) << setiosflags(ios::fixed) << setprecision(3) << low;
		cout << setw(10) << setiosflags(ios::fixed) << setprecision(3) << high;
		cout << endl;
	}
};


void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) {
	cout << pMessage << endl;
}

void PrintTest1Description(int numTrials) {
	cout << endl;
	cout << "Test1 creates a kernel, runs the test suite once, and destroys the kernel." << endl;
	cout << "This is repeated " << numTrials << " times to measure average performance." << endl;
	cout << "Importantly, since the kernel is destroyed between each run, memory needs to be reallocated each time." << endl;
}
void Test1(int numTrials, StatsTracker* pSt, vector<string>* commands) {

	for(int i = 0; i < numTrials; i++) {
		cout << endl << "***** Trial " << (i+1) << " of " << numTrials << " Begin *****" << endl;

		Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
		Agent* agent = kernel->CreateAgent("Soar1");
		
		agent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, NULL);

		for(vector<string>::iterator itr = commands->begin(); itr != commands->end(); itr++) {
			agent->ExecuteCommandLine((*itr).c_str());
		}

		ClientAnalyzedXML response;
		agent->ExecuteCommandLineXML("time run", &response);
		
		pSt->realtimes.push_back(response.GetArgFloat(sml_Names::kParamRealSeconds, 0.0));
		pSt->proctimes.push_back(response.GetArgFloat(sml_Names::kParamProcSeconds, 0.0));

		agent->ExecuteCommandLineXML("stats", &response);
		
		pSt->kerneltimes.push_back(response.GetArgFloat(sml_Names::kParamStatsKernelCPUTime, 0.0));
		pSt->totaltimes.push_back(response.GetArgFloat(sml_Names::kParamStatsTotalCPUTime, 0.0));
		
		agent->ExecuteCommandLine("stats");

		kernel->Shutdown();
		delete kernel;

		cout << endl << "***** Trial " << (i+1) << " of " << numTrials << " Complete *****" << endl;
	}
}

int main() {

	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 73 ;

	{ // create local scope to allow for local memory cleanup before we check at end

		StatsTracker stTest1_learnoff, stTest1_learnon;
		vector<string> commands;
		commands.push_back("source ../Tests/TestSoarPerformance.soar");
		commands.push_back("watch 0");
		
		int numTrials = 3;

		cout << endl << "The test suite will be run in two phases, using " << numTrials << " trials each time." << endl;
		cout << "The first phase will be with learning off. The second will be with learning on." << endl;
		cout << "All results will be reported at the very end." << endl;
		cout << "Press enter to begin." << endl;
		cin.get();

		cout << endl << "***** Running suite with learning off *****" << endl;
		Test1(numTrials, &stTest1_learnoff, &commands);
		commands.push_back("learn --on");
		cout << endl << "***** Running suite with learning on *****" << endl;
		Test1(numTrials, &stTest1_learnon, &commands);

		//PrintTest1Description(numTrials);

		cout << endl << "watch 0 learning off" << endl;
		stTest1_learnoff.PrintResults();
		cout << endl << "watch 0 learning on" << endl;
		stTest1_learnon.PrintResults();

		cout << endl << endl << "Press enter to exit.";
		cin.get();

	} // end local scope

#ifdef _MSC_VER
	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER
	
	return 0;
}

