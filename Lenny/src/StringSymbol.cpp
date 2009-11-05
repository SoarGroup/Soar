
#include <string>

#include "StringSymbol.h"

using namespace std;

StringSymbol::StringSymbol( long newUID, const char* newValue )
{
	// copy string representation
	valueS.assign( newValue );
	value = valueS.c_str();

	// initialize underlying symbol
	InitSymbol( newUID, valueS, true );
}

const char* StringSymbol::GetValue()
{
	return value;
}
