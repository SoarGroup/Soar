
#include <string>

#include "FloatSymbol.h"
#include "misc.h"

using namespace EpmemNS;
using std::string;

FloatSymbol::FloatSymbol( SymbolUID newUID, double newValue )
{
	// get string representation
	string temp;
	toString( newValue, temp );

	// initialize underlying symbol
	InitSymbol( newUID, temp.c_str() );

	// set my data structures
	value = newValue;
}

double FloatSymbol::GetValue()
{
	return value;
}

Symbol::SymbolType FloatSymbol::GetType()
{
	return FloatSym;
}

bool FloatSymbol::IsConst()
{
	return true;
}
