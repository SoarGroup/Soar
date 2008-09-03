#include "board.h"

#include "gSKISoarAgent.h"

/*
#include "sml_ClientAgent.h"
#include "sml_ClientInputLink.h"
#include "sml_ClientOutputLink.h"
#include "sml_ClientWme.h"
#include "sml_ClientSymbol.h"
*/
//#include "IgSKI_Agent.h"
//#include "IgSKI_InputLink.h"
//#include "IgSKI_OutputLink.h"
//#include "IgSKI_Wme.h"
//#include "IgSKI_Symbol.h"
//#include "gSKI_Structures.h"
#include <cassert>

#include <iostream>//devvan needed for testing, remove afterwards
#include <string>

using std::string;
using std::cout;
using std::endl;

using namespace sml;

/*************************************/
/*   Soar Agent Function Definitions */
/*************************************/
SoarAgent::SoarAgent(Marker inMarker, Board* board, IAgent* inAgent) : agent(inAgent), myMarker(inMarker), p_GameBoard(board)
{
	/* ==================== SGIO IMPLEMENTATION ===========================
	//mem = new sgio::WorkingMemory(agent);

		loops and attaches squares, same as gSKI version

	 sgio::StringElement* myMarker = mem->CreateStringWME(mem->GetILink(),"my-marker",MarkerToString(m));
	 mem->Commit();
	/* ==================== SGIO IMPLEMENTATION ===========================*/	


	/*p_InputMem = agent->GetWorkingMemory(); 
	This only LOOKS like the gSKI call needed to get a pointer to working memory , but this would get a pointer
	to main working memory, not the InputWorkingMemory subset that should be used*/

	p_InputMem = agent->GetInputLink()->GetInputLinkMemory();

	//create a 3x3 grid, initialize squares, and creates a wme describing each one
	for(int i = 0; i < 3; ++i)
		for(int j = 0; j < 3; ++j)
		{
			squares[i][j].Initialize(board);
			squares[i][j].Attach(p_InputMem, p_GameBoard->GetAt(i,j),i,j);
		}

	IInputLink* iLink = agent->GetInputLink();
	IWMObject* iLinkRootObject;
	iLink->GetRootObject(&iLinkRootObject);

	//Create a working memory element for this agent's marker type, whichever enum value that is
	IWme* marker = p_InputMem->AddWmeString(iLinkRootObject, "my-marker", MarkerToString(myMarker));

	//NOTE: After modifying working memory, gSKI does not need to "commit" changes like SGIO

	//gSKI should call ProcessOutput on the game board whenever Soar creates output.  Alleviates need for polling
	agent->GetOutputLink()->SetAutomaticUpdate(true);
}


SoarAgent::~SoarAgent()
{
	/* SGIO way
	delete mem;   
	delete soar;*/

	// iterate through the grid, call detach on every square(which releases all of the associated wmes)
	for(int i = 0; i < 3; ++i)
		for(int j = 0; j < 3; ++j)
			squares[i][j].Detach(p_InputMem);
}

void SoarAgent::MakeMove()
{
	/* ==================== SGIO IMPLEMENTATION ===========================*/
	//The SGIO version required that the agent be updated before it was run
	//The gSKI version requires that input producers are registered as such, 
	//and automatically has them update the input link for the agent.  The
	//explicit update is not needed
	/*for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			squares[i][j].Update(mem, iLinkRootObject);
		}
	}*/

	//gSKI doesn't need/have an equivalent function to Commit
	//mem->Commit();
	/* ==================== SGIO IMPLEMENTATION ===========================*/


	//The second param in this gSKI equivalent function is ignored for the chosen run type;
	egSKIRunResult runResult = agent->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1);
	assert(runResult != gSKI_RUN_ERROR);
		
	/* ==================== SGIO IMPLEMENTATION ===========================*/
	//soar->RunTilOutput();//SGIO version

	//sgio version of outputlink command reading - the gSKI version is done in ProcessOutput()
	/* if(agent->Commands())
	{
		std::auto_ptr<sgio::Command> cmd = agent->GetCommand();//point to the first of a set of commands
		fixme, do gSKI equivalent

		//Read the row and column output from Soar
		try
		{
			int row, col;
			std::istringstream rowStream(cmd->GetParameterValue("row"));
			std::istringstream colStream(cmd->GetParameterValue("col"));
			rowStream >> row;
			colStream >> col;
			if(rowStream && colStream)
			{
				board.PlaceMarker(myMarker,row,col);
			}
			else
			{
				throw std::exception("invalid command passed from soar agent");
			}
		}
		catch(...)
		{
			cmd->AddStatusError();
			throw;
		}

		cmd->AddStatusComplete();
	}
	else
		throw std::exception("Error in Soar Agent");*/
	/* ==================== SGIO IMPLEMENTATION ===========================*/
}//MakeMove function



SoarSquare* SoarAgent::GetSquareAt(int row, int column)
{
	return &(squares[row][column]);
}

void SoarAgent::DebugPrintSoarSquares(void)
{
	cout << "SOAR SQUARES, from WMEs" << endl;

	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			squares[i][j].PrintMarker();
			if(j != 2)
				cout << '|';
		}
		cout << endl;
		if(i != 2)
			cout << "-----" << endl;
	}
	cout << endl;
}

void SoarAgent::ProcessOutput(IWorkingMemory* wMemory, IWMObject* moveIdentifier)
{
	int row(0);
	int col(0);
	IWme* wmeValue;

	tIWmeIterator* rowWMEIterator = moveIdentifier->GetWMEs("row");
	tIWmeIterator* colWMEIterator = moveIdentifier->GetWMEs("col");

	// Get the agent's output decision
	if (rowWMEIterator->IsValid() && colWMEIterator->IsValid())
	{
		// Get the row value
		wmeValue = rowWMEIterator->GetVal();
		row = wmeValue->GetValue()->GetInt();

		// Get the col value
		wmeValue = colWMEIterator->GetVal();
		col = wmeValue->GetValue()->GetInt();

		// Write the agent's decision to the board
		p_GameBoard->PlaceMarker(myMarker, row, col);
		wMemory->AddWmeString(moveIdentifier, "status", "complete");
	}
	else
	{
		wMemory->AddWmeString(moveIdentifier, "status", "error");
	}
}