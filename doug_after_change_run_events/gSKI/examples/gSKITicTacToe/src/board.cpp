#include "board.h"
#include <stdexcept>
#include <iostream>

using namespace std;

Board::Board() {
	Clear();
}

void Board::Clear() {
	for(int i = 0; i < 3; ++i) 
		for(int j = 0; j < 3; ++j)
			d_board[i][j] = MARKER_NONE;
}

void Board::PlaceMarker(Marker m,int row,int col)
{
	if(row >= 0 && row <= 2 && col >= 0 && col <= 2)
	{
		d_board[row][col] = m; 
	}
	else
	{
		cout << "Row and col are: " << row << " " << col << endl;
		throw std::out_of_range("row or col are out of range");
	}
}

Marker Board::GetAt(int row,int col) const
{
	if(row >= 0 && row <= 2 && col >= 0 && col <=2)
	{
		return d_board[row][col];
	}
	else
	{
		throw std::out_of_range("row or col are out of range");
	}
}

void Board::Print(void)
{
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			cout << MarkerToChar(d_board[i][j]);
			if(j != 2)
				cout << '|';
		}
		cout << endl;
		if(i != 2)
			cout << "-----" << endl;
	}
	cout << endl;
}

Marker Board::CheckForWinner(void)
{
	// Check Horizontals
	if(d_board[0][0] == d_board[0][1] && d_board[0][0] == d_board[0][2] && d_board[0][0] != MARKER_NONE)
		return d_board[0][0];
	else if(d_board[1][0] == d_board[1][1] && d_board[1][0] == d_board[1][2] && d_board[1][0] != MARKER_NONE)
		return d_board[1][0];
	else if(d_board[2][0] == d_board[2][1] && d_board[2][0] == d_board[2][2] && d_board[2][0] != MARKER_NONE)
		return d_board[2][0];
	// Check Verticals
	else if(d_board[0][0] == d_board[1][0] && d_board[0][0] == d_board[2][0] && d_board[0][0] != MARKER_NONE)
		return d_board[0][0];
	else if(d_board[0][1] == d_board[1][1] && d_board[0][1] == d_board[2][1] && d_board[0][1] != MARKER_NONE)
		return d_board[0][1];
	else if(d_board[0][2] == d_board[1][2] && d_board[0][2] == d_board[2][2] && d_board[0][2] != MARKER_NONE)
		return d_board[0][2];
	// Check Diagonals
	else if(d_board[0][0] == d_board[1][1] && d_board[0][0] == d_board[2][2] && d_board[0][0] != MARKER_NONE)
		return d_board[0][0];
	else if(d_board[0][2] == d_board[1][1] && d_board[0][2] == d_board[2][0] && d_board[0][2] != MARKER_NONE)
		return d_board[0][2];
	else
		return MARKER_NONE;
}

bool Board::IsCatGame(void)
{
	for(int i = 0; i < 3; ++i)
		for(int j = 0; j < 3; ++j)
			if(d_board[i][j] == MARKER_NONE)
					return false; 
	return CheckForWinner() == MARKER_NONE;
}


inline char MarkerToChar(Marker m)
{
	switch(m)
	{
		case MARKER_NONE:
			return ' ';
		case MARKER_X:
			return 'X';
		case MARKER_O:
			return 'O';
		default:
			throw std::invalid_argument("m is not a valid marker enumeration");
	}
}

const char* MarkerToString(Marker m)
{
	switch(m)
	{
		case MARKER_NONE:
			return "EMPTY";
		case MARKER_X:
			return "X";
		case MARKER_O:
			return "O";
		default:
			throw std::invalid_argument("invalid value for enumeration");
	}
}

Marker StringToMarker(const char* str)
{
	if(!strcmp("EMPTY", str))
		return MARKER_NONE;
	else if(!strcmp("X", str))
		return MARKER_X;
	else if(!strcmp("O", str))
		return MARKER_O;
	else
		throw std::invalid_argument("invalid value for string");	
}






