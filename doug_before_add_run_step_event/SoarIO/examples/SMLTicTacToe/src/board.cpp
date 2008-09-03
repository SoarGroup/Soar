#include "board.h"
#include <stdexcept>

Board::Board() {
	for(int i = 0; i < 3; ++i) 
		for(int j = 0; j < 3; ++j)
			d_board[i][j] = MARKER_NONE;
}

void
Board::Clear() {
	for(int i = 0; i < 3; ++i) 
		for(int j = 0; j < 3; ++j)
			d_board[i][j] = MARKER_NONE;
}

void
Board::PlaceMarker(Marker m,int row,int col) {
	if(row >= 0 && row <= 2 && col >= 0 && col <= 2) {
		d_board[row][col] = m; 
	}
	else {
		throw std::out_of_range("row or col are out of range");
	}
}

Marker
Board::GetAt(int row,int col) const {
	if(row >= 0 && row <= 2 && col >= 0 && col <=2) {
		return d_board[row][col];
	}
	else {
		throw std::out_of_range("row or col are out of range");
	}
}