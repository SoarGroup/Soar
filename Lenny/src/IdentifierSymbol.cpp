
#include <string>

#include "IdentifierSymbol.h"
#include "misc.h"

using namespace EpmemNS;
using std::string;

IdentifierSymbol::IdentifierSymbol( SymbolUID newUID, char newLetter, long newNumber )
{
	// get string representation
	string temp;
	toString( newNumber, temp );

	// initialize underlying symbol
	InitSymbol( newUID, (newLetter + temp).c_str() );

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

Symbol::SymbolType IdentifierSymbol::GetType()
{
	return IdSym;
}

bool IdentifierSymbol::IsConst()
{
	return false;
}
