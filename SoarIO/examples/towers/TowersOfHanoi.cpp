#include <iostream>
#include <string>
#include <crtdbg.h>

////// Define the type of interface to Soar that you're using:
#define GSKI_DIRECT
//#define SML_THROUGH_GSKI
//#define SML_SGIO_HYBRID
//#define SGIO_DIRECT

#ifdef SGIO_DIRECT
	//select which type of sgio interface to use	
	//#define SGIO_API_MODE
	#define SGIO_TSI_MODE
	#include "sgioTowers.h"

#elif defined GSKI_DIRECT
	#include "gSKITowers.h"

#elif defined SML_SGIO_HYBRID
	//SML Directives

#elif defined SML_THROUGH_GSKI
	//SML Directives

#endif


using std::cout; using std::cin; using std::endl;
using std::string;

const int defaultNumTowers = 3;
//const int defaultNumdisks = 11;

int main(int argc, char* argv[])
{
	//_crtBreakAlloc = 74;
	bool doPrinting = true;
	int numTowers = defaultNumTowers;
	//int numdisks = defaultNumdisks;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "false"))
			doPrinting = false;
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
		if(doPrinting)
			cout << "***Welcome to Towers of Hanoi***" << endl << endl;

		HanoiWorld hanoi(doPrinting, numTowers);

		if(doPrinting)
			hanoi.Print();

		while(!hanoi.AtGoalState())
		{
			hanoi.Run();

			if(doPrinting)
				hanoi.Print();
		}

		hanoi.EndGameAction();
	}

#ifdef _DEBUG
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	_CrtDbgReport(_CRT_WARN, NULL, NULL, "TowersOfHanoi", "Checking memory in TowersOfHanoi\n");
	_CrtDumpMemoryLeaks();
#endif

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <non-whitespace char> then enter to exit\n") ;
	string garbage;
	cin>>garbage;
	return 0;
}

