
#ifndef INCLUDED_GSKISOAR_AGENT_H
#define INCLUDED_GSKISOAR_AGENT_H

#include "gSKISoarSquare.h"

#include "IgSKI_InputProducer.h"
#include "IgSKI_Agent.h"
#include "IgSKI_WorkingMemory.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_OutputProcessor.h"

#include <vector>

using namespace gSKI;
using namespace std;

class Board;
extern enum Marker;


//From the perspective of Soar, the SoarAgent class gives the output to the game environment
class SoarAgent: public IOutputProcessor
{
public:
	/**
	* SoarAgent constructor 
	* @param m The Marker enum used to represent this agent's move in the game grid
	* @param board Pointer to the board that the game is played on
	* @param inAgent Pointer to the actual Soar agent that this class is a proxy for
	*/
	SoarAgent(Marker m, Board* board, IAgent* inAgent);

	/**
	* SoarAgent destructor
	* cleans up after the agent, releasing references
	*/
	~SoarAgent();

	/**
	* MakeMove runs the agent until it decides on a move.  The run function causes in update callback in the 
	* InputProducer objects, causing Soar to have the most updated view of the world
	* When the agent produces output, the ProcessOutput function in the OutputProcessors will be called automatically
	*/
	void MakeMove();

	/**
	* Inherited from IOutputProcessor, ProcessOutput is a callback function that
	* is called after the agent has run and has placed WMEs on the output link.
	* Reads the command and its parameters, and them places the agent's marker in the 
	* corresponding square
	* @param wMemory pointer to OutputWorkingMemory
	* @param moveIdentifier pointer to the WME on the output link that has "move" as its identifier
	*/	
	void ProcessOutput(gSKI::IWorkingMemory*, gSKI::IWMObject*);

	/**
	* GetSquareAt breaks encapsulation on the squares 2D array
	* @param row
	* @param column
	* @return SoarSquare pointer corresponding to the queried square
	*/
	SoarSquare* GetSquareAt(int row, int column);

	/**
	* Prints the game board from the perspective of the SoarAgent, not the actual game board
	* This is for debugging only.  NOTE: Soar's view of the game board lags one move behind
	* the actual game board.  This is because it only gets updated right before it makes a move,
	* so it won't even see it's own move placed on the board until right before it makes its next
	* move
	*/
	void DebugPrintSoarSquares(void);

private:
	SoarAgent();
	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);

	IAgent* agent;
	IWorkingMemory* p_InputMem;
	SoarSquare squares[3][3];
	Marker myMarker;
	Board* p_GameBoard;

	//sgio::Soar* soar; pointer back to the soar instance corresponding to this SoarAgent
	//gSKI doesn't need this member here
};

#endif // INCLUDED_GSKISOAR_AGENT_H