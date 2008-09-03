//TicTacToe #includes
#include "board.h"
#include "gSKISoarAgent.h"

//debugger #includes
#include "TgD.h"
#include "tcl.h"

//gSKI #includes
#include "IgSKI_KernelFactory.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_SymbolFactory.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_WorkingMemory.h"
#include "gSKI.h"
#include "gSKI_Stub.h"

//TgDI directives
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define TGD_SLEEP Sleep
#else
#include <unistd.h>
#define TGD_SLEEP usleep
#endif

//standard #includes
#include <stdexcept>
#include <iostream>
#include <cassert>

using namespace gSKI;

using std::cout;
using std::endl;
using std::cin;
using std::cerr;

//==========================================================================================
//==========================================================================================
int main(int argc,char* argv[])
{	
	//create the Soar part of the agent and load its productions
	IKernelFactory* kFactory = gSKI_CreateKernelFactory();
	IKernel* kernel = kFactory->Create();
	IAgentManager* manager = kernel->GetAgentManager();
	gSKI::IAgent* agent = manager->AddAgent("gskiTicAgent", "tictactoe.soar");

	//=============================================================================
	//========================= Setup the TgDI for the TSI ========================
	// Determine tsi version to use via command line argument
	TgD::TSI_VERSION tsiVersion;
	const char * usage = "[ 25 | 40 ]";
	if (argc > 2)
	{
		// "TgDITestd" should be argv[0] but windows puts the entire path in there
		std::cout << "gSKITicTacToe: too many arguments.\nUsage: gSKITicTacToe " << usage << std::endl;
		return 1;
	} 
	else if (argc == 2)
	{
		if (strcmp(argv[1], "25") == 0)
		{
			tsiVersion = TgD::TSI25;
		}
		else if (strcmp(argv[1], "40") == 0)
		{
			tsiVersion = TgD::TSI40;
		}
		else
		{ 
			// "TgDITestd" should be argv[0] but windows puts the entire path in there
			std::cout << "gSKITicTacToe: incorrect argument.\nUsage: gSKITicTacToe " << usage << std::endl;
			return 1;
		}
	}
	else
	{
		std::cout << "No TSI version specified, defaulting to 4.0.0" << std::endl; 
		tsiVersion = TgD::TSI40;
	}


	//create debugger
	TgD::TgD* debugger = CreateTgD(agent, kernel, kFactory->GetKernelVersion(), tsiVersion, argv[0]);
	debugger->Init();

	//=============================================================================
	//=============================================================================
	try{
		//Create the part of the agent that exists in the simulation
		Board board;
		SoarAgent* ai = new SoarAgent(MARKER_X, &board, agent);

		/* ==================== SGIO IMPLEMENTATION ===========================*/
		//agent = soar->CreateAgent("tictactoe");
		//agent->LoadProductions("tictactoe.soar");
		//NOTE - gSKI doesn't have these options - but does allow selection of where thread runs
		//if(embed)
		//    soar = new sgio::APISoar();
		//else
		//    soar = new sgio::SIOSoar("127.0.0.1",6969,true);
		//
		/* ==================== SGIO IMPLEMENTATION ===========================*/

		IInputLink* iLink = agent->GetInputLink();

		//register each square as an input producer
		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				SoarSquare* soarSquare = ai->GetSquareAt(i, j);
				iLink->AddInputProducer(soarSquare->GetSquareIdentifier(), soarSquare);
			}
		}

		//register the SoarAgent as the output processor
		IOutputLink* oLink = agent->GetOutputLink();
		oLink->AddOutputProcessor("move",ai); 


		Marker won, turn = MARKER_X;
		while(!board.IsCatGame() && (won = board.CheckForWinner()) == MARKER_NONE)
		{
			TgD::TgD::Update(false, debugger);
			board.Print();
			//ai->DebugPrintSoarSquares();

			if(turn == MARKER_X)
			{
				//cout << "Ai moving" << endl << endl;
				ai->MakeMove();
			}
			else
			{
				bool validMove = false;
				while(!validMove)
				{
					cout << "Enter your move " <<  MarkerToChar(turn) << ':';
					int x,y;
					cin >> x >> y;
					if(cin)
					{
						try
						{
							// Place a marker in the board environment
							board.PlaceMarker(turn,x,y);
							validMove = true;
						}
						catch(...)
						{
							cout << "That is not a valid move, try again.\n";
						}
					}
					else
					{
						cout << "Invalid line, clearing input...\n";
						cin.clear();
						while(cin.get() != '\n') {}
					}
				}
			}
			turn = (turn == MARKER_X) ? MARKER_O : MARKER_X;

		}//main loop 'while'

		board.Print();

		if(board.IsCatGame())
			cout << "Cat's game" << endl;
		else
			cout << MarkerToChar(won) << " won!" << endl;

		while(TgD::TgD::Update(false, debugger))
			TGD_SLEEP(50);
	}//try

	catch(std::exception& se)
	{
		cerr << se.what() << "\n";
		while(TgD::TgD::Update(false, debugger))
			TGD_SLEEP(50);
	}
	catch(char* error)
	{
		cout << error << endl;
		while(TgD::TgD::Update(false, debugger))
			TGD_SLEEP(50);
	}
	catch(...)
	{
		cout << "Some sort of exception." << endl;
		while(TgD::TgD::Update(false, debugger))
			TGD_SLEEP(50);
	}

	return 0;
}

