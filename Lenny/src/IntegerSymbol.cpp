
#include <string>

#include "IntegerSymbol.h"
#include "misc.h"

using namespace std;

IntegerSymbol::IntegerSymbol( long newUID, long newValue )
{
	// get string representation
	string temp;
	toString( newValue, temp );

	// initialize underlying symbol
	InitSymbol( newUID, temp.c_str() );

	// set my data structures
	value = newValue;
}

long IntegerSymbol::GetValue()
{
	return value;
}

Symbol::SymbolType IntegerSymbol::GetType()
{
	return IntSym;
}

bool IntegerSymbol::IsConst()
{
	return true;
}
