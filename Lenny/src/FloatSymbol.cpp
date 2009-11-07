
#include <string>

#include "FloatSymbol.h"
#include "misc.h"

using namespace std;

FloatSymbol::FloatSymbol( long newUID, double newValue )
{
	// get string representation
	string temp;
	toString( newValue, temp );

	// initialize underlying symbol
	InitSymbol( newUID, FloatSym, temp.c_str(), true );

	// set my data structures
	value = newValue;
}

double FloatSymbol::GetValue()
{
	return value;
}
