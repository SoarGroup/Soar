#ifndef INCLUDED_BOARD_H
#define INCLUDED_BOARD_H

enum Marker { MARKER_NONE, MARKER_X, MARKER_O };

class Board {
public:
	Board();

	void Clear();
	void PlaceMarker(Marker m,int row,int col);
	Marker GetAt(int row,int col) const;
private:
	Marker d_board[3][3];
};



#endif // INCLUDED_BOARD_H