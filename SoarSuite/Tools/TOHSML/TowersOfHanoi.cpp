#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

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

// helps quell warnings
#ifndef unused
#define unused(x) (void)(x)
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

int main(int argc, char* argv[])
{
	bool doPrinting = false;
	bool stopAtEnd = true ;
	bool remoteConnection = false ;

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
			if (!strcasecmp(argv[i], "-remote"))
				remoteConnection = true ;
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

		HanoiWorld hanoi(remoteConnection, doPrinting, numTowers);

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

//#ifdef _DEBUG
//	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
//	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

//	_CrtDbgReport(_CRT_WARN, NULL, NULL, "TowersOfHanoi", "Checking memory in TowersOfHanoi\n");
//	_CrtDumpMemoryLeaks();
//#endif

#ifdef _MSC_VER
	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	if (stopAtEnd)
	{
		printf("\n\nPress <return> to exit\n") ;
		char line[100] ;
		char* str = gets(line) ;
		unused(str);
	}
#endif //_MSC_VER

	return 0;
}

