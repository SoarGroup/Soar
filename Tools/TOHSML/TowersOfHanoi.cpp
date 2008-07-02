#include <portability.h>

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <cstdlib>

//#include <crtdbg.h>

// Quick addition so we can time this easily.
//#include "../../Profiler/include/simple_timer.h"

////// Define the type of interface to Soar that you're using:
//#define GSKI_DIRECT
//#define SML_THROUGH_GSKI
//#define SML_SGIO_HYBRID
//#define SGIO_DIRECT
#define SML_STANDARD

#ifdef SGIO_DIRECT

	#include "sgioTowers.h"

#elif defined GSKI_DIRECT
	#include "gSKITowers.h"

#elif defined SML_STANDARD
	#include "smlTowers.h"

#elif defined SML_GSKI_STYLE
	//SML Directives

#endif

using std::cout; using std::cin; using std::endl;
using std::string;

const int defaultNumTowers = 3;
//const int defaultNumdisks = 11;

// We create a file to say we succeeded or not, deleting any existing results beforehand
// The filename shows if things succeeded or not and the contents can explain further.
void ReportResult(std::string testName, bool success)
{
	// Decide on the filename to use for success/failure
	std::string kSuccess = testName + "-success.txt" ;
	std::string kFailure = testName + "-failure.txt" ;

	// Remove any existing result files
	remove(kSuccess.c_str()) ;
	remove(kFailure.c_str()) ;

	// Create the output file
	std::ofstream outfile (success ? kSuccess.c_str() : kFailure.c_str());

	if (success)
	{
		outfile << "\nTests SUCCEEDED" << endl ;
		cout << "\nTests SUCCEEDED" << endl ;
	}
	else
	{
		outfile << "\n*** ERROR *** Tests FAILED" << endl ;
		cout << "\n*** ERROR *** Tests FAILED" << endl ;
	}

	outfile.close();
}

/*
 * Tries to parse the next arg as an int and returns it, 
 * if that fails it returns def (default)
 */
int getIntArg(int def, int next, int argc, char* argv[]) 
{
	if (next < argc)
	{
		int portIn = atoi(argv[next]) ;
		if (portIn > 0)
			return portIn;
	}
	return def;
}

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	//_crtBreakAlloc = 2263;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
#endif // _DEBUG

	{
		bool doPrinting = false;
		bool stopAtEnd = true ;
		bool remoteConnection = false ;
		int port = 12121;

		int numTowers = defaultNumTowers;
		//int numdisks = defaultNumdisks;

		cout << "Start" << endl ;

		// Read the command line options:
		// -nostop : don't ask user to hit return at the end
		// -remote : run the test over a remote connection -- needs a listening client (usually TestCommandLineInterface) to already be running.
		if (argc > 1)
		{
			for (int i = 1 ; i < argc ; i++)
			{
				if (!strcasecmp(argv[i], "-nostop"))
					stopAtEnd = false ;
				if (!strcasecmp(argv[i], "-remote")) {
					remoteConnection = true ;

					port = getIntArg(port, i+1, argc, argv);
				} 
				
				if (!strcasecmp(argv[1], "true"))
					doPrinting = true;
			}
		}

		//if(argc > 2)
		//{
		//	numTowers = atoi(argv[3]);
		//	if(numTowers < 3)
		//		numTowers = 3;
		//}

		//It would be flexible to read in the number of disks, but the productions are hard-coded to 11
		//if(argc > 3)
		//{
		//	numdisks = atoi(argv[3]);
		//	if(numdisks < 5)
		//		numdisks = 5;

		//}

		//=============================================================================
		//=============================================================================
		{
			//SimpleTimer timer ;
			//SimpleTimer total ;

			if(doPrinting)
				cout << "***Welcome to Towers of Hanoi***" << endl << endl;

			HanoiWorld hanoi(remoteConnection, port, doPrinting, numTowers);

			//double time = timer.Elapsed() ;
			//cout << "Time to initialize: " << time << endl ;
			//timer.Start() ;

			if(doPrinting)
				hanoi.Print();

			clock_t start_time, end_time;

			start_time = clock();
			while(!hanoi.AtGoalState())
			{
				hanoi.Run();

				if(doPrinting)
					hanoi.Print();
			}

			hanoi.EndGameAction();

			end_time = clock();
			double time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

			// DJP: Not sure how we can tell if this has really worked or not.
			// It looks like we'll be stuck in an infinite loop looking for the goal state
			// if something goes wrong, so I think if we get here it's a success.
			ReportResult(remoteConnection ? "towers-sml-remote" : "towers-sml", true) ;

			//time = timer.Elapsed() ;
			cout << "Time after initialization to complete (seconds): " << time << endl ;
			//time = total.Elapsed() ;
			//cout << "Total run time: " << time << endl ;
		}
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG

	return 0;
}

