#include <iostream>
#include <string>
//#include <crtdbg.h>

// Quick addition so we can time this easily.
#include "../../Profiler/include/simple_timer.h"

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

int main(int argc, char* argv[])
{
	bool doPrinting = false;
	int numTowers = defaultNumTowers;
	//int numdisks = defaultNumdisks;

	cout << "Start" << endl ;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "true"))
			doPrinting = true;
		// @TODO more checking, for robustness 
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
		SimpleTimer timer ;
		SimpleTimer total ;

		if(doPrinting)
			cout << "***Welcome to Towers of Hanoi***" << endl << endl;

		HanoiWorld hanoi(doPrinting, numTowers);

		double time = timer.Elapsed() ;
		cout << "Time to initialize: " << time << endl ;
		timer.Start() ;

		if(doPrinting)
			hanoi.Print();

		while(!hanoi.AtGoalState())
		{
			hanoi.Run();

			if(doPrinting)
				hanoi.Print();
		}

		hanoi.EndGameAction();

		time = timer.Elapsed() ;
		cout << "Time after initialization to complete: " << time << endl ;
		time = total.Elapsed() ;
		cout << "Total run time: " << time << endl ;
	}

//#ifdef _DEBUG
//	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
//	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

//	_CrtDbgReport(_CRT_WARN, NULL, NULL, "TowersOfHanoi", "Checking memory in TowersOfHanoi\n");
//	_CrtDumpMemoryLeaks();
//#endif

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <non-whitespace char> then enter to exit\n") ;
	string garbage;
	cin>>garbage;
	return 0;
}

