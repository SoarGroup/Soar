#include <iostream>
#include <string>


////// Define the type of interface to Soar that you're using:
#define GSKI_DIRECT
//#define USE_TGD_WITH_GSKI
//#define SML_THROUGH_GSKI
//#define SML_SGIO_HYBRID
//#define SGIO_DIRECT


#ifdef SGIO_DIRECT
	#include "sgioTowers.h"
#else ifdef GSKI_DIRECT
	#include "gSKITowers.h"
#endif

#ifdef SML_SGIO_HYBRID
	//SML Directives
	#include "sml_Client.h"
	using namespace sml;
#endif

#ifdef SML_THROUGH_GSKI
	//SML Directives
	#include "sml_Client.h"
	using namespace sml;
#endif


#ifdef USE_TGD_WITH_GSKI
	//TgDI directives
	#include "TgD.h"
	#include "tcl.h"

	#ifdef _WIN32
		#define _WINSOCKAPI_
		#include <Windows.h>
		#define TGD_SLEEP Sleep
	#else
		#include <unistd.h>
		#define TGD_SLEEP usleep
	#endif
#endif


using std::cout; using std::cin; using std::string;
using std::endl;

const int defaultNumTowers = 3;
const int defaultNumdisks = 11;

int main(int argc, char* argv[])
{
	bool doPrinting = true;
	int numTowers = defaultNumTowers;
	int numdisks = defaultNumdisks;

	if(argc > 1)
	{
		if(!strcmp(argv[1], "false"))
			doPrinting = false;
		// @TODO more checking, for robustness 
	}

	if(argc > 2)
	{
		numTowers = atoi(argv[3]);
		if(numTowers < 3)
			numTowers = 3;
	}

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
		{
			hanoi.Print();
cout << "pre-main loop, initial print is above..." << endl;
		}

		while(!hanoi.AtGoalState())
		{
cout << "Main loop, please run!" << endl;
			hanoi.Run();
cout << "Main loop, ran...." << endl;
			if(doPrinting)
				hanoi.Print();
		}

		hanoi.EndGameAction();
	}

//	while(TgD::TgD::Update(false, debugger))
//		TGD_SLEEP(50);

	// Wait for the user to press return to exit the program. (So window doesn't just vanish).
	printf("\n\nPress <non-whitespace char> then enter to exit\n") ;
	string garbage;
	cin>>garbage;
	return 0;
}

