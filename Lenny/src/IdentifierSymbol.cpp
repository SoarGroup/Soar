
#include <string>

#include "IdentifierSymbol.h"
#include "misc.h"

using namespace std;

IdentifierSymbol::IdentifierSymbol( long newUID, char newLetter, long newNumber )
{
	// get string representation
	string temp, temp2;
	toString( newLetter, temp );
	toString( newNumber, temp2 );
	temp.append( temp2 );

	// initialize underlying symbol
	InitSymbol( newUID, temp, false );

	// set my data structures
	letter = newLetter;
	number = newNumber;
}

long IdentifierSymbol::GetNumber()
{
	return number;
}

char IdentifierSymbol::GetLetter()
{
	return letter;
}
