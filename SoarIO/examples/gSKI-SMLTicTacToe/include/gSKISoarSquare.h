#ifndef INCLUDED_GSKISOAR_SQUARE_H
#define INCLUDED_GSKISOAR_SQUARE_H

#include "sml_Client.h"
//#include "sml_ClientWorkingMemory.h"
//#include "sml_ClientWMObject.h"
//#include "sml_ClientWME.h"
//#include "IgSKI_InputProducer.h"
//#include "IgSKI_WorkingMemory.h"
//#include "IgSKI_WMObject.h"

extern enum Marker;

using namespace sml ;

class Board;

/**
* This is the child class of the abstract virtual class IInputProducer
* 
*/
class SoarSquare : public IInputProducer 
{
public:

	/**
	* Initializes the class by initializing all the IWME* 
	* structures to NULL
	*/
	SoarSquare() : squareWMEStructure(0),row(0),col(0),symbol(0),board(0){}

	/**
	* Attach adds this square to InputWorkingMemory
	* The square and it's 3 child wmes will all need to be released in Detach()
	* The IWorkingMemory pointer passed in here is actually an InputWorkingMemory pointer, not a main
	* working memory pointer
	* @param wmemory The working memory interface associated with the agent's input link
	* @param m The Marker type associated with this WME 
	* @param inRow The row position of this WME
	* @param inCol The column position of this WME
	*/
	void Attach(IWorkingMemory* wmemory, Marker m, int inRow, int inCol);

	/**
	* Inherited from the IInputroducer class, Update is called by the agent-running functions
	* (as an automatic callback).  This function ensures that Soar has the most updated view of
	* the working memory elements associated with this square.  The row and column never change,
	* so only the current "symbol" wme is replaced, regardless of whether it's changed.
	* @param wmemory Pointer to working memory associated with the agent's input link
	* @param object The input wme associated with this InputProducer
	*/
	void Update(IWorkingMemory* wmemory, IWMObject* object);
	
	/**
	* Detach removes this square from InputWorkingMemory
	* @param mem Pointer to InputWorkingMemory that this square exists in
	*/
	void Detach(IWorkingMemory* mem);

	/**
	* Debug function prints contents of this square
	*/
	void PrintMarker();

	/**
	* Returns a pointer to this squares identifier
	* @return Pointer to this squares identifier
	*/
	IWMObject* GetSquareIdentifier();

	/**
	* Initialize points the private board* to the game board that's in use
	* @param The game board in the sim
	*/
	void Initialize(Board*);

private:
	
	IWme* squareWMEStructure;//the parent wme for the row, symbol, col
	//sgio::SoarId* soarId;  id as in "identifier", the parent used in the SGIO version

	IWme* row;
	IWme* col;
	IWme* symbol;
	Board* board;
};

#endif // INCLUDED_GSKISOAR_AGENT_H