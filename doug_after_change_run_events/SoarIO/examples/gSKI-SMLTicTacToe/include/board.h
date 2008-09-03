#ifndef INCLUDED_BOARD_H
#define INCLUDED_BOARD_H

enum Marker { MARKER_NONE, MARKER_X, MARKER_O };

class Board
{
public:

	/**
	* The board is the game environment that Soar will interface with. The constructor calls 
	* Clear on the board to initialize it.
	*/
	Board();

	/**
	* Initializes the multi-dimensional array to have no elements (MARKER_NONE)
	*/
	void Clear();
	
	/**
	* Places a mark of type Marker on the board at row "row" and column "col" This
	* function only checks to see if the mark can be placed inside the range of the board
	* domain. It does not check to see if a marker is already placed at proposed position
	* @param m the Marker type to place at this position
	* @param row the row that the mark should be placed
	* @param col the column that the mark should be placed 
	*/
	void PlaceMarker(Marker m,int row,int col);
	
	/**
	* Returns the type of mark at passed position. Error checks to make sure that the position
	* is in the game world.
	* @param row the row that is queried
	* @param col the col that is queried
	* @return Marker the mark at the position queried
	*/
	Marker GetAt(int row,int col) const;
	
	/**
	* Iterates through the multi-dimensional board array and prints to the console
	* the contents of the game board.
	*/
	void Print(void);
	
	/**
	* Iterates through the game board looking to see if there is a winner.
	* If there is no winner this function returns false
	* @return bool false if the game has resulted in a tie, true otherwise
	*/
	bool IsCatGame(void);
	
	/**
	* Checks all possible combinations for a winner. If a winner is found
	* this function will return the marker type that has one the game.
	* Marker will return MARKER_X if the X player has one, MARKER_O if the 0 player
	* has one, and will return MARKER_NONE otherwise
	*/
	Marker CheckForWinner(void);

private:
	Marker d_board[3][3];
};

/**
* Utility function to change an enum to a character
*/
inline char MarkerToChar(Marker m);

/**
* Utility function to change an enum to a C string
*/
const char* MarkerToString(Marker m);

/**
* Utility function to change a C string to a Marker enum
*/
Marker StringToMarker(const char* str);

#endif // INCLUDED_BOARD_H