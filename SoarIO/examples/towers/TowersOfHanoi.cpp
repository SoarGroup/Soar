#include <iostream>
#include <string>

////// Define the type of interface to Soar that you're using:
#define GSKI_DIRECT
//#define SML_THROUGH_GSKI
//#define SML_SGIO_HYBRID
//#define SGIO_DIRECT


#ifdef SGIO_DIRECT
	#include "sgioTowers.h"
#endif

#ifdef GSKI_DIRECT
	#include "gSKITowers.h"
#endif

#ifdef SML_SGIO_HYBRID
	//SML Directives
#endif

#ifdef SML_THROUGH_GSKI
	//SML Directives
#endif


using std::cout; using std::cin; using std::endl;
using std::string;

const int defaultNumTowers = 3;
//const int defaultNumdisks = 11;

int main(int argc, char* argv[])
{
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

	/*	while(!hanoi.AtGoalState())
		{
			hanoi.Run();

			if(doPrinting)
				hanoi.Print();
		}

		hanoi.EndGameAction();*/
	}


	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <non-whitespace char> then enter to exit\n") ;
	string garbage;
	cin>>garbage;
	return 0;
}

